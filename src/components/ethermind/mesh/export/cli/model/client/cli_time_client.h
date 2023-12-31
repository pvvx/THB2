/**
    \file cli_time_client.h

    \brief This file defines the Mesh Time Model Application Interface
    - includes Data Structures and Methods for Client.
*/

/*
    Copyright (C) 2018. Mindtree Ltd.
    All rights reserved.
*/

#ifndef _H_CLI_TIME_CLIENT_
#define _H_CLI_TIME_CLIENT_


/* --------------------------------------------- Header File Inclusion */
#include "MS_time_api.h"
#include "cli_main.h"


/* --------------------------------------------- Global Definitions */


/* --------------------------------------------- Data Types/ Structures */


/* --------------------------------------------- Function */
/* time client CLI entry point */
CLI_CMD_HANDLER_DECL(cli_modelc_time);

/* time client CLI entry point */
CLI_CMD_HANDLER_DECL(cli_modelc_time_setup);

/* Send Tai Utc Delta Get */
CLI_CMD_HANDLER_DECL(cli_modelc_tai_utc_delta_get);

/* Send Tai Utc Delta Set */
CLI_CMD_HANDLER_DECL(cli_modelc_tai_utc_delta_set);

/* Send Time Get */
CLI_CMD_HANDLER_DECL(cli_modelc_time_get);

/* Send Time Role Get */
CLI_CMD_HANDLER_DECL(cli_modelc_time_role_get);

/* Send Time Role Set */
CLI_CMD_HANDLER_DECL(cli_modelc_time_role_set);

/* Send Time Set */
CLI_CMD_HANDLER_DECL(cli_modelc_time_set);

/* Send Time Zone Get */
CLI_CMD_HANDLER_DECL(cli_modelc_time_zone_get);

/* Send Time Zone Set */
CLI_CMD_HANDLER_DECL(cli_modelc_time_zone_set);

/* Get Model Handle */
CLI_CMD_HANDLER_DECL(cli_modelc_time_get_model_handle);

/* Set Publish Address */
CLI_CMD_HANDLER_DECL(cli_modelc_time_set_publish_address);

/**
    \brief Client Application Asynchronous Notification Callback.

    \par Description
    Time client calls the registered callback to indicate events occurred to the application.

    \param [in] handle        Model Handle.
    \param [in] opcode        Opcode.
    \param [in] data_param    Data associated with the event if any or NULL.
    \param [in] data_len      Size of the event data. 0 if event data is NULL.
*/
API_RESULT cli_time_client_cb
(
    /* IN */ MS_ACCESS_MODEL_HANDLE* handle,
    /* IN */ UINT32                   opcode,
    /* IN */ UCHAR*                   data_param,
    /* IN */ UINT16                   data_len
);

#endif /*_H_CLI_TIME_CLIENT_ */
