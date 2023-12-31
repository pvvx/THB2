
/**
    \file aes_cmac.h


*/

/*
    Copyright (C) 2013. Mindtree Ltd.
    All rights reserved.
*/

#ifndef _H_AES_CMAC_
#define _H_AES_CMAC_

/* --------------------------------------------- Header File Inclusion */
#include "EM_os.h"

/* --------------------------------------------- Global Definitions */
/* TODO: Make it proper error id */
#define AES_CMAC_PROC_PENDING                    0x1100

/* Operations */
#define AES_CMAC_MAC_GENERATE             0x00
#define AES_CMAC_MAC_VERIFY               0x01

/* AES-CMAC key and data block size */
#define AES_CMAC_KEY_SIZE                 16
#define AES_CMAC_BLOCK_SIZE               16
#define AES_128_ENC_KEY_SIZE              16
#define AES_128_ENC_DATA_SIZE             16
#define AES_128_ENC_OUT_SIZE              16

/* AES-CMAC Block size exponent (2^4 = 16) */
#define AES_CMAC_BLOCK_EXPONENT           4

/* AES-CMAC state masks */
#define AES_CMAC_STATE_INIT               0x01
#define AES_CMAC_STATE_OPERATING          0x02
#define AES_CMAC_STATE_IN_ENCRYPT         0x04

#define AES_CMAC_STATE_SUBKEY_GEN         0x10
#define AES_CMAC_STATE_MAC_GENERATE       0x20
#define AES_CMAC_STATE_MAC_VERIFY         0x40

/* --------------------------------------------- Structures/Data Types */
/* AES-CMAC context callback type */
typedef void (* AES_CMAC_CB) (void);

/* AES-CMAC Context */
typedef struct _AES_CMAC_CONTEXT
{
    /** Application Use */

    /* 128-Bit Key to be used for the CMAC algorithm */
    UCHAR* key;

    /* Message to be used */
    UCHAR* data;

    /* Message Length */
    UINT16  datalen;

    /* MAC */
    UCHAR* mac;

    /* MAC length, in bytes */
    UCHAR maclen;

    /* Action - Generate MAC or Verify MAC */
    UCHAR action;

    /* Context callback */
    AES_CMAC_CB cb;

    /** Internal Use */

    /* Context state */
    UCHAR state;

    /* Number of data blocks */
    UINT16 num_blocks;

    /* Number of processed data blocks */
    UINT16 processed;

    /* Subkey references */
    UCHAR subkey1[AES_CMAC_KEY_SIZE];
    UCHAR subkey2[AES_CMAC_KEY_SIZE];

} AES_CMAC_CONTEXT;

/* --------------------------------------------- Macros */

/* --------------------------------------------- API Declarations */
/* Initialize module */
EM_RESULT aes_cmac_init (void);

/* Initialize Context */
EM_RESULT aes_cmac_context_init (AES_CMAC_CONTEXT* context);

/* Generate/Verify MAC for Context */
EM_RESULT aes_cmac (AES_CMAC_CONTEXT* context);

/* Interface to indicate encrypt complete */
void aes_cmac_aes_128_encryption_complete (UCHAR status, UCHAR* data, UINT16 datalen);

#endif /* _H_AES_CMAC_ */

