/**
    \file MS_generic_power_level_api.h

    \brief This file defines the Mesh Generic Power Level Model Application Interface
    - includes Data Structures and Methods for both Server and Client.
*/

/*
    Copyright (C) 2017. Mindtree Ltd.
    All rights reserved.
*/

#ifndef _H_MS_GENERIC_POWER_LEVEL_API_
#define _H_MS_GENERIC_POWER_LEVEL_API_


/* --------------------------------------------- Header File Inclusion */
#include "MS_access_api.h"


/* --------------------------------------------- Global Definitions */
/**
    \defgroup generic_power_level_module GENERIC_POWER_LEVEL (Mesh Generic Power Level Model)
    \{
    This section describes the interfaces & APIs offered by the EtherMind
    Mesh Generic OnOff Model (ONOFF) module to the Application.
*/



/* --------------------------------------------- Data Types/ Structures */
/**
    \defgroup generic_power_level_cb Application Callback
    \{
    This Section Describes the module Notification Callback interface offered
    to the application
*/

/**
    Generic Power Level Server application Asynchronous Notification Callback.

    Generic Power Level Server calls the registered callback to indicate events occurred to the
    application.

    \param [in] ctx           Context of the message received for a specific model instance.
    \param [in] msg_raw       Uninterpreted/raw received message.
    \param [in] req_type      Requested message type.
    \param [in] state_params  Model specific state parameters.
    \param [in] ext_params    Additional parameters.
*/
typedef API_RESULT (* MS_GENERIC_POWER_LEVEL_SERVER_CB)
(
    MS_ACCESS_MODEL_REQ_MSG_CONTEXT*     ctx,
    MS_ACCESS_MODEL_REQ_MSG_RAW*         msg_raw,
    MS_ACCESS_MODEL_REQ_MSG_T*           req_type,
    MS_ACCESS_MODEL_STATE_PARAMS*        state_params,
    MS_ACCESS_MODEL_EXT_PARAMS*          ext_params

) DECL_REENTRANT;

/**
    Generic Power Level Client application Asynchronous Notification Callback.

    Generic Power Level Client calls the registered callback to indicate events occurred to the
    application.

    \param handle        Model Handle.
    \param opcode        Opcode.
    \param data_param    Data associated with the event if any or NULL.
    \param data_len      Size of the event data. 0 if event data is NULL.
*/
typedef API_RESULT (* MS_GENERIC_POWER_LEVEL_CLIENT_CB)
(
    MS_ACCESS_MODEL_HANDLE* handle,
    UINT32                   opcode,
    UCHAR*                   data_param,
    UINT16                   data_len
) DECL_REENTRANT;

/**
    Generic Power Level Setup Server application Asynchronous Notification Callback.

    Generic Power Level Setup Server calls the registered callback to indicate events occurred to the
    application.

    \param [in] ctx           Context of the message received for a specific model instance.
    \param [in] msg_raw       Uninterpreted/raw received message.
    \param [in] req_type      Requested message type.
    \param [in] state_params  Model specific state parameters.
    \param [in] ext_params    Additional parameters.
*/
typedef API_RESULT (* MS_GENERIC_POWER_LEVEL_SETUP_SERVER_CB)
(
    MS_ACCESS_MODEL_REQ_MSG_CONTEXT*     ctx,
    MS_ACCESS_MODEL_REQ_MSG_RAW*         msg_raw,
    MS_ACCESS_MODEL_REQ_MSG_T*           req_type,
    MS_ACCESS_MODEL_STATE_PARAMS*        state_params,
    MS_ACCESS_MODEL_EXT_PARAMS*          ext_params

) DECL_REENTRANT;
/** \} */

/**
    \defgroup generic_power_level_structures Structures
    \{
*/

/**
    Generic Power Level Set message parameters.
*/
typedef struct MS_generic_power_level_set_struct
{
    /** The target value of the Generic Power Actual state */
    UINT16 power;

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

} MS_GENERIC_POWER_LEVEL_SET_STRUCT;

