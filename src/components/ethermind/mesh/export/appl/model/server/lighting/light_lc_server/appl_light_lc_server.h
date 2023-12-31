/**
    \file appl_light_lc_server.h
*/

/*
    Copyright (C) 2017. Mindtree Ltd.
    All rights reserved.
*/

#ifndef _H_APPL_LIGHT_LC_SERVER_
#define _H_APPL_LIGHT_LC_SERVER_


/* --------------------------------------------- Header File Inclusion */
#include "MS_light_lc_api.h"
#include "appl_main.h"


/* --------------------------------------------- Global Definitions */


/* --------------------------------------------- Data Types/ Structures */


/* --------------------------------------------- Function */
/* light_lc server application entry point */
void main_light_lc_server_operations(/* IN */ UINT8 have_menu);

/* Get Model Handle */
void appl_light_lc_server_get_model_handle(void);

/* Set Publish Address */
void appl_light_lc_server_set_publish_address(void);

/* Set Default Transition timeout in ms */
API_RESULT appl_model_light_lc_server_set_default_trans_timeout_in_ms(/* IN */ UINT32 time_in_ms);

/**
    \brief Server Application Asynchronous Notification Callback.

    \par Description
    Light_Lc server calls the registered callback to indicate events occurred to the application.

    \param [in] ctx           Context of message received for a specific model instance.
    \param [in] msg_raw       Uninterpreted/raw received message.
    \param [in] req_type      Requested message type.
    \param [in] state_params  Model specific state parameters.
    \param [in] ext_params    Additional parameters.
*/
API_RESULT appl_light_lc_server_cb
(
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_CONTEXT*     ctx,
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_RAW*         msg_raw,
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_T*           req_type,
    /* IN */ MS_ACCESS_MODEL_STATE_PARAMS*        state_params,
    /* IN */ MS_ACCESS_MODEL_EXT_PARAMS*          ext_params
);

/**
    \brief Server Application Asynchronous Notification Callback.

    \par Description
    Generic_Onoff server calls the registered callback to indicate events occurred to the application.

    \param [in] ctx           Context of message received for a specific model instance.
    \param [in] msg_raw       Uninterpreted/raw received message.
    \param [in] req_type      Requested message type.
    \param [in] state_params  Model specific state parameters.
    \param [in] ext_params    Additional parameters.
*/
API_RESULT appl_generic_onoff_server_cb
(
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_CONTEXT*     ctx,
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_RAW*         msg_raw,
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_T*           req_type,
    /* IN */ MS_ACCESS_MODEL_STATE_PARAMS*        state_params,
    /* IN */ MS_ACCESS_MODEL_EXT_PARAMS*          ext_params
);

/**
    \brief Server Application Asynchronous Notification Callback.

    \par Description
    Light_Lightness server calls the registered callback to indicate events occurred to the application.

    \param [in] ctx           Context of message received for a specific model instance.
    \param [in] msg_raw       Uninterpreted/raw received message.
    \param [in] req_type      Requested message type.
    \param [in] state_params  Model specific state parameters.
    \param [in] ext_params    Additional parameters.
*/
API_RESULT appl_light_lightness_server_cb
(
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_CONTEXT*     ctx,
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_RAW*         msg_raw,
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_T*           req_type,
    /* IN */ MS_ACCESS_MODEL_STATE_PARAMS*        state_params,
    /* IN */ MS_ACCESS_MODEL_EXT_PARAMS*          ext_params
);

/**
    \brief Server Application Asynchronous Notification Callback.

    \par Description
    Scene server calls the registered callback to indicate events occurred to the application.

    \param [in] ctx           Context of message received for a specific model instance.
    \param [in] msg_raw       Uninterpreted/raw received message.
    \param [in] req_type      Requested message type.
    \param [in] state_params  Model specific state parameters.
    \param [in] ext_params    Additional parameters.

    TODO: Update
*/
void* appl_scene_server_cb
(
    /* IN */ MS_ACCESS_MODEL_HANDLE*              handle,
    /* IN */ UINT8                                event_type,
    /* IN */ void*                                event_param,
    /* IN */ UINT16                               event_length,
    /* IN */ void*                                context
);

#endif /*_H_APPL_LIGHT_LC_SERVER_ */
