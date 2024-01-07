/**
    \file MS_light_xyl_api.h

    \brief This file defines the Mesh Light Xyl Model Application Interface
    - includes Data Structures and Methods for both Server and Client.
*/

/*
    Copyright (C) 2017. Mindtree Ltd.
    All rights reserved.
*/

#ifndef _H_MS_LIGHT_XYL_API_
#define _H_MS_LIGHT_XYL_API_


/* --------------------------------------------- Header File Inclusion */
#include "MS_access_api.h"


/* --------------------------------------------- Global Definitions */
/**
    \defgroup light_xyl_module LIGHT_XYL (Mesh Light Xyl Model)
    \{
    This section describes the interfaces & APIs offered by the EtherMind
    Mesh Generic OnOff Model (ONOFF) module to the Application.
*/



/* --------------------------------------------- Data Types/ Structures */
/**
    \defgroup light_xyl_cb Application Callback
    \{
    This Section Describes the module Notification Callback interface offered
    to the application
*/

/**
    Light Xyl Server application Asynchronous Notification Callback.

    Light Xyl Server calls the registered callback to indicate events occurred to the
    application.

    \param [in] ctx           Context of the message received for a specific model instance.
    \param [in] msg_raw       Uninterpreted/raw received message.
    \param [in] req_type      Requested message type.
    \param [in] state_params  Model specific state parameters.
    \param [in] ext_params    Additional parameters.
*/
typedef API_RESULT (* MS_LIGHT_XYL_SERVER_CB)
(
    MS_ACCESS_MODEL_REQ_MSG_CONTEXT*     ctx,
    MS_ACCESS_MODEL_REQ_MSG_RAW*         msg_raw,
    MS_ACCESS_MODEL_REQ_MSG_T*           req_type,
    MS_ACCESS_MODEL_STATE_PARAMS*        state_params,
    MS_ACCESS_MODEL_EXT_PARAMS*          ext_params

) DECL_REENTRANT;

/**
    Light Xyl Client application Asynchronous Notification Callback.

    Light Xyl Client calls the registered callback to indicate events occurred to the
    application.

    \param handle        Model Handle.
    \param opcode        Opcode.
    \param data_param    Data associated with the event if any or NULL.
    \param data_len      Size of the event data. 0 if event data is NULL.
*/
typedef API_RESULT (* MS_LIGHT_XYL_CLIENT_CB)
(
    MS_ACCESS_MODEL_HANDLE* handle,
    UINT32                   opcode,
    UCHAR*                   data_param,
    UINT16                   data_len
) DECL_REENTRANT;
/** \} */

/**
    \defgroup light_xyl_structures Structures
    \{
*/

/**
    Light xyL Set message parameters.
*/
typedef struct MS_light_xyl_set_struct
{
    /** The target value of the Light xyL Lightness state */
    UINT16 xyl_lightness;

    /** The target value of the Light xyL x state */
    UINT16 xyl_x;

    /** The target value of the Light xyL y state */
    UINT16 xyl_y;

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

} MS_LIGHT_XYL_SET_STRUCT;

/**
    Light xyL Status Unacknowledged message parameters.
*/
typedef struct MS_light_xyl_status_struct
{
    /** The present value of the Light xyL Lightness state */
    UINT16 xyl_lightness;

    /** The present value of the Light xyL x state */
    UINT16 xyl_x;

    /** The present value of the Light xyL y state */
    UINT16 xyl_y;

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

    /** Flag: To represent if optional field Remaining Time is valid */
    UCHAR optional_fields_present;

} MS_LIGHT_XYL_STATUS_STRUCT;

/**
    Light xyL Target Status Unacknowledged message parameters.
*/
typedef struct MS_light_xyl_target_status_struct
{
    /** The target value of the Light xyL Lightness state */
    UINT16 target_xyl_lightness;

    /** The target value of the Light xyL x state */
    UINT16 target_xyl_x;

    /** The target value of the Light xyL y state */
    UINT16 target_xyl_y;

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

    /** Flag: To represent if optional field Remaining Time is valid */
    UCHAR optional_fields_present;

} MS_LIGHT_XYL_TARGET_STATUS_STRUCT;

