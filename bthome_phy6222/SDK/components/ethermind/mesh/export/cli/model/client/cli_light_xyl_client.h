/**
    \file cli_light_xyl_client.h

    \brief This file defines the Mesh Light Xyl Model Application Interface
    - includes Data Structures and Methods for Client.
*/

/*
    Copyright (C) 2018. Mindtree Ltd.
    All rights reserved.
*/

#ifndef _H_CLI_LIGHT_XYL_CLIENT_
#define _H_CLI_LIGHT_XYL_CLIENT_


/* --------------------------------------------- Header File Inclusion */
#include "MS_light_xyl_api.h"
#include "cli_main.h"


/* --------------------------------------------- Global Definitions */


/* --------------------------------------------- Data Types/ Structures */


/* --------------------------------------------- Function */
/* light_xyl client CLI entry point */
CLI_CMD_HANDLER_DECL(cli_modelc_light_xyl);

/* light_xyl client CLI entry point */
CLI_CMD_HANDLER_DECL(cli_modelc_light_xyl_setup);

/* Send Light Xyl Default Get */
CLI_CMD_HANDLER_DECL(cli_modelc_light_xyl_default_get);

/* Send Light Xyl Default Set */
CLI_CMD_HANDLER_DECL(cli_modelc_light_xyl_default_set);

/* Send Light Xyl Default Set Unacknowledged */
CLI_CMD_HANDLER_DECL(cli_modelc_light_xyl_default_set_unacknowledged);

/* Send Light Xyl Get */
CLI_CMD_HANDLER_DECL(cli_modelc_light_xyl_get);

/* Send Light Xyl Range Get */
CLI_CMD_HANDLER_DECL(cli_modelc_light_xyl_range_get);

/* Send Light Xyl Range Set */
CLI_CMD_HANDLER_DECL(cli_modelc_light_xyl_range_set);

/* Send Light Xyl Range Set Unacknowledged */
CLI_CMD_HANDLER_DECL(cli_modelc_light_xyl_range_set_unacknowledged);

/* Send Light Xyl Set */
CLI_CMD_HANDLER_DECL(cli_modelc_light_xyl_set);

/* Send Light Xyl Set Unacknowledged */
CLI_CMD_HANDLER_DECL(cli_modelc_light_xyl_set_unacknowledged);

/* Send Light Xyl Target Get */
CLI_CMD_HANDLER_DECL(cli_modelc_light_xyl_target_get);

/* Get Model Handle */
CLI_CMD_HANDLER_DECL(cli_modelc_light_xyl_get_model_handle);

/* Set Publish Address */
CLI_CMD_HANDLER_DECL(cli_modelc_light_xyl_set_publish_address);

/**
    \brief Client Application Asynchronous Notification Callback.

    \par Description
    Light_Xyl client calls the registered callback to indicate events occurred to the application.

    \param [in] handle        Model Handle.
    \param [in] opcode        Opcode.
    \param [in] data_param    Data associated with the event if any or NULL.
    \param [in] data_len      Size of the event data. 0 if event data is NULL.
*/
API_RESULT cli_light_xyl_client_cb
(
    /* IN */ MS_ACCESS_MODEL_HANDLE* handle,
    /* IN */ UINT32                   opcode,
    /* IN */ UCHAR*                   data_param,
    /* IN */ UINT16                   data_len
);

#endif /*_H_CLI_LIGHT_XYL_CLIENT_ */
