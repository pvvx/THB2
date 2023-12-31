/**
    \file MS_light_lightness_api.h

    \brief This file defines the Mesh Light Lightness Model Application Interface
    - includes Data Structures and Methods for both Server and Client.
*/

/*
    Copyright (C) 2017. Mindtree Ltd.
    All rights reserved.
*/

#ifndef _H_MS_LIGHT_LIGHTNESS_API_
#define _H_MS_LIGHT_LIGHTNESS_API_


/* --------------------------------------------- Header File Inclusion */
#include "MS_access_api.h"


/* --------------------------------------------- Global Definitions */
/**
    \defgroup light_lightness_module LIGHT_LIGHTNESS (Mesh Light Lightness Model)
    \{
    This section describes the interfaces & APIs offered by the EtherMind
    Mesh Generic OnOff Model (ONOFF) module to the Application.
*/



/* --------------------------------------------- Data Types/ Structures */
/**
    \defgroup light_lightness_cb Application Callback
    \{
    This Section Describes the module Notification Callback interface offered
    to the application
*/

/**
    Light Lightness Server application Asynchronous Notification Callback.

    Light Lightness Server calls the registered callback to indicate events occurred to the
    application.

    \param [in] ctx           Context of the message received for a specific model instance.
    \param [in] msg_raw       Uninterpreted/raw received message.
    \param [in] req_type      Requested message type.
    \param [in] state_params  Model specific state parameters.
    \param [in] ext_params    Additional parameters.
*/
typedef API_RESULT (* MS_LIGHT_LIGHTNESS_SERVER_CB)
(
    MS_ACCESS_MODEL_REQ_MSG_CONTEXT*     ctx,
    MS_ACCESS_MODEL_REQ_MSG_RAW*         msg_raw,
    MS_ACCESS_MODEL_REQ_MSG_T*           req_type,
    MS_ACCESS_MODEL_STATE_PARAMS*        state_params,
    MS_ACCESS_MODEL_EXT_PARAMS*          ext_params

) DECL_REENTRANT;

/**
    Light Lightness Setup Server application Asynchronous Notification Callback.

    Light Lightness Setup Server calls the registered callback to indicate events occurred to the
    application.

    \param [in] ctx           Context of the message received for a specific model instance.
    \param [in] msg_raw       Uninterpreted/raw received message.
    \param [in] req_type      Requested message type.
    \param [in] state_params  Model specific state parameters.
    \param [in] ext_params    Additional parameters.
*/
typedef API_RESULT (* MS_LIGHT_LIGHTNESS_SETUP_SERVER_CB)
(
    MS_ACCESS_MODEL_REQ_MSG_CONTEXT*     ctx,
    MS_ACCESS_MODEL_REQ_MSG_RAW*         msg_raw,
    MS_ACCESS_MODEL_REQ_MSG_T*           req_type,
    MS_ACCESS_MODEL_STATE_PARAMS*        state_params,
    MS_ACCESS_MODEL_EXT_PARAMS*          ext_params

) DECL_REENTRANT;

/**
    Light Lightness Client application Asynchronous Notification Callback.

    Light Lightness Client calls the registered callback to indicate events occurred to the
    application.

    \param handle        Model Handle.
    \param opcode        Opcode.
    \param data_param    Data associated with the event if any or NULL.
    \param data_len      Size of the event data. 0 if event data is NULL.
*/
typedef API_RESULT (* MS_LIGHT_LIGHTNESS_CLIENT_CB)
(
    MS_ACCESS_MODEL_HANDLE* handle,
    UINT32                   opcode,
    UCHAR*                   data_param,
    UINT16                   data_len
) DECL_REENTRANT;
/** \} */

/**
    \defgroup light_lightness_structures Structures
    \{
*/

/**
    Light Lightness Set message parameters.
*/
typedef struct MS_light_lightness_set_struct
{
    /** The target value of the Light Lightness Actual state. */
    UINT16 lightness;

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

} MS_LIGHT_LIGHTNESS_SET_STRUCT;

/**
    Light Lightness Status message parameters.
*/
typedef struct MS_light_lightness_status_struct
{
    /** The present value of the Light Lightness Actual state. */
    UINT16 present_lightness;

    /** The target value of the Light Lightness Actual state. (Optional) */
    UINT16 target_lightness;

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

    /** Flag: To represent if optional fields Target Light Lightness Actual and Remaining Time are valid */
    UCHAR optional_fields_present;

} MS_LIGHT_LIGHTNESS_STATUS_STRUCT;

/**
    Light Lightness Linear Set message parameters.
*/
typedef struct MS_light_lightness_linear_set_struct
{
    /** The target value of the Light Lightness Linear state. */
    UINT16 lightness;

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

} MS_LIGHT_LIGHTNESS_LINEAR_SET_STRUCT;

