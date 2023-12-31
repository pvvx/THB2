/**
    \file appl_light_xyl_client.h

    \brief This file defines the Mesh Light Xyl Model Application Interface
    - includes Data Structures and Methods for Client.
*/

/*
    Copyright (C) 2018. Mindtree Ltd.
    All rights reserved.
*/

#ifndef _H_APPL_LIGHT_XYL_CLIENT_
#define _H_APPL_LIGHT_XYL_CLIENT_


/* --------------------------------------------- Header File Inclusion */
#include "MS_light_xyl_api.h"
#include "appl_main.h"


/* --------------------------------------------- Global Definitions */


/* --------------------------------------------- Data Types/ Structures */


/* --------------------------------------------- Function */
/* light_xyl client application entry point */
void main_light_xyl_client_operations(void);

/* Send Light Xyl Default Get */
void appl_send_light_xyl_default_get(void);

/* Send Light Xyl Default Set */
void appl_send_light_xyl_default_set(void);

/* Send Light Xyl Default Set Unacknowledged */
void appl_send_light_xyl_default_set_unacknowledged(void);

/* Send Light Xyl Get */
void appl_send_light_xyl_get(void);

/* Send Light Xyl Range Get */
void appl_send_light_xyl_range_get(void);

/* Send Light Xyl Range Set */
void appl_send_light_xyl_range_set(void);

/* Send Light Xyl Range Set Unacknowledged */
void appl_send_light_xyl_range_set_unacknowledged(void);

/* Send Light Xyl Set */
void appl_send_light_xyl_set(void);

/* Send Light Xyl Set Unacknowledged */
void appl_send_light_xyl_set_unacknowledged(void);

/* Send Light Xyl Target Get */
void appl_send_light_xyl_target_get(void);

/* Get Model Handle */
void appl_light_xyl_client_get_model_handle(void);

/* Set Publish Address */
void appl_light_xyl_client_set_publish_address(void);

/**
    \brief Client Application Asynchronous Notification Callback.

    \par Description
    Light_Xyl client calls the registered callback to indicate events occurred to the application.

    \param [in] handle        Model Handle.
    \param [in] opcode        Opcode.
    \param [in] data_param    Data associated with the event if any or NULL.
    \param [in] data_len      Size of the event data. 0 if event data is NULL.
*/
API_RESULT appl_light_xyl_client_cb
(
    /* IN */ MS_ACCESS_MODEL_HANDLE* handle,
    /* IN */ UINT32                   opcode,
    /* IN */ UCHAR*                   data_param,
    /* IN */ UINT16                   data_len
);

#endif /*_H_APPL_LIGHT_XYL_CLIENT_ */
