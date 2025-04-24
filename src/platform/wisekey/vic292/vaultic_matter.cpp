/**
*
* @note    THIS PRODUCT IS SUPPLIED FOR EVALUATION, TESTING AND/OR DEMONSTRATION PURPOSES ONLY.
*
* @note    <b>DISCLAIMER</b>
*
* @note    Copyright (C) 2025 Seal SQ
*
* @note    All products are provided by Seal SQ subject to Seal SQ Evaluation License Terms and Conditions
* @note    and the provisions of any agreements made between Seal SQ and the Customer concerning the same
* @note    subject matter.
* @note    In ordering a product covered by this document the Customer agrees to be bound by those Seal SQ's
* @note    Evaluation License Terms and Conditions and agreements and nothing contained in this document
* @note    constitutes or forms part of a contract (with the exception of the contents of this disclaimer notice).
* @note    A copy of Seal SQ's Evaluation License Terms and Conditions is available on request. Export of any
* @note    Seal SQ product outside of the EU may require an export license.
*
* @note    The information in this document is provided in connection with Seal SQ products. No license,
* @note    express or implied, by estoppel or otherwise, to any intellectual property right is granted by this
* @note    document or in connection with the provisions of Seal SQ products.
*
* @note    Seal SQ makes no representations or warranties with respect to the accuracy or completeness of the
* @note    contents of this document and reserves the right to make changes to specifications and product
* @note    descriptions at any time without notice.
*
* @note    THE PRODUCT AND THE RELATED DOCUMENTATION ARE PROVIDED "AS IS", AND CUSTOMER UNDERSTANDS
* @note    THAT IT ASSUMES ALL RISKS IN RELATION TO ITS USE OF THE PRODUCT AND THE PRODUCT'S
* @note    QUALITY AND PERFORMANCE.
*
* @note    EXCEPT AS SET FORTH IN INSIDE SECURE'S EVALUATION LICENSE TERMS AND CONDITIONS OR IN ANY
* @note    AGREEMENTS MADE BETWEEN Seal SQ AND THE CUSTOMER CONCERNING THE SAME SUBJECT MATTER,
* @note    Seal SQ OR ITS SUPPLIERS OR LICENSORS ASSUME NO LIABILITY WHATSOEVER. CUSTOMER
* @note    AGREES AND ACKNOWLEDGES THAT Seal SQ SHALL HAVE NO RESPONSIBILITIES TO CUSTOMER TO
* @note    CORRECT ANY DEFECTS OR PROBLEMS IN THE PRODUCT OR THE RELATED DOCUMENTATION, OR TO
* @note    ENSURE THAT THE PRODUCT OPERATES PROPERLY.  Seal SQ DISCLAIMS ANY AND ALL WARRANTIES
* @note    WITH RESPECT TO THE PRODUCT AND THE RELATED DOCUMENTATION, WHETHER EXPRESS, STATUTORY
* @note    OR IMPLIED INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTY OF MERCHANTABILITY,
* @note    SATISFACTORY QUALITY, FITNESS FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT.
*
* @note    Seal SQ SHALL HAVE NO LIABILITY WHATSOEVER TO CUSTOMER IN CONNECTION WITH THIS
* @note    Seal SQ'S EVALUATION TERMS AND CONDITIONS, INCLUDING WITHOUT LIMITATION, LIABILITY FOR
* @note    ANY PROBLEMS IN OR CAUSED BY THE PRODUCT OR THE RELATED DOCUMENTATION, WHETHER DIRECT,
* @note    INDIRECT, CONSEQUENTIAL, PUNITIVE, EXEMPLARY, SPECIAL OR INCIDENTAL DAMAGES (INCLUDING,
* @note    WITHOUT LIMITATION, DAMAGES FOR LOSS OF PROFITS, LOSS OF REVENUE, BUSINESS INTERRUPTION,
* @note    LOSS OF GOODWILL, OR LOSS OF INFORMATION OR DATA) NOTWITHSTANDING THE THEORY OF
* @note    LIABILITY UNDER WHICH SAID DAMAGES ARE SOUGHT, INCLUDING BUT NOT LIMITED TO CONTRACT,
* @note    TORT (INCLUDING NEGLIGENCE), PRODUCTS LIABILITY, STRICT LIABILITY, STATUTORY LIABILITY OR
* @note    OTHERWISE, EVEN IF Seal SQ HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
*
*/

