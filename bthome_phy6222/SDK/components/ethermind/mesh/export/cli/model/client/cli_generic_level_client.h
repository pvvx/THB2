/**
    \file cli_generic_level_client.h

    \brief This file defines the Mesh Generic Level Model Application Interface
    - includes Data Structures and Methods for Client.
*/

/*
    Copyright (C) 2018. Mindtree Ltd.
    All rights reserved.
*/

#ifndef _H_CLI_GENERIC_LEVEL_CLIENT_
#define _H_CLI_GENERIC_LEVEL_CLIENT_


/* --------------------------------------------- Header File Inclusion */
#include "MS_generic_level_api.h"
#include "cli_main.h"


/* --------------------------------------------- Global Definitions */


/* --------------------------------------------- Data Types/ Structures */


/* --------------------------------------------- Function */
/* generic_level client CLI entry point */
CLI_CMD_HANDLER_DECL(cli_modelc_generic_level);

/* generic_level client CLI entry point */
CLI_CMD_HANDLER_DECL(cli_modelc_generic_level_setup);

/* Send Generic Delta Set */
CLI_CMD_HANDLER_DECL(cli_modelc_generic_delta_set);

/* Send Generic Delta Set Unacknowledged */
CLI_CMD_HANDLER_DECL(cli_modelc_generic_delta_set_unacknowledged);

/* Send Generic Level Get */
CLI_CMD_HANDLER_DECL(cli_modelc_generic_level_get);

/* Send Generic Level Set */
CLI_CMD_HANDLER_DECL(cli_modelc_generic_level_set);

/* Send Generic Level Set Unacknowledged */
CLI_CMD_HANDLER_DECL(cli_modelc_generic_level_set_unacknowledged);

/* Send Generic Move Set */
CLI_CMD_HANDLER_DECL(cli_modelc_generic_move_set);

/* Send Generic Move Set Unacknowledged */
CLI_CMD_HANDLER_DECL(cli_modelc_generic_move_set_unacknowledged);

/* Get Model Handle */
CLI_CMD_HANDLER_DECL(cli_modelc_generic_level_get_model_handle);

/* Set Publish Address */
CLI_CMD_HANDLER_DECL(cli_modelc_generic_level_set_publish_address);

/**
    \brief Client Application Asynchronous Notification Callback.

    \par Description
    Generic_Level client calls the registered callback to indicate events occurred to the application.

    \param [in] handle        Model Handle.
    \param [in] opcode        Opcode.
    \param [in] data_param    Data associated with the event if any or NULL.
    \param [in] data_len      Size of the event data. 0 if event data is NULL.
*/
API_RESULT cli_generic_level_client_cb
(
    /* IN */ MS_ACCESS_MODEL_HANDLE* handle,
    /* IN */ UINT32                   opcode,
    /* IN */ UCHAR*                   data_param,
    /* IN */ UINT16                   data_len
);

#endif /*_H_CLI_GENERIC_LEVEL_CLIENT_ */
