/**
    \file cli_generic_power_onoff_client.h

    \brief This file defines the Mesh Generic Power Onoff Model Application Interface
    - includes Data Structures and Methods for Client.
*/

/*
    Copyright (C) 2018. Mindtree Ltd.
    All rights reserved.
*/

#ifndef _H_CLI_GENERIC_POWER_ONOFF_CLIENT_
#define _H_CLI_GENERIC_POWER_ONOFF_CLIENT_


/* --------------------------------------------- Header File Inclusion */
#include "MS_generic_power_onoff_api.h"
#include "cli_main.h"


/* --------------------------------------------- Global Definitions */


/* --------------------------------------------- Data Types/ Structures */


/* --------------------------------------------- Function */
/* generic_power_onoff client CLI entry point */
CLI_CMD_HANDLER_DECL(cli_modelc_generic_power_onoff);

/* generic_power_onoff client CLI entry point */
CLI_CMD_HANDLER_DECL(cli_modelc_generic_power_onoff_setup);

/* Send Generic Onpowerup Get */
CLI_CMD_HANDLER_DECL(cli_modelc_generic_onpowerup_get);

/* Send Generic Onpowerup Set */
CLI_CMD_HANDLER_DECL(cli_modelc_generic_onpowerup_set);

/* Send Generic Onpowerup Set Unacknowledged */
CLI_CMD_HANDLER_DECL(cli_modelc_generic_onpowerup_set_unacknowledged);

/* Get Model Handle */
CLI_CMD_HANDLER_DECL(cli_modelc_generic_power_onoff_get_model_handle);

/* Set Publish Address */
CLI_CMD_HANDLER_DECL(cli_modelc_generic_power_onoff_set_publish_address);

/**
    \brief Client Application Asynchronous Notification Callback.

    \par Description
    Generic_Power_Onoff client calls the registered callback to indicate events occurred to the application.

    \param [in] handle        Model Handle.
    \param [in] opcode        Opcode.
    \param [in] data_param    Data associated with the event if any or NULL.
    \param [in] data_len      Size of the event data. 0 if event data is NULL.
*/
API_RESULT cli_generic_power_onoff_client_cb
(
    /* IN */ MS_ACCESS_MODEL_HANDLE* handle,
    /* IN */ UINT32                   opcode,
    /* IN */ UCHAR*                   data_param,
    /* IN */ UINT16                   data_len
);

#endif /*_H_CLI_GENERIC_POWER_ONOFF_CLIENT_ */
