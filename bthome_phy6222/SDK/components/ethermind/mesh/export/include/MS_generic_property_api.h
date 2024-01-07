/**
    \file MS_generic_property_api.h

    \brief This file defines the Mesh Generic User, Admin, Manufacturer and Client Property Model Application Interface
    - includes Data Structures and Methods for both Server and Client.
*/

/*
    Copyright (C) 2017. Mindtree Ltd.
    All rights reserved.
*/

#ifndef _H_MS_GENERIC_PROPERTY_API_
#define _H_MS_GENERIC_PROPERTY_API_


/* --------------------------------------------- Header File Inclusion */
#include "MS_access_api.h"


/* --------------------------------------------- Global Definitions */
/**
    \defgroup generic_property_module GENERIC_PROPERTY (Mesh Generic Property Model)
    \{
    This section describes the interfaces & APIs offered by the EtherMind
    Mesh Generic Property Model (GENERIC_PROPERTY) module to the Application.
*/

/** Generic Manufacturer Properties */
#define MS_GENERIC_PROP_TYPE_MANUFACTURER    0x00

/** Generic Admin Properties */
#define MS_GENERIC_PROP_TYPE_ADMIN           0x01

/** Generic User Properties */
#define MS_GENERIC_PROP_TYPE_USER            0x02

/** User Access field values */
/** User Access - Prohibited */
#define MS_GENERIC_USER_ACCESS_PROHIBITED    0x00
/** User Access - the device property can be read */
#define MS_GENERIC_USER_ACCESS_READ          0x01
/** User Access - the device property can be written */
#define MS_GENERIC_USER_ACCESS_WRITE         0x02
/** User Access - the device property can be read and written */
#define MS_GENERIC_USER_ACCESS_READ_WRITE    0x03

#define MS_GENERIC_USER_ACCESS_INVALID_PROPERTY_ID 0xFF

/** Device Property - Light Control Time Occupancy Delay */
#define MS_DEV_PROP_LIGHT_CONTROL_TIME_OCCUPANCY_DELAY          0x003A

/** Device Property -  Light Control Time Fade On */
#define MS_DEV_PROP_LIGHT_CONTROL_TIME_FADE_ON                  0x0037

/** Device Property - Light Control Time Run On */
#define MS_DEV_PROP_LIGHT_CONTROL_TIME_RUN_ON                   0x003C

/** Device Property - Light Control Time Fade */
#define MS_DEV_PROP_LIGHT_CONTROL_TIME_FADE                     0x0036

/** Device Property - Light Control Time Prolong */
#define MS_DEV_PROP_LIGHT_CONTROL_TIME_PROLONG                  0x003B

/** Device Property - Light Control Time Fade Standby Auto */
#define MS_DEV_PROP_LIGHT_CONTROL_TIME_FADE_STANDBY_AUTO        0x0038

/** Device Property - Light Control Time Fade Standby Manual */
#define MS_DEV_PROP_LIGHT_CONTROL_TIME_FADE_STANDBY_MANUAL      0x0039

/** Device Property - Light Control Lightness On */
#define MS_DEV_PROP_LIGHT_CONTROL_LIGHTNESS_ON                  0x002E

/** Device Property - Light Control Lightness Prolong */
#define MS_DEV_PROP_LIGHT_CONTROL_LIGHTNESS_PROLONG             0x002F

/** Device Property - Light Control Lightness Standby */
#define MS_DEV_PROP_LIGHT_CONTROL_LIGHTNESS_STANDBY             0x0030

/** Device Property - Light Control Ambient LuxLevel On */
#define MS_DEV_PROP_LIGHT_CONTROL_AMBIENT_LUXLEVEL_ON           0x002B

/** Device Property - Light Control Ambient LuxLevel Prolong */
#define MS_DEV_PROP_LIGHT_CONTROL_AMBIENT_LUXLEVEL_PROLONG      0x002C