/**
    Light HSL Default Set message parameters.
*/
typedef struct MS_light_xyl_default_set_struct
{
    /** The value of the Light Lightness Default state */
    UINT16 lightness;

    /** The value of the Light xyL x Default state */
    UINT16 xyl_x;

    /** The value of the Light xyL y Default state */
    UINT16 xyl_y;

} MS_LIGHT_XYL_DEFAULT_SET_STRUCT;

/**
    Light xyL Default Status message parameters.
*/
typedef struct MS_light_xyl_default_status_struct
{
    /** The value of the Light Lightness Default state */
    UINT16 lightness;

    /** The value of the Light xyL x Default state */
    UINT16 xyl_x;

    /** The value of the Light xyL y Default state */
    UINT16 xyl_y;

} MS_LIGHT_XYL_DEFAULT_STATUS_STRUCT;

/**
    Light xyL Range Set message parameters.
*/
typedef struct MS_light_xyl_range_set_struct
{
    /** The value of the xyL x Range Min field of the Light xyL x Range state */
    UINT16 xyl_x_range_min;

    /** The value of the xyL x Range Max field of the Light xyL x Range state */
    UINT16 xyl_x_range_max;

    /** The value of the xyL y Range Min field of the Light xyL y Range state */
    UINT16 xyl_y_range_min;

    /** The value of the xyL y Range Max field of the Light xyL y Range state */
    UINT16 xyl_y_range_max;

} MS_LIGHT_XYL_RANGE_SET_STRUCT;

/**
    Light xyL Range Status message parameters.
*/
typedef struct MS_light_xyl_range_status_struct
{
    /** Status Code for the requesting message. */
    UCHAR  status_code;

    /** The value of the xyL x Range Min field of the Light xyL x Range state */
    UINT16 xyl_x_range_min;

    /** The value of the xyL x Range Max field of the Light xyL x Range state */
    UINT16 xyl_x_range_max;

    /** The value of the xyL y Range Min field of the Light xyL y Range state */
    UINT16 xyl_y_range_min;

    /** The value of the xyL y Range Max field of the Light xyL y Range state */
    UINT16 xyl_y_range_max;

} MS_LIGHT_XYL_RANGE_STATUS_STRUCT;

/** \} */



/* --------------------------------------------- Function */
/**
    \defgroup light_xyl_api_defs API Definitions
    \{
    This section describes the EtherMind Mesh Light Xyl Model APIs.
*/
/**
    \defgroup light_xyl_ser_api_defs Light Xyl Server API Definitions
    \{
    This section describes the Light Xyl Server APIs.
*/

/**
    \brief API to initialize Light_Xyl Server model

    \par Description
    This is to initialize Light_Xyl Server model and to register with Acess layer.

    \param [in] element_handle
                Element identifier to be associated with the model instance.

    \param [in, out] xyl_model_handle
                     Model identifier associated with the Light xyl model instance on successful initialization.
                     After power cycle of an already provisioned node, the model handle will have
                     valid value and the same will be reused for registration.

    \param [in, out] xyl_setup_model_handle
                     Model identifier associated with the Light xyl Setup model instance on successful initialization.
                     After power cycle of an already provisioned node, the model handle will have
                     valid value and the same will be reused for registration.

    \param [in] appl_cb    Application Callback to be used by the Light_Xyl Server.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_light_xyl_server_init
(
    /* IN */    MS_ACCESS_ELEMENT_HANDLE    element_handle,
    /* INOUT */ MS_ACCESS_MODEL_HANDLE*     xyl_model_handle,
    /* INOUT */ MS_ACCESS_MODEL_HANDLE*     xyl_setup_model_handle,
    /* IN */    MS_LIGHT_XYL_SERVER_CB appl_cb
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
API_RESULT MS_light_xyl_server_state_update
(
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_CONTEXT*     ctx,
    /* IN */ MS_ACCESS_MODEL_STATE_PARAMS*        current_state_params,
    /* IN */ MS_ACCESS_MODEL_STATE_PARAMS*        target_state_params,
    /* IN */ UINT16                               remaining_time,
    /* IN */ MS_ACCESS_MODEL_EXT_PARAMS*          ext_params
);
/** \} */

