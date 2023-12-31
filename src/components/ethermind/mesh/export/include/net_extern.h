
/**
    \file net_extern.h

*/

/*
    Copyright (C) 2016. Mindtree Ltd.
    All rights reserved.
*/

#ifndef _H_NET_EXTERN_
#define _H_NET_EXTERN_

#include "MS_net_api.h"


/* --------------------------------------------- External Global Definitions */
/* Module Mutex */
MS_DEFINE_MUTEX_TYPE(extern, net_mutex)

/* Module callback pointer */
extern NET_NTF_CB net_callback;

/* Network Sequence Number */
extern NET_SEQ_NUMBER_STATE net_seq_number_state;

extern UINT8 seq_num_init_flag;
extern UINT32 g_iv_update_index;
extern UINT8 g_iv_update_state;
extern UINT8 g_iv_update_start_timer;
extern UINT8   MS_key_refresh_active;

extern UINT16 ms_proxy_filter_addr[5];

/* Network Cache */
//MS_DECLARE_GLOBAL_ARRAY(NET_CACHE_ELEMENT, net_cache, MS_CONFIG_LIMITS(MS_NET_CACHE_SIZE));

//extern UINT16 net_cache_start;
//extern UINT16 net_cache_size;

API_RESULT net_delete_from_cache
(
    /* IN */ MS_NET_ADDR  addr
);


#endif /* _H_NET_EXTERN_ */

