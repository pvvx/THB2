/**
    \file appl_scene_client.h

    \brief This file defines the Mesh Scene Model Application Interface
    - includes Data Structures and Methods for Client.
*/

/*
    Copyright (C) 2018. Mindtree Ltd.
    All rights reserved.
*/

#ifndef _H_APPL_SCENE_CLIENT_
#define _H_APPL_SCENE_CLIENT_


/* --------------------------------------------- Header File Inclusion */
#include "MS_scene_api.h"
#include "appl_main.h"


/* --------------------------------------------- Global Definitions */


/* --------------------------------------------- Data Types/ Structures */


/* --------------------------------------------- Function */
/* scene client application entry point */
void main_scene_client_operations(void);

/* Send Scene Delete */
void appl_send_scene_delete(void);

/* Send Scene Delete Unacknowledged */
void appl_send_scene_delete_unacknowledged(void);

/* Send Scene Get */
void appl_send_scene_get(void);

/* Send Scene Recall */
void appl_send_scene_recall(void);

/* Send Scene Recall Unacknowledged */
void appl_send_scene_recall_unacknowledged(void);

/* Send Scene Register Get */
void appl_send_scene_register_get(void);

/* Send Scene Store */
void appl_send_scene_store(void);

/* Send Scene Store Unacknowledged */
void appl_send_scene_store_unacknowledged(void);

/* Get Model Handle */
void appl_scene_client_get_model_handle(void);

/* Set Publish Address */
void appl_scene_client_set_publish_address(void);

/**
    \brief Client Application Asynchronous Notification Callback.

    \par Description
    Scene client calls the registered callback to indicate events occurred to the application.

    \param [in] handle        Model Handle.
    \param [in] opcode        Opcode.
    \param [in] data_param    Data associated with the event if any or NULL.
    \param [in] data_len      Size of the event data. 0 if event data is NULL.
*/
API_RESULT appl_scene_client_cb
(
    /* IN */ MS_ACCESS_MODEL_HANDLE* handle,
    /* IN */ UINT32                   opcode,
    /* IN */ UCHAR*                   data_param,
    /* IN */ UINT16                   data_len
);

#endif /*_H_APPL_SCENE_CLIENT_ */