/** Device Property - Light Control Ambient LuxLevel Standby */
#define MS_DEV_PROP_LIGHT_CONTROL_AMBIENT_LUXLEVEL_STANDBY      0x002D

/** Device Property - Light Control Regulator Kiu */
#define MS_DEV_PROP_LIGHT_CONTROL_REGULATOR_KIU                 0x0033

/** Device Property - Light Control Regulator Kid */
#define MS_DEV_PROP_LIGHT_CONTROL_REGULATOR_KID                 0x0032

/** Device Property - Light Control Regulator Kpu */
#define MS_DEV_PROP_LIGHT_CONTROL_REGULATOR_KPU                 0x0035

/** Device Property - Light Control Regulator Kpd */
#define MS_DEV_PROP_LIGHT_CONTROL_REGULATOR_KPD                 0x0034

/** Device Property - Light Control Regulator Accuracy */
#define MS_DEV_PROP_LIGHT_CONTROL_REGULATOR_ACCURACY            0x0031

/** Device Property - Motion Sensed */
#define MS_DEV_PROP_MOTION_SENSED                               0x0042

/** Device Property - Time Since Motion Sensed */
#define MS_DEV_PROP_TIME_SINCE_MOTION_SENSED                    0x0068

/** Device Property - People Count */
#define MS_DEV_PROP_PEOPLE_COUNT                                0x004C

/** Device Property - Presence Detected */
#define MS_DEV_PROP_PRESENCE_DETECTED                           0x004D

/** Device Property - Present Ambient Light Level */
#define MS_DEV_PROP_PRESENT_AMBIENT_LIGHT_LEVEL                 0x004E


/* --------------------------------------------- Data Types/ Structures */
/**
    \defgroup generic_property_cb Application Callback
    \{
    This Section Describes the module Notification Callback interface offered
    to the application
*/

/**
    Generic Property Server application Asynchronous Notification Callback.

    Generic Property Server calls the registered callback to indicate events occurred to the
    application.

    \param [in] ctx           Context of the message received for a specific model instance.
    \param [in] msg_raw       Uninterpreted/raw received message.
    \param [in] req_type      Requested message type.
    \param [in] state_params  Model specific state parameters.
    \param [in] ext_params    Additional parameters.
*/
typedef API_RESULT (* MS_GENERIC_PROPERTY_SERVER_CB)
(
    MS_ACCESS_MODEL_REQ_MSG_CONTEXT*     ctx,
    MS_ACCESS_MODEL_REQ_MSG_RAW*         msg_raw,
    MS_ACCESS_MODEL_REQ_MSG_T*           req_type,
    MS_ACCESS_MODEL_STATE_PARAMS*        state_params,
    MS_ACCESS_MODEL_EXT_PARAMS*          ext_params

) DECL_REENTRANT;

/**
    Generic Property Client application Asynchronous Notification Callback.

    Generic Property Client calls the registered callback to indicate events occurred to the
    application.

    \param handle        Model Handle.
    \param opcode        Opcode.
    \param data_param    Data associated with the event if any or NULL.
    \param data_len      Size of the event data. 0 if event data is NULL.
*/
typedef API_RESULT (* MS_GENERIC_PROPERTY_CLIENT_CB)
(
    MS_ACCESS_MODEL_HANDLE* handle,
    UINT32                   opcode,
    UCHAR*                   data_param,
    UINT16                   data_len
) DECL_REENTRANT;
/** \} */

/**
    \defgroup generic_property_structures Structures
    \{
*/

/**
    Generic User Properties Status message parameters.
*/
typedef struct MS_generic_user_properties_status_struct
{
    /**
        A sequence of N User Property IDs present within an element,
        where N is the number of device property IDs included in the message.
    */
    UINT16* user_property_ids;

    /** Number of User Property IDs */
    UINT16 user_property_ids_count;

} MS_GENERIC_USER_PROPERTIES_STATUS_STRUCT;

