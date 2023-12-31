/**
    \file MS_generic_location_api.h

    \brief This file defines the Mesh Generic Location Model Application Interface
    - includes Data Structures and Methods for both Server and Client.
*/

/*
    Copyright (C) 2017. Mindtree Ltd.
    All rights reserved.
*/

#ifndef _H_MS_GENERIC_LOCATION_API_
#define _H_MS_GENERIC_LOCATION_API_


/* --------------------------------------------- Header File Inclusion */
#include "MS_access_api.h"


/* --------------------------------------------- Global Definitions */
/**
    \defgroup generic_location_module GENERIC_LOCATION (Mesh Generic Location Model)
    \{
    This section describes the interfaces & APIs offered by the EtherMind
    Mesh Generic OnOff Model (ONOFF) module to the Application.
*/



/* --------------------------------------------- Data Types/ Structures */
/**
    \defgroup generic_location_cb Application Callback
    \{
    This Section Describes the module Notification Callback interface offered
    to the application
*/

/**
    Generic Location Server application Asynchronous Notification Callback.

    Generic Location Server calls the registered callback to indicate events occurred to the
    application.

    \param [in] ctx           Context of the message received for a specific model instance.
    \param [in] msg_raw       Uninterpreted/raw received message.
    \param [in] req_type      Requested message type.
    \param [in] state_params  Model specific state parameters.
    \param [in] ext_params    Additional parameters.
*/
typedef API_RESULT (* MS_GENERIC_LOCATION_SERVER_CB)
(
    MS_ACCESS_MODEL_REQ_MSG_CONTEXT*     ctx,
    MS_ACCESS_MODEL_REQ_MSG_RAW*         msg_raw,
    MS_ACCESS_MODEL_REQ_MSG_T*           req_type,
    MS_ACCESS_MODEL_STATE_PARAMS*        state_params,
    MS_ACCESS_MODEL_EXT_PARAMS*          ext_params

) DECL_REENTRANT;

/**
    Generic Location Client application Asynchronous Notification Callback.

    Generic Location Client calls the registered callback to indicate events occurred to the
    application.

    \param handle        Model Handle.
    \param opcode        Opcode.
    \param data_param    Data associated with the event if any or NULL.
    \param data_len      Size of the event data. 0 if event data is NULL.
*/
typedef API_RESULT (* MS_GENERIC_LOCATION_CLIENT_CB)
(
    MS_ACCESS_MODEL_HANDLE* handle,
    UINT32                   opcode,
    UCHAR*                   data_param,
    UINT16                   data_len
) DECL_REENTRANT;

/**
    Generic Location Setup Server application Asynchronous Notification Callback.

    Generic Location Setup Server calls the registered callback to indicate events occurred to the
    application.

    \param [in] ctx           Context of the message received for a specific model instance.
    \param [in] msg_raw       Uninterpreted/raw received message.
    \param [in] req_type      Requested message type.
    \param [in] state_params  Model specific state parameters.
    \param [in] ext_params    Additional parameters.
*/
typedef API_RESULT (* MS_GENERIC_LOCATION_SETUP_SERVER_CB)
(
    MS_ACCESS_MODEL_REQ_MSG_CONTEXT*     ctx,
    MS_ACCESS_MODEL_REQ_MSG_RAW*         msg_raw,
    MS_ACCESS_MODEL_REQ_MSG_T*           req_type,
    MS_ACCESS_MODEL_STATE_PARAMS*        state_params,
    MS_ACCESS_MODEL_EXT_PARAMS*          ext_params

) DECL_REENTRANT;
/** \} */


/**
    \defgroup generic_location_structures Structures
    \{
*/

