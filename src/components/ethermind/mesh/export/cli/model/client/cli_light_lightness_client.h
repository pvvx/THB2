/**
    \file cli_light_lightness_client.h

    \brief This file defines the Mesh Light Lightness Model Application Interface
    - includes Data Structures and Methods for Client.
*/

/*
    Copyright (C) 2018. Mindtree Ltd.
    All rights reserved.
*/

#ifndef _H_CLI_LIGHT_LIGHTNESS_CLIENT_
#define _H_CLI_LIGHT_LIGHTNESS_CLIENT_


/* --------------------------------------------- Header File Inclusion */
#include "MS_light_lightness_api.h"
#include "cli_main.h"


/* --------------------------------------------- Global Definitions */


/* --------------------------------------------- Data Types/ Structures */


/* --------------------------------------------- Function */
/* light_lightness client CLI entry point */
CLI_CMD_HANDLER_DECL(cli_modelc_light_lightness);

/* light_lightness client CLI entry point */
CLI_CMD_HANDLER_DECL(cli_modelc_light_lightness_setup);

/* Send Light Lightness Default Get */
CLI_CMD_HANDLER_DECL(cli_modelc_light_lightness_default_get);

/* Send Light Lightness Default Set */
CLI_CMD_HANDLER_DECL(cli_modelc_light_lightness_default_set);

/* Send Light Lightness Default Set Unacknowledged */
CLI_CMD_HANDLER_DECL(cli_modelc_light_lightness_default_set_unacknowledged);

/* Send Light Lightness Get */
CLI_CMD_HANDLER_DECL(cli_modelc_light_lightness_get);

/* Send Light Lightness Last Get */
CLI_CMD_HANDLER_DECL(cli_modelc_light_lightness_last_get);

/* Send Light Lightness Linear Get */
CLI_CMD_HANDLER_DECL(cli_modelc_light_lightness_linear_get);

/* Send Light Lightness Linear Set */
CLI_CMD_HANDLER_DECL(cli_modelc_light_lightness_linear_set);

/* Send Light Lightness Linear Set Unacknowledged */
CLI_CMD_HANDLER_DECL(cli_modelc_light_lightness_linear_set_unacknowledged);

/* Send Light Lightness Range Get */
CLI_CMD_HANDLER_DECL(cli_modelc_light_lightness_range_get);

/* Send Light Lightness Range Set */
CLI_CMD_HANDLER_DECL(cli_modelc_light_lightness_range_set);

/* Send Light Lightness Range Set Unacknowledged */
CLI_CMD_HANDLER_DECL(cli_modelc_light_lightness_range_set_unacknowledged);

/* Send Light Lightness Set */
CLI_CMD_HANDLER_DECL(cli_modelc_light_lightness_set);

/* Send Light Lightness Set Unacknowledged */
CLI_CMD_HANDLER_DECL(cli_modelc_light_lightness_set_unacknowledged);

/* Get Model Handle */
CLI_CMD_HANDLER_DECL(cli_modelc_light_lightness_get_model_handle);

/* Set Publish Address */
CLI_CMD_HANDLER_DECL(cli_modelc_light_lightness_set_publish_address);

/**
    \brief Client Application Asynchronous Notification Callback.

    \par Description
    Light_Lightness client calls the registered callback to indicate events occurred to the application.

    \param [in] handle        Model Handle.
    \param [in] opcode        Opcode.
    \param [in] data_param    Data associated with the event if any or NULL.
    \param [in] data_len      Size of the event data. 0 if event data is NULL.
*/
API_RESULT cli_light_lightness_client_cb
(
    /* IN */ MS_ACCESS_MODEL_HANDLE* handle,
    /* IN */ UINT32                   opcode,
    /* IN */ UCHAR*                   data_param,
    /* IN */ UINT16                   data_len
);

#endif /*_H_CLI_LIGHT_LIGHTNESS_CLIENT_ */
