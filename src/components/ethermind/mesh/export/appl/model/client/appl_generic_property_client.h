/**
    \file appl_generic_property_client.h

    \brief This file defines the Mesh Generic Property Model Application Interface
    - includes Data Structures and Methods for Client.
*/

/*
    Copyright (C) 2018. Mindtree Ltd.
    All rights reserved.
*/

#ifndef _H_APPL_GENERIC_PROPERTY_CLIENT_
#define _H_APPL_GENERIC_PROPERTY_CLIENT_


/* --------------------------------------------- Header File Inclusion */
#include "MS_generic_property_api.h"
#include "appl_main.h"


/* --------------------------------------------- Global Definitions */


/* --------------------------------------------- Data Types/ Structures */


/* --------------------------------------------- Function */
/* generic_property client application entry point */
void main_generic_property_client_operations(void);

/* Send Generic Admin Properties Get */
void appl_send_generic_admin_properties_get(void);

/* Send Generic Admin Property Get */
void appl_send_generic_admin_property_get(void);

/* Send Generic Admin Property Set */
void appl_send_generic_admin_property_set(void);

/* Send Generic Admin Property Set Unacknowledged */
void appl_send_generic_admin_property_set_unacknowledged(void);

/* Send Generic Client Properties Get */
void appl_send_generic_client_properties_get(void);

/* Send Generic Manufacturer Properties Get */
void appl_send_generic_manufacturer_properties_get(void);

/* Send Generic Manufacturer Property Get */
void appl_send_generic_manufacturer_property_get(void);

/* Send Generic Manufacturer Property Set */
void appl_send_generic_manufacturer_property_set(void);

/* Send Generic Manufacturer Property Set Unacknowledged */
void appl_send_generic_manufacturer_property_set_unacknowledged(void);

/* Send Generic User Properties Get */
void appl_send_generic_user_properties_get(void);

/* Send Generic User Property Get */
void appl_send_generic_user_property_get(void);

/* Send Generic User Property Set */
void appl_send_generic_user_property_set(void);

/* Send Generic User Property Set Unacknowledged */
void appl_send_generic_user_property_set_unacknowledged(void);

/* Get Model Handle */
void appl_generic_property_client_get_model_handle(void);

/* Set Publish Address */
void appl_generic_property_client_set_publish_address(void);

/**
    \brief Client Application Asynchronous Notification Callback.

    \par Description
    Generic_Property client calls the registered callback to indicate events occurred to the application.

    \param [in] handle        Model Handle.
    \param [in] opcode        Opcode.
    \param [in] data_param    Data associated with the event if any or NULL.
    \param [in] data_len      Size of the event data. 0 if event data is NULL.
*/
API_RESULT appl_generic_property_client_cb
(
    /* IN */ MS_ACCESS_MODEL_HANDLE* handle,
    /* IN */ UINT32                   opcode,
    /* IN */ UCHAR*                   data_param,
    /* IN */ UINT16                   data_len
);

#endif /*_H_APPL_GENERIC_PROPERTY_CLIENT_ */