/**
    Generic Location Global Set message parameters.
*/
typedef struct MS_generic_location_global_struct
{
    /**
        Global Coordinates (Latitude).
        The Global Latitude field describes the global WGS84 North coordinate of the element.
    */
    UINT32 global_latitude;

    /**
        Global Coordinates (Longitude).
        The Global Longitude field describes the global WGS84 East coordinate of the element.
    */
    UINT32 global_longitude;

    /**
        Global Altitude.
        The Global Altitude field determines the altitude of the device above the WGS84 datum.
        It expresses the altitude beyond the WGS84 ellipsoid of the element that exposed its position.

        Value         | Description
        --------------|------------
        0x7FFF        | Global Altitude is not configured.
        0x7FFE        | Global Altitude is greater than or equal to 32766 meters.
        0x8000-0x7FFD | Global Altitude is (field value) from -32768 meters through 32765 meters.
    */
    UINT16 global_altitude;

} MS_GENERIC_LOCATION_GLOBAL_STRUCT;

/**
    Generic Location Local Set message parameters.
*/
typedef struct MS_generic_location_local_struct
{
    /**
        Local Coordinates (North).
        The Local North field describes the North coordinate of the device using a local coordinate system.
        It is relative to the north orientation on a predefined map.

        The Local North value is encoded in decimeters and has a range of -32767 decimeters through 32767 decimeters.
        The value 0x8000 means the Local North information is not configured.
    */
    UINT16 local_north;

    /**
        Local Coordinates (East).
        The Local East field describes the East coordinate of the device using a local coordinate system.
        It is relative to the east orientation of a predefined map.

        The Local East value is encoded decimeters and it ranges from -32767 decimeters through 32767 decimeters.
        The value 0x8000 means the Local East information is not configured.
    */
    UINT16 local_east;

    /**
        Local Altitude.
        The Local Altitude field determines the altitude of the device relative to the Generic Location Global Altitude.

        Value         | Description
        --------------|------------
        0x7FFF        | Local Altitude is not configured.
        0x7FFE        | Local Altitude is greater than or equal to 32766 meters.
        0x8000-0x7FFD | Local Altitude is (field value) from -32768 meters through 32765 meters.
    */
    UINT16 local_altitude;

    /**
        Floor Number.

        The Floor Number field describes the floor number where the element is installed.
        The floor number, N, is encoded as X = N + 20, where X is the encoded floor number.
        Floor number = -20 (X=0) has a special meaning, indicating the floor -20, and also any floor below that.
        Floor number = 232 (X=252) has a special meaning, indicating the floor 232, and also any floor above that.

        Encoded Value X | Floor number N
        ----------------|---------------
        0x00            | Floor -20 or any floor below -20.
        0x01-0xFB       | Floor number N, encoded as X = N + 20.
        0xFC            | Floor 232 or any floor above 232.
        0xFD            | Ground floor. Floor 0.
        0xFE            | Ground floor. Floor 1.
        0xFF            | Not configured

        Note: The reason for having two definitions of ground floor (0 or 1) is to allow for
        different conventions applicable in different countries.
    */
    UCHAR  floor_number;

    /**
        Uncertainty.
        The Uncertainty field is a 16-bit bit field that describes the uncertainty of
        the location information the element exposes.

        bits  | Field       | Description
        ------|-------------|------------
        0     | Stationary  | This bit indicates whether the device broadcasting the location information
                             has a stationary location or is mobile. (0 = Stationary, 1 = Mobile)
        1-7   | RFU         | Reserved for Future Use
        8-11  | Update Time | This value (x) is a 4-bit value ranging from 0 through 15.
                             It represents the time (t) elapsed since the last update of the device's position,
                             measured in seconds using the following formula: t=2^(x-3)
                             The represented range is from 0.125 seconds through 4096 seconds.
                             Note: If 'stationary' is set, this value can be ignored.
        12-15 | Precision   | This value (y) is a 4-bit value ranging from 0 through 15.
                             It represents a location precision with the formula: Precision = 2^(y-3)
                             The represented range is from 0.125 meters through 4096 meters.
    */
    UINT16 uncertainty;

} MS_GENERIC_LOCATION_LOCAL_STRUCT;

/** \} */



