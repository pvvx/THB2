/**
    \file appl_generic_onoff_client.h

    \brief This file defines the Mesh Generic Onoff Model Application Interface
    - includes Data Structures and Methods for Client.
*/

/*
    Copyright (C) 2018. Mindtree Ltd.
    All rights reserved.
*/

#ifndef _H_APPL_GENERIC_ONOFF_CLIENT_
#define _H_APPL_GENERIC_ONOFF_CLIENT_


/* --------------------------------------------- Header File Inclusion */
#include "MS_generic_onoff_api.h"
#include "appl_main.h"


/* --------------------------------------------- Global Definitions */


/* --------------------------------------------- Data Types/ Structures */


/* --------------------------------------------- Function */
/* generic_onoff client application entry point */
void main_generic_onoff_client_operations(/* IN */ UINT8 have_menu);

/* Send Generic Onoff Get */
void appl_send_generic_onoff_get(void);

/* Send Generic Onoff Set */
void appl_send_generic_onoff_set(void);

/* Send Generic Onoff Set Unacknowledged */
void appl_send_generic_onoff_set_unacknowledged(void);

/* Get Model Handle */
void appl_generic_onoff_client_get_model_handle(void);

/* Set Publish Address */
void appl_generic_onoff_client_set_publish_address(void);

/**
    \brief Client Application Asynchronous Notification Callback.

    \par Description
    Generic_Onoff client calls the registered callback to indicate events occurred to the application.

    \param [in] handle        Model Handle.
    \param [in] opcode        Opcode.
    \param [in] data_param    Data associated with the event if any or NULL.
    \param [in] data_len      Size of the event data. 0 if event data is NULL.
*/
API_RESULT appl_generic_onoff_client_cb
(
    /* IN */ MS_ACCESS_MODEL_HANDLE* handle,
    /* IN */ UINT32                   opcode,
    /* IN */ UCHAR*                   data_param,
    /* IN */ UINT16                   data_len
);

#endif /*_H_APPL_GENERIC_ONOFF_CLIENT_ */
