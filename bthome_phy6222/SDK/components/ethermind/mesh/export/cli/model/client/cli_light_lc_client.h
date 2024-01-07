/**
    \file cli_light_lc_client.h

    \brief This file defines the Mesh Light Lc Model Application Interface
    - includes Data Structures and Methods for Client.
*/

/*
    Copyright (C) 2018. Mindtree Ltd.
    All rights reserved.
*/

#ifndef _H_CLI_LIGHT_LC_CLIENT_
#define _H_CLI_LIGHT_LC_CLIENT_


/* --------------------------------------------- Header File Inclusion */
#include "MS_light_lc_api.h"
#include "cli_main.h"


/* --------------------------------------------- Global Definitions */


/* --------------------------------------------- Data Types/ Structures */


/* --------------------------------------------- Function */
/* light_lc client CLI entry point */
CLI_CMD_HANDLER_DECL(cli_modelc_light_lc);

/* light_lc client CLI entry point */
CLI_CMD_HANDLER_DECL(cli_modelc_light_lc_setup);

/* Send Light Lc Light Onoff Get */
CLI_CMD_HANDLER_DECL(cli_modelc_light_lc_light_onoff_get);

/* Send Light Lc Light Onoff Set */
CLI_CMD_HANDLER_DECL(cli_modelc_light_lc_light_onoff_set);

/* Send Light Lc Light Onoff Set Unacknowledged */
CLI_CMD_HANDLER_DECL(cli_modelc_light_lc_light_onoff_set_unacknowledged);

/* Send Light Lc Mode Get */
CLI_CMD_HANDLER_DECL(cli_modelc_light_lc_mode_get);

/* Send Light Lc Mode Set */
CLI_CMD_HANDLER_DECL(cli_modelc_light_lc_mode_set);

/* Send Light Lc Mode Set Unacknowledged */
CLI_CMD_HANDLER_DECL(cli_modelc_light_lc_mode_set_unacknowledged);

/* Send Light Lc Om Get */
CLI_CMD_HANDLER_DECL(cli_modelc_light_lc_om_get);

/* Send Light Lc Om Set */
CLI_CMD_HANDLER_DECL(cli_modelc_light_lc_om_set);

/* Send Light Lc Om Set Unacknowledged */
CLI_CMD_HANDLER_DECL(cli_modelc_light_lc_om_set_unacknowledged);

/* Send Light Lc Property Get */
CLI_CMD_HANDLER_DECL(cli_modelc_light_lc_property_get);

/* Send Light Lc Property Set */
CLI_CMD_HANDLER_DECL(cli_modelc_light_lc_property_set);

/* Send Light Lc Property Set Unacknowledged */
CLI_CMD_HANDLER_DECL(cli_modelc_light_lc_property_set_unacknowledged);

/* Get Model Handle */
CLI_CMD_HANDLER_DECL(cli_modelc_light_lc_get_model_handle);

/* Set Publish Address */
CLI_CMD_HANDLER_DECL(cli_modelc_light_lc_set_publish_address);

/**
    \brief Client Application Asynchronous Notification Callback.

    \par Description
    Light_Lc client calls the registered callback to indicate events occurred to the application.

    \param [in] handle        Model Handle.
    \param [in] opcode        Opcode.
    \param [in] data_param    Data associated with the event if any or NULL.
    \param [in] data_len      Size of the event data. 0 if event data is NULL.
*/
API_RESULT cli_light_lc_client_cb
(
    /* IN */ MS_ACCESS_MODEL_HANDLE* handle,
    /* IN */ UINT32                   opcode,
    /* IN */ UCHAR*                   data_param,
    /* IN */ UINT16                   data_len
);

#endif /*_H_CLI_LIGHT_LC_CLIENT_ */
