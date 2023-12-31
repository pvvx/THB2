/**
    \file MS_light_lc_api.h

    \brief This file defines the Mesh Light Lc Model Application Interface
    - includes Data Structures and Methods for both Server and Client.
*/

/*
    Copyright (C) 2017. Mindtree Ltd.
    All rights reserved.
*/

#ifndef _H_MS_LIGHT_LC_API_
#define _H_MS_LIGHT_LC_API_


/* --------------------------------------------- Header File Inclusion */
#include "MS_access_api.h"


/* --------------------------------------------- Global Definitions */
/**
    \defgroup light_lc_module LIGHT_LC (Mesh Light Lc Model)
    \{
    This section describes the interfaces & APIs offered by the EtherMind
    Mesh Generic OnOff Model (ONOFF) module to the Application.
*/



/* --------------------------------------------- Data Types/ Structures */
/**
    \defgroup light_lc_cb Application Callback
    \{
    This Section Describes the module Notification Callback interface offered
    to the application
*/

/**
    Light Lc Server application Asynchronous Notification Callback.

    Light Lc Server calls the registered callback to indicate events occurred to the
    application.

    \param [in] ctx           Context of the message received for a specific model instance.
    \param [in] msg_raw       Uninterpreted/raw received message.
    \param [in] req_type      Requested message type.
    \param [in] state_params  Model specific state parameters.
    \param [in] ext_params    Additional parameters.
*/
typedef API_RESULT (* MS_LIGHT_LC_SERVER_CB)
(
    MS_ACCESS_MODEL_REQ_MSG_CONTEXT*     ctx,
    MS_ACCESS_MODEL_REQ_MSG_RAW*         msg_raw,
    MS_ACCESS_MODEL_REQ_MSG_T*           req_type,
    MS_ACCESS_MODEL_STATE_PARAMS*        state_params,
    MS_ACCESS_MODEL_EXT_PARAMS*          ext_params

) DECL_REENTRANT;

/**
    Light Lc Client application Asynchronous Notification Callback.

    Light Lc Client calls the registered callback to indicate events occurred to the
    application.

    \param handle        Model Handle.
    \param opcode        Opcode.
    \param data_param    Data associated with the event if any or NULL.
    \param data_len      Size of the event data. 0 if event data is NULL.
*/
typedef API_RESULT (* MS_LIGHT_LC_CLIENT_CB)
(
    MS_ACCESS_MODEL_HANDLE* handle,
    UINT32                   opcode,
    UCHAR*                   data_param,
    UINT16                   data_len
) DECL_REENTRANT;
/** \} */

/**
    \defgroup light_lc_structures Structures
    \{
*/

/**
    Light LC Mode Set/Status message parameters.
*/
typedef struct MS_light_lc_mode_struct
{
    /** The target value of the Light LC Mode state */
    UCHAR  mode;

} MS_LIGHT_LC_MODE_STRUCT;

typedef struct MS_light_lc_om_struct
{
    /** The target value of the Light LC Occupancy Mode state */
    UCHAR  mode;

} MS_LIGHT_LC_OM_STRUCT;

/**
    Light LC Light OnOff Set message parameters.
*/
typedef struct MS_light_lc_light_onoff_set_struct
{
    /** The target value of the Light LC Light OnOff state */
    UCHAR  light_onoff;

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

} MS_LIGHT_LC_LIGHT_ONOFF_SET_STRUCT;

/**
    Light LC Light OnOff Status message parameters.
*/
typedef struct MS_light_lc_light_onoff_status_struct
{
    /** The present value of the Light LC Light OnOff state */
    UCHAR  present_light_onoff;

    /** The target value of the Light LC Light OnOff state (Optional) */
    UCHAR  target_light_onoff;

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

    /** Flag: To represent if optional fields Target LC Light OnOff and Remaining Time are valid */
    UCHAR optional_fields_present;

} MS_LIGHT_LC_LIGHT_ONOFF_STATUS_STRUCT;