#include "vaultic_matter.h"

using namespace std;

// extern VLT_STS VltCrc16( VLT_U16 *pu16Crc, const VLT_U8 *pu8Block, VLT_U16 u16Length );

#define CHECK_OK(label,a) { VIC_LOGV(label);\
                                CHIP_ERROR ret_status = a;\
                                if (ret_status!= CHIP_NO_ERROR) {VIC_LOGE("%s error %d",label,(int)ret_status.AsInteger());return ret_status;}\
                            }

static int vlt_matter_init_done = FALSE;

/** @cond SHOW_INTERNAL */
typedef struct
{
    int tag; // record tag
    int len; // record len
    int data_offset; // offset of data in binary area
} ST_REC;
/** @endcond  */

static int nb_recs;                // number of records in binary area
static ST_REC records[MAX_NB_RECS];
static VLT_PAIRING_MODE pairingMode;
static int StaticPrivKeyIndex=STATIC_KEY_INDEX; // default value for index of static device private key

/**
 * \brief Returns the record id of known certificates as defined in vaultic_tls_config.h
 *
 * \return -1 in case of error
 */

static int GetCertificateID(ssl_vic_cert_type cert_type)
{
    switch (cert_type)
    {
        case SSL_VIC_PBKDF:
            return REC_ID_PBKDF;
        case SSL_VIC_VERIFIER:
            return REC_ID_VERIFIER;
        case SSL_VIC_PAI_CERT:
            return REC_ID_PAI_CERT;
        case SSL_VIC_DAC_CERT:
            return REC_ID_DAC_CERT;
        default:
            return -1;
    }
    return -1;
}

static int check_rec_id(int rec_id)
{
    if( (rec_id <=0) || (rec_id > nb_recs) )
    {
        return -1;
    }

    return VLT_OK;
}

/**
 * \brief Check vlt_matter_init_done
 *
 * \return TRUE(1u) if session is open with VaultIC or FALSE(0u)
 */
int vlt_matter_is_init()
{
    return vlt_matter_init_done;
}

/**
 * \brief Analyzes the binary area, check its format and retrieve the header of the records it contains
 *
 * \return -1 in case of error
 */
static CHIP_ERROR analyze_bin_area()
{
    unsigned char binarea_header[BINAREA_HDR_SZ];
    unsigned char rec_header[TLV_HEADER_SZ];

    // read header of binary area
    VIC_LOGV("VltReadBin binary area header");
    if(VltReadBin( binarea_header, 0, BINAREA_HDR_SZ) != VLT_OK) {
        return CHIP_ERROR_INTERNAL;
    }

    // check format
    if(binarea_header[OFFSET_FORMAT] != BINAREA_FORMAT)
    {
        VIC_LOGE( "vlt_analyze_bin_area error: Unsupported binary area format" );
        return CHIP_ERROR_INTERNAL;
    }

    // check version
    if(binarea_header[OFFSET_VERSION] != BINAREA_VERSION)
    {
        VIC_LOGE( "vlt_analyze_bin_area error: Unsupported binary area version" );
        return CHIP_ERROR_INTERNAL;
    }

    // total number of records stored in binary area
    nb_recs= INT16(binarea_header[OFFSET_VAL], binarea_header[OFFSET_VAL+1]);

    if(nb_recs > MAX_NB_RECS)
    {
        VIC_LOGE( "vlt_analyze_bin_area error: Unexpected value for nb_recs" );
        return CHIP_ERROR_INTERNAL;
    }

    // Point of first record
    int offset = BINAREA_HDR_SZ;
    putchar('\n');
    // Get details of each records in binary area
    for (int i=0; i < nb_recs; i++)
    {
        VIC_LOGV("VltReadBin binary area record header");
        if(VltReadBin( rec_header, offset, sizeof(rec_header)) != VLT_OK) {
            return CHIP_ERROR_INTERNAL;
        }

        records[i].tag = rec_header[OFFSET_TAG];
        records[i].len = INT16(rec_header[OFFSET_LEN], rec_header[OFFSET_LEN+1]);
        offset += TLV_HEADER_SZ;
        records[i].data_offset = offset;
        offset += records[i].len + CRC_SZ;
    }

    return CHIP_NO_ERROR;
}