/**
    Generic User Property Get message parameters.
*/
typedef struct MS_generic_user_property_get_struct
{
    /** Property ID identifying a Generic User Property. */
    UINT16 user_property_id;

} MS_GENERIC_USER_PROPERTY_GET_STRUCT;

/**
    Generic User Property Set message parameters.
*/
typedef struct MS_generic_user_property_set_struct
{
    /** Property ID identifying a Generic User Property */
    UINT16 user_property_id;

    /** Raw value for the User Property */
    UCHAR* user_property_value;

    /** Raw value length for the User Property */
    UINT16 user_property_value_len;

} MS_GENERIC_USER_PROPERTY_SET_STRUCT;

/**
    Generic User Property Status message parameters.
*/
typedef struct MS_generic_user_property_status_struct
{
    /** Property ID identifying a Generic User Property. */
    UINT16 user_property_id;

    /** Enumeration indicating user access. */
    UCHAR  user_access;

    /** Raw value for the User Property */
    UCHAR* user_property_value;

    /** Raw value length for the User Property */
    UINT16 user_property_value_len;

    /** Flag: To represent if optional fields User Access and Raw Value are valid */
    UCHAR optional_fields_present;

} MS_GENERIC_USER_PROPERTY_STATUS_STRUCT;

/**
    Generic Admin Properties Status message parameters.
*/
typedef struct MS_generic_admin_properties_status_struct
{
    /**
        A sequence of N Admin Property IDs present within an element,
        where N is the number of device property IDs included in the message.
    */
    UINT16* admin_property_ids;

    /** Number of Admin Property IDs */
    UINT16 admin_property_ids_count;

} MS_GENERIC_ADMIN_PROPERTIES_STATUS_STRUCT;

/**
    Generic Admin Property Get message parameters.
*/
typedef struct MS_generic_admin_property_get_struct
{
    /** Property ID identifying a Generic Admin Property. */
    UINT16 admin_property_id;

} MS_GENERIC_ADMIN_PROPERTY_GET_STRUCT;

/**
    Generic Admin Property Set message parameters.
*/
typedef struct MS_generic_admin_property_set_struct
{
    /** Property ID identifying a Generic Admin Property. */
    UINT16 admin_property_id;

    /** Enumeration indicating user access. */
    UCHAR  admin_user_access;

    /** Raw value for the Admin Property */
    UCHAR* admin_property_value;

    /** Raw value length for the Admin Property */
    UINT16 admin_property_value_len;

} MS_GENERIC_ADMIN_PROPERTY_SET_STRUCT;

/**
    Generic Admin Property Status message parameters.
*/
typedef struct MS_generic_admin_property_status_struct
{
    /** Property ID identifying a Generic Admin Property */
    UINT16 admin_property_id;

    /** Enumeration indicating user access (Optional) */
    UCHAR  admin_user_access;

    /** Raw value for the Admin Property */
    UCHAR* admin_property_value;

    /** Raw value length for the Admin Property */
    UINT16 admin_property_value_len;

} MS_GENERIC_ADMIN_PROPERTY_STATUS_STRUCT;

/**
    Generic Manufacturer Properties Status message parameters.
*/
typedef struct MS_generic_manufacturer_properties_status_struct
{
    /**
        A sequence of N Manufacturer Property IDs present within an element,
        where N is the number of device property IDs included in the message.
    */
    UINT16* manufacturer_property_ids;

    /** Number of Manufacturer Property IDs */
    UINT16 manufacturer_property_ids_count;

} MS_GENERIC_MANUFACTURER_PROPERTIES_STATUS_STRUCT;

/**
    Generic Manufacturer Property Get message parameters.
*/
typedef struct MS_generic_manufacturer_property_get_struct
{
    /** Property ID identifying a Generic Manufacturer Property */
    UINT16 manufacturer_property_id;

} MS_GENERIC_MANUFACTURER_PROPERTY_GET_STRUCT;

