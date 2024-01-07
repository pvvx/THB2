/**
    \file appl_time_server.h
*/

/*
    Copyright (C) 2017. Mindtree Ltd.
    All rights reserved.
*/

#ifndef _H_APPL_TIME_SERVER_
#define _H_APPL_TIME_SERVER_


/* --------------------------------------------- Header File Inclusion */
#include "MS_time_api.h"
#include "appl_main.h"


/* --------------------------------------------- Global Definitions */


/* --------------------------------------------- Data Types/ Structures */


/* --------------------------------------------- Function */
/* time server application entry point */
void main_time_server_operations(/* IN */ UINT8 have_menu);

/* Get Model Handle */
void appl_time_server_get_model_handle(void);

/* Set Publish Address */
void appl_time_server_set_publish_address(void);

/**
    \brief Server Application Asynchronous Notification Callback.

    \par Description
    Time server calls the registered callback to indicate events occurred to the application.

    \param [in] ctx           Context of message received for a specific model instance.
    \param [in] msg_raw       Uninterpreted/raw received message.
    \param [in] req_type      Requested message type.
    \param [in] state_params  Model specific state parameters.
    \param [in] ext_params    Additional parameters.
*/
API_RESULT appl_time_server_cb
(
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_CONTEXT*     ctx,
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_RAW*         msg_raw,
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_T*           req_type,
    /* IN */ MS_ACCESS_MODEL_STATE_PARAMS*        state_params,
    /* IN */ MS_ACCESS_MODEL_EXT_PARAMS*          ext_params
);

#endif /*_H_APPL_TIME_SERVER_ */
