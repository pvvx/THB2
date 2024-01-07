/**
    \file MS_generic_default_transition_time_api.h

    \brief This file defines the Mesh Generic Default Transition Time Model Application Interface
    - includes Data Structures and Methods for both Server and Client.
*/

/*
    Copyright (C) 2017. Mindtree Ltd.
    All rights reserved.
*/

#ifndef _H_MS_GENERIC_DEFAULT_TRANSITION_TIME_API_
#define _H_MS_GENERIC_DEFAULT_TRANSITION_TIME_API_


/* --------------------------------------------- Header File Inclusion */
#include "MS_access_api.h"
#include "MS_model_states.h"


/* --------------------------------------------- Global Definitions */
/**
    \defgroup generic_default_transition_time_module GENERIC_DEFAULT_TRANSITION_TIME (Mesh Generic Default Transition Time Model)
    \{
    This section describes the interfaces & APIs offered by the EtherMind
    Mesh Generic OnOff Model (ONOFF) module to the Application.
*/



/* --------------------------------------------- Data Types/ Structures */
/**
    \defgroup generic_default_transition_time_cb Application Callback
    \{
    This Section Describes the module Notification Callback interface offered
    to the application
*/

/**
    Generic Default Transition Time Server application Asynchronous Notification Callback.

    Generic Default Transition Time Server calls the registered callback to indicate events occurred to the
    application.

    \param [in] ctx           Context of the message received for a specific model instance.
    \param [in] msg_raw       Uninterpreted/raw received message.
    \param [in] req_type      Requested message type.
    \param [in] state_params  Model specific state parameters.
    \param [in] ext_params    Additional parameters.
*/
typedef API_RESULT (* MS_GENERIC_DEFAULT_TRANSITION_TIME_SERVER_CB)
(
    MS_ACCESS_MODEL_REQ_MSG_CONTEXT*     ctx,
    MS_ACCESS_MODEL_REQ_MSG_RAW*         msg_raw,
    MS_ACCESS_MODEL_REQ_MSG_T*           req_type,
    MS_ACCESS_MODEL_STATE_PARAMS*        state_params,
    MS_ACCESS_MODEL_EXT_PARAMS*          ext_params

) DECL_REENTRANT;

/**
    Generic Default Transition Time Client application Asynchronous Notification Callback.

    Generic Default Transition Time Client calls the registered callback to indicate events occurred to the
    application.

    \param handle        Model Handle.
    \param opcode        Opcode.
    \param data_param    Data associated with the event if any or NULL.
    \param data_len      Size of the event data. 0 if event data is NULL.
*/
typedef API_RESULT (* MS_GENERIC_DEFAULT_TRANSITION_TIME_CLIENT_CB)
(
    MS_ACCESS_MODEL_HANDLE* handle,
    UINT32                   opcode,
    UCHAR*                   data_param,
    UINT16                   data_len
) DECL_REENTRANT;
/** \} */

/**
    \defgroup generic_default_transition_time_structures Structures
    \{
*/

/**
    The Generic Default Transition Time state determines how long an element shall take to transition
    from a present state to a new state.
    This is a 1-octet value that consists of two fields:
    - a 2-bit bit field representing the step resolution
    - a 6-bit bit field representing the number of transition steps.

    This mechanism covers a wide range of times that may be required by different applications:
    - For 100 millisecond step resolution, the range is 0 through 6.2 seconds.
    - For 1 second step resolution, the range is 0 through 62 seconds.
    - For 10 seconds step resolution, the range is 0 through 620 seconds (10.5 minutes).
    -  For 10 minutes step resolution, the range is 0 through 620 minutes (10.5 hours).

    The Generic Default Transition Time is calculated using the following formula:
    Generic Default Transition Time = Default Transition Step Resolution * Default Transition Number of Steps
*/
typedef struct MS_generic_default_transition_time_struct
{
    /**
        The Default Transition Step Resolution field is a 2-bit bit field that determines
        the resolution of the Generic Default Transition Time state.

        Value | Description
        ------|------------
        0b00  | The Default Transition Step Resolution is 100 milliseconds
        0b01  | The Default Transition Step Resolution is 1 second
        0b10  | The Default Transition Step Resolution is 10 seconds
        0b11  | The Default Transition Step Resolution is 10 minutes
    */
    UCHAR  transition_number_of_steps;

    /**
        The Default Transition Number of Steps field is a 6-bit value representing
        the number of transition steps.

        Value     | Description
        ----------|------------
        0x00      | The Generic Default Transition Time is immediate.
        0x01-0x3E | The number of steps.
        0x3F      | The value is unknown. The state cannot be set to this value,
                 | but an element may report an unknown value if a transition is higher than 0x3E
                 | or not determined.
    */
    UCHAR  transition_step_resolution;

} MS_GENERIC_DEFAULT_TRANSITION_TIME_STRUCT;

/** \} */



