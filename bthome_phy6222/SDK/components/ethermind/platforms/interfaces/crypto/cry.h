
/**
    \file cry.h

    Platform header file for cryptographic functions that will be used from
    other IP modules.
*/

/*
    Copyright (C) 2016. Mindtree Limited.
    All rights reserved.
*/

/* --------------------------------------------- Header File Inclusion */
#include "EM_os.h"

//#define CRY_ECDH_TIMESLICE

#define MESH_ECDH_ENABLE

/* --------------------------------------------- Global Definitions */
/* Size definitions */
#define CRY_AES128_DATALEN              16
#define CRY_AES128_KEYLEN               16
#define CRY_AES128CCM_DATALEN_MAX       150
#define CRY_AES128CMAC_DATALEN_MAX      150
#define CRY_AES128CMAC_MACLEN           16
#define CRY_ECDH_PUBKEY_SIZE            64
#define CRY_ECDH_PVTKEY_SIZE            32
#define CRY_ECDH_SECRET_SIZE            32

/* AES-CMAC operations */
#define CRY_AES_128_CMAC_SIGN           0x00
#define CRY_AES_128_CMAC_VERIFY         0x01

/* --------------------------------------------- Structures/Data Types */

/* --------------------------------------------- Macros */
#define CRY_LE2BE(s,d,l)    cry_reverse_bytestream_endianness((s),(d),(l))
#define CRY_BE2LE(s,d,l)    cry_reverse_bytestream_endianness((s),(d),(l))

/* --------------------------------------------- Internal Functions */

/* --------------------------------------------- API Declarations */
#define cry_aes_128_encrypt_be(p, k, c, r) \
    (r) = cry_aes_128_encrypt((p), (k), (c))

#define cry_aes_128_decrypt_be(c, k, p, r) \
    (r) = cry_aes_128_decrypt((c), (k), (p))

#define cry_aes_128_ccm_encrypt_be(k, n, nl, p, pl, a, al, c, m, ml, r) \
    (r) = cry_aes_128_ccm_encrypt((k), (n), (nl), (p), (pl), (a), (al), (c), (m), (ml))

#define cry_aes_128_ccm_decrypt_be(k, n, nl, c, cl, a, al, p, m, ml, r) \
    (r) = cry_aes_128_ccm_decrypt((k), (n), (nl), (c), (cl), (a), (al), (p), (m), (ml))

#define cry_aes_128_cmac_sign_be(p, pl, k, m, ml, r) \
    (r) = cry_aes_128_cmac(CRY_AES_128_CMAC_SIGN, (p), (pl), (k), (m), (ml))

#define cry_aes_128_cmac_verify_be(p, pl, k, m, ml, r) \
    (r) = cry_aes_128_cmac(CRY_AES_128_CMAC_VERIFY, (p), (pl), (k), (m), (ml))

#define cry_ecdh_get_public_key_be(p, r) \
    (r) = cry_ecdh_get_public_key((p))

#define cry_ecdh_generate_secret_be(p, s, sl, r, cb) \
    (r) = cry_ecdh_generate_secret((p), (s), (sl), (cb)); \


#define cry_aes_128_encrypt_le(p, k, c, r) \
    { \
        UCHAR cry_plain[CRY_AES128_DATALEN]; \
        UCHAR cry_key[CRY_AES128_KEYLEN]; \
        UCHAR cry_cipher[CRY_AES128_DATALEN]; \
        \
        CRY_LE2BE((p), cry_plain, CRY_AES128_DATALEN); \
        CRY_LE2BE((k), cry_key, CRY_AES128_KEYLEN); \
        \
        (r) = cry_aes_128_encrypt(cry_plain, cry_key, cry_cipher); \
        \
        CRY_BE2LE(cry_cipher, (c), CRY_AES128_DATALEN); \
    }

