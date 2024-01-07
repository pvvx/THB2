/**
    \file MS_generic_power_onoff_api.h

    \brief This file defines the Mesh Generic Power Onoff Model Application Interface
    - includes Data Structures and Methods for both Server and Client.
*/

/*
    Copyright (C) 2017. Mindtree Ltd.
    All rights reserved.
*/

#ifndef _H_MS_GENERIC_POWER_ONOFF_API_
#define _H_MS_GENERIC_POWER_ONOFF_API_


/* --------------------------------------------- Header File Inclusion */
#include "MS_access_api.h"


/* --------------------------------------------- Global Definitions */
/**
    \defgroup generic_power_onoff_module GENERIC_POWER_ONOFF (Mesh Generic Power Onoff Model)
    \{
    This section describes the interfaces & APIs offered by the EtherMind
    Mesh Generic OnOff Model (ONOFF) module to the Application.
*/



/* --------------------------------------------- Data Types/ Structures */
/**
    \defgroup generic_power_onoff_cb Application Callback
    \{
    This Section Describes the module Notification Callback interface offered
    to the application
*/

/**
    Generic Power Onoff Server application Asynchronous Notification Callback.

    Generic Power Onoff Server calls the registered callback to indicate events occurred to the
    application.

    \param [in] ctx           Context of the message received for a specific model instance.
    \param [in] msg_raw       Uninterpreted/raw received message.
    \param [in] req_type      Requested message type.
    \param [in] state_params  Model specific state parameters.
    \param [in] ext_params    Additional parameters.
*/
typedef API_RESULT (* MS_GENERIC_POWER_ONOFF_SERVER_CB)
(
    MS_ACCESS_MODEL_REQ_MSG_CONTEXT*     ctx,
    MS_ACCESS_MODEL_REQ_MSG_RAW*         msg_raw,
    MS_ACCESS_MODEL_REQ_MSG_T*           req_type,
    MS_ACCESS_MODEL_STATE_PARAMS*        state_params,
    MS_ACCESS_MODEL_EXT_PARAMS*          ext_params

) DECL_REENTRANT;

/**
    Generic Power Onoff Client application Asynchronous Notification Callback.

    Generic Power Onoff Client calls the registered callback to indicate events occurred to the
    application.

    \param handle        Model Handle.
    \param opcode        Opcode.
    \param data_param    Data associated with the event if any or NULL.
    \param data_len      Size of the event data. 0 if event data is NULL.
*/
typedef API_RESULT (* MS_GENERIC_POWER_ONOFF_CLIENT_CB)
(
    MS_ACCESS_MODEL_HANDLE* handle,
    UINT32                   opcode,
    UCHAR*                   data_param,
    UINT16                   data_len
) DECL_REENTRANT;

/**
    Generic Power Onoff Setup Server application Asynchronous Notification Callback.

    Generic Power Onoff Setup Server calls the registered callback to indicate events occurred to the
    application.

    \param [in] ctx           Context of the message received for a specific model instance.
    \param [in] msg_raw       Uninterpreted/raw received message.
    \param [in] req_type      Requested message type.
    \param [in] state_params  Model specific state parameters.
    \param [in] ext_params    Additional parameters.
*/
typedef API_RESULT (* MS_GENERIC_POWER_ONOFF_SETUP_SERVER_CB)
(
    MS_ACCESS_MODEL_REQ_MSG_CONTEXT*     ctx,
    MS_ACCESS_MODEL_REQ_MSG_RAW*         msg_raw,
    MS_ACCESS_MODEL_REQ_MSG_T*           req_type,
    MS_ACCESS_MODEL_STATE_PARAMS*        state_params,
    MS_ACCESS_MODEL_EXT_PARAMS*          ext_params

) DECL_REENTRANT;
/** \} */

/**
    \defgroup generic_power_onoff_structures Structures
    \{
*/

/**
    Generic OnPowerUp Set message parameters.
*/
typedef struct MS_generic_onpowerup_struct
{
    /**
        The Generic OnPowerUp state is an enumeration representing the behavior of an element when powered up.

        Value     | Description
        ----------|------------
        0x00      | Off. After being powered up, the element is in an off state.
        0x01      | Default. After being powered up, the element is in an On state and uses default state values.
        0x02      | Restore. If a transition was in progress when powered down, the element restores the target
                   state when powered up. Otherwise the element restores the state it was in when powered down.
        0x03-0xFF | Prohibited
    */
    UCHAR  onpowerup;

} MS_GENERIC_ONPOWERUP_STRUCT;

/** \} */



/* --------------------------------------------- Function */
/**
    \defgroup generic_power_onoff_api_defs API Definitions
    \{
    This section describes the EtherMind Mesh Generic Power Onoff Model APIs.
*/
/**
    \defgroup generic_power_onoff_ser_api_defs Generic Power Onoff Server API Definitions
    \{
    This section describes the Generic Power Onoff Server APIs.
*/

/**
    \brief API to initialize Generic_Power_Onoff Server model

    \par Description
    This is to initialize Generic_Power_Onoff Server model and to register with Acess layer.

    \param [in] element_handle
                Element identifier to be associated with the model instance.

    \param [in, out] model_handle
                     Model identifier associated with the model instance on successful initialization.
                     After power cycle of an already provisioned node, the model handle will have
                     valid value and the same will be reused for registration.

    \param [in] appl_cb    Application Callback to be used by the Generic_Power_Onoff Server.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_generic_power_onoff_server_init
(
    /* IN */    MS_ACCESS_ELEMENT_HANDLE    element_handle,
    /* INOUT */ MS_ACCESS_MODEL_HANDLE*     model_handle,
    /* IN */    MS_GENERIC_POWER_ONOFF_SERVER_CB appl_cb
);

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
API_RESULT MS_generic_power_onoff_server_state_update
(
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_CONTEXT*     ctx,
    /* IN */ MS_ACCESS_MODEL_STATE_PARAMS*        current_state_params,
    /* IN */ MS_ACCESS_MODEL_STATE_PARAMS*        target_state_params,
    /* IN */ UINT16                               remaining_time,
    /* IN */ MS_ACCESS_MODEL_EXT_PARAMS*          ext_params
);