/* --------------------------------------------- Function */
/**
    \defgroup generic_location_api_defs API Definitions
    \{
    This section describes the EtherMind Mesh Generic Location Model APIs.
*/
/**
    \defgroup generic_location_ser_api_defs Generic Location Server API Definitions
    \{
    This section describes the Generic Location Server APIs.
*/

/**
    \brief API to initialize Generic_Location Server model

    \par Description
    This is to initialize Generic_Location Server model and to register with Acess layer.

    \param [in] element_handle
                Element identifier to be associated with the model instance.

    \param [in, out] model_handle
                     Model identifier associated with the model instance on successful initialization.
                     After power cycle of an already provisioned node, the model handle will have
                     valid value and the same will be reused for registration.

    \param [in] appl_cb    Application Callback to be used by the Generic_Location Server.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_generic_location_server_init
(
    /* IN */    MS_ACCESS_ELEMENT_HANDLE    element_handle,
    /* INOUT */ MS_ACCESS_MODEL_HANDLE*     model_handle,
    /* IN */    MS_GENERIC_LOCATION_SERVER_CB appl_cb
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
API_RESULT MS_generic_location_server_state_update
(
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_CONTEXT*     ctx,
    /* IN */ MS_ACCESS_MODEL_STATE_PARAMS*        current_state_params,
    /* IN */ MS_ACCESS_MODEL_STATE_PARAMS*        target_state_params,
    /* IN */ UINT16                               remaining_time,
    /* IN */ MS_ACCESS_MODEL_EXT_PARAMS*          ext_params
);

/**
    \brief API to initialize Generic_Location_Setup Server model

    \par Description
    This is to initialize Generic_Location_Setup Server model and to register with Acess layer.

    \param [in] element_handle
                Element identifier to be associated with the model instance.

    \param [in, out] model_handle
                     Model identifier associated with the model instance on successful initialization.
                     After power cycle of an already provisioned node, the model handle will have
                     valid value and the same will be reused for registration.

    \param [in] appl_cb    Application Callback to be used by the Generic_Location_Setup Server.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_generic_location_setup_server_init
(
    /* IN */    MS_ACCESS_ELEMENT_HANDLE    element_handle,
    /* INOUT */ MS_ACCESS_MODEL_HANDLE*     model_handle,
    /* IN */    MS_GENERIC_LOCATION_SETUP_SERVER_CB appl_cb
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
API_RESULT MS_generic_location_setup_server_state_update
(
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_CONTEXT*     ctx,
    /* IN */ MS_ACCESS_MODEL_STATE_PARAMS*        current_state_params,
    /* IN */ MS_ACCESS_MODEL_STATE_PARAMS*        target_state_params,
    /* IN */ UINT16                               remaining_time,
    /* IN */ MS_ACCESS_MODEL_EXT_PARAMS*          ext_params
);
/** \} */

/**
    \defgroup generic_location_cli_api_defs Generic Location Client API Definitions
    \{
    This section describes the Generic Location Client APIs.
*/

/**
    \brief API to initialize Generic_Location Client model

    \par Description
    This is to initialize Generic_Location Client model and to register with Acess layer.

    \param [in] element_handle
                Element identifier to be associated with the model instance.

    \param [in, out] model_handle
                     Model identifier associated with the model instance on successful initialization.
                     After power cycle of an already provisioned node, the model handle will have
                     valid value and the same will be reused for registration.

    \param [in] appl_cb    Application Callback to be used by the Generic_Location Client.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_generic_location_client_init
(
    /* IN */    MS_ACCESS_ELEMENT_HANDLE    element_handle,
    /* INOUT */ MS_ACCESS_MODEL_HANDLE*     model_handle,
    /* IN */    MS_GENERIC_LOCATION_CLIENT_CB appl_cb
);