/**
    Light LC Property Get message parameters.
*/
typedef struct MS_light_lc_property_get_struct
{
    /** Property ID identifying a Light LC Property. */
    UINT16 light_lc_property_id;

} MS_LIGHT_LC_PROPERTY_GET_STRUCT;

/**
    Light LC Property Set message parameters.
*/
typedef struct MS_light_lc_property_set_struct
{
    /** Property ID identifying a Light LC Property. */
    UINT16 light_lc_property_id;

    /** Raw value for the Light LC Property */
    UCHAR* light_lc_property_value;

    /** Raw value length for the Light LC Property */
    UINT16 light_lc_property_value_len;

} MS_LIGHT_LC_PROPERTY_SET_STRUCT;

/**
    Light LC Property Status message parameters.
*/
typedef struct MS_light_lc_property_status_struct
{
    /** Property ID identifying a Light LC Property. */
    UINT16 light_lc_property_id;

    /** Raw value for the Light LC Property */
    UCHAR* light_lc_property_value;

    /** Raw value length for the Light LC Property */
    UINT16 light_lc_property_value_len;

} MS_LIGHT_LC_PROPERTY_STATUS_STRUCT;

/** \} */



/* --------------------------------------------- Function */
/**
    \defgroup light_lc_api_defs API Definitions
    \{
    This section describes the EtherMind Mesh Light Lc Model APIs.
*/
/**
    \defgroup light_lc_ser_api_defs Light Lc Server API Definitions
    \{
    This section describes the Light Lc Server APIs.
*/

/**
    \brief API to initialize Light_Lc Server model

    \par Description
    This is to initialize Light_Lc Server model and to register with Acess layer.

    \param [in] element_handle
                Element identifier to be associated with the model instance.

    \param [in, out] lc_model_handle
                     Model identifier associated with the Light LC model instance on successful initialization.
                     After power cycle of an already provisioned node, the model handle will have
                     valid value and the same will be reused for registration.

    \param [in, out] lc_setup_model_handle
                     Model identifier associated with the Light LC Setup model instance on successful initialization.
                     After power cycle of an already provisioned node, the model handle will have
                     valid value and the same will be reused for registration.

    \param [in] appl_cb    Application Callback to be used by the Light_Lc Server.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_light_lc_server_init
(
    /* IN */    MS_ACCESS_ELEMENT_HANDLE    element_handle,
    /* INOUT */ MS_ACCESS_MODEL_HANDLE*     lc_model_handle,
    /* INOUT */ MS_ACCESS_MODEL_HANDLE*     lc_setup_model_handle,
    /* IN */    MS_LIGHT_LC_SERVER_CB appl_cb
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
API_RESULT MS_light_lc_server_state_update
(
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_CONTEXT*     ctx,
    /* IN */ MS_ACCESS_MODEL_STATE_PARAMS*        current_state_params,
    /* IN */ MS_ACCESS_MODEL_STATE_PARAMS*        target_state_params,
    /* IN */ UINT16                               remaining_time,
    /* IN */ MS_ACCESS_MODEL_EXT_PARAMS*          ext_params
);
/** \} */

/**
    \defgroup light_lc_cli_api_defs Light Lc Client API Definitions
    \{
    This section describes the Light Lc Client APIs.
*/

/**
    \brief API to initialize Light_Lc Client model

    \par Description
    This is to initialize Light_Lc Client model and to register with Acess layer.

    \param [in] element_handle
                Element identifier to be associated with the model instance.

    \param [in, out] model_handle
                     Model identifier associated with the model instance on successful initialization.
                     After power cycle of an already provisioned node, the model handle will have
                     valid value and the same will be reused for registration.

    \param [in] appl_cb    Application Callback to be used by the Light_Lc Client.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_light_lc_client_init
(
    /* IN */    MS_ACCESS_ELEMENT_HANDLE    element_handle,
    /* INOUT */ MS_ACCESS_MODEL_HANDLE*     model_handle,
    /* IN */    MS_LIGHT_LC_CLIENT_CB appl_cb
);