/**
    Generic Manufacturer Property Set message parameters.
*/
typedef struct MS_generic_manufacturer_property_set_struct
{
    /** Property ID identifying a Generic Manufacturer Property */
    UINT16 manufacturer_property_id;

    /** Enumeration indicating user access */
    UCHAR  manufacturer_user_access;

} MS_GENERIC_MANUFACTURER_PROPERTY_SET_STRUCT;

/**
    Generic Manufacturer Property Status message parameters.
*/
typedef struct MS_generic_manufacturer_property_status_struct
{
    /** Property ID identifying a Generic Manufacturer Property */
    UINT16 manufacturer_property_id;

    /** Enumeration indicating user access */
    UCHAR  manufacturer_user_access;

    /** Raw value for the Manufacturer Property */
    UCHAR* manufacturer_property_value;

    /** Raw value length for the Manufacturer Property */
    UINT16 manufacturer_property_value_len;

} MS_GENERIC_MANUFACTURER_PROPERTY_STATUS_STRUCT;

/**
    Generic Client Properties Get message parameters.
*/
typedef struct MS_generic_client_properties_get_struct
{
    /** A starting Client Property ID present within an element */
    UINT16 client_property_id;

} MS_GENERIC_CLIENT_PROPERTIES_GET_STRUCT;

/**
    Generic Client Properties Status message parameters.
*/
typedef struct MS_generic_client_properties_status_struct
{
    /**
        A sequence of N Client Property IDs present within an element,
        where N is the number of device property IDs included in the message.
    */
    UINT16* client_property_ids;

    /** Number of Client Property IDs */
    UINT16 client_property_ids_count;

} MS_GENERIC_CLIENT_PROPERTIES_STATUS_STRUCT;

/** \} */



/* --------------------------------------------- Function */
/**
    \defgroup generic_property_api_defs API Definitions
    \{
    This section describes the EtherMind Mesh Generic Property Model APIs.
*/
/**
    \defgroup generic_user_property_ser_api_defs Generic User Property Server API Definitions
    \{
    This section describes the Generic User Property Server APIs.
*/

/**
    \brief API to initialize Generic_User_Property Server model

    \par Description
    This is to initialize Generic_User_Property Server model and to register with Acess layer.

    \param [in] element_handle
                Element identifier to be associated with the model instance.

    \param [in, out] model_handle
                     Model identifier associated with the model instance on successful initialization.
                     After power cycle of an already provisioned node, the model handle will have
                     valid value and the same will be reused for registration.

    \param [in] appl_cb    Application Callback to be used by the Generic_User_Property Server.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_generic_user_property_server_init
(
    /* IN */    MS_ACCESS_ELEMENT_HANDLE        element_handle,
    /* INOUT */ MS_ACCESS_MODEL_HANDLE*         model_handle,
    /* IN */    MS_GENERIC_PROPERTY_SERVER_CB   appl_cb
);

/**
    \brief API to initialize Generic_Admin_Property Server model

    \par Description
    This is to initialize Generic_Admin_Property Server model and to register with Acess layer.

    \param [in] element_handle
                Element identifier to be associated with the model instance.

    \param [in, out] model_handle
                     Model identifier associated with the model instance on successful initialization.
                     After power cycle of an already provisioned node, the model handle will have
                     valid value and the same will be reused for registration.

    \param [in] appl_cb    Application Callback to be used by the Generic_Admin_Property Server.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_generic_admin_property_server_init
(
    /* IN */    MS_ACCESS_ELEMENT_HANDLE        element_handle,
    /* INOUT */ MS_ACCESS_MODEL_HANDLE*         model_handle,
    /* IN */    MS_GENERIC_PROPERTY_SERVER_CB   appl_cb
);

