
/**
    \file access_extern.h

*/

/*
    Copyright (C) 2016. Mindtree Ltd.
    All rights reserved.
*/

#ifndef _H_ACCESS_EXTERN_
#define _H_ACCESS_EXTERN_

#include "MS_access_api.h"

/* --------------------------------------------- Data Types/ Structures */

/** Current IV Index and Update State */
typedef struct _MS_ACCESS_IV_INDEX
{
    /** Current IV Index */
    UINT32 iv_index;

    /** Current IV Update State */
    UINT8  iv_update_state;

    /** IV Update time expire */
    UINT32 iv_expire_time;

} MS_ACCESS_IV_INDEX;


/* --------------------------------------------- Global Definitions */
#define NVS_FLASH_BASE1     0x39000
#define NVS_FLASH_BASE2     0x3a000

/* --------------------------------------------- External Global Definitions */
/**
    Current IV Index and associated update state
*/
extern MS_ACCESS_IV_INDEX ms_iv_index;

extern UINT8 access_default_ttl;

/** Start unicast Address */
extern MS_NET_ADDR  ms_start_unicast_addr;

/** Stop unicast Address */
extern MS_NET_ADDR  ms_stop_unicast_addr;


extern UINT8   ms_ps_store_disable_flag;
extern MS_NET_ADDR ms_provisioner_addr;

extern UINT8        rx_test_ttl;
extern UINT8        vendor_tid;




/* Macro to get default TTL primary unicast address */
#define ACCESS_CM_GET_DEFAULT_TTL(ttl) \
    (ttl) = access_default_ttl

/* Macro to get rx TTL primary unicast address */
#define ACCESS_CM_GET_RX_TTL(ttl) \
    (ttl) = rx_test_ttl


API_RESULT MS_access_raw_data
(
    /* IN */ MS_ACCESS_MODEL_HANDLE*   handle,
    /* IN */ UINT32                    opcode,
    /* IN */ MS_NET_ADDR               dst_addr,
    /* IN */ MS_APPKEY_HANDLE        appKeyHandle,
    /* IN */ UCHAR*                    data_param,
    /* IN */ UINT16                    data_len,
    /* IN */ UINT8                     reliable
);

/** Get Publish Address */
API_RESULT MS_access_get_publish_addr
(
    /* IN */ MS_ACCESS_MODEL_HANDLE*   handle,
    /* IN */ MS_NET_ADDR*              publish_addr
);


/** Publish */
API_RESULT MS_access_publish_ex
(
    /* IN */ MS_ACCESS_MODEL_HANDLE*   handle,
    /* IN */ UINT32                    opcode,
    /* IN */ MS_NET_ADDR               dst_addr,
    /* IN */ UCHAR*                    data_param,
    /* IN */ UINT16                    data_len,
    /* IN */ UINT8                     reliable
);

/** Store all record */
void MS_access_ps_store_all_record(void);

API_RESULT MS_access_ps_store_disable(UINT8 enable);

API_RESULT MS_access_ps_crc_check(void);
#endif /* _H_ACCESS_EXTERN_ */

