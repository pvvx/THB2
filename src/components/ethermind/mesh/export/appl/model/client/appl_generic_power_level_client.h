/**
    \file appl_generic_power_level_client.h

    \brief This file defines the Mesh Generic Power Level Model Application Interface
    - includes Data Structures and Methods for Client.
*/

/*
    Copyright (C) 2018. Mindtree Ltd.
    All rights reserved.
*/

#ifndef _H_APPL_GENERIC_POWER_LEVEL_CLIENT_
#define _H_APPL_GENERIC_POWER_LEVEL_CLIENT_


/* --------------------------------------------- Header File Inclusion */
#include "MS_generic_power_level_api.h"
#include "appl_main.h"


/* --------------------------------------------- Global Definitions */


/* --------------------------------------------- Data Types/ Structures */


/* --------------------------------------------- Function */
/* generic_power_level client application entry point */
void main_generic_power_level_client_operations(void);

/* Send Generic Power Default Get */
void appl_send_generic_power_default_get(void);

/* Send Generic Power Default Set */
void appl_send_generic_power_default_set(void);

/* Send Generic Power Default Set Unacknowledged */
void appl_send_generic_power_default_set_unacknowledged(void);

/* Send Generic Power Last Get */
void appl_send_generic_power_last_get(void);

/* Send Generic Power Level Get */
void appl_send_generic_power_level_get(void);

/* Send Generic Power Level Set */
void appl_send_generic_power_level_set(void);

/* Send Generic Power Level Set Unacknowledged */
void appl_send_generic_power_level_set_unacknowledged(void);

/* Send Generic Power Range Get */
void appl_send_generic_power_range_get(void);

/* Send Generic Power Range Set */
void appl_send_generic_power_range_set(void);

/* Send Generic Power Range Set Unacknowledged */
void appl_send_generic_power_range_set_unacknowledged(void);

/* Get Model Handle */
void appl_generic_power_level_client_get_model_handle(void);

/* Set Publish Address */
void appl_generic_power_level_client_set_publish_address(void);

/**
    \brief Client Application Asynchronous Notification Callback.

    \par Description
    Generic_Power_Level client calls the registered callback to indicate events occurred to the application.

    \param [in] handle        Model Handle.
    \param [in] opcode        Opcode.
    \param [in] data_param    Data associated with the event if any or NULL.
    \param [in] data_len      Size of the event data. 0 if event data is NULL.
*/
API_RESULT appl_generic_power_level_client_cb
(
    /* IN */ MS_ACCESS_MODEL_HANDLE* handle,
    /* IN */ UINT32                   opcode,
    /* IN */ UCHAR*                   data_param,
    /* IN */ UINT16                   data_len
);

#endif /*_H_APPL_GENERIC_POWER_LEVEL_CLIENT_ */
