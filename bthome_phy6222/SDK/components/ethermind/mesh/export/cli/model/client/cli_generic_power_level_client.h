/**
    \file cli_generic_power_level_client.h

    \brief This file defines the Mesh Generic Power Level Model Application Interface
    - includes Data Structures and Methods for Client.
*/

/*
    Copyright (C) 2018. Mindtree Ltd.
    All rights reserved.
*/

#ifndef _H_CLI_GENERIC_POWER_LEVEL_CLIENT_
#define _H_CLI_GENERIC_POWER_LEVEL_CLIENT_


/* --------------------------------------------- Header File Inclusion */
#include "MS_generic_power_level_api.h"
#include "cli_main.h"


/* --------------------------------------------- Global Definitions */


/* --------------------------------------------- Data Types/ Structures */


/* --------------------------------------------- Function */
/* generic_power_level client CLI entry point */
CLI_CMD_HANDLER_DECL(cli_modelc_generic_power_level);

/* generic_power_level client CLI entry point */
CLI_CMD_HANDLER_DECL(cli_modelc_generic_power_level_setup);

/* Send Generic Power Default Get */
CLI_CMD_HANDLER_DECL(cli_modelc_generic_power_default_get);

/* Send Generic Power Default Set */
CLI_CMD_HANDLER_DECL(cli_modelc_generic_power_default_set);

/* Send Generic Power Default Set Unacknowledged */
CLI_CMD_HANDLER_DECL(cli_modelc_generic_power_default_set_unacknowledged);

/* Send Generic Power Last Get */
CLI_CMD_HANDLER_DECL(cli_modelc_generic_power_last_get);

/* Send Generic Power Level Get */
CLI_CMD_HANDLER_DECL(cli_modelc_generic_power_level_get);

/* Send Generic Power Level Set */
CLI_CMD_HANDLER_DECL(cli_modelc_generic_power_level_set);

/* Send Generic Power Level Set Unacknowledged */
CLI_CMD_HANDLER_DECL(cli_modelc_generic_power_level_set_unacknowledged);

/* Send Generic Power Range Get */
CLI_CMD_HANDLER_DECL(cli_modelc_generic_power_range_get);

/* Send Generic Power Range Set */
CLI_CMD_HANDLER_DECL(cli_modelc_generic_power_range_set);

/* Send Generic Power Range Set Unacknowledged */
CLI_CMD_HANDLER_DECL(cli_modelc_generic_power_range_set_unacknowledged);

/* Get Model Handle */
CLI_CMD_HANDLER_DECL(cli_modelc_generic_power_level_get_model_handle);

/* Set Publish Address */
CLI_CMD_HANDLER_DECL(cli_modelc_generic_power_level_set_publish_address);

/**
    \brief Client Application Asynchronous Notification Callback.

    \par Description
    Generic_Power_Level client calls the registered callback to indicate events occurred to the application.

    \param [in] handle        Model Handle.
    \param [in] opcode        Opcode.
    \param [in] data_param    Data associated with the event if any or NULL.
    \param [in] data_len      Size of the event data. 0 if event data is NULL.
*/
API_RESULT cli_generic_power_level_client_cb
(
    /* IN */ MS_ACCESS_MODEL_HANDLE* handle,
    /* IN */ UINT32                   opcode,
    /* IN */ UCHAR*                   data_param,
    /* IN */ UINT16                   data_len
);

#endif /*_H_CLI_GENERIC_POWER_LEVEL_CLIENT_ */