/**
 * \brief Get the size of a certificate stored in VaultIC
 *
 * \param[in]  cert_type  type of certificate
 *
 * \return length of the certificate
 * \return -1 in case of error
 */

int vlt_matter_get_cert_size(ssl_vic_cert_type cert_type)
{
    int cert_length;
    int rec_id;
    int rec_tag;

    // check matter init done
    if(vlt_matter_init_done == FALSE) {
        VIC_LOGE("vlt_matter_get_cert_size error: VaultIC matter not initialized" );
        return - 1;
    }

    // check rec_id range
    rec_id = GetCertificateID(cert_type);
    if(check_rec_id(rec_id) != VLT_OK) {
        VIC_LOGE("vlt_matter_get_cert_size error: Invalid record id" );
        return - 1;
    }

    if((rec_tag = vlt_matter_get_cert_tag(cert_type)) == - 1) {
        VIC_LOGE("vlt_matter_get_cert_tag error: Invalid cert tag" );
        return - 1;
    }

    rec_id--;

    // check rec_id points on a certificate
    //printf("\nCert type: %d, Record tag: %d, rec_tag: %d\n", cert_type, records[rec_id].tag, rec_tag);
    if(records[rec_id].tag != rec_tag) {
        VIC_LOGE("vlt_matter_get_cert_size error: Invalid record tag" );
        return - 1;
    }

    // return length of the certificate
    cert_length = records[rec_id].len;
    if (cert_length > MAX_CERT_SZ) {
        VIC_LOGE("vlt_matter_get_cert_size error: Suspicious certificate size" );
        return - 1;
    }

    return cert_length;
}

/**
 * \brief Get the tag of a certificate stored in VaultIC
 *
 * \param[in]  cert_type  type of certificate
 *
 * \return tag of the certificate
 * \return -1 in case of error
 */
int vlt_matter_get_cert_tag(ssl_vic_cert_type cert_type)
{
    switch (cert_type)
    {
        case SSL_VIC_PAI_CERT:
            return TAG_PAI_CERTIFICATE;
        case SSL_VIC_DAC_CERT:
            return TAG_DAC_CERTIFICATE;
        case SSL_VIC_VERIFIER:
            return TAG_VERIFIER;
        case SSL_VIC_PBKDF:
            return TAG_PBKDF;
        default:
            return -1;
    }
    return -1;
}

/**
 * \brief Read a certificate stored in VaultIC
 *
 * \param[out]	cert_buf    buffer to store the certificate
 * \param[in]  cert_type    type of certificate
 *
 * \return 0 in case of success
 * \return -1 in case of error
 */
CHIP_ERROR vlt_matter_read_data(CERT_DATA cert_buf, ssl_vic_cert_type cert_type)
{
    int offset;
    int size_cert;
    int cert_crc;
    int rec_index;
    unsigned char crc_buff[CRC_SZ];

	VIC_LOGI("vlt_matter_read_data");
    // read certificate size
    if((size_cert=vlt_matter_get_cert_size(cert_type))==-1)
    {
        VIC_LOGE( "vlt_matter_read_data error: invalid index" );
        return CHIP_ERROR_INTERNAL;
    }

    // read certificate
    rec_index = GetCertificateID(cert_type) - 1;
    offset = records[rec_index].data_offset;
    VIC_LOGV("VltReadBinArea record data");
    if(VltReadBinArea( cert_buf, offset, size_cert) != VLT_OK) {
        return CHIP_ERROR_INTERNAL;
    }

    offset += size_cert; // jump to crc

    // read crc
    VIC_LOGV("VltReadBin record crc");
    if(VltReadBin(crc_buff, offset, sizeof(crc_buff)) != VLT_OK) {
        return CHIP_ERROR_INTERNAL;
    }
    cert_crc= INT16(crc_buff[0], crc_buff[1]);

    // compute crc on data received
    VLT_U16 computed_crc = CRC_INIT;
    VIC_LOGV("VltCrc16");
    if(VltCrc16(&computed_crc, cert_buf, size_cert) != VLT_OK) {     // CRC on DATA bytes received
        return CHIP_ERROR_INTERNAL;
    }

    if(cert_crc != computed_crc)
    {
        VIC_LOGE( "vlt_matter_read_data error: wrong crc" );
        return CHIP_ERROR_INTERNAL;

    }

	return CHIP_NO_ERROR;
}

