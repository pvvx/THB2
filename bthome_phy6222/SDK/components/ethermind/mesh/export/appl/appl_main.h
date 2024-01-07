
/**
    \file appl_main.h

    Header File for the Test Application to test the Mindtree
    Mesh stack.
*/

/*
    Copyright (C) 2016. Mindtree Ltd.
    All rights reserved.
*/

#ifndef _H_APPL_MAIN_
#define _H_APPL_MAIN_

/* --------------------------------------------- Header File Inclusion */
#include "MS_access_api.h"
#include "MS_config_api.h"

/* --------------------------------------------- Global Definitions */

/* --------------------------------------------- Structures/Data Types */

/* --------------------------------------------- Macros */

/* Console Input/Output */
#define CONSOLE_OUT(...)    printf(__VA_ARGS__)
#define CONSOLE_IN(...)     scanf(__VA_ARGS__)

/* --------------------------------------------- Internal Functions */
void main_register_models(void);
void appl_dump_bytes(UCHAR* buffer, UINT16 length);
void appl_model_power_cycle(void);
void appl_prov_bind(UCHAR brr, UCHAR index);
void appl_prov_get_local_public_key(void);
void appl_prov_input_auth_val (UCHAR mode, void* data, UINT16 datalen);
void appl_prov_register(void);
void appl_prov_set_auth_action
(
    UCHAR mode,
    UCHAR* s_oob_val,
    UCHAR oob_act,
    UCHAR oob_sz
);
void appl_prov_setup(UCHAR role, UCHAR brr);
API_RESULT appl_prov_update_remote_uuid
(
    UCHAR*   uuid,
    UINT16 uuid_len,
    UINT16 oob_info,
    UCHAR*   uri
);
void appl_prov_set_uuid (UCHAR* uuid);
void appl_prov_update_uuid (UCHAR octet);

void appl_proxy_adv
(
    UCHAR identification_type,
    MS_SUBNET_HANDLE subnet_handle
);
void appl_proxy_register(void);
void appl_prov_set_dev_public_key(void);
void appl_prov_bind_device(UCHAR brr);
void appl_proxy_start_net_id_adv(MS_SUBNET_HANDLE subnet_handle);
API_RESULT appl_is_configured (void);

#endif /* _H_APPL_MAIN_ */

