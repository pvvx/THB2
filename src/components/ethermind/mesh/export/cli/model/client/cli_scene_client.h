/**
    \file cli_scene_client.h

    \brief This file defines the Mesh Scene Model Application Interface
    - includes Data Structures and Methods for Client.
*/

/*
    Copyright (C) 2018. Mindtree Ltd.
    All rights reserved.
*/

#ifndef _H_CLI_SCENE_CLIENT_
#define _H_CLI_SCENE_CLIENT_


/* --------------------------------------------- Header File Inclusion */
#include "MS_scene_api.h"
#include "cli_main.h"


/* --------------------------------------------- Global Definitions */


/* --------------------------------------------- Data Types/ Structures */


/* --------------------------------------------- Function */
/* scene client CLI entry point */
CLI_CMD_HANDLER_DECL(cli_modelc_scene);

/* scene client CLI entry point */
CLI_CMD_HANDLER_DECL(cli_modelc_scene_setup);

/* Send Scene Delete */
CLI_CMD_HANDLER_DECL(cli_modelc_scene_delete);

/* Send Scene Delete Unacknowledged */
CLI_CMD_HANDLER_DECL(cli_modelc_scene_delete_unacknowledged);

/* Send Scene Get */
CLI_CMD_HANDLER_DECL(cli_modelc_scene_get);

/* Send Scene Recall */
CLI_CMD_HANDLER_DECL(cli_modelc_scene_recall);

/* Send Scene Recall Unacknowledged */
CLI_CMD_HANDLER_DECL(cli_modelc_scene_recall_unacknowledged);

/* Send Scene Register Get */
CLI_CMD_HANDLER_DECL(cli_modelc_scene_register_get);

/* Send Scene Store */
CLI_CMD_HANDLER_DECL(cli_modelc_scene_store);

/* Send Scene Store Unacknowledged */
CLI_CMD_HANDLER_DECL(cli_modelc_scene_store_unacknowledged);

/* Get Model Handle */
CLI_CMD_HANDLER_DECL(cli_modelc_scene_get_model_handle);

/* Set Publish Address */
CLI_CMD_HANDLER_DECL(cli_modelc_scene_set_publish_address);

/**
    \brief Client Application Asynchronous Notification Callback.

    \par Description
    Scene client calls the registered callback to indicate events occurred to the application.

    \param [in] handle        Model Handle.
    \param [in] opcode        Opcode.
    \param [in] data_param    Data associated with the event if any or NULL.
    \param [in] data_len      Size of the event data. 0 if event data is NULL.
*/
API_RESULT cli_scene_client_cb
(
    /* IN */ MS_ACCESS_MODEL_HANDLE* handle,
    /* IN */ UINT32                   opcode,
    /* IN */ UCHAR*                   data_param,
    /* IN */ UINT16                   data_len
);

#endif /*_H_CLI_SCENE_CLIENT_ */