#define cry_aes_128_decrypt_le(c, k, p, r) \
    { \
        UCHAR cry_plain[CRY_AES128_DATALEN]; \
        UCHAR cry_key[CRY_AES128_KEYLEN]; \
        UCHAR cry_cipher[CRY_AES128_DATALEN]; \
        \
        CRY_LE2BE((c), cry_cipher, CRY_AES128_DATALEN); \
        CRY_LE2BE((k), cry_key, CRY_AES128_KEYLEN); \
        \
        (r) = cry_aes_128_decrypt(cry_cipher, cry_key, cry_plain); \
        \
        CRY_BE2LE(cry_plain, (p), CRY_AES128_DATALEN); \
    }

#define cry_aes_128_ccm_encrypt_le(k, n, nl, p, pl, a, al, c, m, ml, r) \
    { \
        UCHAR cry_plain[CRY_AES128CCM_DATALEN_MAX]; \
        UCHAR cry_nonce[CRY_AES128CCM_DATALEN_MAX]; \
        UCHAR cry_addl[CRY_AES128CCM_DATALEN_MAX]; \
        UCHAR cry_cipher[CRY_AES128CCM_DATALEN_MAX]; \
        UCHAR cry_mac[CRY_AES128CCM_DATALEN_MAX]; \
        UCHAR cry_key[CRY_AES128_KEYLEN]; \
        \
        CRY_LE2BE((p), cry_plain, (pl)); \
        CRY_LE2BE((n), cry_nonce, (nl)); \
        CRY_LE2BE((a), cry_addl, (al)); \
        CRY_LE2BE((k), cry_key, CRY_AES128_KEYLEN); \
        \
        (r) = cry_aes_128_ccm_encrypt(cry_key, cry_nonce, (nl), cry_plain, (pl), cry_addl, (al), cry_cipher, cry_mac, (ml)); \
        \
        CRY_BE2LE(cry_cipher, (c), (pl)); \
        CRY_BE2LE(cry_mac, (m), (ml)); \
    }

#define cry_aes_128_ccm_decrypt_le(k, n, nl, c, cl, a, al, p, m, ml, r) \
    { \
        UCHAR cry_plain[CRY_AES128CCM_DATALEN_MAX]; \
        UCHAR cry_nonce[CRY_AES128CCM_DATALEN_MAX]; \
        UCHAR cry_addl[CRY_AES128CCM_DATALEN_MAX]; \
        UCHAR cry_cipher[CRY_AES128CCM_DATALEN_MAX]; \
        UCHAR cry_mac[CRY_AES128CCM_DATALEN_MAX]; \
        UCHAR cry_key[CRY_AES128_KEYLEN]; \
        \
        CRY_LE2BE((c), cry_cipher, (cl)); \
        CRY_LE2BE((n), cry_nonce, (nl)); \
        CRY_LE2BE((a), cry_addl, (al)); \
        CRY_LE2BE((m), cry_mac, (ml)); \
        CRY_LE2BE((k), cry_key, CRY_AES128_KEYLEN); \
        \
        (r) = cry_aes_128_ccm_decrypt(cry_key, cry_nonce, (nl), cry_cipher, (cl), cry_addl, (al), cry_plain, cry_mac, (ml)); \
        \
        CRY_BE2LE(cry_plain, (p), (cl)); \
    }

#define cry_aes_128_cmac_sign_le(p, pl, k, m, ml, r) \
    { \
        UCHAR cry_plain[CRY_AES128CMAC_DATALEN_MAX]; \
        UCHAR cry_key[CRY_AES128_KEYLEN]; \
        UCHAR cry_mac[CRY_AES128CMAC_MACLEN]; \
        \
        CRY_LE2BE((p), cry_plain, (pl)); \
        CRY_LE2BE((k), cry_key, CRY_AES128_KEYLEN); \
        \
        (r) = cry_aes_128_cmac(CRY_AES_128_CMAC_SIGN, cry_plain, (pl), cry_key, cry_mac, (ml)); \
        \
        CRY_BE2LE(cry_mac, (m), (ml)); \
    }

