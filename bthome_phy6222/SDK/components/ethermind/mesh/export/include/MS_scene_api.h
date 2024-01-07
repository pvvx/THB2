/**
    \file MS_scene_api.h

    \brief This file defines the Mesh Scene Model Application Interface
    - includes Data Structures and Methods for both Server and Client.
*/

/*
    Copyright (C) 2017. Mindtree Ltd.
    All rights reserved.
*/

#ifndef _H_MS_SCENE_API_
#define _H_MS_SCENE_API_


/* --------------------------------------------- Header File Inclusion */
#include "MS_access_api.h"


/* --------------------------------------------- Global Definitions */
/**
    \defgroup scene_module SCENE (Mesh Scene Model)
    \{
    This section describes the interfaces & APIs offered by the EtherMind
    Mesh Scene Model (SCENE) module to the Application.
*/

/** Scene Event Types */
/** Scene Event - Store */
#define MS_SCENE_EVENT_STORE               0x01

/** Scene Event - Delete */
#define MS_SCENE_EVENT_DELETE              0x02

/** Scene Event - Recall Start */
#define MS_SCENE_EVENT_RECALL_START        0x03

/** Scene Event - Recall Complete */
#define MS_SCENE_EVENT_RECALL_COMPLETE     0x04

/** Scene Event - Recall Immediate */
#define MS_SCENE_EVENT_RECALL_IMMEDIATE    0x05

/* --------------------------------------------- Data Types/ Structures */
/**
    \defgroup scene_cb Application Callback
    \{
    This Section Describes the module Notification Callback interface offered
    to the application
*/

/**
    Scene Server application Asynchronous Notification Callback.

    Scene Server calls the registered callback to indicate events occurred to the
    application.

    \param [in] ctx           Context of the message received for a specific model instance.
    \param [in] msg_raw       Uninterpreted/raw received message.
    \param [in] req_type      Requested message type.
    \param [in] state_params  Model specific state parameters.
    \param [in] ext_params    Additional parameters.

    TODO: Update
*/
typedef void* (* MS_SCENE_SERVER_CB)
(
    MS_ACCESS_MODEL_HANDLE* handle,
    UINT8                    event_type,
    void*                    event_param,
    UINT16                   event_length,
    void*                    context
) DECL_REENTRANT;

/**
    Scene Client application Asynchronous Notification Callback.

    Scene Client calls the registered callback to indicate events occurred to the
    application.

    \param handle        Model Handle.
    \param opcode        Opcode.
    \param data_param    Data associated with the event if any or NULL.
    \param data_len      Size of the event data. 0 if event data is NULL.
*/
typedef API_RESULT (* MS_SCENE_CLIENT_CB)
(
    MS_ACCESS_MODEL_HANDLE* handle,
    UINT32                   opcode,
    UCHAR*                   data_param,
    UINT16                   data_len
) DECL_REENTRANT;
/** \} */

/**
    \defgroup scene_structures Structures
    \{
*/

/**
    Scene Recall message parameters.
*/
typedef struct MS_scene_recall_struct
{
    /** The number of the scene to be recalled. */
    UINT16 scene_number;

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

} MS_SCENE_RECALL_STRUCT;

/**
    Scene Status message parameters.
*/
typedef struct MS_scene_status_struct
{
    /** Status Code */
    UCHAR  status_code;

    /** Scene Number of a current scene. */
    UINT16 current_scene;

    /** Scene Number of a target scene. (Optional) */
    UINT16 target_scene;

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

    /** Flag: To represent if optional fields Target Scene and Remaining Time are valid */
    UCHAR optional_fields_present;

} MS_SCENE_STATUS_STRUCT;

/**
    Scene Register Status message parameters.
*/
typedef struct MS_scene_register_status_struct
{
    /** Status Code */
    UCHAR  status_code;

    /** Scene Number of a current scene */
    UINT16 current_scene;

    /** A list of scenes stored within an element */
    UCHAR* scenes;

    /** Number of Scenes */
    UINT16 scenes_len;

} MS_SCENE_REGISTER_STATUS_STRUCT;

/**
    Scene Store message parameters.
*/
typedef struct MS_scene_struct
{
    /** The number of the scene to be stored. */
    UINT16 scene_number;

} MS_SCENE_STRUCT;

/** \} */



/* --------------------------------------------- Function */
/**
    \defgroup scene_api_defs API Definitions
    \{
    This section describes the EtherMind Mesh Scene Model APIs.
*/
/**
    \defgroup scene_ser_api_defs Scene Server API Definitions
    \{
    This section describes the Scene Server APIs.
*/

/**
    \brief API to initialize Scene Server model

    \par Description
    This is to initialize Scene Server model and to register with Acess layer.

    \param [in] element_handle
                Element identifier to be associated with the model instance.

    \param [in, out] scene_model_handle
                     Model identifier associated with the Scene model instance on successful initialization.
                     After power cycle of an already provisioned node, the model handle will have
                     valid value and the same will be reused for registration.

    \param [in, out] scene_setup_model_handle
                     Model identifier associated with the Scene Setup model instance on successful initialization.
                     After power cycle of an already provisioned node, the model handle will have
                     valid value and the same will be reused for registration.

    \param [in] appl_cb    Application Callback to be used by the Scene Server.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_scene_server_init
(
    /* IN */    MS_ACCESS_ELEMENT_HANDLE    element_handle,
    /* INOUT */ MS_ACCESS_MODEL_HANDLE*     scene_model_handle,
    /* INOUT */ MS_ACCESS_MODEL_HANDLE*     scene_setup_model_handle,
    /* IN */    MS_SCENE_SERVER_CB appl_cb
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
API_RESULT MS_scene_server_state_update
(
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_CONTEXT*     ctx,
    /* IN */ MS_ACCESS_MODEL_STATE_PARAMS*        current_state_params,
    /* IN */ MS_ACCESS_MODEL_STATE_PARAMS*        target_state_params,
    /* IN */ UINT16                               remaining_time,
    /* IN */ MS_ACCESS_MODEL_EXT_PARAMS*          ext_params
);
/** \} */

