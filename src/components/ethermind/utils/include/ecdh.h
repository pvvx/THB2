/***************************************************************************
    Copyright (C) Mindtree Consulting Ltd.
    This module is a confidential and proprietary property of Mindtree and
    a possession or use of this module requires written permission of Mindtree.
 ***************************************************************************/

/**
    \file ecdh.h
    Contains the Interface definition of ECDH functions required by
    the Cryptographic modules.

    \date 2008-03-07
*/

#ifndef _ECDH_H_
#define _ECDH_H_

/* ========================= Include File Section ========================= */
#include "btypes.h"

#ifdef ENABLE_LE_ECDH
    #include "mpal.h"

    /* ============================= API Section ============================== */
    u8 verify_point_on_curve( DIGIT_S* X, DIGIT_S* Y);

    #ifdef ECDH_TIME_SLICE
        u8 mixed_scalar_multiply(u8* S, u8* X, u8* Y);
        u8 conv_coord(DIGIT_S* S, OUT DIGIT_S* X, OUT DIGIT_S* Y);
    #else
        void mixed_scalar_multiply(u8* S, u8* X, u8* Y);
    #endif

#endif /* ENABLE_LE_ECDH */

#endif /* _ECDH_H_ */