/**
 * \brief Read DAC certificate stored in VaultIC
 *
 * \param[out]	dac_buf    buffer to store the certificate
 *
 * \return 0 in case of success
 * \return -1 in case of error
 */
CHIP_ERROR vlt_matter_read_dac(CERT_DATA dac_buf)
{
    return vlt_matter_read_data(dac_buf, SSL_VIC_DAC_CERT);
}

/**
 * \brief Read PAI certificate stored in VaultIC
 *
 * \param[out]	pai_buf    buffer to store the certificate
 *
 * \return 0 in case of success
 * \return -1 in case of error
 */
CHIP_ERROR vlt_matter_read_pai(CERT_DATA pai_buf)
{
    return vlt_matter_read_data(pai_buf, SSL_VIC_PAI_CERT);
}

/**
 * \brief Read Verifier stored in VaultIC
 *
 * \param[out]	verifier_buf    buffer to store the certificate
 *
 * \return 0 in case of success
 * \return -1 in case of error
 */
CHIP_ERROR vlt_matter_read_verifier(CERT_DATA verifier_buf)
{
    return vlt_matter_read_data(verifier_buf, SSL_VIC_VERIFIER);
}

/**
 * \brief Read PBKDF stored in VaultIC
 *
 * \param[out]	pbkdf_buf    buffer to store the certificate
 *
 * \return 0 in case of success
 * \return -1 in case of error
 */
CHIP_ERROR vlt_matter_read_pbkdf(CERT_DATA pbkdf_buf)
{
    return vlt_matter_read_data(pbkdf_buf, SSL_VIC_PBKDF);
}

/**
 * \brief Get iteration stored in VaultIC
 *
 * \param[out]	iteration_buff    buffer to store the interation
 *
 * \return 0 in case of success
 * \return -1 in case of error
 */
CHIP_ERROR vlt_matter_get_iteration(VLT_U32 *iteration)
{
    CERT_DATA pbkdf_buff;
    CERT_DATA iteration_buff;
    int sizeof_pbkdf;

    sizeof_pbkdf = vlt_matter_get_cert_size(SSL_VIC_PBKDF);
    if (sizeof_pbkdf <= 0) {
        return CHIP_ERROR_INTERNAL;
    }
    pbkdf_buff = (CERT_DATA)malloc(sizeof_pbkdf);

    if(vlt_matter_read_pbkdf(pbkdf_buff) != CHIP_NO_ERROR) {
        return CHIP_ERROR_INTERNAL;
    }

    iteration_buff = (CERT_DATA)malloc(VIC292_ITERATION_SIZE);

    int i, j;

    for(i = 0; i < VIC292_ITERATION_SIZE; i++) {
        iteration_buff[i] = pbkdf_buff[i];
    }

    i = VIC292_ITERATION_SIZE - 1;
    j = 0;
    *iteration=0;
    
    while(i != 0) {
        *iteration |= iteration_buff[i] << (j*8);
        i--, j++;
    }

    return CHIP_NO_ERROR;
}

/**
 * \brief Get salt stored in VaultIC
 *
 * \param[out]	salt_buff    buffer to store the salt
 *
 * \return 0 in case of success
 * \return -1 in case of error
 */
CHIP_ERROR vlt_matter_get_salt(CERT_DATA salt_buff)
{
    CERT_DATA pbkdf_buff;
    int sizeof_pbkdf, salt_offset;

    sizeof_pbkdf = vlt_matter_get_cert_size(SSL_VIC_PBKDF);
    if (sizeof_pbkdf <= 0) {
        return CHIP_ERROR_INTERNAL;
    }
    pbkdf_buff = (CERT_DATA)malloc(sizeof_pbkdf);
    if(vlt_matter_read_pbkdf(pbkdf_buff) != CHIP_NO_ERROR) {
        return CHIP_ERROR_INTERNAL;
    }
    salt_offset = 0;
    for(int i = VIC292_ITERATION_SIZE; i < (VIC292_ITERATION_SIZE + VIC292_SALT_SIZE); i++) {
        salt_buff[salt_offset] = pbkdf_buff[i];
        salt_offset++;
    }

    return CHIP_NO_ERROR;
}

