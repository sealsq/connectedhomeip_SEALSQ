#pragma once

#ifndef VLT_MATTER_H
#define VLT_MATTER_H

#include "vaultic_common.h"
#include "vaultic_api.h"
#include "vaultic_matter_priv.h"
#include "vaultic_matter_config.h"
#include "vaultic_crc16.h"

#include <string.h>

#include <lib/support/CodeUtils.h>

using namespace std;

#ifndef VAULTIC_LOG_LEVEL
#define VAULTIC_LOG_LEVEL 0
#endif

#if(VAULTIC_LOG_LEVEL>0)
#include <stdio.h>
#endif

#if(VAULTIC_LOG_LEVEL>=1)
#define VIC_LOGE(...) printf("[VaultIC_MATTER] ERROR ");printf(__VA_ARGS__);printf("\n");
#else
#define VIC_LOGE(...) do { } while(0);
#endif

#if(VAULTIC_LOG_LEVEL>=2)
#define VIC_LOGW(...) printf("[VaultIC_MATTER] WARNING ");printf(__VA_ARGS__);printf("\n");
#else
#define VIC_LOGW(...) do { } while(0);
#endif

#if(VAULTIC_LOG_LEVEL>=3)
#define VIC_LOGI(...) printf("[VaultIC_MATTER] INFO ");printf(__VA_ARGS__);printf("\n");
#else
#define VIC_LOGI(...) do { } while(0);
#endif

#if(VAULTIC_LOG_LEVEL>=4)
#define VIC_LOGV(...) printf("[VaultIC_MATTER] DEBUG ");printf(__VA_ARGS__);printf("\n");
#else
#define VIC_LOGV(...) do { } while(0);
#endif

#if(VAULTIC_LOG_LEVEL>=4)
#define VIC_LOG_PRINT_BUFFER(buf, len) PrintHexBuffer((unsigned char*)buf, len);printf("\n");
#else
#define VIC_LOG_PRINT_BUFFER(buf, len) do { } while(0);
#endif

typedef uint8_t * CERT_DATA;

/* Definition of public constants*/
typedef enum {
	SSL_VIC_PAI_CERT,
	SSL_VIC_DAC_CERT,
	SSL_VIC_VERIFIER,
	SSL_VIC_PBKDF,
} ssl_vic_cert_type;

#define P256_BYTE_SZ       32

/* Definition of public functions */

int vlt_matter_is_init(void);
CHIP_ERROR vlt_matter_init(void);
CHIP_ERROR vlt_matter_close(void);
CHIP_ERROR vlt_matter_read_pub_key_P256(unsigned char pubKeyX[P256_BYTE_SZ], unsigned char pubKeyY[P256_BYTE_SZ]);

int vlt_matter_get_cert_size(ssl_vic_cert_type cert_type);
int vlt_matter_get_cert_tag(ssl_vic_cert_type cert_type);

CHIP_ERROR vlt_matter_read_dac(CERT_DATA dac_buf);
CHIP_ERROR vlt_matter_read_pai(CERT_DATA pai_buf);
CHIP_ERROR vlt_matter_read_verifier(CERT_DATA verifier_buf);
CHIP_ERROR vlt_matter_read_pbkdf(CERT_DATA pbkdf_buf);

CHIP_ERROR vlt_matter_get_iteration(VLT_U32 *iteration_buff);
CHIP_ERROR vlt_matter_get_salt(CERT_DATA salt_buff);
CHIP_ERROR vlt_matter_generate_signature(VLT_U8 key_id, KEY_GROUP key_group, const VLT_U8 hash[VLT_LEN_DIGEST], VLT_U8 signature[VLT_LEN_SIGNATURE]);

CHIP_ERROR vlt_matter_read_data(CERT_DATA cert_buf, ssl_vic_cert_type cert_type);

#endif /* VLT_MATTER_H */
