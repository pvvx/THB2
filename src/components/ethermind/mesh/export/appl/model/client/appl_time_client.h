/**
    \file appl_time_client.h

    \brief This file defines the Mesh Time Model Application Interface
    - includes Data Structures and Methods for Client.
*/

/*
    Copyright (C) 2018. Mindtree Ltd.
    All rights reserved.
*/

#ifndef _H_APPL_TIME_CLIENT_
#define _H_APPL_TIME_CLIENT_


/* --------------------------------------------- Header File Inclusion */
#include "MS_time_api.h"
#include "appl_main.h"


/* --------------------------------------------- Global Definitions */


/* --------------------------------------------- Data Types/ Structures */


/* --------------------------------------------- Function */
/* time client application entry point */
void main_time_client_operations(void);

/* Send Tai Utc Delta Get */
void appl_send_tai_utc_delta_get(void);

/* Send Tai Utc Delta Set */
void appl_send_tai_utc_delta_set(void);

/* Send Time Get */
void appl_send_time_get(void);

/* Send Time Role Get */
void appl_send_time_role_get(void);

/* Send Time Role Set */
void appl_send_time_role_set(void);

/* Send Time Set */
void appl_send_time_set(void);

/* Send Time Zone Get */
void appl_send_time_zone_get(void);

/* Send Time Zone Set */
void appl_send_time_zone_set(void);

/* Get Model Handle */
void appl_time_client_get_model_handle(void);

/* Set Publish Address */
void appl_time_client_set_publish_address(void);

/**
    \brief Client Application Asynchronous Notification Callback.

    \par Description
    Time client calls the registered callback to indicate events occurred to the application.

    \param [in] handle        Model Handle.
    \param [in] opcode        Opcode.
    \param [in] data_param    Data associated with the event if any or NULL.
    \param [in] data_len      Size of the event data. 0 if event data is NULL.
*/
API_RESULT appl_time_client_cb
(
    /* IN */ MS_ACCESS_MODEL_HANDLE* handle,
    /* IN */ UINT32                   opcode,
    /* IN */ UCHAR*                   data_param,
    /* IN */ UINT16                   data_len
);

#endif /*_H_APPL_TIME_CLIENT_ */
