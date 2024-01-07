/**
    \file MS_light_ctl_api.h

    \brief This file defines the Mesh Light Ctl Model Application Interface
    - includes Data Structures and Methods for both Server and Client.
*/

/*
    Copyright (C) 2017. Mindtree Ltd.
    All rights reserved.
*/

#ifndef _H_MS_LIGHT_CTL_API_
#define _H_MS_LIGHT_CTL_API_


/* --------------------------------------------- Header File Inclusion */
#include "MS_access_api.h"


/* --------------------------------------------- Global Definitions */
/**
    \defgroup light_ctl_module LIGHT_CTL (Mesh Light Ctl Model)
    \{
    This section describes the interfaces & APIs offered by the EtherMind
    Mesh Generic OnOff Model (ONOFF) module to the Application.
*/



/* --------------------------------------------- Data Types/ Structures */
/**
    \defgroup light_ctl_cb Application Callback
    \{
    This Section Describes the module Notification Callback interface offered
    to the application
*/

/**
    Light Ctl Server application Asynchronous Notification Callback.

    Light Ctl Server calls the registered callback to indicate events occurred to the
    application.

    \param [in] ctx           Context of the message received for a specific model instance.
    \param [in] msg_raw       Uninterpreted/raw received message.
    \param [in] req_type      Requested message type.
    \param [in] state_params  Model specific state parameters.
    \param [in] ext_params    Additional parameters.
*/
typedef API_RESULT (* MS_LIGHT_CTL_SERVER_CB)
(
    MS_ACCESS_MODEL_REQ_MSG_CONTEXT*     ctx,
    MS_ACCESS_MODEL_REQ_MSG_RAW*         msg_raw,
    MS_ACCESS_MODEL_REQ_MSG_T*           req_type,
    MS_ACCESS_MODEL_STATE_PARAMS*        state_params,
    MS_ACCESS_MODEL_EXT_PARAMS*          ext_params

) DECL_REENTRANT;

/**
    Light Ctl Temperature Server application Asynchronous Notification Callback.

    Light Ctl Temperature Server calls the registered callback to indicate events occurred to the
    application.

    \param [in] ctx           Context of the message received for a specific model instance.
    \param [in] msg_raw       Uninterpreted/raw received message.
    \param [in] req_type      Requested message type.
    \param [in] state_params  Model specific state parameters.
    \param [in] ext_params    Additional parameters.
*/
typedef API_RESULT (* MS_LIGHT_CTL_TEMPERATURE_SERVER_CB)
(
    MS_ACCESS_MODEL_REQ_MSG_CONTEXT*     ctx,
    MS_ACCESS_MODEL_REQ_MSG_RAW*         msg_raw,
    MS_ACCESS_MODEL_REQ_MSG_T*           req_type,
    MS_ACCESS_MODEL_STATE_PARAMS*        state_params,
    MS_ACCESS_MODEL_EXT_PARAMS*          ext_params

) DECL_REENTRANT;

/**
    Light Ctl Client application Asynchronous Notification Callback.

    Light Ctl Client calls the registered callback to indicate events occurred to the
    application.

    \param handle        Model Handle.
    \param opcode        Opcode.
    \param data_param    Data associated with the event if any or NULL.
    \param data_len      Size of the event data. 0 if event data is NULL.
*/
typedef API_RESULT (* MS_LIGHT_CTL_CLIENT_CB)
(
    MS_ACCESS_MODEL_HANDLE* handle,
    UINT32                   opcode,
    UCHAR*                   data_param,
    UINT16                   data_len
) DECL_REENTRANT;
/** \} */

/**
    \defgroup light_ctl_structures Structures
    \{
*/

/**
    Light CTL Set message parameters.
*/
typedef struct MS_light_ctl_set_struct
{
    /** The target value of the Light CTL Lightness state. */
    UINT16 ctl_lightness;

    /** The target value of the Light CTL Temperature state. */
    UINT16 ctl_temperature;

    /** The target value of the Light CTL Delta UV state. */
    UINT16 ctl_delta_uv;

    /** Transaction Identifier */
    UCHAR  tid;

    /**
        Transition Time is a 1-octet value that consists of two fields:
        - a 2-bit bit field representing the step resolution
        - a 6-bit bit field representing the number of transition steps.

        Field                      | Size (bits) | Description
        ---------------------------|-------------|----------------
        Transition Number of Steps | 6           | The number of Steps
        Transition Step Resolution | 2           | The resolution of the Default Transition
                                                | Number of Steps field
    */
    UCHAR  transition_time;

    /** Message execution delay in 5 milliseconds steps */
    UCHAR  delay;

    /** Flag: To represent if optional Transaction time and Delay fields are valid */
    UCHAR optional_fields_present;

} MS_LIGHT_CTL_SET_STRUCT;