/**
    Generic Power Level Status message parameters.
*/
typedef struct MS_generic_power_level_status_struct
{
    /** The present value of the Generic Power Actual state. */
    UINT16 present_power;

    /** The target value of the Generic Power Actual state. */
    UINT16 target_power;

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

    /** Flag: To represent if optional fields Target Power and Remaining Time are valid */
    UCHAR optional_fields_present;

} MS_GENERIC_POWER_LEVEL_STATUS_STRUCT;

/**
    Generic Power Last Status message parameters.
*/
typedef struct MS_generic_power_last_status_struct
{
    /** The value of the Generic Power Last state. */
    UINT16 power;

} MS_GENERIC_POWER_LAST_STATUS_STRUCT;

/**
    Generic Power Default Status message parameters.
*/
typedef struct MS_generic_power_default_status_struct
{
    /** The value of the Generic Power Default state. */
    UINT16 power;

} MS_GENERIC_POWER_DEFAULT_STATUS_STRUCT;

/**
    Generic Power Range Status message parameters.
*/
typedef struct MS_generic_power_range_status_struct
{
    /** Status Code for the requesting message. */
    UCHAR  status;

    /** The value of the Generic Power Range Min field of the Generic Power Range state. */
    UINT16 range_min;

    /** The value of the Generic Power Range Max field of the Generic Power Range state. */
    UINT16 range_max;

} MS_GENERIC_POWER_RANGE_STATUS_STRUCT;

/**
    Generic Power Default Status message parameters.
*/
typedef struct MS_generic_power_default_set_struct
{
    /** The value of the Generic Power Default state. */
    UINT16 power;

} MS_GENERIC_POWER_DEFAULT_SET_STRUCT;

/**
    Generic Power Range Set message parameters.
*/
typedef struct MS_generic_power_range_set_struct
{
    /** The value of the Generic Power Min field of the Generic Power Range state. */
    UINT16 range_min;

    /** The value of the Generic Power Range Max field of the Generic Power Range state. */
    UINT16 range_max;

} MS_GENERIC_POWER_RANGE_SET_STRUCT;
/** \} */



/* --------------------------------------------- Function */
/**
    \defgroup generic_power_level_api_defs API Definitions
    \{
    This section describes the EtherMind Mesh Generic Power Level Model APIs.
*/
/**
    \defgroup generic_power_level_ser_api_defs Generic Power Level Server API Definitions
    \{
    This section describes the Generic Power Level Server APIs.
*/

/**
    \brief API to initialize Generic_Power_Level Server model

    \par Description
    This is to initialize Generic_Power_Level Server model and to register with Acess layer.

    \param [in] element_handle
                Element identifier to be associated with the model instance.

    \param [in, out] model_handle
                     Model identifier associated with the model instance on successful initialization.
                     After power cycle of an already provisioned node, the model handle will have
                     valid value and the same will be reused for registration.

    \param [in] appl_cb    Application Callback to be used by the Generic_Power_Level Server.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_generic_power_level_server_init
(
    /* IN */    MS_ACCESS_ELEMENT_HANDLE    element_handle,
    /* INOUT */ MS_ACCESS_MODEL_HANDLE*     model_handle,
    /* IN */    MS_GENERIC_POWER_LEVEL_SERVER_CB appl_cb
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
API_RESULT MS_generic_power_level_server_state_update
(
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_CONTEXT*     ctx,
    /* IN */ MS_ACCESS_MODEL_STATE_PARAMS*        current_state_params,
    /* IN */ MS_ACCESS_MODEL_STATE_PARAMS*        target_state_params,
    /* IN */ UINT16                               remaining_time,
    /* IN */ MS_ACCESS_MODEL_EXT_PARAMS*          ext_params
);

