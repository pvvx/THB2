/**
    \file appl_light_lightness_client.h

    \brief This file defines the Mesh Light Lightness Model Application Interface
    - includes Data Structures and Methods for Client.
*/

/*
    Copyright (C) 2018. Mindtree Ltd.
    All rights reserved.
*/

#ifndef _H_APPL_LIGHT_LIGHTNESS_CLIENT_
#define _H_APPL_LIGHT_LIGHTNESS_CLIENT_


/* --------------------------------------------- Header File Inclusion */
#include "MS_light_lightness_api.h"
#include "appl_main.h"


/* --------------------------------------------- Global Definitions */


/* --------------------------------------------- Data Types/ Structures */


/* --------------------------------------------- Function */
/* light_lightness client application entry point */
void main_light_lightness_client_operations(void);

/* Send Light Lightness Default Get */
void appl_send_light_lightness_default_get(void);

/* Send Light Lightness Default Set */
void appl_send_light_lightness_default_set(void);

/* Send Light Lightness Default Set Unacknowledged */
void appl_send_light_lightness_default_set_unacknowledged(void);

/* Send Light Lightness Get */
void appl_send_light_lightness_get(void);

/* Send Light Lightness Last Get */
void appl_send_light_lightness_last_get(void);

/* Send Light Lightness Linear Get */
void appl_send_light_lightness_linear_get(void);

/* Send Light Lightness Linear Set */
void appl_send_light_lightness_linear_set(void);

/* Send Light Lightness Linear Set Unacknowledged */
void appl_send_light_lightness_linear_set_unacknowledged(void);

/* Send Light Lightness Range Get */
void appl_send_light_lightness_range_get(void);

/* Send Light Lightness Range Set */
void appl_send_light_lightness_range_set(void);

/* Send Light Lightness Range Set Unacknowledged */
void appl_send_light_lightness_range_set_unacknowledged(void);

/* Send Light Lightness Set */
void appl_send_light_lightness_set(void);

/* Send Light Lightness Set Unacknowledged */
void appl_send_light_lightness_set_unacknowledged(void);

/* Get Model Handle */
void appl_light_lightness_client_get_model_handle(void);

/* Set Publish Address */
void appl_light_lightness_client_set_publish_address(void);

/**
    \brief Client Application Asynchronous Notification Callback.

    \par Description
    Light_Lightness client calls the registered callback to indicate events occurred to the application.

    \param [in] handle        Model Handle.
    \param [in] opcode        Opcode.
    \param [in] data_param    Data associated with the event if any or NULL.
    \param [in] data_len      Size of the event data. 0 if event data is NULL.
*/
API_RESULT appl_light_lightness_client_cb
(
    /* IN */ MS_ACCESS_MODEL_HANDLE* handle,
    /* IN */ UINT32                   opcode,
    /* IN */ UCHAR*                   data_param,
    /* IN */ UINT16                   data_len
);

#endif /*_H_APPL_LIGHT_LIGHTNESS_CLIENT_ */