/**
    Light CTL Status message parameters.
*/
typedef struct MS_light_ctl_status_struct
{
    /** The present value of the Light CTL Lightness state */
    UINT16 present_ctl_lightness;

    /** The present value of the Light CTL Temperature state */
    UINT16 present_ctl_temperature;

    /** The target value of the Light CTL Lightness state (Optional) */
    UINT16 target_ctl_lightness;

    /** The target value of the Light CTL Temperature state */
    UINT16 target_ctl_temperature;

    /**
        Remaining Time is a 1-octet value that consists of two fields:
        - a 2-bit bit field representing the step resolution
        - a 6-bit bit field representing the number of transition steps.

        Field                      | Size (bits) | Description
        ---------------------------|-------------|----------------
        Transition Number of Steps | 6           | The number of Steps
        Transition Step Resolution | 2           | The resolution of the Default Transition
                                                | Number of Steps field
    */
    UCHAR  remaining_time;

    /** Flag: To represent if optional fields Target CTL Lightness and Temperature are valid */
    UCHAR optional_fields_present;

} MS_LIGHT_CTL_STATUS_STRUCT;

/**
    Light CTL Temperature Set message parameters.
*/
typedef struct MS_light_ctl_temperature_set_struct
{
    /** The target value of the Light CTL Temperature state. */
    UINT16 ctl_temperature;

    /** The target value of the Light CTL Delta UV state. */
    UINT16 ctl_delta_uv;

    /** Transaction Identifier */
    UCHAR  tid;

    /**
        Transition Time is a 1-octet value that consists of two fields:
        - a 2-bit bit field representing the step resolution
        - a 6-bit bit field representing the number of transition steps.

        Field                      | Size (bits) | Description
        ---------------------------|-------------|----------------
        Transition Number of Steps | 6           | The number of Steps
        Transition Step Resolution | 2           | The resolution of the Default Transition
                                                | Number of Steps field
    */
    UCHAR  transition_time;

    /** Message execution delay in 5 milliseconds steps */
    UCHAR  delay;

    /** Flag: To represent if optional Transaction time and Delay fields are valid */
    UCHAR optional_fields_present;

} MS_LIGHT_CTL_TEMPERATURE_SET_STRUCT;

/**
    Light CTL Temperature Status message parameters.
*/
typedef struct MS_light_ctl_temperature_status_struct
{
    /** The present value of the Light CTL Temperature state. */
    UINT16 present_ctl_temperature;

    /** The present value of the Light CTL Delta UV state */
    UINT16 present_ctl_delta_uv;

    /** The target value of the Light CTL Temperature state (Optional) */
    UINT16 target_ctl_temperature;

    /** The target value of the Light CTL Delta UV state */
    UINT16 target_ctl_delta_uv;

    /**
        Remaining Time is a 1-octet value that consists of two fields:
        - a 2-bit bit field representing the step resolution
        - a 6-bit bit field representing the number of transition steps.

        Field                      | Size (bits) | Description
        ---------------------------|-------------|----------------
        Transition Number of Steps | 6           | The number of Steps
        Transition Step Resolution | 2           | The resolution of the Default Transition
                                                | Number of Steps field
    */
    UCHAR  remaining_time;

    /** Flag: To represent if optional fields Target CTL Temperature, Delta UV and Remaining Time are valid */
    UCHAR optional_fields_present;

} MS_LIGHT_CTL_TEMPERATURE_STATUS_STRUCT;

/**
    Light CTL Default Set message parameters.
*/
typedef struct MS_light_ctl_default_set_struct
{
    /** The value of the Light Lightness Default state. */
    UINT16 lightness;

    /** The value of the Light CTL Temperature Default state. */
    UINT16 temperature;

    /** The value of the Light CTL Delta UV Default state. */
    UINT16 delta_uv;

} MS_LIGHT_CTL_DEFAULT_SET_STRUCT;

/**
    Light CTL Default Status message parameters.
*/
typedef struct MS_light_ctl_default_status_struct
{
    /** The value of the Light Lightness Default state */
    UINT16 lightness;

    /** The value of the Light CTL Temperature Default state */
    UINT16 temperature;

    /** The value of the Light CTL Delta UV Default state */
    UINT16 delta_uv;

} MS_LIGHT_CTL_DEFAULT_STATUS_STRUCT;