/**
    \brief API to initialize Generic_Power_Level_Setup Server model

    \par Description
    This is to initialize Generic_Power_Level_Setup Server model and to register with Acess layer.

    \param [in] element_handle
                Element identifier to be associated with the model instance.

    \param [in, out] model_handle
                     Model identifier associated with the model instance on successful initialization.
                     After power cycle of an already provisioned node, the model handle will have
                     valid value and the same will be reused for registration.

    \param [in] appl_cb    Application Callback to be used by the Generic_Power_Level_Setup Server.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_generic_power_level_setup_server_init
(
    /* IN */    MS_ACCESS_ELEMENT_HANDLE    element_handle,
    /* INOUT */ MS_ACCESS_MODEL_HANDLE*     model_handle,
    /* IN */    MS_GENERIC_POWER_LEVEL_SETUP_SERVER_CB appl_cb
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
API_RESULT MS_generic_power_level_setup_server_state_update
(
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_CONTEXT*     ctx,
    /* IN */ MS_ACCESS_MODEL_STATE_PARAMS*        current_state_params,
    /* IN */ MS_ACCESS_MODEL_STATE_PARAMS*        target_state_params,
    /* IN */ UINT16                               remaining_time,
    /* IN */ MS_ACCESS_MODEL_EXT_PARAMS*          ext_params
);
/** \} */

/**
    \defgroup generic_power_level_cli_api_defs Generic Power Level Client API Definitions
    \{
    This section describes the Generic Power Level Client APIs.
*/

/**
    \brief API to initialize Generic_Power_Level Client model

    \par Description
    This is to initialize Generic_Power_Level Client model and to register with Acess layer.

    \param [in] element_handle
                Element identifier to be associated with the model instance.

    \param [in, out] model_handle
                     Model identifier associated with the model instance on successful initialization.
                     After power cycle of an already provisioned node, the model handle will have
                     valid value and the same will be reused for registration.

    \param [in] appl_cb    Application Callback to be used by the Generic_Power_Level Client.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_generic_power_level_client_init
(
    /* IN */    MS_ACCESS_ELEMENT_HANDLE    element_handle,
    /* INOUT */ MS_ACCESS_MODEL_HANDLE*     model_handle,
    /* IN */    MS_GENERIC_POWER_LEVEL_CLIENT_CB appl_cb
);

/**
    \brief API to get Generic_Power_Level client model handle

    \par Description
    This is to get the handle of Generic_Power_Level client model.

    \param [out] model_handle   Address of model handle to be filled/returned.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_generic_power_level_client_get_model_handle
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
API_RESULT MS_generic_power_level_client_send_reliable_pdu
(
    /* IN */ UINT32    req_opcode,
    /* IN */ void*     param,
    /* IN */ UINT32    rsp_opcode
);