/**
    Light Lightness Linear Status message parameters.
*/
typedef struct MS_light_lightness_linear_status_struct
{
    /** The present value of the Light Lightness Linear state */
    UINT16 present_lightness;

    /** The target value of the Light Lightness Linear state (Optional) */
    UINT16 target_lightness;

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

    /** Flag: To represent if optional fields Target Light Lightness Linear and Remaining Time are valid */
    UCHAR optional_fields_present;

} MS_LIGHT_LIGHTNESS_LINEAR_STATUS_STRUCT;

/**
    Light Lightness Last Status message parameters.
*/
typedef struct MS_light_lightness_last_status_struct
{
    /** The value of the Light Lightness Last */
    UINT16 lightness;

} MS_LIGHT_LIGHTNESS_LAST_STATUS_STRUCT;

/**
    Light Lightness Default Set message parameters.
*/
typedef struct MS_light_lightness_default_set_struct
{
    /** The value of the Light Lightness Default state */
    UINT16 lightness;

} MS_LIGHT_LIGHTNESS_DEFAULT_SET_STRUCT;

/**
    Light Lightness Range Set message parameters.
*/
typedef struct MS_light_lightness_range_set_struct
{
    /** The value of the Lightness Range Min field of the Light Lightness Range state */
    UINT16 range_min;

    /** The value of the Lightness Range Max field of the Light Lightness Range state */
    UINT16 range_max;

} MS_LIGHT_LIGHTNESS_RANGE_SET_STRUCT;

/**
    Light Lightness Range Status message parameters.
*/
typedef struct MS_light_lightness_range_status_struct
{
    /** Status Code for the requesting message. */
    UCHAR  status_code;

    /** The value of the Lightness Range Min field of the Light Lightness Range state */
    UINT16 range_min;

    /** The value of the Lightness Range Max field of the Light Lightness Range state */
    UINT16 range_max;

} MS_LIGHT_LIGHTNESS_RANGE_STATUS_STRUCT;

/**
    Light Lightness Default Status message parameters.
*/
typedef struct MS_light_lightness_default_status_struct
{
    /** The value of the Light Lightness Default state */
    UINT16 lightness;

} MS_LIGHT_LIGHTNESS_DEFAULT_STATUS_STRUCT;

typedef struct MS_light_lightness_last_or_default_status_struct
{
    UINT16 lightness;

} MS_LIGHT_LIGHTNESS_LAST_OR_DEFAULT_STATUS_STRUCT;

/** \} */



/* --------------------------------------------- Function */
/**
    \defgroup light_lightness_api_defs API Definitions
    \{
    This section describes the EtherMind Mesh Light Lightness Model APIs.
*/
/**
    \defgroup light_lightness_ser_api_defs Light Lightness Server API Definitions
    \{
    This section describes the Light Lightness Server APIs.
*/

/**
    \brief API to initialize Light_Lightness Server model

    \par Description
    This is to initialize Light_Lightness Server model and to register with Acess layer.

    \param [in] element_handle
                Element identifier to be associated with the model instance.

    \param [in, out] model_handle
                     Model identifier associated with the model instance on successful initialization.
                     After power cycle of an already provisioned node, the model handle will have
                     valid value and the same will be reused for registration.

    \param [in] appl_cb    Application Callback to be used by the Light_Lightness Server.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_light_lightness_server_init
(
    /* IN */    MS_ACCESS_ELEMENT_HANDLE    element_handle,
    /* INOUT */ MS_ACCESS_MODEL_HANDLE*     model_handle,
    /* IN */    MS_LIGHT_LIGHTNESS_SERVER_CB appl_cb
);

/**
    \brief API to initialize Light_Lightness_Setup Server model

    \par Description
    This is to initialize Light_Lightness_Setup Server model and to register with Acess layer.

    \param [in] element_handle
                Element identifier to be associated with the model instance.

    \param [in, out] model_handle
                     Model identifier associated with the model instance on successful initialization.
                     After power cycle of an already provisioned node, the model handle will have
                     valid value and the same will be reused for registration.

    \param [in] appl_cb    Application Callback to be used by the Light_Lightness_Setup Server.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_light_lightness_setup_server_init
(
    /* IN */    MS_ACCESS_ELEMENT_HANDLE    element_handle,
    /* INOUT */ MS_ACCESS_MODEL_HANDLE*     model_handle,
    /* IN */    MS_LIGHT_LIGHTNESS_SETUP_SERVER_CB appl_cb
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
API_RESULT MS_light_lightness_server_state_update
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
API_RESULT MS_light_lightness_setup_server_state_update
(
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_CONTEXT*     ctx,
    /* IN */ MS_ACCESS_MODEL_STATE_PARAMS*        current_state_params,
    /* IN */ MS_ACCESS_MODEL_STATE_PARAMS*        target_state_params,
    /* IN */ UINT16                               remaining_time,
    /* IN */ MS_ACCESS_MODEL_EXT_PARAMS*          ext_params
);
/** \} */