/**
    Light CTL Temperature Range Set message parameters.
*/
typedef struct MS_light_ctl_temperature_range_set_struct
{
    /** The value of the Temperature Range Min field of the Light CTL Temperature Range state */
    UINT16 range_min;

    /** The value of the Temperature Range Max field of the Light CTL Temperature Range state */
    UINT16 range_max;

} MS_LIGHT_CTL_TEMPERATURE_RANGE_SET_STRUCT;

/**
    Light CTL Temperature Range Status message parameters.
*/
typedef struct MS_light_ctl_temperature_range_status_struct
{
    /** Status Code for the requesting message. */
    UCHAR  status_code;

    /** The value of the Temperature Range Min field of the Light CTL Temperature Range state */
    UINT16 range_min;

    /** The value of the Temperature Range Max field of the Light CTL Temperature Range state */
    UINT16 range_max;

} MS_LIGHT_CTL_TEMPERATURE_RANGE_STATUS_STRUCT;

/** \} */



/* --------------------------------------------- Function */
/**
    \defgroup light_ctl_api_defs API Definitions
    \{
    This section describes the EtherMind Mesh Light Ctl Model APIs.
*/
/**
    \defgroup light_ctl_ser_api_defs Light Ctl Server API Definitions
    \{
    This section describes the Light Ctl Server APIs.
*/

/**
    \brief API to initialize Light_Ctl Server model

    \par Description
    This is to initialize Light_Ctl Server model and to register with Acess layer.

    \param [in] element_handle
                Element identifier to be associated with the model instance.

    \param [in, out] ctl_model_handle
                     Model identifier associated with the Light CTL model instance on successful initialization.
                     After power cycle of an already provisioned node, the model handle will have
                     valid value and the same will be reused for registration.

    \param [in, out] ctl_setup_model_handle
                     Model identifier associated with the Light CTL Setup model instance on successful initialization.
                     After power cycle of an already provisioned node, the model handle will have
                     valid value and the same will be reused for registration.

    \param [in] appl_cb    Application Callback to be used by the Light_Ctl Server.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_light_ctl_server_init
(
    /* IN */    MS_ACCESS_ELEMENT_HANDLE    element_handle,
    /* INOUT */ MS_ACCESS_MODEL_HANDLE*     ctl_model_handle,
    /* INOUT */ MS_ACCESS_MODEL_HANDLE*     ctl_setup_model_handle,
    /* IN */    MS_LIGHT_CTL_SERVER_CB      appl_cb
);

/**
    \brief API to initialize Light_Ctl_Temperature Server model

    \par Description
    This is to initialize Light_Ctl_Temperature Server model and to register with Acess layer.

    \param [in] element_handle
                Element identifier to be associated with the model instance.

    \param [in, out] model_handle
                     Model identifier associated with the model instance on successful initialization.
                     After power cycle of an already provisioned node, the model handle will have
                     valid value and the same will be reused for registration.

    \param [in] appl_cb    Application Callback to be used by the Light_Ctl_Temperature Server.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_light_ctl_temperature_server_init
(
    /* IN */    MS_ACCESS_ELEMENT_HANDLE    element_handle,
    /* INOUT */ MS_ACCESS_MODEL_HANDLE*     model_handle,
    /* IN */    MS_LIGHT_CTL_TEMPERATURE_SERVER_CB appl_cb
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
API_RESULT MS_light_ctl_server_state_update
(
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_CONTEXT*     ctx,
    /* IN */ MS_ACCESS_MODEL_STATE_PARAMS*        current_state_params,
    /* IN */ MS_ACCESS_MODEL_STATE_PARAMS*        target_state_params,
    /* IN */ UINT16                               remaining_time,
    /* IN */ MS_ACCESS_MODEL_EXT_PARAMS*          ext_params
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
API_RESULT MS_light_ctl_temperature_server_state_update
(
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_CONTEXT*     ctx,
    /* IN */ MS_ACCESS_MODEL_STATE_PARAMS*        current_state_params,
    /* IN */ MS_ACCESS_MODEL_STATE_PARAMS*        target_state_params,
    /* IN */ UINT16                               remaining_time,
    /* IN */ MS_ACCESS_MODEL_EXT_PARAMS*          ext_params
);

/** \} */

/**
    \defgroup light_ctl_cli_api_defs Light Ctl Client API Definitions
    \{
    This section describes the Light Ctl Client APIs.
*/

/**
    \brief API to initialize Light_Ctl Client model

    \par Description
    This is to initialize Light_Ctl Client model and to register with Acess layer.

    \param [in] element_handle
                Element identifier to be associated with the model instance.

    \param [in, out] model_handle
                     Model identifier associated with the model instance on successful initialization.
                     After power cycle of an already provisioned node, the model handle will have
                     valid value and the same will be reused for registration.

    \param [in] appl_cb    Application Callback to be used by the Light_Ctl Client.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_light_ctl_client_init
(
    /* IN */    MS_ACCESS_ELEMENT_HANDLE    element_handle,
    /* INOUT */ MS_ACCESS_MODEL_HANDLE*     model_handle,
    /* IN */    MS_LIGHT_CTL_CLIENT_CB appl_cb
);

