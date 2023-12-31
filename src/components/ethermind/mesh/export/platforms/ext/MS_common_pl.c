
/**
    \file MS_common_pl.c

    Common routines and start-up initialization & shutdown handlers
    (Platform: Windows User Mode)
*/

/*
    Copyright (C) 2016. Mindtree Ltd.
    All rights reserved.
*/

/* ------------------------------------------- Header File Inclusion */
#include "MS_common.h"
#include "MS_access_api.h"
#include "blebrr.h"

/* ------------------------------------------- External Global Variables */


/* ------------------------------------------- External Global Variables */


/* ------------------------------------------- External Global Variables */


/* ------------------------------------------- Functions */
/* EtherMind-Init: Platform Handler */
void ms_init_pl (void)
{
}

__ATTR_SECTION_XIP__ UINT8  MS_common_reset(void)
{
    UINT8   retval;
    UINT8  proxy_state,proxy;
    retval = MS_TRUE;
    MS_access_cm_get_features_field(&proxy, MS_FEATURE_PROXY);
    MS_proxy_fetch_state(&proxy_state);

    if((MS_TRUE == proxy) && (proxy_state == MS_PROXY_CONNECTED))
    {
        blebrr_disconnect_pl();
        retval = MS_FALSE;
    }

    #if(BLEMESH_ROLE == PROV_ROLE_PROVISIONER)
    MS_access_cm_reset(PROV_ROLE_PROVISIONER);
    #else
    nvs_reset(NVS_BANK_PERSISTENT);
    MS_access_cm_reset(PROV_ROLE_DEVICE);
    #endif
    return retval;
}



#ifndef MS_NO_SHUTDOWN

/* Mesh Shutdown: Platform Handler */
void ms_shutdown_pl (void)
{
}

#endif /* MS_NO_SHUTDOWN */