/**
    \brief API to initialize Generic_Manufacturer_Property Server model

    \par Description
    This is to initialize Generic_Manufacturer_Property Server model and to register with Acess layer.

    \param [in] element_handle
                Element identifier to be associated with the model instance.

    \param [in, out] model_handle
                     Model identifier associated with the model instance on successful initialization.
                     After power cycle of an already provisioned node, the model handle will have
                     valid value and the same will be reused for registration.

    \param [in] appl_cb    Application Callback to be used by the Generic_Manufacturer_Property Server.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_generic_manufacturer_property_server_init
(
    /* IN */    MS_ACCESS_ELEMENT_HANDLE        element_handle,
    /* INOUT */ MS_ACCESS_MODEL_HANDLE*         model_handle,
    /* IN */    MS_GENERIC_PROPERTY_SERVER_CB   appl_cb
);

/**
    \brief API to initialize Generic_Client_Property Server model

    \par Description
    This is to initialize Generic_Client_Property Server model and to register with Acess layer.

    \param [in] element_handle
                Element identifier to be associated with the model instance.

    \param [in, out] model_handle
                     Model identifier associated with the model instance on successful initialization.
                     After power cycle of an already provisioned node, the model handle will have
                     valid value and the same will be reused for registration.

    \param [in] appl_cb    Application Callback to be used by the Generic_Client_Property Server.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_generic_client_property_server_init
(
    /* IN */    MS_ACCESS_ELEMENT_HANDLE        element_handle,
    /* INOUT */ MS_ACCESS_MODEL_HANDLE*         model_handle,
    /* IN */    MS_GENERIC_PROPERTY_SERVER_CB   appl_cb
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
API_RESULT MS_generic_user_property_server_state_update
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
API_RESULT MS_generic_admin_property_server_state_update
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
API_RESULT MS_generic_manufacturer_property_server_state_update
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
API_RESULT MS_generic_client_property_server_state_update
(
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_CONTEXT*     ctx,
    /* IN */ MS_ACCESS_MODEL_STATE_PARAMS*        current_state_params,
    /* IN */ MS_ACCESS_MODEL_STATE_PARAMS*        target_state_params,
    /* IN */ UINT16                               remaining_time,
    /* IN */ MS_ACCESS_MODEL_EXT_PARAMS*          ext_params
);

/** \} */

/**
    \defgroup generic_property_cli_api_defs Generic Property Client API Definitions
    \{
    This section describes the Generic Property Client APIs.
*/

/**
    \brief API to initialize Generic_Property Client model

    \par Description
    This is to initialize Generic_Property Client model and to register with Acess layer.

    \param [in] element_handle
                Element identifier to be associated with the model instance.

    \param [in, out] model_handle
                     Model identifier associated with the model instance on successful initialization.
                     After power cycle of an already provisioned node, the model handle will have
                     valid value and the same will be reused for registration.

    \param [in] appl_cb    Application Callback to be used by the Generic_Property Client.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_generic_property_client_init
(
    /* IN */    MS_ACCESS_ELEMENT_HANDLE    element_handle,
    /* INOUT */ MS_ACCESS_MODEL_HANDLE*     model_handle,
    /* IN */    MS_GENERIC_PROPERTY_CLIENT_CB appl_cb
);

/**
    \brief API to get Generic_Property client model handle

    \par Description
    This is to get the handle of Generic_Property client model.

    \param [out] model_handle   Address of model handle to be filled/returned.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_generic_property_client_get_model_handle
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
API_RESULT MS_generic_property_client_send_reliable_pdu
(
    /* IN */ UINT32    req_opcode,
    /* IN */ void*     param,
    /* IN */ UINT32    rsp_opcode
);