/**
    \brief API to get the Generic Power Actual state of an element.

    \par Description
    Generic Power Level Get message is an acknowledged message used to get the Generic Power Actual state of an element.
    The response to the Generic Power Level Get message is a Generic Power Level Status message.
    There are no parameters for this message.

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_generic_power_level_get() \
    MS_generic_power_level_client_send_reliable_pdu \
    (\
     MS_ACCESS_GENERIC_POWER_LEVEL_GET_OPCODE,\
     NULL,\
     MS_ACCESS_GENERIC_POWER_LEVEL_STATUS_OPCODE\
    )

/**
    \brief API to set the Generic Power Actual state of an element.

    \par Description
    Generic Power Level Set is an acknowledged message used to set the Generic Power Actual state of an element.
    The response to the Generic Power Level Set message is a Generic Power Level Status message.

    \param [in] param Generic Power Level Set message

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_generic_power_level_set(param) \
    MS_generic_power_level_client_send_reliable_pdu \
    (\
     MS_ACCESS_GENERIC_POWER_LEVEL_SET_OPCODE,\
     param,\
     MS_ACCESS_GENERIC_POWER_LEVEL_STATUS_OPCODE\
    )

/**
    \brief API to set the Generic Power Actual state of an element.

    \par Description
    Generic Power Level Set Unacknowledged is an unacknowledged message used
    to set the Generic Power Actual state of an element.

    \param [in] param Generic Power Level Set message

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_generic_power_level_set_unacknowledged(param) \
    MS_generic_power_level_client_send_reliable_pdu \
    (\
     MS_ACCESS_GENERIC_POWER_LEVEL_SET_UNACKNOWLEDGED_OPCODE,\
     param,\
     0xFFFFFFFF\
    )

/**
    \brief API to get the Generic Power Last state of an element.

    \par Description
    Generic Power Last Get is an acknowledged message used to get the Generic Power Last state of an element.
    The response to a Generic Power Last Get message is a Generic Power Last Status message.
    There are no parameters for this message.

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_generic_power_last_get() \
    MS_generic_power_level_client_send_reliable_pdu \
    (\
     MS_ACCESS_GENERIC_POWER_LAST_GET_OPCODE,\
     NULL,\
     MS_ACCESS_GENERIC_POWER_LAST_STATUS_OPCODE\
    )

/**
    \brief API to get the Generic Power Default state of an element.

    \par Description
    Generic Power Default Get is an acknowledged message used to get the Generic Power Default state of an element.
    The response to a Generic Power Default Get message is a Generic Power Default Status message.
    There are no parameters for this message.

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_generic_power_default_get() \
    MS_generic_power_level_client_send_reliable_pdu \
    (\
     MS_ACCESS_GENERIC_POWER_DEFAULT_GET_OPCODE,\
     NULL,\
     MS_ACCESS_GENERIC_POWER_DEFAULT_STATUS_OPCODE\
    )

/**
    \brief API to set the Generic Power Default state of an element.

    \par Description
    Generic Power Default Set is an acknowledged message used to set the Generic Power Default state of an element.
    The response to the Generic Power Default Set message is a Generic Power Default Status message.

    \param [in] param Generic Power Default Set message

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_generic_power_default_set(param) \
    MS_generic_power_level_client_send_reliable_pdu \
    (\
     MS_ACCESS_GENERIC_POWER_DEFAULT_SET_OPCODE,\
     param,\
     MS_ACCESS_GENERIC_POWER_DEFAULT_STATUS_OPCODE\
    )

/**
    \brief API to set the Generic Power Default state of an element.

    \par Description
    Generic Power Default Set Unacknowledged is an unacknowledged message used to set
    the Generic Power Default state of an element.

    \param [in] param Generic Power Default Set message

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_generic_power_default_set_unacknowledged(param) \
    MS_generic_power_level_client_send_reliable_pdu \
    (\
     MS_ACCESS_GENERIC_POWER_DEFAULT_SET_UNACKNOWLEDGED_OPCODE,\
     param,\
     0xFFFFFFFF\
    )

/**
    \brief API to get the Generic Power Range state of an element.

    \par Description
    Generic Power Range Get is an acknowledged message used to get the Generic Power Range state of an element.
    The response to the Generic Power Range Get message is a Generic Power Range Status message.
    There are no parameters for this message.

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_generic_power_range_get() \
    MS_generic_power_level_client_send_reliable_pdu \
    (\
     MS_ACCESS_GENERIC_POWER_RANGE_GET_OPCODE,\
     NULL,\
     MS_ACCESS_GENERIC_POWER_RANGE_STATUS_OPCODE\
    )

/**
    \brief API to set the Generic Power Range state of an element.

    \par Description
    Generic Power Range Set is an acknowledged message used to set the Generic Power Range state of an element.
    The response to the Generic Power Range Set message is a Generic Power Range Status message.

    \param [in] param Generic Power Range Set message`

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_generic_power_range_set(param) \
    MS_generic_power_level_client_send_reliable_pdu \
    (\
     MS_ACCESS_GENERIC_POWER_RANGE_SET_OPCODE,\
     param,\
     MS_ACCESS_GENERIC_POWER_RANGE_STATUS_OPCODE\
    )

/**
    \brief API to set the Generic Power Range state of an element.

    \par Description
    Generic Power Range Set Unacknowledged is an unacknowledged message used to set
    the Generic Power Range state of an element.

    \param [in] param Generic Power Range Set message`

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_generic_power_range_set_unacknowledged(param) \
    MS_generic_power_level_client_send_reliable_pdu \
    (\
     MS_ACCESS_GENERIC_POWER_RANGE_SET_UNACKNOWLEDGED_OPCODE,\
     param,\
     0xFFFFFFFF\
    )
/** \} */
/** \} */
/** \} */

#endif /* _H_MS_GENERIC_POWER_LEVEL_API_ */