/**
    \defgroup light_lightness_cli_api_defs Light Lightness Client API Definitions
    \{
    This section describes the Light Lightness Client APIs.
*/

/**
    \brief API to initialize Light_Lightness Client model

    \par Description
    This is to initialize Light_Lightness Client model and to register with Acess layer.

    \param [in] element_handle
                Element identifier to be associated with the model instance.

    \param [in, out] model_handle
                     Model identifier associated with the model instance on successful initialization.
                     After power cycle of an already provisioned node, the model handle will have
                     valid value and the same will be reused for registration.

    \param [in] appl_cb    Application Callback to be used by the Light_Lightness Client.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_light_lightness_client_init
(
    /* IN */    MS_ACCESS_ELEMENT_HANDLE    element_handle,
    /* INOUT */ MS_ACCESS_MODEL_HANDLE*     model_handle,
    /* IN */    MS_LIGHT_LIGHTNESS_CLIENT_CB appl_cb
);

/**
    \brief API to get Light_Lightness client model handle

    \par Description
    This is to get the handle of Light_Lightness client model.

    \param [out] model_handle   Address of model handle to be filled/returned.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_light_lightness_client_get_model_handle
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
API_RESULT MS_light_lightness_client_send_reliable_pdu
(
    /* IN */ UINT32    req_opcode,
    /* IN */ void*     param,
    /* IN */ UINT32    rsp_opcode
);