#define cry_aes_128_cmac_verify_le(p, pl, k, m, ml, r) \
    { \
        UCHAR cry_plain[CRY_AES128CMAC_DATALEN_MAX]; \
        UCHAR cry_key[CRY_AES128_KEYLEN]; \
        UCHAR cry_mac[CRY_AES128CMAC_MACLEN]; \
        \
        CRY_LE2BE((p), cry_plain, (pl)); \
        CRY_LE2BE((k), cry_key, CRY_AES128_KEYLEN); \
        CRY_LE2BE((m), cry_mac, (ml)); \
        \
        (r) = cry_aes_128_cmac(CRY_AES_128_CMAC_VERIFY, cry_plain, (pl), cry_key, cry_mac, (ml)); \
    }

#define cry_ecdh_get_public_key_le(p, r) \
    { \
        UCHAR cry_pub[CRY_ECDH_PUBKEY_SIZE]; \
        \
        (r) = cry_ecdh_get_public_key(cry_pub); \
        \
        CRY_BE2LE(cry_pub, (p), CRY_ECDH_PUBKEY_SIZE); \
        \
    }

#define cry_ecdh_generate_secret_le(p, s, sl, r, cb) \
    { \
        UCHAR cry_pub[CRY_ECDH_PUBKEY_SIZE]; \
        UCHAR cry_skey[CRY_ECDH_SECRET_SIZE]; \
        \
        CRY_LE2BE((p), cry_pub, CRY_ECDH_PUBKEY_SIZE); \
        \
        (r) = cry_ecdh_generate_secret(cry_pub, cry_skey, (sl), (cb)); \
        \
        CRY_BE2LE(cry_skey, (s), (sl)); \
        \
    }


void cry_reverse_bytestream_endianness(UCHAR* src, UCHAR* dst, UINT16 len);
void cry_reverse_bytestream_in_place(UCHAR* src, UINT16 len);

INT32 cry_rand_generate(UCHAR* prand, UINT16 randlen);

INT32 cry_aes_128_encrypt
(
    UCHAR* pdata,
    UCHAR* pkey,
    UCHAR* pencdata
);

INT32 cry_aes_128_decrypt
(
    UCHAR* pencdata,
    UCHAR* pkey,
    UCHAR* pdata
);

INT32 cry_aes_128_ccm_encrypt
(
    UCHAR* pkey,
    UCHAR* pnonce,
    UINT16  noncelen,
    UCHAR* pdata,
    UINT16  datalen,
    UCHAR* paddata,
    UINT16  addatalen,
    UCHAR* pencdata,
    UCHAR* pmac,
    UINT16  maclen
);

INT32 cry_aes_128_ccm_decrypt
(
    UCHAR* pkey,
    UCHAR* pnonce,
    UINT16  noncelen,
    UCHAR* pencdata,
    UINT16  encdatalen,
    UCHAR* paddata,
    UINT16  addatalen,
    UCHAR* pdata,
    UCHAR* pmac,
    UINT16  maclen
);

INT32 cry_aes_128_cmac
(
    UCHAR   op,
    UCHAR* pdata,
    UINT16  datalen,
    UCHAR* pkey,
    UCHAR* pmac,
    UINT16  maclen
);

INT32 cry_ecdh_init(void);
INT32 cry_ecdh_get_public_key(UCHAR* pubkey);
INT32 cry_ecdh_generate_secret
(
    UCHAR* peer_pubkey,
    UCHAR* secret,
    UINT16 secret_len,
    void (*ecdh_cb)(UCHAR*)
);
INT32 cry_set_ecdh_debug_keypair(UCHAR* pvtkey, UCHAR* pubkey);

#ifdef CRY_ECDH_TIMESLICE
    INT32 cry_ecdh_process_secret(void);
#endif /* CRY_ECDH_TIMESLICE */

typedef int(*uCrypto_RNG_Function)(uint8_t* dest, uint32_t size);
void uCrypto_set_rng(uCrypto_RNG_Function rng_function);

uCrypto_RNG_Function uCrypto_get_rng(void);
INT32 mesh_ecdh_init(void);
INT32 mesh_ecdh_generate_secret(UCHAR* peer_pubkey, UCHAR* secret, UINT16 secret_len);
INT32 mesh_ecdh_get_public_key(UCHAR* pubkey);


void test_verify_mesh_ecdh(void);