/**
    \defgroup light_xyl_cli_api_defs Light Xyl Client API Definitions
    \{
    This section describes the Light Xyl Client APIs.
*/

/**
    \brief API to initialize Light_Xyl Client model

    \par Description
    This is to initialize Light_Xyl Client model and to register with Acess layer.

    \param [in] element_handle
                Element identifier to be associated with the model instance.

    \param [in, out] model_handle
                     Model identifier associated with the model instance on successful initialization.
                     After power cycle of an already provisioned node, the model handle will have
                     valid value and the same will be reused for registration.

    \param [in] appl_cb    Application Callback to be used by the Light_Xyl Client.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_light_xyl_client_init
(
    /* IN */    MS_ACCESS_ELEMENT_HANDLE    element_handle,
    /* INOUT */ MS_ACCESS_MODEL_HANDLE*     model_handle,
    /* IN */    MS_LIGHT_XYL_CLIENT_CB appl_cb
);

/**
    \brief API to get Light_Xyl client model handle

    \par Description
    This is to get the handle of Light_Xyl client model.

    \param [out] model_handle   Address of model handle to be filled/returned.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_light_xyl_client_get_model_handle
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
API_RESULT MS_light_xyl_client_send_reliable_pdu
(
    /* IN */ UINT32    req_opcode,
    /* IN */ void*     param,
    /* IN */ UINT32    rsp_opcode
);

