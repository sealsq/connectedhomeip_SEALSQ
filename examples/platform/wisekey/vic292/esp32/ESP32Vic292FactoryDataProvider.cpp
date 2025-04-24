/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
#include <crypto/CHIPCryptoPAL.h>
#include <lib/support/Base64.h>
#include <platform/ESP32/ESP32Config.h>
#include <platform/ESP32/ESP32FactoryDataProvider.h>
#include <platform/ESP32/ScopedNvsHandle.h>

#include "ESP32Vic292FactoryDataProvider.h"

#include "sdkconfig.h"

#include "vaultic_matter_config.h"
#include "vaultic_matter_priv.h"
#include "vaultic_matter.h"
#include "vaultic_common.h"
#include "vaultic_api.h"

namespace chip {
namespace DeviceLayer {

using namespace chip::Credentials;
using namespace chip::DeviceLayer::Internal;

CHIP_ERROR ESP32Vic292FactoryDataProvider::GetSetupDiscriminator(uint16_t & setupDiscriminator)
{
    // Use config discrimator provide by user in menuconfig
    setupDiscriminator = CONFIG_DISCRIMINATOR_VIC292;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ESP32Vic292FactoryDataProvider::GetSpake2pIterationCount(uint32_t & iterationCount)
{
    if(vlt_matter_is_init() == FALSE)
    {
        ReturnErrorOnFailure(vlt_matter_init());
    }

    ESP_LOGI(TAG, "Get Iteration from vic292");

    VLT_U32 it;

    ReturnErrorOnFailure(vlt_matter_get_iteration((&it)));
    iterationCount = (uint32_t)it;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ESP32Vic292FactoryDataProvider::GetSpake2pSalt(MutableByteSpan & saltBuf)
{
    if(vlt_matter_is_init() == FALSE)
    {
        ReturnErrorOnFailure(vlt_matter_init());
    }

    ESP_LOGI(TAG, "Get Salt from vic292");

    ReturnErrorOnFailure(vlt_matter_get_salt(saltBuf.data()));
    saltBuf.reduce_size(VIC292_SALT_SIZE);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ESP32Vic292FactoryDataProvider::GetSpake2pVerifier(MutableByteSpan & verifierBuf, size_t & verifierLen)
{
    if(vlt_matter_is_init() == FALSE)
    {
        ReturnErrorOnFailure(vlt_matter_init());
    }

    ESP_LOGI(TAG, "Get Verifier from vic292");

    ReturnErrorOnFailure(vlt_matter_read_verifier(verifierBuf.data()));
    verifierBuf.reduce_size(VIC292_VERIFIER_SIZE);
    verifierLen = VIC292_VERIFIER_SIZE;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ESP32Vic292FactoryDataProvider::GetSetupPasscode(uint32_t & setupPasscode)
{
    // Use config passcode provide by user in menuconfig
    setupPasscode = CONFIG_PASSCODE_VIC292;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ESP32Vic292FactoryDataProvider::GetCertificationDeclaration(MutableByteSpan & outBuffer)
{
    const uint8_t kCdForAllExamples[] = CHIP_DEVICE_CONFIG_CERTIFICATION_DECLARATION;
    ESP_LOGI(TAG, "Get Certificate Declaration");
    return CopySpanToMutableSpan(ByteSpan{ kCdForAllExamples }, outBuffer);
}

CHIP_ERROR ESP32Vic292FactoryDataProvider::GetFirmwareInformation(MutableByteSpan & out_firmware_info_buffer)
{
    // We do not provide any FirmwareInformation.
    out_firmware_info_buffer.reduce_size(0);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ESP32Vic292FactoryDataProvider::GetDeviceAttestationCert(MutableByteSpan & outBuffer)
{
    if(vlt_matter_is_init() == FALSE)
    {
        ReturnErrorOnFailure(vlt_matter_init());
    }

    ESP_LOGI(TAG, "Get DAC certificate from vic292");

    size_t sizeof_dac_cert = vlt_matter_get_cert_size(SSL_VIC_DAC_CERT);

    ReturnErrorOnFailure(vlt_matter_read_dac(outBuffer.data()));
    outBuffer.reduce_size(sizeof_dac_cert);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ESP32Vic292FactoryDataProvider::GetProductAttestationIntermediateCert(MutableByteSpan & outBuffer)
{
    if(vlt_matter_is_init() == FALSE)
    {
        ReturnErrorOnFailure(vlt_matter_init());
    }

    ESP_LOGI(TAG, "Get PAI certificate from vic292");

    size_t sizeof_pai_cert = vlt_matter_get_cert_size(SSL_VIC_PAI_CERT);

    ReturnErrorOnFailure(vlt_matter_read_pai(outBuffer.data()));
    outBuffer.reduce_size(sizeof_pai_cert);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ESP32Vic292FactoryDataProvider::SignWithDeviceAttestationKey(const ByteSpan & messageToSign, MutableByteSpan & outSignBuffer)
{
    VLT_U8 signature[VLT_LEN_SIGNATURE];
    VLT_U8  hash[VLT_LEN_DIGEST];

    if(vlt_matter_is_init() == FALSE)
    {
        ReturnErrorOnFailure(vlt_matter_init());
    }

    ESP_LOGI(TAG, "Get Attestation Signature from vic292");

    VerifyOrReturnError(!outSignBuffer.empty(), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(!messageToSign.empty(), CHIP_ERROR_INVALID_ARGUMENT);

    ReturnErrorOnFailure(Crypto::Hash_SHA256(messageToSign.data(), messageToSign.size(), hash));

    ReturnErrorOnFailure(vlt_matter_generate_signature(0, KEY_GROUP_KEYRING, hash, signature));

    ReturnErrorOnFailure(vlt_matter_close());

    return CopySpanToMutableSpan(ByteSpan{ signature, sizeof(signature) }, outSignBuffer);
}

} // namespace DeviceLayer
} // namespace chip
