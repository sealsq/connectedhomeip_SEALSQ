#pragma once

#ifndef VLT_MATTER_CONFIG_H
#define VLT_MATTER_CONFIG_H

/* VaultIC configuration */

#define MATTER_CONFIG   // Use Matter config

/* Definition of VaultIC resources */
#define I2C_BITRATE     400     //400kHz
#define I2C_ADDRESS     0x5F

#define SELF_TESTS_DELAY        200 // 200ms
#define APDU_TIMEOUT            5000 // 5s

#define INITIAL_PAIRING_KEY { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,\
                              0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F }

// Device Certificate
#define REC_ID_PAI_CERT  1        // PAI certificate stored in rec 1 of binary area
#define REC_ID_DAC_CERT  2        // DAC certificate stored in rec 2 of binary area
#define REC_ID_VERIFIER  3        // Verifier stored in rec 3 of binary area
#define REC_ID_PBKDF     4        // PBKDF stored in rec 4 of binary area

// Definition of default key index values used by vaultic_tls
// NOTE: the key indexes can be set using vlt_tls_select_static_priv_key() and vlt_tls_select_ephemeral_priv_key func()
#define STATIC_KEY_INDEX    0x00    // Index of default static key pair in VaultIC
#define EPH_KEY_KEY_INDEX   0x00    // Index of default ephemeral key pair in VaultIC

#define END_CHARACTER       0xFF

#define ENABLE_VIC292_DEVICE_ATTESTATION 1

#endif /* VLT_MATTER_CONFIG_H */