/**
    \brief API to get the Light xyL Lightness, Light xyL x, and Light xyL y states of an element.

    \par Description
    The Light xyL Get is an acknowledged message used to get the Light xyL
    Lightness, Light xyL x, and Light xyL y states of an element.
    Upon receiving a Light xyL Get message, the element shall respond with a Light xyL Status message.
    The response to the Light xyL Get message is a Light xyL Status message.
    There are no parameters for this message.

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_light_xyl_get() \
    MS_light_xyl_client_send_reliable_pdu \
    (\
     MS_ACCESS_LIGHT_XYL_GET_OPCODE,\
     NULL,\
     MS_ACCESS_LIGHT_XYL_STATUS_OPCODE\
    )

/**
    \brief API to set the Light xyL Lightness, Light xyL x state, and the Light xyL y states of an element.

    \par Description
    The Light xyL Set is an acknowledged message used to set the Light xyL Lightness, Light xyL x state, and the Light xyL y states of an element.
    The response to the Light xyL Set message is a Light xyL Status message.

    \param [in] param Light xyL Set message

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_light_xyl_set(param) \
    MS_light_xyl_client_send_reliable_pdu \
    (\
     MS_ACCESS_LIGHT_XYL_SET_OPCODE,\
     param,\
     MS_ACCESS_LIGHT_XYL_STATUS_OPCODE\
    )

/**
    \brief API to set the Light xyL Lightness, Light xyL x state, and the Light xyL y states of an element.

    \par Description
    The Light xyL Set Unacknowledged is an unacknowledged message used to set
    the Light xyL Lightness, Light xyL x, and the Light xyL y states of an
    element.

    \param [in] param Light xyL Set message

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_light_xyl_set_unacknowledged(param) \
    MS_light_xyl_client_send_reliable_pdu \
    (\
     MS_ACCESS_LIGHT_XYL_SET_UNACKNOWLEDGED_OPCODE,\
     param,\
     0xFFFFFFFF\
    )

/**
    \brief API to get the target Light xyL Lightness, Light xyL x, and Light xyL y states of an element.

    \par Description
    The Light xyL Target Get is an acknowledged message used to get the target Light xyL Lightness, Light xyL x, and Light xyL y states of an element.
    The response to the Light xyL Target Get message is a Light xyL Target Status message.
    There are no parameters for this message.

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_light_xyl_target_get() \
    MS_light_xyl_client_send_reliable_pdu \
    (\
     MS_ACCESS_LIGHT_XYL_TARGET_GET_OPCODE,\
     NULL,\
     MS_ACCESS_LIGHT_XYL_TARGET_STATUS_OPCODE\
    )

/**
    \brief API to get the Light Lightness Default, the Light xyL x Default, and Light xyL y Default states of an element.

    \par Description
    Light xyL Default Get is an acknowledged message used to get the Light Lightness Default, the Light xyL x Default, and Light xyL y Default states of an element.
    The response to the Light xyL Default Get message is a Light xyL Default Status message.
    There are no parameters for this message.

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_light_xyl_default_get() \
    MS_light_xyl_client_send_reliable_pdu \
    (\
     MS_ACCESS_LIGHT_XYL_DEFAULT_GET_OPCODE,\
     NULL,\
     MS_ACCESS_LIGHT_XYL_DEFAULT_STATUS_OPCODE\
    )

/**
    \brief API to set the Light Lightness Default, the Light xyL x Default, and Light xyL y Default states of an element.

    \par Description
    Light xyL Default Set is an acknowledged message used to set the Light Lightness Default, the Light xyL x Default, and Light xyL y Default states of an element.
    The response to the Light xyL Default Set message is a Light xyL Default Status message.

    \param [in] param Light HSL Default Set message

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_light_xyl_default_set(param) \
    MS_light_xyl_client_send_reliable_pdu \
    (\
     MS_ACCESS_LIGHT_XYL_DEFAULT_SET_OPCODE,\
     param,\
     MS_ACCESS_LIGHT_XYL_DEFAULT_STATUS_OPCODE\
    )

/**
    \brief API to set the Light Lightness Default, the Light xyL x Default, and Light xyL y Default states of an element.

    \par Description
    Light xyL Default Set Unacknowledged is an unacknowledged message used to
    set the Light Lightness Default, the Light xyL x Default, and Light xyL y
    Default states of an element.

    \param [in] param Light HSL Default Set message

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_light_xyl_default_set_unacknowledged(param) \
    MS_light_xyl_client_send_reliable_pdu \
    (\
     MS_ACCESS_LIGHT_XYL_DEFAULT_SET_UNACKNOWLEDGED_OPCODE,\
     param,\
     0xFFFFFFFF\
    )

/**
    \brief API to get the Light xyL x Range and Light xyL y Range states of an element.

    \par Description
    The Light xyL Range Get is an acknowledged message used to get the Light xyL x Range and Light xyL y Range states of an element.
    The response to the Light xyL Range Get message is a Light xyL Range Status message.
    There are no parameters for this message.

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_light_xyl_range_get() \
    MS_light_xyl_client_send_reliable_pdu \
    (\
     MS_ACCESS_LIGHT_XYL_RANGE_GET_OPCODE,\
     NULL,\
     MS_ACCESS_LIGHT_XYL_RANGE_STATUS_OPCODE\
    )

/**
    \brief API to set the Light xyL x Range and Light xyL y Range states of an element.

    \par Description
    Light xyL Range Set is an acknowledged message used to set the Light xyL x Range and Light xyL y Range states of an element.
    The response to the Light xyL Range Set message is a Light xyL Range Status message.

    \param [in] param Light xyL Range Set message

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_light_xyl_range_set(param) \
    MS_light_xyl_client_send_reliable_pdu \
    (\
     MS_ACCESS_LIGHT_XYL_RANGE_SET_OPCODE,\
     param,\
     MS_ACCESS_LIGHT_XYL_RANGE_STATUS_OPCODE\
    )

/**
    \brief API to set the Light xyL x Range and Light xyL y Range states of an element.

    \par Description
    Light xyL Range Set Unacknowledged is an unacknowledged message used to set
    the Light xyL x Range and Light xyL y Range states of an element.

    \param [in] param Light xyL Range Set message

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_light_xyl_range_set_unacknowledged(param) \
    MS_light_xyl_client_send_reliable_pdu \
    (\
     MS_ACCESS_LIGHT_XYL_RANGE_SET_UNACKNOWLEDGED_OPCODE,\
     param,\
     0xFFFFFFFF\
    )
/** \} */
/** \} */
/** \} */

#endif /*_H_MS_LIGHT_XYL_API_ */
