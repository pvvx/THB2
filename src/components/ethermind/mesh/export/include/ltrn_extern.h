
/**
    \file ltrn_extern.h

*/

/*
    Copyright (C) 2016. Mindtree Ltd.
    All rights reserved.
*/

#ifndef _H_LTRN_EXTERN_
#define _H_LTRN_EXTERN_

/* --------------------------------------------- External Global Definitions */
/* Module Mutex */
MS_DEFINE_MUTEX_TYPE(extern, ltrn_mutex)

/* Module callback pointer */
extern LTRN_NTF_CB ltrn_callback;



API_RESULT ltrn_delete_from_reassembled_cache
(
    /* IN */ MS_NET_ADDR  addr
);

API_RESULT ltrn_delete_from_replay_cache
(
    /* IN */ MS_NET_ADDR  addr
);



#endif /* _H_LTRN_EXTERN_ */

