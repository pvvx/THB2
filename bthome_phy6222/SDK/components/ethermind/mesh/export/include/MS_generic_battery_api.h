/**
    \file MS_generic_battery_api.h

    \brief This file defines the Mesh Generic Battery Model Application Interface
    - includes Data Structures and Methods for both Server and Client.
*/

/*
    Copyright (C) 2017. Mindtree Ltd.
    All rights reserved.
*/

#ifndef _H_MS_GENERIC_BATTERY_API_
#define _H_MS_GENERIC_BATTERY_API_


/* --------------------------------------------- Header File Inclusion */
#include "MS_access_api.h"


/* --------------------------------------------- Global Definitions */
/**
    \defgroup generic_battery_module GENERIC_BATTERY (Mesh Generic Battery Model)
    \{
    This section describes the interfaces & APIs offered by the EtherMind
    Mesh Generic OnOff Model (ONOFF) module to the Application.
*/



/* --------------------------------------------- Data Types/ Structures */
/**
    \defgroup generic_battery_cb Application Callback
    \{
    This Section Describes the module Notification Callback interface offered
    to the application
*/

/**
    Generic Battery Server application Asynchronous Notification Callback.

    Generic Battery Server calls the registered callback to indicate events occurred to the
    application.

    \param [in] ctx           Context of the message received for a specific model instance.
    \param [in] msg_raw       Uninterpreted/raw received message.
    \param [in] req_type      Requested message type.
    \param [in] state_params  Model specific state parameters.
    \param [in] ext_params    Additional parameters.
*/
typedef API_RESULT (* MS_GENERIC_BATTERY_SERVER_CB)
(
    MS_ACCESS_MODEL_REQ_MSG_CONTEXT*     ctx,
    MS_ACCESS_MODEL_REQ_MSG_RAW*         msg_raw,
    MS_ACCESS_MODEL_REQ_MSG_T*           req_type,
    MS_ACCESS_MODEL_STATE_PARAMS*        state_params,
    MS_ACCESS_MODEL_EXT_PARAMS*          ext_params

) DECL_REENTRANT;

/**
    Generic Battery Client application Asynchronous Notification Callback.

    Generic Battery Client calls the registered callback to indicate events occurred to the
    application.

    \param handle        Model Handle.
    \param opcode        Opcode.
    \param data_param    Data associated with the event if any or NULL.
    \param data_len      Size of the event data. 0 if event data is NULL.
*/
typedef API_RESULT (* MS_GENERIC_BATTERY_CLIENT_CB)
(
    MS_ACCESS_MODEL_HANDLE* handle,
    UINT32                   opcode,
    UCHAR*                   data_param,
    UINT16                   data_len
) DECL_REENTRANT;
/** \} */

/**
    \defgroup generic_battery_structures Structures
    \{
*/

/**
    The Generic Battery state is a set of four values representing the state of a battery:
    - a charge level (Generic Battery Level)
    - remaining time to complete discharging (Generic Battery Time to Discharge)
    - remaining time to complete charging (Generic Battery Time to Charge)
    - flags bit field (Generic Battery Flags)
*/
typedef struct MS_generic_battery_status_struct
{
    /**
        Generic Battery Level.
        The Generic Battery Level state is a value ranging from 0 percent through 100 percent.

        Value     | Description
        ----------|------------
        0x00-0x64 | The percentage of the charge level. 100% represents fully charged. 0% represents fully discharged.
        0x65-0xFE | Prohibited
        0xFF      | The percentage of the charge level is unknown.
    */
    UCHAR  battery_level;

    /**
        The Generic Battery Time to Discharge state is a 24-bit unsigned value ranging from 0 through 0xFFFFFF.

        Value             | Description
        ------------------|------------
        0x000000-0xFFFFFE | The remaining time (in minutes) of the discharging process
        0xFFFFFF          | The remaining time of the discharging process is not known.
    */
    UINT32 time_to_discharge;

    /**
        The Generic Battery Time to Charge state is a 24-bit unsigned value ranging from 0 through 0xFFFFFF.

        Value             | Description
        ------------------|------------
        0x000000-0xFFFFFE | The remaining time (in minutes) of the charging process
        0xFFFFFF          | The remaining time of the charging process is not known.
    */
    UINT32 time_to_charge;

    /**
        The Generic Battery Flags state is a concatenation of four 2-bit bit fields: Presence, Indicator, Charging, and Serviceability

        Bit  | Description
        -----|------------
        0-1  | Generic Battery Flags Presence
        2-3  | Generic Battery Flags Indicator
        4-5  | Generic Battery Flags Charging
        6-7  | Generic Battery Flags Serviceability
    */
    UCHAR  flags;

} MS_GENERIC_BATTERY_STATUS_STRUCT;

