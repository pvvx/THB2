/***************************************************************************
    Copyright (C) Mindtree Consulting Ltd.
    This module is a confidential and proprietary property of Mindtree and
    a possession or use of this module requires written permission of Mindtree.
 ***************************************************************************/

/**
    \file crypto.h
    Contains the Interface definition of Cryptographic functions required by
    the Host Controller Firmware.

    \date 2007-05-09
*/

#ifndef _CRYPTO_H_
#define _CRYPTO_H_

/* ========================= Include File Section ========================= */
#include "btypes.h"

#ifdef ENABLE_LE_ECDH

/* #define FILE_INPUT */
#define ECPD_ALT_ALG1
#define ECPA_ALT_ALG1
#include "mpal.h"

/* ====================== Macro Declaration Section ======================= */
/* ==================== Data Types Declaration Section ==================== */
/**
    SSP Private Key type. The private key is represented in binary format with
    MSB byte ordering.
*/

typedef u8 ssp_prkey_t[DHKEY_LEN];

/**
    SSP Publick Key type. Each coordinate of the public key is represented in
    binary format with MSB byte ordering.
*/

typedef struct
{
    u8 x[DHKEY_LEN];           /**< X coordinate (Format(MSB): 23..0) */
    u8 y[DHKEY_LEN];           /**< Y coordinate (Format(MSB): 47..DHKEY_LEN) */
} ssp_pukey_t;
/**
    SSP Diffie-Hellman Key (generated shared key) type. The dhkey is
    represented in binary format with MSB byte ordering.
*/

typedef u8 ssp_dhkey_t[DHKEY_LEN];


/* ================ Exported Variables Declaration Section ================ */


/* ============================= Macros Section ============================== */
#ifdef CRYPTO_STANDALONE_ECDH
    /**
    The ECDH algorithm will be used in standalone configuration (in Windows
    builds) . So, there should be no references to /platform/<platform_name>
    APIs.
    This define should be enabled for Windows builds, and should not be enabled
    for other platforms.
    */

    /* Return 32-bit value here. */
    #define CRYPTO_GET_RNG_SEED() 0xDEADC0DE

#else /* CRYPTO_STANDALONE_ECDH */

    #define CRYPTO_GET_RNG_SEED pf_get_rng_seed

#endif /* CRYPTO_STANDALONE_ECDH */


/* ============================= API Section ============================== */
void ssp_init(void);
void ssp_shutdown(void);

#ifdef ECDH_TIME_SLICE

#ifdef CRYPTO_TEST_FRAMEWORK
    u8 ssp_get_ecdh_keypair(const ssp_prkey_t* priv, OUT ssp_pukey_t* pub);
#else
    u8 ssp_get_ecdh_keypair(OUT ssp_prkey_t* priv, OUT ssp_pukey_t* pub);
#endif

u8 ssp_get_dhkey(ssp_prkey_t* priv, ssp_pukey_t* pub,
                 OUT ssp_dhkey_t* dhkey);
#else

#ifdef CRYPTO_TEST_FRAMEWORK
    u8 ssp_get_ecdh_keypair(const ssp_prkey_t* priv, OUT ssp_pukey_t* pub);
#else
    u8 ssp_get_ecdh_keypair(OUT ssp_prkey_t* priv, OUT ssp_pukey_t* pub);
#endif

u8 ssp_get_dhkey(ssp_prkey_t* priv, ssp_pukey_t* pub,
                 OUT ssp_dhkey_t* dhkey);
#endif

#endif /* ENABLE_LE_ECDH */
#endif /* _CRYPTO_H_ */
