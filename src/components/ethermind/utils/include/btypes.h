/***************************************************************************
    Copyright (C) Mindtree Consulting Ltd.
    This module is a confidential and proprietary property of Mindtree and
    a possession or use of this module requires written permission of Mindtree.
 ***************************************************************************/

/**
    \file btypes.h
    Basic types definition. This file should be modified based on the target
    platform.

    \date 2007-08-20
*/

#ifndef _BTYPES_H_
#define _BTYPES_H_

/* #define CRYPTO_TEST_FRAMEWORK */
#define CRYPTO_STANDALONE_ECDH
#define ENABLE_LE_ECDH
#define SUPPORT_REMOTE_PUBLIC_KEY_VALIDATION

#ifndef CRYPTO_TEST_FRAMEWORK
    #ifndef CRYPTO_STANDALONE_ECDH
        #include "bt_le_features.h"
        #include "platform.h"
        #include "bt_os_common.h"
    #else
        #include "EM_os.h"
    #endif /* CRYPTO_STANDALONE_ECDH */

    /* Enable if using drbg random generator */
    #ifndef CRYPTO_STANDALONE_ECDH
        #define USE_DRBG_RAND_GEN
    #endif /* CRYPTO_STANDALONE_ECDH */

    /* Enable if the using big endian input/out format */
    /* #define ECDH_BIG_ENDIAN_IO */

#else
    /* Test framework */
    #define ENABLE_LE_ECDH

    #include <string.h>
    #include <stdlib.h>
    #include <string.h>

#endif /* CRYPTO_TEST_FRAMEWORK */

#ifdef ENABLE_LE_ECDH
#define ECDH_FALSE                       (0)
#define ECDH_TRUE                        (!(ECDH_FALSE))

/* ========================= Include File Section ========================= */


/* ====================== Macro Declaration Section ======================= */
#ifndef IN
    #define IN              /*@in@*/
#endif
#ifndef OUT
    #define OUT             /*@out@*/
#endif
#ifndef INOUT
    #define INOUT           IN OUT
#endif

#ifndef CRYPTO_STANDALONE_ECDH
    #ifndef ALIGN
        #define ALIGN(n)    __attribute__((aligned(n)))
    #endif
#else
    #define ALIGN(n)
#endif /* CRYPTO_STANDALONE_ECDH */
/* Macro to remove: unsed param warning */
#define CRYPTO_IGNORE_PARAM(v)       (void)(v)
#define ecdh_memset                   memset
#define ecdh_memcpy                   memcpy

#ifdef BLUEWIZ_BIG_ENDIAN
#define ld32_msb(v, m)  (v) = (*((u32*)(m)))
#define st32_msb(m, v)  ecdh_memcpy((m), &(v), 4)
#else
#define ld32_msb(v, m)  (v) = (((m)[0]<<24) | ((m)[1]<<16) | \
                               ((m)[2]<<8) | (m)[3])
#define st32_msb(m, v)  (m)[0]=(u8)((v)>>24); (m)[1]=(u8)((v)>>16); \
    (m)[2]=(u8)((v)>>8); (m)[3]=(u8)(v);
#endif /* BLUEWIZ_BIG_ENDIAN */

#define rol32(r, n)     (((r) << (n)) | ((r) >> (32-(n))))
#define ror32(r, n)     (((r) >> (n)) | ((r) << (32-(n))))

#define USE_32BIT_PROC
#define ECDH_TIME_SLICE

/* ==================== Data Types Declaration Section ==================== */
typedef unsigned char u8;
typedef unsigned short int u16;
typedef unsigned int u32;
typedef unsigned long long u64;
typedef signed char s8;
typedef signed short int s16;
typedef signed int s32;
typedef signed long long s64;

/* ==================== Data Types Declaration Section ==================== */

/* ================ Exported Variables Declaration Section ================ */


/* ============================= API Section ============================== */


#endif /* ENABLE_LE_ECDH */

#endif /* _BTYPES_H_ */
