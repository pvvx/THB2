/**
    \file appl_generic_onoff_server.h
*/

/*
    Copyright (C) 2017. Mindtree Ltd.
    All rights reserved.
*/

#ifndef _H_VENDORMODEL_SERVER_
#define _H_VENDORMODEL_SERVER_


/* --------------------------------------------- Header File Inclusion */
#include "vendormodel_common.h"
#include "MS_access_api.h"
#include "access_extern.h"




/* --------------------------------------------- Global Definitions */


/* --------------------------------------------- Data Types/ Structures */
/**
    Vendor Example Server application Asynchronous Notification Callback.

    Vendor Example Server calls the registered callback to indicate events occurred to the
    application.

    \param [in] ctx           Context of the message received for a specific model instance.
    \param [in] msg_raw       Uninterpreted/raw received message.
    \param [in] req_type      Requested message type.
    \param [in] state_params  Model specific state parameters.
    \param [in] ext_params    Additional parameters.
*/
typedef API_RESULT (* MS_VENDORMODEL_SERVER_CB)
(
    MS_ACCESS_MODEL_REQ_MSG_CONTEXT*      ctx,
    MS_ACCESS_MODEL_REQ_MSG_RAW*          msg_raw,
    MS_ACCESS_MODEL_REQ_MSG_T*            req_type,
    MS_ACCESS_VENDORMODEL_STATE_PARAMS* state_params,
    MS_ACCESS_MODEL_EXT_PARAMS*           ext_params

) DECL_REENTRANT;

/* ----------------------------------------- Functions */
/**
    \brief API to send reply or to update state change

    \par Description
    This is to send reply for a request or to inform change in state.

    \param [in] ctx                     Context of the message.
    \param [in] current_state_params    Model specific current state parameters.
    \param [in] target_state_params     Model specific target state parameters (NULL: to be ignored).
    \param [in] remaining_time          Time from current state to target state (0: to be ignored).
    \param [in] ext_params              Additional parameters (NULL: to be ignored).

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_vendormodel_server_state_update
(
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_CONTEXT*      ctx,
    /* IN */ MS_ACCESS_VENDORMODEL_STATE_PARAMS*     current_state_params,
    /* IN */ MS_ACCESS_MODEL_STATE_PARAMS*         target_state_params,
    /* IN */ UINT16                              remaining_time,
    /* IN */ MS_ACCESS_MODEL_EXT_PARAMS*           ext_params,
    /* IN */ UINT16                              data_length
);




/* --------------------------------------------- Function */
API_RESULT MS_vendormodel_server_init
(
    /* IN */    MS_ACCESS_ELEMENT_HANDLE    element_handle,
    /* INOUT */ MS_ACCESS_MODEL_HANDLE*       model_handle,
    /* IN */    MS_VENDORMODEL_SERVER_CB      UI_cb
);


#endif /*_H_APPL_GENERIC_ONOFF_SERVER_ */