/**
    \brief API to get the list of Generic User Property states of an element.

    \par Description
    Generic User Properties Get is an acknowledged message used to get the list of Generic User Property states of an element.
    The response to the Generic User Properties Get message is a Generic User Properties Status message.
    The message has no parameters.

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_generic_user_properties_get() \
    MS_generic_property_client_send_reliable_pdu \
    (\
     MS_ACCESS_GENERIC_USER_PROPERTIES_GET_OPCODE,\
     NULL,\
     MS_ACCESS_GENERIC_USER_PROPERTIES_STATUS_OPCODE\
    )

/**
    \brief API to get the Generic User Property state of an element.

    \par Description
    Generic User Property Get is an acknowledged message used to get the Generic User Property state of an element.
    The response to the Generic User Property Get message is a Generic User Property Status message.

    \param [in] param Generic User Property Get message

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_generic_user_property_get(param) \
    MS_generic_property_client_send_reliable_pdu \
    (\
     MS_ACCESS_GENERIC_USER_PROPERTY_GET_OPCODE,\
     param,\
     MS_ACCESS_GENERIC_USER_PROPERTY_STATUS_OPCODE\
    )

/**
    \brief API to set the Generic User Property state of an element.

    \par Description
    Generic User Property Set is an acknowledged message used to set the Generic User Property state of an element.
    The response to the Generic User Property Set message is a Generic User Property Status message.

    \param [in] param Generic User Property Set message

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_generic_user_property_set(param) \
    MS_generic_property_client_send_reliable_pdu \
    (\
     MS_ACCESS_GENERIC_USER_PROPERTY_SET_OPCODE,\
     param,\
     MS_ACCESS_GENERIC_USER_PROPERTY_STATUS_OPCODE\
    )

/**
    \brief API to set the Generic User Property state of an element.

    \par Description
    Generic User Property Set Unacknowledged is an unacknowledged message used to set
    the Generic User Property state of an element.

    \param [in] param Generic User Property Set message

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_generic_user_property_set_unacknowledged(param) \
    MS_generic_property_client_send_reliable_pdu \
    (\
     MS_ACCESS_GENERIC_USER_PROPERTY_SET_UNACKNOWLEDGED_OPCODE,\
     param,\
     0xFFFFFFFF\
    )

/**
    \brief API to get the list of Generic Admin Property states of an element.

    \par Description
    Generic Admin Properties Get is an acknowledged message used to get the list of Generic Admin Property states of an element.
    The response to the Generic Admin Properties Get message is a Generic Admin Properties Status message.
    The message has no parameters.

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_generic_admin_properties_get() \
    MS_generic_property_client_send_reliable_pdu \
    (\
     MS_ACCESS_GENERIC_ADMIN_PROPERTIES_GET_OPCODE,\
     NULL,\
     MS_ACCESS_GENERIC_ADMIN_PROPERTIES_STATUS_OPCODE\
    )

/**
    \brief API to get the Generic Admin Property state of an element.

    \par Description
    Generic Admin Property Get is an acknowledged message used to get the Generic Admin Property state of an element.
    The response to the Generic Admin Property Get message is a Generic Admin Property Status message.

    \param [in] param Generic Admin Property Get message

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_generic_admin_property_get(param) \
    MS_generic_property_client_send_reliable_pdu \
    (\
     MS_ACCESS_GENERIC_ADMIN_PROPERTY_GET_OPCODE,\
     param,\
     MS_ACCESS_GENERIC_ADMIN_PROPERTY_STATUS_OPCODE\
    )

/**
    \brief API to set the Generic Admin Property state of an element.

    \par Description
    Generic Admin Property Set is an acknowledged message used to set the Generic Admin Property state of an element.
    The response to the Generic Admin Property Set message is a Generic Admin Property Status message.

    \param [in] param Generic Admin Property Set message

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_generic_admin_property_set(param) \
    MS_generic_property_client_send_reliable_pdu \
    (\
     MS_ACCESS_GENERIC_ADMIN_PROPERTY_SET_OPCODE,\
     param,\
     MS_ACCESS_GENERIC_ADMIN_PROPERTY_STATUS_OPCODE\
    )

/**
    \brief API to set the Generic Admin Property state of an element.

    \par Description
    Generic Admin Property Set Unacknowledged is an unacknowledged message used to set
    the Generic Admin Property state of an element.

    \param [in] param Generic Admin Property Set message

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_generic_admin_property_set_unacknowledged(param) \
    MS_generic_property_client_send_reliable_pdu \
    (\
     MS_ACCESS_GENERIC_ADMIN_PROPERTY_SET_UNACKNOWLEDGED_OPCODE,\
     param,\
     0xFFFFFFFF\
    )

/**
    \brief API to get the list of Generic Manufacturer Property states of an element.

    \par Description
    Generic Manufacturer Properties Get is an acknowledged message used to get
    the list of Generic Manufacturer Property states of an element.
    The response to the Generic Manufacturer Properties Get message
    is a Generic Manufacturer Properties Status message.
    The message has no parameters.

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_generic_manufacturer_properties_get() \
    MS_generic_property_client_send_reliable_pdu \
    (\
     MS_ACCESS_GENERIC_MANUFACTURER_PROPERTIES_GET_OPCODE,\
     NULL,\
     MS_ACCESS_GENERIC_MANUFACTURER_PROPERTIES_STATUS_OPCODE\
    )

/**
    \brief API to get the Generic Manufacturer Property state of an element.

    \par Description
    Generic Manufacturer Property Get is an acknowledged message used to get the Generic Manufacturer Property state of an element.
    The response to the Generic Manufacturer Property Get message is a Generic Manufacturer Property Status message.

    \param [in] param Generic Manufacturer Property Get message

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_generic_manufacturer_property_get(param) \
    MS_generic_property_client_send_reliable_pdu \
    (\
     MS_ACCESS_GENERIC_MANUFACTURER_PROPERTY_GET_OPCODE,\
     param,\
     MS_ACCESS_GENERIC_MANUFACTURER_PROPERTY_STATUS_OPCODE\
    )

/**
    \brief API to the Generic Manufacturer Property User Access state of an element.

    \par Description
    Generic Manufacturer Property Set is an acknowledged message used to set the Generic Manufacturer Property User Access state of an element.
    The response to the Generic Manufacturer Property Set message is a Generic Manufacturer Property Status message.

    \param [in] param Generic Manufacturer Property Set message

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_generic_manufacturer_property_set(param) \
    MS_generic_property_client_send_reliable_pdu \
    (\
     MS_ACCESS_GENERIC_MANUFACTURER_PROPERTY_SET_OPCODE,\
     param,\
     MS_ACCESS_GENERIC_MANUFACTURER_PROPERTY_STATUS_OPCODE\
    )

/**
    \brief API to the Generic Manufacturer Property User Access state of an element.

    \par Description
    The Generic Manufacturer Property Set Unacknowledged is an unacknowledged message used to set the Generic Manufacturer Property User Access state of an element.

    \param [in] param Generic Manufacturer Property Set message

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_generic_manufacturer_property_set_unacknowledged(param) \
    MS_generic_property_client_send_reliable_pdu \
    (\
     MS_ACCESS_GENERIC_MANUFACTURER_PROPERTY_SET_UNACKNOWLEDGED_OPCODE,\
     param,\
     0xFFFFFFFF\
    )

/**
    \brief API to get the list of Generic Client Property states of an element.

    \par Description
    Generic Client Properties Get is an acknowledged message used to get the list of Generic Client Property states of an element.
    The response to the Generic Client Properties Get message is a Generic Client Properties Status message.

    \param [in] param Generic Client Properties Get message

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_generic_client_properties_get(param) \
    MS_generic_property_client_send_reliable_pdu \
    (\
     MS_ACCESS_GENERIC_CLIENT_PROPERTIES_GET_OPCODE,\
     param,\
     MS_ACCESS_GENERIC_CLIENT_PROPERTIES_STATUS_OPCODE\
    )
/** \} */
/** \} */
/** \} */

#endif /*_H_MS_GENERIC_PROPERTY_API_ */