/**
    \brief API to get Light_Ctl client model handle

    \par Description
    This is to get the handle of Light_Ctl client model.

    \param [out] model_handle   Address of model handle to be filled/returned.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_light_ctl_client_get_model_handle
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
API_RESULT MS_light_ctl_client_send_reliable_pdu
(
    /* IN */ UINT32    req_opcode,
    /* IN */ void*     param,
    /* IN */ UINT32    rsp_opcode
);

/**
    \brief API to get the Light CTL state of an element.

    \par Description
    Light CTL Get is an acknowledged message used to get the Light CTL state of an element.
    The response to the Light CTL Get message is a Light CTL Status message.
    There are no parameters for this message.

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_light_ctl_get() \
    MS_light_ctl_client_send_reliable_pdu \
    (\
     MS_ACCESS_LIGHT_CTL_GET_OPCODE,\
     NULL,\
     MS_ACCESS_LIGHT_CTL_STATUS_OPCODE\
    )

/**
    \brief API to set the Light CTL Lightness state, Light CTL Temperature state, and the Light CTL Delta UV state of an element.

    \par Description
    Light CTL Set is an acknowledged message used to set the Light CTL Lightness state, Light CTL Temperature state,
    and the Light CTL Delta UV state of an element.
    The response to the Light CTL Set message is a Light CTL Status message.

    \param [in] param Light CTL Set message

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_light_ctl_set(param) \
    MS_light_ctl_client_send_reliable_pdu \
    (\
     MS_ACCESS_LIGHT_CTL_SET_OPCODE,\
     param,\
     MS_ACCESS_LIGHT_CTL_STATUS_OPCODE\
    )

/**
    \brief API to set the Light CTL Lightness state, Light CTL Temperature state, and the Light CTL Delta UV state of an element.

    \par Description
    Light CTL Set Unacknowledged is an unacknowledged message used to set the Light CTL Lightness state, Light CTL Temperature state,
    and the Light CTL Delta UV state of an element

    \param [in] param Light CTL Set message

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_light_ctl_set_unacknowledged(param) \
    MS_light_ctl_client_send_reliable_pdu \
    (\
     MS_ACCESS_LIGHT_CTL_SET_UNACKNOWLEDGED_OPCODE,\
     param,\
     0xFFFFFFFF\
    )

/**
    \brief API to get the Light CTL Temperature state of an element.

    \par Description
    Light CTL Temperature Get is an acknowledged message used to get the Light CTL Temperature state of an element.
    The response to the Light CTL Temperature Get message is a Light CTL Temperature Status message.
    There are no parameters for this message.

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_light_ctl_temperature_get() \
    MS_light_ctl_client_send_reliable_pdu \
    (\
     MS_ACCESS_LIGHT_CTL_TEMPERATURE_GET_OPCODE,\
     NULL,\
     MS_ACCESS_LIGHT_CTL_TEMPERATURE_STATUS_OPCODE\
    )

/**
    \brief API to set the Light CTL Temperature state and the Light CTL Delta UV state of an element.

    \par Description
    The Light CTL Temperature Set is an acknowledged message used to set the Light CTL Temperature state
    and the Light CTL Delta UV state of an element.
    The response to the Light CTL Temperature Set message is a Light CTL Temperature Status message.

    \param [in] param Light CTL Temperature Set message

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_light_ctl_temperature_set(param) \
    MS_light_ctl_client_send_reliable_pdu \
    (\
     MS_ACCESS_LIGHT_CTL_TEMPERATURE_SET_OPCODE,\
     param,\
     MS_ACCESS_LIGHT_CTL_TEMPERATURE_STATUS_OPCODE\
    )

/**
    \brief API to set the Light CTL Temperature state and the Light CTL Delta UV state of an element.

    \par Description
    The Light CTL Temperature Set Unacknowledged is an unacknowledged message used to set the Light CTL Temperature state
    and the Light CTL Delta UV state of an element

    \param [in] param Light CTL Temperature Set message

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_light_ctl_temperature_set_unacknowledged(param) \
    MS_light_ctl_client_send_reliable_pdu \
    (\
     MS_ACCESS_LIGHT_CTL_TEMPERATURE_SET_UNACKNOWLEDGED_OPCODE,\
     param,\
     0xFFFFFFFF\
    )

/**
    \brief API to get the Light CTL Temperature Default and Light CTL Delta UV Default states of an element.

    \par Description
    Light CTL Default Get is an acknowledged message used to get the Light CTL Temperature Default and Light CTL Delta UV Default states of an element.
    The response to the Light CTL Default Get message is a Light CTL Default Status message.
    There are no parameters for this message.

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_light_ctl_default_get() \
    MS_light_ctl_client_send_reliable_pdu \
    (\
     MS_ACCESS_LIGHT_CTL_DEFAULT_GET_OPCODE,\
     NULL,\
     MS_ACCESS_LIGHT_CTL_DEFAULT_STATUS_OPCODE\
    )

/**
    \brief API to set the Light CTL Temperature Default state and the Light CTL Delta UV Default state of an element.

    \par Description
    The Light CTL Default Set is an acknowledged message used to set the Light CTL Temperature Default state
    and the Light CTL Delta UV Default state of an element.
    The response to the Light CTL Set message is a Light CTL Status message.

    \param [in] param Light CTL Default Set message

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_light_ctl_default_set(param) \
    MS_light_ctl_client_send_reliable_pdu \
    (\
     MS_ACCESS_LIGHT_CTL_DEFAULT_SET_OPCODE,\
     param,\
     MS_ACCESS_LIGHT_CTL_DEFAULT_STATUS_OPCODE\
    )

/**
    \brief API to set the Light CTL Temperature Default state and the Light CTL Delta UV Default state of an element.

    \par Description
    The Light CTL Default Set Unacknowledged is an unacknowledged message used to set the Light CTL Temperature
    Default state and the Light CTL Delta UV Default state of an element.

    \param [in] param Light CTL Default Set message

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_light_ctl_default_set_unacknowledged(param) \
    MS_light_ctl_client_send_reliable_pdu \
    (\
     MS_ACCESS_LIGHT_CTL_DEFAULT_SET_UNACKNOWLEDGED_OPCODE,\
     param,\
     0xFFFFFFFF\
    )

/**
    \brief API to get the Light CTL Temperature Range state of an element.

    \par Description
    The Light CTL Temperature Range Get is an acknowledged message used to get the Light CTL Temperature Range state of an element.
    The response to the Light CTL Temperature Range Get message is a Light CTL Temperature Range Status message.
    There are no parameters for this message.

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_light_ctl_temperature_range_get() \
    MS_light_ctl_client_send_reliable_pdu \
    (\
     MS_ACCESS_LIGHT_CTL_TEMPERATURE_RANGE_GET_OPCODE,\
     NULL,\
     MS_ACCESS_LIGHT_CTL_TEMPERATURE_RANGE_STATUS_OPCODE\
    )

/**
    \brief API to set the Light CTL Temperature Range state of an element.

    \par Description
    Light CTL Temperature Range Set Unacknowledged is an unacknowledged message used to set
    the Light CTL Temperature Range state of an element.

    \param [in] param Light CTL Temperature Range Set message

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_light_ctl_temperature_range_set(param) \
    MS_light_ctl_client_send_reliable_pdu \
    (\
     MS_ACCESS_LIGHT_CTL_TEMPERATURE_RANGE_SET_OPCODE,\
     param,\
     MS_ACCESS_LIGHT_CTL_TEMPERATURE_RANGE_STATUS_OPCODE\
    )

/**
    \brief API to set the Light CTL Temperature Range state of an element.

    \par Description
    Light CTL Temperature Range Set is an acknowledged message used to set the Light CTL Temperature Range state of an element.
    The response to the Light CTL Temperature Range Get message is a Light CTL Temperature Range Status message.

    \param [in] param Light CTL Temperature Range Set message

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_light_ctl_temperature_range_set_unacknowledged(param) \
    MS_light_ctl_client_send_reliable_pdu \
    (\
     MS_ACCESS_LIGHT_CTL_TEMPERATURE_RANGE_SET_UNACKNOWLEDGED_OPCODE,\
     param,\
     0xFFFFFFFF\
    )
/** \} */
/** \} */
/** \} */

#endif /*_H_MS_LIGHT_CTL_API_ */