/**
    \brief API to get Generic_Location client model handle

    \par Description
    This is to get the handle of Generic_Location client model.

    \param [out] model_handle   Address of model handle to be filled/returned.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_generic_location_client_get_model_handle
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
API_RESULT MS_generic_location_client_send_reliable_pdu
(
    /* IN */ UINT32    req_opcode,
    /* IN */ void*     param,
    /* IN */ UINT32    rsp_opcode
);

/**
    \brief API to get the selected fields of the Generic Location state of an element.

    \par Description
    Generic Location Global Get message is an acknowledged message used to get the selected fields
    of the Generic Location state of an element.
    The response to the Generic Location Global Get message is a Generic Location Global Status message.
    There are no parameters for this message.

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_generic_location_global_get() \
    MS_generic_location_client_send_reliable_pdu \
    (\
     MS_ACCESS_GENERIC_LOCATION_GLOBAL_GET_OPCODE,\
     NULL,\
     MS_ACCESS_GENERIC_LOCATION_GLOBAL_STATUS_OPCODE\
    )

/**
    \brief API to set the selected fields of the Generic Location state of an element.

    \par Description
    Generic Location Global Set is an acknowledged message used to set the selected fields of the Generic Location state of an element.
    The response to the Generic Location Global Set message is a Generic Location Global Status message.

    \param [in] param Generic Location Global Set message

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_generic_location_global_set(param) \
    MS_generic_location_client_send_reliable_pdu \
    (\
     MS_ACCESS_GENERIC_LOCATION_GLOBAL_SET_OPCODE,\
     param,\
     MS_ACCESS_GENERIC_LOCATION_GLOBAL_STATUS_OPCODE\
    )

/**
    \brief API to set the selected fields of the Generic Location state of an element.

    \par Description
    Generic Location Global Set Unacknowledged is an unacknowledged message used to set the selected fields
    of the Generic Location state of an element.

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_generic_location_global_set_unacknowledged(param) \
    MS_generic_location_client_send_reliable_pdu \
    (\
     MS_ACCESS_GENERIC_LOCATION_GLOBAL_SET_UNACKNOWLEDGED_OPCODE,\
     param,\
     0xFFFFFFFF\
    )

/**
    \brief API to get the selected fields of the Generic Location state of an element.

    \par Description
    Generic Location Local Get message is an acknowledged message used to get the selected fields
    of the Generic Location state of an element.
    The response to the Generic Location Local Get message is a Generic Location Local Status message.
    There are no parameters for this message.

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_generic_location_local_get() \
    MS_generic_location_client_send_reliable_pdu \
    (\
     MS_ACCESS_GENERIC_LOCATION_LOCAL_GET_OPCODE,\
     NULL,\
     MS_ACCESS_GENERIC_LOCATION_LOCAL_STATUS_OPCODE\
    )

/**
    \brief API to set the selected fields of the Generic Location state of an element.

    \par Description
    Generic Location Local Set is an acknowledged message used to set the selected fields
    of the Generic Location state of an element.
    The response to the Generic Location Local Set message is a Generic Location Local Status message.

    \param [in] param Generic Location Local Set message

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_generic_location_local_set(param) \
    MS_generic_location_client_send_reliable_pdu \
    (\
     MS_ACCESS_GENERIC_LOCATION_LOCAL_SET_OPCODE,\
     param,\
     MS_ACCESS_GENERIC_LOCATION_LOCAL_STATUS_OPCODE\
    )

/**
    \brief API to set the selected fields of the Generic Location state of an element.

    \par Description
    Generic Location Local Set Unacknowledged is an unacknowledged message used to set the selected fields
    of the Generic Location state of an element.

    \param [in] param Generic Location Local Set message

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_generic_location_local_set_unacknowledged(param) \
    MS_generic_location_client_send_reliable_pdu \
    (\
     MS_ACCESS_GENERIC_LOCATION_LOCAL_SET_UNACKNOWLEDGED_OPCODE,\
     param,\
     0xFFFFFFFF\
    )
/** \} */
/** \} */
/** \} */

#endif /*_H_MS_GENERIC_LOCATION_API_ */
