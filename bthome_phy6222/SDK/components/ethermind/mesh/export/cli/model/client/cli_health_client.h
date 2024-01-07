/**
    \file cli_health_client.h

    \brief This file defines the Mesh Health Model Application Interface
    - includes Data Structures and Methods for Client.
*/

/*
    Copyright (C) 2018. Mindtree Ltd.
    All rights reserved.
*/

#ifndef _H_CLI_HEALTH_CLIENT_
#define _H_CLI_HEALTH_CLIENT_


/* --------------------------------------------- Header File Inclusion */
#include "MS_health_client_api.h"
#include "cli_main.h"


/* --------------------------------------------- Global Definitions */


/* --------------------------------------------- Data Types/ Structures */


/* --------------------------------------------- Function */
/* health client CLI entry point */
CLI_CMD_HANDLER_DECL(cli_modelc_health);

/* health client CLI entry point */
CLI_CMD_HANDLER_DECL(cli_modelc_health_setup);

/* Send Health Attention Get */
CLI_CMD_HANDLER_DECL(cli_modelc_health_attention_get);

/* Send Health Attention Set */
CLI_CMD_HANDLER_DECL(cli_modelc_health_attention_set);

/* Send Health Attention Set Unacknowledged */
CLI_CMD_HANDLER_DECL(cli_modelc_health_attention_set_unacknowledged);

/* Send Health Fault Clear */
CLI_CMD_HANDLER_DECL(cli_modelc_health_fault_clear);

/* Send Health Fault Clear Unacknowledged */
CLI_CMD_HANDLER_DECL(cli_modelc_health_fault_clear_unacknowledged);

/* Send Health Fault Get */
CLI_CMD_HANDLER_DECL(cli_modelc_health_fault_get);

/* Send Health Fault Test */
CLI_CMD_HANDLER_DECL(cli_modelc_health_fault_test);

/* Send Health Fault Test Unacknowledged */
CLI_CMD_HANDLER_DECL(cli_modelc_health_fault_test_unacknowledged);

/* Send Health Period Get */
CLI_CMD_HANDLER_DECL(cli_modelc_health_period_get);

/* Send Health Period Set */
CLI_CMD_HANDLER_DECL(cli_modelc_health_period_set);

/* Send Health Period Set Unacknowledged */
CLI_CMD_HANDLER_DECL(cli_modelc_health_period_set_unacknowledged);

/* Get Model Handle */
CLI_CMD_HANDLER_DECL(cli_modelc_health_get_model_handle);

/* Set Publish Address */
CLI_CMD_HANDLER_DECL(cli_modelc_health_set_publish_address);

/**
    \brief Client Application Asynchronous Notification Callback.

    \par Description
    Health client calls the registered callback to indicate events occurred to the application.

    \param [in] handle        Model Handle.
    \param [in] opcode        Opcode.
    \param [in] data_param    Data associated with the event if any or NULL.
    \param [in] data_len      Size of the event data. 0 if event data is NULL.
*/
API_RESULT cli_health_client_cb
(
    /* IN */ MS_ACCESS_MODEL_HANDLE* handle,
    /* IN */ UINT32                   opcode,
    /* IN */ UCHAR*                   data_param,
    /* IN */ UINT16                   data_len
);

#endif /*_H_CLI_HEALTH_CLIENT_ */