/**
    \defgroup scene_cli_api_defs Scene Client API Definitions
    \{
    This section describes the Scene Client APIs.
*/

/**
    \brief API to initialize Scene Client model

    \par Description
    This is to initialize Scene Client model and to register with Acess layer.

    \param [in] element_handle
                Element identifier to be associated with the model instance.

    \param [in, out] model_handle
                     Model identifier associated with the model instance on successful initialization.
                     After power cycle of an already provisioned node, the model handle will have
                     valid value and the same will be reused for registration.

    \param [in] appl_cb    Application Callback to be used by the Scene Client.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_scene_client_init
(
    /* IN */    MS_ACCESS_ELEMENT_HANDLE    element_handle,
    /* INOUT */ MS_ACCESS_MODEL_HANDLE*     model_handle,
    /* IN */    MS_SCENE_CLIENT_CB appl_cb
);

/**
    \brief API to get Scene client model handle

    \par Description
    This is to get the handle of Scene client model.

    \param [out] model_handle   Address of model handle to be filled/returned.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_scene_client_get_model_handle
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
API_RESULT MS_scene_client_send_reliable_pdu
(
    /* IN */ UINT32    req_opcode,
    /* IN */ void*     param,
    /* IN */ UINT32    rsp_opcode
);

/**
    \brief API to get the current status of a currently active scene of an element.

    \par Description
    Scene Get is an acknowledged message used to get the current status of a currently active scene of an element.
    The response to the Scene Get message is a Scene Status message.
    There are no parameters for this message.

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_scene_get() \
    MS_scene_client_send_reliable_pdu \
    (\
     MS_ACCESS_SCENE_GET_OPCODE,\
     NULL,\
     MS_ACCESS_SCENE_STATUS_OPCODE\
    )

/**
    \brief API to ecall the current state of an element.

    \par Description
    Scene Recall is an acknowledged message that is used to recall the current state of an element from a previously stored scene.
    The response to the Scene Recall message is a Scene Status message.

    \param [in] param Scene Recall message

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_scene_recall(param) \
    MS_scene_client_send_reliable_pdu \
    (\
     MS_ACCESS_SCENE_RECALL_OPCODE,\
     param,\
     MS_ACCESS_SCENE_STATUS_OPCODE\
    )

/**
    \brief API to ecall the current state of an element.

    \par Description
    Scene Recall Unacknowledged is an unacknowledged message used to recall the current state of an element from a previously stored Scene.

    \param [in] param Scene Recall message

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_scene_recall_unacknowledged(param) \
    MS_scene_client_send_reliable_pdu \
    (\
     MS_ACCESS_SCENE_RECALL_UNACKNOWLEDGED_OPCODE,\
     param,\
     0xFFFFFFFF\
    )

/**
    \brief API to get the current status of the Scene Register of an element.

    \par Description
    Scene Register Get is an acknowledged message used to get the current status of the Scene Register of an element.
    The response to the Scene Register Get message is a Scene Register Status message.
    There are no parameters for this message.

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_scene_register_get() \
    MS_scene_client_send_reliable_pdu \
    (\
     MS_ACCESS_SCENE_REGISTER_GET_OPCODE,\
     NULL,\
     MS_ACCESS_SCENE_REGISTER_STATUS_OPCODE\
    )

/**
    \brief API to store the current state of an element as a Scene.

    \par Description
    Scene Store is an acknowledged message used to store the current state of an element as a Scene, which can be recalled later.
    The response to the Scene Store message is a Scene Register Status message.

    \param [in] param Scene Store message

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_scene_store(param) \
    MS_scene_client_send_reliable_pdu \
    (\
     MS_ACCESS_SCENE_STORE_OPCODE,\
     param,\
     MS_ACCESS_SCENE_REGISTER_STATUS_OPCODE\
    )

/**
    \brief API to store the current state of an element as a Scene.

    \par Description
    Scene Store Unacknowledged is an unacknowledged message used to store the current state of an element as a Scene, which can be recalled later.

    \param [in] param Scene Store message

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_scene_store_unacknowledged(param) \
    MS_scene_client_send_reliable_pdu \
    (\
     MS_ACCESS_SCENE_STORE_UNACKNOWLEDGED_OPCODE,\
     param,\
     0xFFFFFFFF\
    )

/**
    \brief API to delete a Scene from the Scene Register state of an element.

    \par Description
    Scene Delete is an acknowledged message used to delete a Scene from the Scene Register state of an element.
    The response to the Scene Delete message is a Scene Register Status message.

    \param [in] param Scene Delete parameter

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_scene_delete(param) \
    MS_scene_client_send_reliable_pdu \
    (\
     MS_ACCESS_SCENE_DELETE_OPCODE,\
     param,\
     MS_ACCESS_SCENE_REGISTER_STATUS_OPCODE\
    )

/**
    \brief API to delete a Scene from the Scene Register state of an element.

    \par Description
    Scene Delete Unacknowledged is an unacknowledged message used to delete a scene from the Scene Register state of an element.

    \param [in] param Scene Delete parameter

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_scene_delete_unacknowledged(param) \
    MS_scene_client_send_reliable_pdu \
    (\
     MS_ACCESS_SCENE_DELETE_UNACKNOWLEDGED_OPCODE,\
     param,\
     0xFFFFFFFF\
    )
/** \} */
/** \} */
/** \} */

#endif /*_H_MS_SCENE_API_ */