/** \} */



/* --------------------------------------------- Function */
/**
    \defgroup generic_battery_api_defs API Definitions
    \{
    This section describes the EtherMind Mesh Generic Battery Model APIs.
*/
/**
    \defgroup generic_battery_ser_api_defs Generic Battery Server API Definitions
    \{
    This section describes the Generic Battery Server APIs.
*/

/**
    \brief API to initialize Generic_Battery Server model

    \par Description
    This is to initialize Generic_Battery Server model and to register with Acess layer.

    \param [in] element_handle
                Element identifier to be associated with the model instance.

    \param [in, out] model_handle
                     Model identifier associated with the model instance on successful initialization.
                     After power cycle of an already provisioned node, the model handle will have
                     valid value and the same will be reused for registration.

    \param [in] appl_cb    Application Callback to be used by the Generic_Battery Server.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_generic_battery_server_init
(
    /* IN */    MS_ACCESS_ELEMENT_HANDLE    element_handle,
    /* INOUT */ MS_ACCESS_MODEL_HANDLE*     model_handle,
    /* IN */    MS_GENERIC_BATTERY_SERVER_CB appl_cb
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
API_RESULT MS_generic_battery_server_state_update
(
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_CONTEXT*     ctx,
    /* IN */ MS_ACCESS_MODEL_STATE_PARAMS*        current_state_params,
    /* IN */ MS_ACCESS_MODEL_STATE_PARAMS*        target_state_params,
    /* IN */ UINT16                               remaining_time,
    /* IN */ MS_ACCESS_MODEL_EXT_PARAMS*          ext_params
);
/** \} */

/**
    \defgroup generic_battery_cli_api_defs Generic Battery Client API Definitions
    \{
    This section describes the Generic Battery Client APIs.
*/

/**
    \brief API to initialize Generic_Battery Client model

    \par Description
    This is to initialize Generic_Battery Client model and to register with Acess layer.

    \param [in] element_handle
                Element identifier to be associated with the model instance.

    \param [in, out] model_handle
                     Model identifier associated with the model instance on successful initialization.
                     After power cycle of an already provisioned node, the model handle will have
                     valid value and the same will be reused for registration.

    \param [in] appl_cb    Application Callback to be used by the Generic_Battery Client.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_generic_battery_client_init
(
    /* IN */    MS_ACCESS_ELEMENT_HANDLE    element_handle,
    /* INOUT */ MS_ACCESS_MODEL_HANDLE*     model_handle,
    /* IN */    MS_GENERIC_BATTERY_CLIENT_CB appl_cb
);

/**
    \brief API to get Generic_Battery client model handle

    \par Description
    This is to get the handle of Generic_Battery client model.

    \param [out] model_handle   Address of model handle to be filled/returned.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_generic_battery_client_get_model_handle
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
API_RESULT MS_generic_battery_client_send_reliable_pdu
(
    /* IN */ UINT32    req_opcode,
    /* IN */ void*     param,
    /* IN */ UINT32    rsp_opcode
);

/**
    \brief API to get the Generic Battery state of an element.

    \par Description
    Generic Battery Get message is an acknowledged message used to get the Generic Battery state of an element.
    The response to the Generic Battery Get message is a Generic Battery Status message.
    There are no parameters for this message.

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_generic_battery_get() \
    MS_generic_battery_client_send_reliable_pdu \
    (\
     MS_ACCESS_GENERIC_BATTERY_GET_OPCODE,\
     NULL,\
     MS_ACCESS_GENERIC_BATTERY_STATUS_OPCODE\
    )
/** \} */
/** \} */
/** \} */

#endif /*_H_MS_GENERIC_BATTERY_API_ */
