#pragma once

#ifndef VLT_MATTER_PRIV_H
#define VLT_MATTER_PRIV_H

#include "vaultic_matter_config.h"

/* Credentials struct */
typedef struct credentials_file_data {
    unsigned char *passcode;
    int passcode_size;
    unsigned char *verifier;
    int verifier_size;
    unsigned char *salt;
    int salt_size;
    unsigned char *iteration;
    int iteration_size;
    unsigned char *pbkdf;
    int pbkdf_size;
} credentials_file_data_t;

/* Private Constants Definition */
#define MAX_NB_RECS        4     // Max number of records expected in binary area
#define MAX_CERT_SZ        1000  // Max reasonable size for a certificate (used for integrity check)

#define TAG_PAI_CERTIFICATE    0xD0 // tag of PAI certificate
#define TAG_DAC_CERTIFICATE    0xD1 // tag of DAC certificate
#define TAG_VERIFIER           0xD2 // tag of passcode verifier
#define TAG_PBKDF              0xD3 // tag of pbkdf parameters

#define VIC292_ITERATION_SIZE          4 // size of iteration in binary area
#define VIC292_VERIFIER_SIZE           97 // size of verifier in binary area
#define VIC292_SALT_SIZE               32 // size of salt in binary area

#define CRC_SZ              2   // size of CRC
#define TLV_HEADER_SZ       1+2 // size of Tag | length

#define NB_REC_SZ   2   // size of number record in header

#define BINAREA_HDR_SZ          4
#define BINAREA_FORMAT          0x01
#define BINAREA_VERSION         0x02

#define OFFSET_FORMAT       0
#define OFFSET_VERSION      1

#define OFFSET_TAG          0
#define OFFSET_LEN          1
#define OFFSET_VAL          2

#define MSB16(x) ((x>>8)&0xFF)
#define LSB16(x) (x&0xFF)
#define INT16(a,b) ((a<<8)+b)

#define CRC_INIT 0x0000

#endif /* VLT_MATTER_PRIV_H */
