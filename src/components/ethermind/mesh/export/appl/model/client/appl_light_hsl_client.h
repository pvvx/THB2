/**
    \file appl_light_hsl_client.h

    \brief This file defines the Mesh Light Hsl Model Application Interface
    - includes Data Structures and Methods for Client.
*/

/*
    Copyright (C) 2018. Mindtree Ltd.
    All rights reserved.
*/

#ifndef _H_APPL_LIGHT_HSL_CLIENT_
#define _H_APPL_LIGHT_HSL_CLIENT_


/* --------------------------------------------- Header File Inclusion */
#include "MS_light_hsl_api.h"
#include "appl_main.h"


/* --------------------------------------------- Global Definitions */


/* --------------------------------------------- Data Types/ Structures */


/* --------------------------------------------- Function */
/* light_hsl client application entry point */
void main_light_hsl_client_operations(void);

/* Send Light Hsl Default Get */
void appl_send_light_hsl_default_get(void);

/* Send Light Hsl Default Set */
void appl_send_light_hsl_default_set(void);

/* Send Light Hsl Default Set Unacknowledged */
void appl_send_light_hsl_default_set_unacknowledged(void);

/* Send Light Hsl Get */
void appl_send_light_hsl_get(void);

/* Send Light Hsl Hue Get */
void appl_send_light_hsl_hue_get(void);

/* Send Light Hsl Hue Set */
void appl_send_light_hsl_hue_set(void);

/* Send Light Hsl Hue Set Unacknowledged */
void appl_send_light_hsl_hue_set_unacknowledged(void);

/* Send Light Hsl Range Get */
void appl_send_light_hsl_range_get(void);

/* Send Light Hsl Range Set */
void appl_send_light_hsl_range_set(void);

/* Send Light Hsl Range Set Unacknowledged */
void appl_send_light_hsl_range_set_unacknowledged(void);

/* Send Light Hsl Saturation Get */
void appl_send_light_hsl_saturation_get(void);

/* Send Light Hsl Saturation Set */
void appl_send_light_hsl_saturation_set(void);

/* Send Light Hsl Saturation Set Unacknowledged */
void appl_send_light_hsl_saturation_set_unacknowledged(void);

/* Send Light Hsl Set */
void appl_send_light_hsl_set(void);

/* Send Light Hsl Set Unacknowledged */
void appl_send_light_hsl_set_unacknowledged(void);

/* Send Light Hsl Target Get */
void appl_send_light_hsl_target_get(void);

/* Get Model Handle */
void appl_light_hsl_client_get_model_handle(void);

/* Set Publish Address */
void appl_light_hsl_client_set_publish_address(void);

/**
    \brief Client Application Asynchronous Notification Callback.

    \par Description
    Light_Hsl client calls the registered callback to indicate events occurred to the application.

    \param [in] handle        Model Handle.
    \param [in] opcode        Opcode.
    \param [in] data_param    Data associated with the event if any or NULL.
    \param [in] data_len      Size of the event data. 0 if event data is NULL.
*/
API_RESULT appl_light_hsl_client_cb
(
    /* IN */ MS_ACCESS_MODEL_HANDLE* handle,
    /* IN */ UINT32                   opcode,
    /* IN */ UCHAR*                   data_param,
    /* IN */ UINT16                   data_len
);

#endif /*_H_APPL_LIGHT_HSL_CLIENT_ */
