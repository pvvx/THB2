/**
    \file appl_light_lc_client.h

    \brief This file defines the Mesh Light Lc Model Application Interface
    - includes Data Structures and Methods for Client.
*/

/*
    Copyright (C) 2018. Mindtree Ltd.
    All rights reserved.
*/

#ifndef _H_APPL_LIGHT_LC_CLIENT_
#define _H_APPL_LIGHT_LC_CLIENT_


/* --------------------------------------------- Header File Inclusion */
#include "MS_light_lc_api.h"
#include "appl_main.h"


/* --------------------------------------------- Global Definitions */


/* --------------------------------------------- Data Types/ Structures */


/* --------------------------------------------- Function */
/* light_lc client application entry point */
void main_light_lc_client_operations(void);

/* Send Light Lc Light Onoff Get */
void appl_send_light_lc_light_onoff_get(void);

/* Send Light Lc Light Onoff Set */
void appl_send_light_lc_light_onoff_set(void);

/* Send Light Lc Light Onoff Set Unacknowledged */
void appl_send_light_lc_light_onoff_set_unacknowledged(void);

/* Send Light Lc Mode Get */
void appl_send_light_lc_mode_get(void);

/* Send Light Lc Mode Set */
void appl_send_light_lc_mode_set(void);

/* Send Light Lc Mode Set Unacknowledged */
void appl_send_light_lc_mode_set_unacknowledged(void);

/* Send Light Lc Om Get */
void appl_send_light_lc_om_get(void);

/* Send Light Lc Om Set */
void appl_send_light_lc_om_set(void);

/* Send Light Lc Om Set Unacknowledged */
void appl_send_light_lc_om_set_unacknowledged(void);

/* Send Light Lc Property Get */
void appl_send_light_lc_property_get(void);

/* Send Light Lc Property Set */
void appl_send_light_lc_property_set(void);

/* Send Light Lc Property Set Unacknowledged */
void appl_send_light_lc_property_set_unacknowledged(void);

/* Get Model Handle */
void appl_light_lc_client_get_model_handle(void);

/* Set Publish Address */
void appl_light_lc_client_set_publish_address(void);

/**
    \brief Client Application Asynchronous Notification Callback.

    \par Description
    Light_Lc client calls the registered callback to indicate events occurred to the application.

    \param [in] handle        Model Handle.
    \param [in] opcode        Opcode.
    \param [in] data_param    Data associated with the event if any or NULL.
    \param [in] data_len      Size of the event data. 0 if event data is NULL.
*/
API_RESULT appl_light_lc_client_cb
(
    /* IN */ MS_ACCESS_MODEL_HANDLE* handle,
    /* IN */ UINT32                   opcode,
    /* IN */ UCHAR*                   data_param,
    /* IN */ UINT16                   data_len
);

#endif /*_H_APPL_LIGHT_LC_CLIENT_ */