/**
    \brief API to get the Light Lightness Actual state of an element.

    \par Description
    Light Lightness Get is an acknowledged message used to get the Light Lightness Actual state of an element.
    The response to the Light Lightness Get message is a Light Lightness Status message.
    There are no parameters for this message.

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_light_lightness_get() \
    MS_light_lightness_client_send_reliable_pdu \
    (\
     MS_ACCESS_LIGHT_LIGHTNESS_GET_OPCODE,\
     NULL,\
     MS_ACCESS_LIGHT_LIGHTNESS_STATUS_OPCODE\
    )

/**
    \brief API to set the Light Lightness Actual state of an element.

    \par Description
    The Light Lightness Set is an acknowledged message used to set the Light Lightness Actual state of an element.
    The response to the Light Lightness Set message is a Light Lightness Status message.

    \param [in] param Light Lightness Set message

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_light_lightness_set(param) \
    MS_light_lightness_client_send_reliable_pdu \
    (\
     MS_ACCESS_LIGHT_LIGHTNESS_SET_OPCODE,\
     param,\
     MS_ACCESS_LIGHT_LIGHTNESS_STATUS_OPCODE\
    )

/**
    \brief API to set the Light Lightness Actual state of an element.

    \par Description
    The Light Lightness Set Unacknowledged is an unacknowledged message used to
    set the Light Lightness Actual state of an element.

    \param [in] param Light Lightness Set message

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_light_lightness_set_unacknowledged(param) \
    MS_light_lightness_client_send_reliable_pdu \
    (\
     MS_ACCESS_LIGHT_LIGHTNESS_SET_UNACKNOWLEDGED_OPCODE,\
     param,\
     0xFFFFFFFF\
    )

/**
    \brief API to get the Light Lightness Linear state of an element.

    \par Description
    Light Lightness Linear Get is an acknowledged message used to get the Light Lightness Linear state of an element.
    The response to the Light Lightness Linear Get message is a Light Lightness Linear Status message.
    There are no parameters for this message.

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_light_lightness_linear_get() \
    MS_light_lightness_client_send_reliable_pdu \
    (\
     MS_ACCESS_LIGHT_LIGHTNESS_LINEAR_GET_OPCODE,\
     NULL,\
     MS_ACCESS_LIGHT_LIGHTNESS_LINEAR_STATUS_OPCODE\
    )

/**
    \brief API to set the Light Lightness Linear state of an element.

    \par Description
    The Light Lightness Linear Set is an acknowledged message used to set the Light Lightness Linear state of an element.
    The response to the Light Lightness Linear Set message is a Light Lightness Linear Status message.

    \param [in] param Light Lightness Linear Set message

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_light_lightness_linear_set(param) \
    MS_light_lightness_client_send_reliable_pdu \
    (\
     MS_ACCESS_LIGHT_LIGHTNESS_LINEAR_SET_OPCODE,\
     param,\
     MS_ACCESS_LIGHT_LIGHTNESS_LINEAR_STATUS_OPCODE\
    )

/**
    \brief API to set the Light Lightness Linear state of an element.

    \par Description
    The Light Lightness Linear Set Unacknowledged is an unacknowledged message
    used to set the Light Lightness Linear state of an element.

    \param [in] param Light Lightness Linear Set message

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_light_lightness_linear_set_unacknowledged(param) \
    MS_light_lightness_client_send_reliable_pdu \
    (\
     MS_ACCESS_LIGHT_LIGHTNESS_LINEAR_SET_UNACKNOWLEDGED_OPCODE,\
     param,\
     0xFFFFFFFF\
    )

/**
    \brief API to get the Light Lightness Last state of an element.

    \par Description
    Light Lightness Last Get is an acknowledged message used to get the Light Lightness Last state of an element.
    The response to the Light Lightness Last Get message is a Light Lightness Last Status message.
    There are no parameters for this message.

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_light_lightness_last_get() \
    MS_light_lightness_client_send_reliable_pdu \
    (\
     MS_ACCESS_LIGHT_LIGHTNESS_LAST_GET_OPCODE,\
     NULL,\
     MS_ACCESS_LIGHT_LIGHTNESS_LAST_STATUS_OPCODE\
    )

/**
    \brief API to get the Light Lightness Default state of an element.

    \par Description
    Light Lightness Default Get is an acknowledged message used to get the Light Lightness Default state of an element.
    The response to the Light Lightness Default Get message is a Light Lightness Default Status message.
    There are no parameters for this message.

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_light_lightness_default_get() \
    MS_light_lightness_client_send_reliable_pdu \
    (\
     MS_ACCESS_LIGHT_LIGHTNESS_DEFAULT_GET_OPCODE,\
     NULL,\
     MS_ACCESS_LIGHT_LIGHTNESS_DEFAULT_STATUS_OPCODE\
    )

/**
    \brief API to set the Light Lightness Default state of an element.

    \par Description
    The Light Lightness Default Set is an acknowledged message used to set the Light Lightness Default state of an element.
    The response to the Light Lightness Default Set message is a Light Lightness Default Status message.

    \param [in] param Light Lightness Default Set message

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_light_lightness_default_set(param) \
    MS_light_lightness_client_send_reliable_pdu \
    (\
     MS_ACCESS_LIGHT_LIGHTNESS_DEFAULT_SET_OPCODE,\
     param,\
     MS_ACCESS_LIGHT_LIGHTNESS_DEFAULT_STATUS_OPCODE\
    )

/**
    \brief API to set the Light Lightness Default state of an element.

    \par Description
    The Light Lightness Default Set Unacknowledged is an unacknowledged message
    used to set the Light Lightness Default state of an element.

    \param [in] param Light Lightness Default Set message

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_light_lightness_default_set_unacknowledged(param) \
    MS_light_lightness_client_send_reliable_pdu \
    (\
     MS_ACCESS_LIGHT_LIGHTNESS_DEFAULT_SET_UNACKNOWLEDGED_OPCODE,\
     param,\
     0xFFFFFFFF\
    )

/**
    \brief API to get the Light Lightness Range state of an element.

    \par Description
    The Light Lightness Range Get is an acknowledged message used to get the Light Lightness Range state of an element.
    The response to the Light Lightness Range Get message is a Light Lightness Range Status message.
    There are no parameters for this message.

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_light_lightness_range_get() \
    MS_light_lightness_client_send_reliable_pdu \
    (\
     MS_ACCESS_LIGHT_LIGHTNESS_RANGE_GET_OPCODE,\
     NULL,\
     MS_ACCESS_LIGHT_LIGHTNESS_RANGE_STATUS_OPCODE\
    )

/**
    \brief API to acknowledged message used to set the Light Lightness Range state of an element.

    \par Description
    Light Lightness Range Set is an acknowledged message used to set the Light Lightness Range state of an element.
    The response to the Light Lightness Range Get message is a Light Lightness Range Status message.

    \param [in] param Light Lightness Range Set message

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_light_lightness_range_set(param) \
    MS_light_lightness_client_send_reliable_pdu \
    (\
     MS_ACCESS_LIGHT_LIGHTNESS_RANGE_SET_OPCODE,\
     param,\
     MS_ACCESS_LIGHT_LIGHTNESS_RANGE_STATUS_OPCODE\
    )

/**
    \brief API to acknowledged message used to set the Light Lightness Range state of an element.

    \par Description
    Light Lightness Range Set Unacknowledged is an unacknowledged message used
    to set the Light Lightness Range state of an element.

    \param [in] param Light Lightness Range Set message

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_light_lightness_range_set_unacknowledged(param) \
    MS_light_lightness_client_send_reliable_pdu \
    (\
     MS_ACCESS_LIGHT_LIGHTNESS_RANGE_SET_UNACKNOWLEDGED_OPCODE,\
     param,\
     0xFFFFFFFF\
    )
/** \} */
/** \} */
/** \} */

#endif /*_H_MS_LIGHT_LIGHTNESS_API_ */
