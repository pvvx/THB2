/**
    \file cli_generic_property_client.h

    \brief This file defines the Mesh Generic Property Model Application Interface
    - includes Data Structures and Methods for Client.
*/

/*
    Copyright (C) 2018. Mindtree Ltd.
    All rights reserved.
*/

#ifndef _H_CLI_GENERIC_PROPERTY_CLIENT_
#define _H_CLI_GENERIC_PROPERTY_CLIENT_


/* --------------------------------------------- Header File Inclusion */
#include "MS_generic_property_api.h"
#include "cli_main.h"


/* --------------------------------------------- Global Definitions */


/* --------------------------------------------- Data Types/ Structures */


/* --------------------------------------------- Function */
/* generic_property client CLI entry point */
CLI_CMD_HANDLER_DECL(cli_modelc_generic_property);

/* generic_property client CLI entry point */
CLI_CMD_HANDLER_DECL(cli_modelc_generic_property_setup);

/* Send Generic Admin Properties Get */
CLI_CMD_HANDLER_DECL(cli_modelc_generic_admin_properties_get);

/* Send Generic Admin Property Get */
CLI_CMD_HANDLER_DECL(cli_modelc_generic_admin_property_get);

/* Send Generic Admin Property Set */
CLI_CMD_HANDLER_DECL(cli_modelc_generic_admin_property_set);

/* Send Generic Admin Property Set Unacknowledged */
CLI_CMD_HANDLER_DECL(cli_modelc_generic_admin_property_set_unacknowledged);

/* Send Generic Client Properties Get */
CLI_CMD_HANDLER_DECL(cli_modelc_generic_client_properties_get);

/* Send Generic Manufacturer Properties Get */
CLI_CMD_HANDLER_DECL(cli_modelc_generic_manufacturer_properties_get);

/* Send Generic Manufacturer Property Get */
CLI_CMD_HANDLER_DECL(cli_modelc_generic_manufacturer_property_get);

/* Send Generic Manufacturer Property Set */
CLI_CMD_HANDLER_DECL(cli_modelc_generic_manufacturer_property_set);

/* Send Generic Manufacturer Property Set Unacknowledged */
CLI_CMD_HANDLER_DECL(cli_modelc_generic_manufacturer_property_set_unacknowledged);

/* Send Generic User Properties Get */
CLI_CMD_HANDLER_DECL(cli_modelc_generic_user_properties_get);

/* Send Generic User Property Get */
CLI_CMD_HANDLER_DECL(cli_modelc_generic_user_property_get);

/* Send Generic User Property Set */
CLI_CMD_HANDLER_DECL(cli_modelc_generic_user_property_set);

/* Send Generic User Property Set Unacknowledged */
CLI_CMD_HANDLER_DECL(cli_modelc_generic_user_property_set_unacknowledged);

/* Get Model Handle */
CLI_CMD_HANDLER_DECL(cli_modelc_generic_property_get_model_handle);

/* Set Publish Address */
CLI_CMD_HANDLER_DECL(cli_modelc_generic_property_set_publish_address);

/**
    \brief Client Application Asynchronous Notification Callback.

    \par Description
    Generic_Property client calls the registered callback to indicate events occurred to the application.

    \param [in] handle        Model Handle.
    \param [in] opcode        Opcode.
    \param [in] data_param    Data associated with the event if any or NULL.
    \param [in] data_len      Size of the event data. 0 if event data is NULL.
*/
API_RESULT cli_generic_property_client_cb
(
    /* IN */ MS_ACCESS_MODEL_HANDLE* handle,
    /* IN */ UINT32                   opcode,
    /* IN */ UCHAR*                   data_param,
    /* IN */ UINT16                   data_len
);

#endif /*_H_CLI_GENERIC_PROPERTY_CLIENT_ */