/**
    \brief API to initialize Generic_Power_Onoff_Setup Server model

    \par Description
    This is to initialize Generic_Power_Onoff_Setup Server model and to register with Acess layer.

    \param [in] element_handle
                Element identifier to be associated with the model instance.

    \param [in, out] model_handle
                     Model identifier associated with the model instance on successful initialization.
                     After power cycle of an already provisioned node, the model handle will have
                     valid value and the same will be reused for registration.

    \param [in] appl_cb    Application Callback to be used by the Generic_Power_Onoff_Setup Server.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_generic_power_onoff_setup_server_init
(
    /* IN */    MS_ACCESS_ELEMENT_HANDLE    element_handle,
    /* INOUT */ MS_ACCESS_MODEL_HANDLE*     model_handle,
    /* IN */    MS_GENERIC_POWER_ONOFF_SETUP_SERVER_CB appl_cb
);

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
API_RESULT MS_generic_power_onoff_setup_server_state_update
(
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_CONTEXT*     ctx,
    /* IN */ MS_ACCESS_MODEL_STATE_PARAMS*        current_state_params,
    /* IN */ MS_ACCESS_MODEL_STATE_PARAMS*        target_state_params,
    /* IN */ UINT16                               remaining_time,
    /* IN */ MS_ACCESS_MODEL_EXT_PARAMS*          ext_params
);
/** \} */

/**
    \defgroup generic_power_onoff_cli_api_defs Generic Power Onoff Client API Definitions
    \{
    This section describes the Generic Power Onoff Client APIs.
*/

/**
    \brief API to initialize Generic_Power_Onoff Client model

    \par Description
    This is to initialize Generic_Power_Onoff Client model and to register with Acess layer.

    \param [in] element_handle
                Element identifier to be associated with the model instance.

    \param [in, out] model_handle
                     Model identifier associated with the model instance on successful initialization.
                     After power cycle of an already provisioned node, the model handle will have
                     valid value and the same will be reused for registration.

    \param [in] appl_cb    Application Callback to be used by the Generic_Power_Onoff Client.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_generic_power_onoff_client_init
(
    /* IN */    MS_ACCESS_ELEMENT_HANDLE    element_handle,
    /* INOUT */ MS_ACCESS_MODEL_HANDLE*     model_handle,
    /* IN */    MS_GENERIC_POWER_ONOFF_CLIENT_CB appl_cb
);

/**
    \brief API to get Generic_Power_Onoff client model handle

    \par Description
    This is to get the handle of Generic_Power_Onoff client model.

    \param [out] model_handle   Address of model handle to be filled/returned.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_generic_power_onoff_client_get_model_handle
(
    /* OUT */ MS_ACCESS_MODEL_HANDLE*   model_handle
);

/**
    \brief API to send acknowledged commands

    \par Description
    This is to initialize sending acknowledged commands.

    \param [in] req_opcode    Request Opcode.
    \param [in] param         Parameter associated with Request Opcode.
    \param [in] rsp_opcode    Response Opcode.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_generic_power_onoff_client_send_reliable_pdu
(
    /* IN */ UINT32    req_opcode,
    /* IN */ void*     param,
    /* IN */ UINT32    rsp_opcode
);

/**
    \brief API to get the Generic OnPowerUp state of an element.

    \par Description
    Generic OnPowerUp Get is an acknowledged message used to get the Generic OnPowerUp state of an element.
    The response to the Generic OnPowerUp Get message is a Generic OnPowerUp Status message.
    There are no parameters for this message.

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_generic_onpowerup_get() \
    MS_generic_power_onoff_client_send_reliable_pdu \
    (\
     MS_ACCESS_GENERIC_ONPOWERUP_GET_OPCODE,\
     NULL,\
     MS_ACCESS_GENERIC_ONPOWERUP_STATUS_OPCODE\
    )

/**
    \brief API to set the Generic OnPowerUp state of an element.

    \par Description
    Generic OnPowerUp Set is an acknowledged message used to set the Generic OnPowerUp state of an element.
    The response to the Generic OnPowerUp Set message is a Generic OnPowerUp Status message.

    \param [in] param Generic OnPowerUp Set message

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_generic_onpowerup_set(param) \
    MS_generic_power_onoff_client_send_reliable_pdu \
    (\
     MS_ACCESS_GENERIC_ONPOWERUP_SET_OPCODE,\
     param,\
     MS_ACCESS_GENERIC_ONPOWERUP_STATUS_OPCODE\
    )

/**
    \brief API to set the Generic OnPowerUp state of an element.

    \par Description
    Generic OnPowerUp Set Unacknowledged is an unacknowledged message used to set
    the Generic OnPowerUp state of an element.

    \param [in] param Generic OnPowerUp Set message

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_generic_onpowerup_set_unacknowledged(param) \
    MS_generic_power_onoff_client_send_reliable_pdu \
    (\
     MS_ACCESS_GENERIC_ONPOWERUP_SET_UNACKNOWLEDGED_OPCODE,\
     param,\
     0xFFFFFFFF\
    )
/** \} */
/** \} */
/** \} */

#endif /* _H_MS_GENERIC_POWER_ONOFF_API_ */
