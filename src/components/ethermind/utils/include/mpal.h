/***************************************************************************
    Copyright (C) Mindtree Consulting Ltd.
    This module is a confidential and proprietary property of Mindtree and
    a possession or use of this module requires written permission of Mindtree.
 ***************************************************************************/

/**
    \file mpal.h
    Multi-Precision Arithmetic Library(MPAL) interface.

    \date 2007-08-20
*/

#ifndef _MPAL_H_
#define _MPAL_H_

/* ========================= Include File Section ========================= */
#include "btypes.h"

/* ====================== Macro Declaration Section ======================= */

#ifdef ENABLE_LE_ECDH

/* #define USE_ARM_ASM */

#ifdef USE_32BIT_PROC
    #define MPAL_MAX_LEN   16
    #define MPAL_BASE                32
    #define MPAL_BASE_MASK           0xFFFFFFFF
#else /* USE_32BIT_PROC */
    #define MPAL_MAX_LEN   32
    #define MPAL_BASE                16
    #define MPAL_BASE_MASK           0xFFFF
#endif /* USE_32BIT_PROC */

#define PRIME_BITS 256
#define DHKEY_LEN (PRIME_BITS/8)    /* Number of bytes of key as per ECDH procedure */
#define MPAL_PRIME_LEN (PRIME_BITS/MPAL_BASE)


#define mpal_set_number(A, len_A, fill) ecdh_memset((A), (fill), (MPAL_BASE/8) * (len_A));

/*  Unsafe, fix this - or use proper Notation. Also depending on sizeof
    (DIGIT ) multiplier*/
#define MPAL_MACRO_DEF  u16 macro_len_A

/* Call MPAL_MACRO_DEF in initialization section, before calling this macro */
#define mpal_trim(A,len,out_len)                    \
    macro_len_A = (u16)(len);                   \
    while(macro_len_A  && A[--macro_len_A]==0); \
    (out_len) = (u16)(macro_len_A+1);           \

void convert_to_lsb(u8* bytes, u8 len);
#define convert_to_msb(b, l) convert_to_lsb((b), (l))

/* ==================== Data Types Declaration Section ==================== */
#ifdef USE_32BIT_PROC
    typedef s64 SIGNED_BIG_DIGIT;
    typedef u64 UNSIGNED_BIG_DIGIT;
    typedef u32 DIGIT_S;
#else /* USE_32BIT_PROC */
    typedef s32 SIGNED_BIG_DIGIT;
    typedef u32 UNSIGNED_BIG_DIGIT;
    typedef u16 DIGIT_S;
#endif /* USE_32BIT_PROC */

/* ===================== Variable Declaration Section ===================== */
extern const DIGIT_S PRIME[MPAL_PRIME_LEN];
extern const DIGIT_S  curve_a[MPAL_PRIME_LEN];
extern const DIGIT_S  curve_b[MPAL_PRIME_LEN];

/* ============================= API Section ============================== */
void mpal_add_u8(INOUT u8* A, u16 len_A, const u8* B, u16 len_B);
void mpal_add(INOUT DIGIT_S* A, const DIGIT_S* B);
void mpal_sub(INOUT DIGIT_S* A, const DIGIT_S* B);
void mpal_mult(const DIGIT_S* A, const DIGIT_S* B, OUT DIGIT_S* C);
s8   mpal_compare(const DIGIT_S* A, const DIGIT_S* B);
u16  mpal_right_shift(INOUT DIGIT_S* A,u16 len_A);
void mpal_mod_by_sub(INOUT DIGIT_S* A);
void mpal_div_by_2(INOUT DIGIT_S* A, INOUT DIGIT_S* B);
void mpal_mult_by_2(INOUT DIGIT_S* A, INOUT DIGIT_S* B);

#endif /* ENABLE_LE_ECDH */
#endif /* _MPAL_H_ */