/* --------------------------------------------- Function */
/**
    \defgroup generic_default_transition_time_api_defs API Definitions
    \{
    This section describes the EtherMind Mesh Generic Default Transition Time Model APIs.
*/
/**
    \defgroup generic_default_transition_time_ser_api_defs Generic Default Transition Time Server API Definitions
    \{
    This section describes the Generic Default Transition Time Server APIs.
*/

/**
    \brief API to initialize Generic_Default_Transition_Time Server model

    \par Description
    This is to initialize Generic_Default_Transition_Time Server model and to register with Acess layer.

    \param [in] element_handle
                Element identifier to be associated with the model instance.

    \param [in, out] model_handle
                     Model identifier associated with the model instance on successful initialization.
                     After power cycle of an already provisioned node, the model handle will have
                     valid value and the same will be reused for registration.

    \param [in] appl_cb    Application Callback to be used by the Generic_Default_Transition_Time Server.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_generic_default_transition_time_server_init
(
    /* IN */    MS_ACCESS_ELEMENT_HANDLE    element_handle,
    /* INOUT */ MS_ACCESS_MODEL_HANDLE*     model_handle,
    /* IN */    MS_GENERIC_DEFAULT_TRANSITION_TIME_SERVER_CB appl_cb
);

/**
    \brief API to get default transition time

    \par Description
    This is to get default transition time.

    \param [out] default_time    Default Transition Time.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_generic_default_transition_time_server_get_time
(
    /* OUT */ MS_STATE_GENERIC_DEFAULT_TRANSITION_TIME_STRUCT*     default_time
);
/** \} */

/**
    \defgroup generic_default_transition_time_cli_api_defs Generic Default Transition Time Client API Definitions
    \{
    This section describes the Generic Default Transition Time Client APIs.
*/

/**
    \brief API to initialize Generic_Default_Transition_Time Client model

    \par Description
    This is to initialize Generic_Default_Transition_Time Client model and to register with Acess layer.

    \param [in] element_handle
                Element identifier to be associated with the model instance.

    \param [in, out] model_handle
                     Model identifier associated with the model instance on successful initialization.
                     After power cycle of an already provisioned node, the model handle will have
                     valid value and the same will be reused for registration.

    \param [in] appl_cb    Application Callback to be used by the Generic_Default_Transition_Time Client.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_generic_default_transition_time_client_init
(
    /* IN */    MS_ACCESS_ELEMENT_HANDLE    element_handle,
    /* INOUT */ MS_ACCESS_MODEL_HANDLE*     model_handle,
    /* IN */    MS_GENERIC_DEFAULT_TRANSITION_TIME_CLIENT_CB appl_cb
);

/**
    \brief API to get Generic_Default_Transition_Time client model handle

    \par Description
    This is to get the handle of Generic_Default_Transition_Time client model.

    \param [out] model_handle   Address of model handle to be filled/returned.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_generic_default_transition_time_client_get_model_handle
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
API_RESULT MS_generic_default_transition_time_client_send_reliable_pdu
(
    /* IN */ UINT32    req_opcode,
    /* IN */ void*     param,
    /* IN */ UINT32    rsp_opcode
);

/**
    \brief API to get the Generic Default Transition Time state of an element.

    \par Description
    Generic Default Transition Time Get is an acknowledged message used to get
    the Generic Default Transition Time state of an element.
    The response to the Generic Default Transition Time Get message is a Generic Default
    Transition Time Status message.
    There are no parameters for this message.

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_generic_default_transition_time_get() \
    MS_generic_default_transition_time_client_send_reliable_pdu \
    (\
     MS_ACCESS_GENERIC_DEFAULT_TRANSITION_TIME_GET_OPCODE,\
     NULL,\
     MS_ACCESS_GENERIC_DEFAULT_TRANSITION_TIME_STATUS_OPCODE\
    )

/**
    \brief API to set the Generic Default Transition Time state of an element.

    \par Description
    Generic Default Transition Time Set is an acknowledged message used to set
    the Generic Default Transition Time state of an element.
    The response to the Generic Default Transition Time Set message is a Generic Default
    Transition Time Status message.

    \param [in] param Transition Time

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_generic_default_transition_time_set(param) \
    MS_generic_default_transition_time_client_send_reliable_pdu \
    (\
     MS_ACCESS_GENERIC_DEFAULT_TRANSITION_TIME_SET_OPCODE,\
     param,\
     MS_ACCESS_GENERIC_DEFAULT_TRANSITION_TIME_STATUS_OPCODE\
    )

/**
    \brief API to set the Generic Default Transition Time state of an element.

    \par Description
    Generic Default Transition Time Set Unacknowledged is an unacknowledged message used to set
    the Generic Default Transition Time state of an element.

    \param [in] param Transition Time

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_generic_default_transition_time_set_unacknowledged(param) \
    MS_generic_default_transition_time_client_send_reliable_pdu \
    (\
     MS_ACCESS_GENERIC_DEFAULT_TRANSITION_TIME_SET_UNACKNOWLEDGED_OPCODE,\
     param,\
     0xFFFFFFFF\
    )
/** \} */
/** \} */
/** \} */

#endif /*_H_MS_GENERIC_DEFAULT_TRANSITION_TIME_API_ */