/**
 * \brief Generates a P256 attestation signature from VaultIC
 *
 * \par Description
 *
 * This method is used to generate a P256 signature from an input hash value
 *
 * \param[in] key_id   Key ID
 * \param[in] key_group  (KEY_GROUP_KEYRING or KEY_GROUP_EPHEMERAL)
 * \param[in] hash  Message digest (32 bytes)
 * \param[out] signature signature (2*32 bytes)
 *
 * \return Upon successful completion a #CHIP_NO_ERROR status will be returned otherwise
 * the appropriate error code will be returned.
 */
CHIP_ERROR vlt_matter_generate_signature(VLT_U8 key_id, KEY_GROUP key_group, const VLT_U8 hash[VLT_LEN_DIGEST], VLT_U8 signature[VLT_LEN_SIGNATURE])
{
    if(VltGenerateSignature(key_id, key_group, hash, signature) != VLT_OK) {
        return CHIP_ERROR_INTERNAL;
    }
    return CHIP_NO_ERROR;
}

/**
 * \brief Read the ECC P256 public key stored in VaultIC
 * \note By default the private key defined by STATIC_KEY_INDEX is used, but can be changed
 *       by calling  vlt_matter_select_static_priv_key() before calling this function
 * \param[out]  pubKeyX  buffer to store the public key part Qx
 * \param[out]  pubKeyY  buffer to store the public key part Qy
 *
 * \return 0 in case of success
 * \return -1 in case of error
 */
CHIP_ERROR vlt_matter_read_pub_key_P256(unsigned char pubKeyX[P256_BYTE_SZ], unsigned char pubKeyY[P256_BYTE_SZ])
{
	VIC_LOGI("vlt_matter_read_pub_key");

    // check matter init done
    if(vlt_matter_init_done == FALSE) {
        VIC_LOGE("vlt_matter_read_pub_key error: VaultIC matter not initialized" );
        return CHIP_ERROR_INTERNAL;
    }

    VIC_LOGV("Read Public Key");
    if(VltReadPubKey(StaticPrivKeyIndex, pubKeyX, pubKeyY) != VLT_OK) {
        return CHIP_ERROR_INTERNAL;
    }

    return CHIP_NO_ERROR;

}

/**
 * \brief Change index of static private key used by vlt_matter_compute_signature_P256
 *
 * \param[in]   key_id          key index in static key ring of VaultIC
 */
void vlt_matter_select_static_priv_key(int key_id)
{
    StaticPrivKeyIndex = key_id;
}

/**
 * \brief Open matter session with VaultIC

 * \return  0 success
 * \return -1 error
 */
CHIP_ERROR vlt_matter_init()
{
    VLT_TARGET_INFO chipInfo;
    VLT_INIT_COMMS_PARAMS params;

	VIC_LOGI( "vlt_matter_init");

    params.VltBlockProtocolParams.u16msSelfTestDelay = SELF_TESTS_DELAY;
    params.VltBlockProtocolParams.u32msTimeout = APDU_TIMEOUT;

    params.enCommsProtocol = VLT_TWI_COMMS;
    params.VltTwiParams.u16BitRate = I2C_BITRATE;
    params.VltTwiParams.u8Address = I2C_ADDRESS; // I2C address

	VIC_LOGV( "VltApiInit starting");
	if (VltApiInit(&params) != VLT_OK) {
    	VIC_LOGE( "VltApiInit failed");
		return CHIP_ERROR_INTERNAL;
	}

    VIC_LOGV( "VltApiInit done");

    VIC_LOGV("VltGetInfo")
	if(VltGetInfo(&chipInfo) != VLT_OK) {
        return CHIP_ERROR_INTERNAL;
    }
	pairingMode = chipInfo.enPairingMode;

	CHECK_OK("analyze_bin_area" , analyze_bin_area());

	vlt_matter_init_done = TRUE;
	return CHIP_NO_ERROR;
}

/**
 * \brief Close matter session with VaultIC

 * \return  0 success
 * \return -1 error
 */

CHIP_ERROR vlt_matter_close()
{
	VIC_LOGI("vlt_matter_close");

    VIC_LOGV("VltApiClose");
    if(VltApiClose() != VLT_OK) {
        return CHIP_ERROR_INTERNAL;
    }
	vlt_matter_init_done = FALSE;
	return CHIP_NO_ERROR;
}
