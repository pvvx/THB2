
/**
    \file aes.c

    AES Interface file for the Open AES-CCM block.

*/

/*
    Copyright (C) 2013. Mindtree Limited.
    All rights reserved.
*/

/* --------------------------------------------- Header File Inclusion */
#include "aes.h"
#include "cry.h"

/* --------------------------------------------- External Global Variables */

/* --------------------------------------------- Exported Global Variables */

/* --------------------------------------------- Static Global Variables */

/* --------------------------------------------- Functions */

void* aes_encrypt_init(const u8* key, size_t len)
{
    u8* ctx;
    ctx = EM_alloc_mem (len);

    if (NULL != ctx)
    {
        memcpy (ctx, key, len);
    }

    return (void*)ctx;
}

void aes_encrypt(void* ctx, const u8* plain, u8* crypt)
{
    INT32 ret;
    cry_aes_128_encrypt_be ((UCHAR*)plain, (UCHAR*)ctx, (UCHAR*)crypt, ret);
    VOID ret;    // resolve warning
}

void aes_encrypt_deinit(void* ctx)
{
    EM_free_mem (ctx);
}