/**
    \brief API to get Light_Lc client model handle

    \par Description
    This is to get the handle of Light_Lc client model.

    \param [out] model_handle   Address of model handle to be filled/returned.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_light_lc_client_get_model_handle
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
API_RESULT MS_light_lc_client_send_reliable_pdu
(
    /* IN */ UINT32    req_opcode,
    /* IN */ void*     param,
    /* IN */ UINT32    rsp_opcode
);

/**
    \brief API to get the Light LC Mode state of an element.

    \par Description
    Light LC Mode Get is an acknowledged message used to get the Light LC Mode state of an element.
    The response to the Light LC Mode Get message is a Light LC Mode Status message.
    There are no parameters for this message.

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_light_lc_mode_get() \
    MS_light_lc_client_send_reliable_pdu \
    (\
     MS_ACCESS_LIGHT_LC_MODE_GET_OPCODE,\
     NULL,\
     MS_ACCESS_LIGHT_LC_MODE_STATUS_OPCODE\
    )

/**
    \brief API to set the Light LC Mode state of an element.

    \par Description
    The Light LC Mode Set is an acknowledged message used to set the Light LC Mode state of an element.
    The response to the Light LC Mode Set message is a Light LC Mode Status message.

    \param [in] param Light LC Mode Set message

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_light_lc_mode_set(param) \
    MS_light_lc_client_send_reliable_pdu \
    (\
     MS_ACCESS_LIGHT_LC_MODE_SET_OPCODE,\
     param,\
     MS_ACCESS_LIGHT_LC_MODE_STATUS_OPCODE\
    )

/**
    \brief API to set the Light LC Mode state of an element.

    \par Description
    The Light LC Mode Set Unacknowledged is an unacknowledged message used to
    set the Light LC Mode state of an element.

    \param [in] param Light LC Mode Set message

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_light_lc_mode_set_unacknowledged(param) \
    MS_light_lc_client_send_reliable_pdu \
    (\
     MS_ACCESS_LIGHT_LC_MODE_SET_UNACKNOWLEDGED_OPCODE,\
     param,\
     0xFFFFFFFF\
    )

/**
    \brief API to get the Light LC Occupancy Mode state of an element.

    \par Description
    Light LC OM Get is an acknowledged message used to get the Light LC Occupancy Mode state of an element.
    The response to the Light LC OM Get message is a Light LC OM Status message.
    There are no parameters for this message.

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_light_lc_om_get() \
    MS_light_lc_client_send_reliable_pdu \
    (\
     MS_ACCESS_LIGHT_LC_OM_GET_OPCODE,\
     NULL,\
     MS_ACCESS_LIGHT_LC_OM_STATUS_OPCODE\
    )

/**
    \brief API to set the Light LC Occupancy Mode state of an element.

    \par Description
    The Light LC OM Set is an acknowledged message used to set the Light LC Occupancy Mode state of an element.
    The response to the Light LC OM Set message is a Light LC OM Status message.

    \param [in] param Light LC OM Set message

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_light_lc_om_set(param) \
    MS_light_lc_client_send_reliable_pdu \
    (\
     MS_ACCESS_LIGHT_LC_OM_SET_OPCODE,\
     param,\
     MS_ACCESS_LIGHT_LC_OM_STATUS_OPCODE\
    )

/**
    \brief API to set the Light LC Occupancy Mode state of an element.

    \par Description
    The Light LC OM Set Unacknowledged is an unacknowledged message used to set
    the Light LC Occupancy Mode state of an element.

    \param [in] param Light LC OM Set message

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_light_lc_om_set_unacknowledged(param) \
    MS_light_lc_client_send_reliable_pdu \
    (\
     MS_ACCESS_LIGHT_LC_OM_SET_UNACKNOWLEDGED_OPCODE,\
     param,\
     0xFFFFFFFF\
    )

/**
    \brief API to get the Light LC Light OnOff state of an element.

    \par Description
    Light LC Light OnOff Get is an acknowledged message used to get the Light LC Light OnOff state of an element.
    The response to the Light LC Light OnOff Get message is a Light LC Light OnOff Status message.
    There are no parameters for this message.

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_light_lc_light_onoff_get() \
    MS_light_lc_client_send_reliable_pdu \
    (\
     MS_ACCESS_LIGHT_LC_LIGHT_ONOFF_GET_OPCODE,\
     NULL,\
     MS_ACCESS_LIGHT_LC_LIGHT_ONOFF_STATUS_OPCODE\
    )

/**
    \brief API to set the Light LC Light OnOff state of an element.

    \par Description
    The Light LC Light OnOff Set is an acknowledged message used to set the Light LC Light OnOff state of an element.
    The response to the Light LC Light OnOff Set message is a Light LC Light OnOff Status message.

    \param [in] param Light LC Light OnOff Set message

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_light_lc_light_onoff_set(param) \
    MS_light_lc_client_send_reliable_pdu \
    (\
     MS_ACCESS_LIGHT_LC_LIGHT_ONOFF_SET_OPCODE,\
     param,\
     MS_ACCESS_LIGHT_LC_LIGHT_ONOFF_STATUS_OPCODE\
    )

/**
    \brief API to set the Light LC Light OnOff state of an element.

    \par Description
    The Light LC Light OnOff Set Unacknowledged is an unacknowledged message
    used to set the Light LC Light OnOff state of an element.

    \param [in] param Light LC Light OnOff Set message

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_light_lc_light_onoff_set_unacknowledged(param) \
    MS_light_lc_client_send_reliable_pdu \
    (\
     MS_ACCESS_LIGHT_LC_LIGHT_ONOFF_SET_UNACKNOWLEDGED_OPCODE,\
     param,\
     0xFFFFFFFF\
    )

/**
    \brief API to to get the Light LC Property state of an element.

    \par Description
    Light LC Property Get is an acknowledged message used to get the Light LC Property state of an element.
    The response to the Light LC Property Get message is a Light LC Property Status message.

    \param [in] param Light LC Property Get message

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_light_lc_property_get(param) \
    MS_light_lc_client_send_reliable_pdu \
    (\
     MS_ACCESS_LIGHT_LC_PROPERTY_GET_OPCODE,\
     param,\
     MS_ACCESS_LIGHT_LC_PROPERTY_STATUS_OPCODE\
    )

/**
    \brief API to set the Light LC Property state of an element.

    \par Description
    The Light LC Property Set is an acknowledged message used to set the Light LC Property state of an element.
    The response to the Light LC Property Set message is a Light LC Property Status message.

    \param [in] param Light LC Property Set message

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_light_lc_property_set(param) \
    MS_light_lc_client_send_reliable_pdu \
    (\
     MS_ACCESS_LIGHT_LC_PROPERTY_SET_OPCODE,\
     param,\
     MS_ACCESS_LIGHT_LC_PROPERTY_STATUS_OPCODE\
    )

/**
    \brief API to set the Light LC Property state of an element.

    \par Description
    The Light LC Property Set Unacknowledged is an unacknowledged message used
    to set the Light LC Property state of an element.

    \param [in] param Light LC Property Set message

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_light_lc_property_set_unacknowledged(param) \
    MS_light_lc_client_send_reliable_pdu \
    (\
     MS_ACCESS_LIGHT_LC_PROPERTY_SET_UNACKNOWLEDGED_OPCODE,\
     param,\
     0xFFFFFFFF\
    )
/** \} */
/** \} */
/** \} */

#endif /*_H_MS_LIGHT_LC_API_ */
