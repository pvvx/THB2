
/**
    \file MS_access_api.h

    \brief This file defines the Mesh Access Application Interface - includes
    Data Structures and Methods.
*/

/*
    Copyright (C) 2017. Mindtree Ltd.
    All rights reserved.
*/

#ifndef _H_MS_ACCESS_API_
#define _H_MS_ACCESS_API_


/* --------------------------------------------- Header File Inclusion */
/* Transport Layer */
#include "MS_trn_api.h"

#ifdef MS_STORAGE
    #include "nvsto.h"
#endif /* MS_STORAGE */

/* --------------------------------------------- Global Definitions */

/**
    \defgroup access_module ACCESS (Mesh Access Layer)
    \{
    This section describes the interfaces & APIs offered by the EtherMind
    Mesh Access (ACCESS) module to the Application and other upper
    layers of the stack.
*/

/**
    \defgroup access_defines Defines
    \{
    Describes defines for the module.
*/

/**
    \defgroup access_constants Constants
    \{
    Describes Constants defined by the module.
*/

/**
    Maximum Access Packet size.
    32 segments of 12 octets each.
*/
#define MS_ACCESS_MAX_PKT_SIZE                                                  384


/** Array sizes for use in the Access layer */
/** Size of Virtual Address (Label UUID) */
#define MS_ACCESS_VADDR_LABEL_UUID_SIZE                                         16
/** Size of NetKey */
#define MS_ACCESS_NETKEY_SIZE                                                   16
/** Size of AppKey */
#define MS_ACCESS_APPKEY_SIZE                                                   16

/** Default Node Identifier */
#define MS_ACCESS_DEFAULT_NODE_ID                                               0x00

/** Default Element Handle */
#define MS_ACCESS_DEFAULT_ELEMENT_HANDLE                                        0x00

/** Get Request */
#define MS_ACCESS_GET_REQ                                                       0x01

/** Set Request */
#define MS_ACCESS_SET_REQ                                                       0x02

/** Invalid default TTL value */
#define ACCESS_INVALID_DEFAULT_TTL                                              0xFF

/** Maximum TTL value - used as initializer */
#define ACCESS_MAX_TTL                                                          0x05 //0x7F

/** Model Specific Request Message Type: Get, Set or Others */
/** Model Specific Request Message Type: Get */
#define MS_ACCESS_MODEL_REQ_MSG_T_GET                                           0
/** Model Specific Request Message Type: Set */
#define MS_ACCESS_MODEL_REQ_MSG_T_SET                                           1
/** Model Specific Request Message Type: Others */
#define MS_ACCESS_MODEL_REQ_MSG_T_OTHERS                                        2

/** Key Refersh Phase states */
/** Key Refersh Phase - Normal */
#define MS_ACCESS_KEY_REFRESH_PHASE_NORMAL                                      0x00
/** Key Refersh Phase - 1 */
#define MS_ACCESS_KEY_REFRESH_PHASE_1                                           0x01
/** Key Refersh Phase - 2 */
#define MS_ACCESS_KEY_REFRESH_PHASE_2                                           0x02
/** Key Refersh Phase - 3 */
#define MS_ACCESS_KEY_REFRESH_PHASE_3                                           0x03

/** Invalid Access Address */
#define MS_ACCESS_ADDRESS_INVALID_HANDLE                                        0xFFFFFFFF

/** \} */

/** \} */

/**
    \defgroup access_events Events
    \{
    This section lists the Asynchronous Events notified to Application by the
    Module.
*/

/** \} */

/**
    \defgroup access_marcos Utility Macros
    \{
    This section defines the utility macros for use by the application.

*/
/** Populates the given element with the Model information */
#define MS_ACCESS_ASSIGN_ELEMENT(pelement, loc) \
    (pelement)->loc = (loc)

/** Initializes the SIG model with the given ID and callback information */
#define MS_ACCESS_INIT_SIG_MODEL(pmodel, id, eh, cb, pub_cb, num_op, op) \
    (pmodel)->model_id.type = MS_ACCESS_MODEL_TYPE_SIG; \
    (pmodel)->model_id.id = (id); \
    (pmodel)->elem_handle = (eh); \
    (pmodel)->cb = (cb); \
    (pmodel)->pub_cb = (pub_cb); \
    (pmodel)->num_opcodes = (num_op); \
    (pmodel)->opcodes = (op)

/** Initializes the Vendor model with the given ID and callback information */
#define MS_ACCESS_INIT_VENDOR_MODEL(pmodel, id, eh, cb, pub_cb, num_op, op) \
    (pmodel)->model_id.type = MS_ACCESS_MODEL_TYPE_VENDOR; \
    (pmodel)->model_id.id = (id); \
    (pmodel)->elem_handle = (eh); \
    (pmodel)->cb = (cb); \
    (pmodel)->pub_cb = (pub_cb); \
    (pmodel)->num_opcodes = (num_op); \
    (pmodel)->opcodes = (op)

/** \} */

/* --------------------------------------------- Data Types/ Structures */

/** Access Model Handle */
typedef UINT16          MS_ACCESS_MODEL_HANDLE;


/**
    \defgroup access_cb Application Callback
    \{
    This Section Describes the module Notification Callback interface offered
    to the application
*/
/**
    Access Layer Application Asynchronous Notification Callback.

    Access Layer calls the registered callback to indicate events occurred to the
    application.

    \param handle        Model Handle.
    \param saddr         16 bit Source Address.
    \param daddr         16 bit Destination Address.
    \param subnet_handle Subnet Handle.
    \param appkey_handle AppKey Handle.
    \param opcode        Opcode.
    \param data_param    Data associated with the event if any or NULL.
    \param data_len      Size of the event data. 0 if event data is NULL.
*/
typedef API_RESULT (* MS_ACCESS_MODEL_CB)
(
    MS_ACCESS_MODEL_HANDLE* handle,
    MS_NET_ADDR              saddr,
    MS_NET_ADDR              daddr,
    MS_SUBNET_HANDLE         subnet_handle,
    MS_APPKEY_HANDLE         appkey_handle,
    UINT32                   opcode,
    UCHAR*                   data_param,
    UINT16                   data_len
) DECL_REENTRANT;

/**
    Access Layer Model Publication Timeout Callback.

    Access Layer calls the registered callback to indicate Publication Timeout
    for the associated model.

    \param handle        Model Handle.
    \param blob          Blob if any or NULL.
*/
typedef API_RESULT (* MS_ACCESS_MODEL_PUBLISH_TIMEOUT_CB)
(
    MS_ACCESS_MODEL_HANDLE* handle,
    void*                    data_param
) DECL_REENTRANT;
/** \} */

/**
    \defgroup access_structures Structures
    \{
*/

/** SIG Model ID */
typedef UINT16    MS_ACCESS_MODEL_ID_SIG;

/** Vendor Model ID */
typedef UINT32    MS_ACCESS_MODEL_ID_VENDOR;

/** Access Node ID */
typedef UINT8 MS_ACCESS_NODE_ID;

/** Access Element Handle */
typedef UINT8 MS_ACCESS_ELEMENT_HANDLE;

/** Access Address Handle */
typedef UINT32 MS_ACCESS_ADDRESS_HANDLE;

/** Device Key Handle */
typedef UINT32 MS_ACCESS_DEV_KEY_HANDLE;

/** Model ID datatype */
typedef struct _MS_ACCESS_MODEL_ID
{
    /** Vendor/SIG ID */
    UINT32 id;

    /**
        Model type
        - SIG or Vendor
    */
    UCHAR type;

} MS_ACCESS_MODEL_ID;

/**
    Data structure for model.

    Models could be bluetooth SIG defined or vendor defined.
*/
typedef struct _MS_ACCESS_MODEL
{
    /** Model ID */
    MS_ACCESS_MODEL_ID                 model_id;

    /** Associated Element Handle */
    MS_ACCESS_ELEMENT_HANDLE           elem_handle;

    /**
        Callback function pointer to receive packets from the underlying
        protocol layers
    */
    MS_ACCESS_MODEL_CB                 cb;

    /**
        Callback function called when Publication Timer expires.
        Set to NULL if model does not support periodic publication.
    */
    MS_ACCESS_MODEL_PUBLISH_TIMEOUT_CB pub_cb;

    /** Number of Opcodes */
    UINT16                             num_opcodes;

    /** List of Opcodes */
    DECL_CONST UINT32*                   opcodes;

} MS_ACCESS_MODEL;


/**
    Element description format.
*/
typedef struct _MS_ACCESS_ELEMENT_DESC
{
    /** Location descriptor */
    UINT16 loc;

} MS_ACCESS_ELEMENT_DESC;

/**
    Unicast/Virtual/Group Address.
*/
typedef struct _MS_ACCESS_ADDRESS
{
    /** Flag - which field to be used */
    UINT8        use_label;

    /** Address */
    MS_NET_ADDR  addr;

    /** Label UUID */
    UINT8        label[MS_LABEL_UUID_LENGTH];

} MS_ACCESS_ADDRESS;

/**
    Access Publication related information
*/
typedef struct _MS_ACCESS_PUBLISH_INFO
{
    /** PublishAddress (Unicast/Virtual/Group) */
    MS_ACCESS_ADDRESS     addr;

    /**
        - AppKey Index  (when set from remote).
        - AppKey Handle (when set from locally for Configuration Client).
    */
    UINT16          appkey_index;

    /** CredentialFlag */
    UINT8           crden_flag;

    /** PublishTTL */
    UINT8           ttl;

    /** PublishPeriod */
    UINT8           period;

    /** PublishRetransmitCount */
    UINT8           rtx_count;

    /** PublishRetransmitIntervalSteps */
    UINT8           rtx_interval_steps;

    /** Flag - if called from local or remote */
    UINT8           remote;

} MS_ACCESS_PUBLISH_INFO;

/**
    Context of message received for a specific model instance.
    This is required to send response appropriately.
*/
typedef struct _MS_ACCESS_MODEL_REQ_MSG_CONTEXT
{
    /** Model Handle - for which request is received */
    MS_ACCESS_MODEL_HANDLE   handle;

    /** Source Address - oritinator of request */
    MS_NET_ADDR              saddr;

    /** Destination Address - of the request */
    MS_NET_ADDR              daddr;

    /** Associated Subnet Identifier */
    MS_SUBNET_HANDLE         subnet_handle;

    /** Associated AppKey Identifier */
    MS_APPKEY_HANDLE         appkey_handle;

} MS_ACCESS_MODEL_REQ_MSG_CONTEXT;

/** Uninterpreted/raw received message for a specific model instance. */
typedef struct _MS_ACCESS_MODEL_REQ_MSG_RAW
{
    /** Request Opcode */
    UINT32                   opcode;

    /** Raw received message */
    UCHAR*                   data_param;

    /** Raw received message length */
    UINT16                   data_len;

} MS_ACCESS_MODEL_REQ_MSG_RAW;

/** Requested message type for a specific model instance. */
typedef struct _MS_ACCESS_MODEL_REQ_MSG_T
{
    /** Flag: GET, SET or Others */
    UINT8      type;

    /** Flag: True or False */
    UINT8      to_be_acked;

} MS_ACCESS_MODEL_REQ_MSG_T;

/** Model specific state parameters in a request or response message */
typedef struct _MS_ACCESS_MODEL_STATE_PARAMS
{
    /** State Type */
    UCHAR state_type;

    /** State pointer */
    void* state;

} MS_ACCESS_MODEL_STATE_PARAMS;

/** Additional paramters in a Model specific request or response message */
typedef struct _MS_ACCESS_MODEL_EXT_PARAMS
{
    /** State/Extended Type */
    UCHAR ext_type;

    /** State/Extended data structure pointer */
    void* ext;

} MS_ACCESS_MODEL_EXT_PARAMS;

/**
    Provisioned Device List Data Structure, containing Primary Element Address
    and number of elements.
*/
typedef struct _MS_PROV_DEV_ENTRY
{
    /** Unicast Address of the first element */
    MS_NET_ADDR    uaddr;

    /** Number of Elements */
    UCHAR          num_elements;

    /*Receive notify count*/
    UINT16          rcv_flag;
} MS_PROV_DEV_ENTRY;

/** \} */


/* --------------------------------------------- Function */

/**
    \defgroup access_api_defs API Definitions
    \{
    This section describes the EtherMind Mesh Access Layer APIs.
*/

#ifdef __cplusplus
extern "C" {
#endif

/**
    \brief Create a new node in the device.

    \par Description
    This routine creates a new node in the device. This can be used by the
    application to create extra nodes if required in addition to the default
    primary node.

    \param [out] node_id Identifier to reference the newly created node.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_access_create_node (/* OUT */ MS_ACCESS_NODE_ID* node_id);

/**
    \brief Register an element with the access layer.

    \par Description
    This routine registers an element that can be populated with the models
    information to a specific node in the device identified by the node id.

    \param [in] node_id Node to which the element needs to be registered. This
    value is always 0 for the default node.

    \param [in] element Pointer to the element descriptor that needs to be
    registered to the node.

    \param [out] element_handle Identifier to reference the newly registered element.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_access_register_element
(
    /* IN */  MS_ACCESS_NODE_ID            node_id,
    /* IN */  MS_ACCESS_ELEMENT_DESC*      element,
    /* OUT */ MS_ACCESS_ELEMENT_HANDLE*    element_handle
);

/**
    \brief Register a model with the access layer.

    \par Description
    This routine registers a model associated with an element with the access layer.

    \param [in] node_id Node to which the model needs to be registered. This
    value is always 0 for the default node.

    \param [in] model Pointer to the model descriptor that needs to be
    registered to the node.

    \param [in, out] model_handle
                     Model identifier associated with the model instance on successful registration.
                     After power cycle of an already provisioned node, the model handle will have
                     valid value and the same will be reused for registration.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_access_register_model
(
    /* IN */     MS_ACCESS_NODE_ID         node_id,
    /* IN */     MS_ACCESS_MODEL*          model,
    /* INOUT */  MS_ACCESS_MODEL_HANDLE*   model_handle
);

/**
    \brief Get element handle.

    \par Description
    This routine searches for the element handle associated with specific element address.

    \param [in]  elem_addr Address of the corresponding element.
    \param [out] handle    Element handle associated with the element address.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_access_get_element_handle
(
    /* IN */   MS_NET_ADDR                 elem_addr,
    /* OUT */  MS_ACCESS_ELEMENT_HANDLE*   handle
);

/**
    \brief Get model handle.

    \par Description
    This routine searches for the model handle associated with specific model ID.

    \param [in]  elem_handle Element Identifier associated with the Model.
    \param [in]  model_id    Model Identifier for which the model handle to be searched.
    \param [out] handle      Model handle associated with model ID.
    If not found, handle will be set as NULL.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_access_get_model_handle
(
    /* IN */   MS_ACCESS_ELEMENT_HANDLE      elem_handle,
    /* IN */   MS_ACCESS_MODEL_ID            model_id,
    /* OUT */  MS_ACCESS_MODEL_HANDLE*       handle
);

API_RESULT MS_access_get_appkey_handle
(
    /* IN */   MS_ACCESS_MODEL_HANDLE*         handle,
    /* OUT */  MS_APPKEY_HANDLE*                appkey_handle
);


/**
    \brief API to publish access layer message.

    \par Description
    This routine publishes Access Layer message to the publish address associated with the model.

    \param [in] handle
           Access Model Handle for which message to be sent.

    \param [in] opcode
           Access Opcode

    \param [in] data_param
           Data packet

    \param [in] data_len
           Data packet length

    \param [in] reliable
           MS_TRUE for reliable message. MS_FALSE otherwise.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_access_publish
(
    /* IN */ MS_ACCESS_MODEL_HANDLE*   handle,
    /* IN */ UINT32                    opcode,
    /* IN */ UCHAR*                    data_param,
    /* IN */ UINT16                    data_len,
    /* IN */ UINT8                     reliable
);

/**
    \brief API to reliably publish access layer message.

    \par Description
    This routine reliably publishes Access Layer message to the publish address associated with the model.

    \param [in] handle
           Access Model Handle for which message to be sent.

    \param [in] req_opcode
           Request Opcode

    \param [in] data_param
           Data packet

    \param [in] data_len
           Data packet length

    \param [in] rsp_opcode
           Response Opcode

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_access_reliable_publish
(
    /* IN */ MS_ACCESS_MODEL_HANDLE*   handle,
    /* IN */ UINT32                    req_opcode,
    /* IN */ UCHAR*                    data_param,
    /* IN */ UINT16                    data_len,
    /* IN */ UINT32                    rsp_opcode
);

/**
    \brief API to reply to access layer message.

    \par Description
    This routine replies to Access Layer message.

    \param [in] handle        Model Handle.
    \param [in] saddr         16 bit Source Address.
    \param [in] daddr         16 bit Destination Address.
    \param [in] subnet_handle Subnet Handle.
    \param [in] appkey_handle AppKey Handle.
    \param [in] ttl           Time to Live.
    \param [in] opcode        Access Opcode
    \param [in] data_param    Access parameter, based on the opcode
    \param [in] data_length   Access parameter length, based on the opcode

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_access_reply
(
    /* IN */ MS_ACCESS_MODEL_HANDLE*   handle,
    /* IN */ MS_NET_ADDR               saddr,
    /* IN */ MS_NET_ADDR               daddr,
    /* IN */ MS_SUBNET_HANDLE          subnet_handle,
    /* IN */ MS_APPKEY_HANDLE          appkey_handle,
    /* IN */ UINT8                     ttl,
    /* IN */ UINT32                    opcode,
    /* IN */ UCHAR*                    data_param,
    /* IN */ UINT16                    data_length
);

/**
    \brief API to reply to access layer message and optionally also to publish.

    \par Description
    This routine replies to Access Layer message and also publish if requested by application.

    \param [in] handle        Model Handle.
    \param [in] saddr         16 bit Source Address.
    \param [in] daddr         16 bit Destination Address.
    \param [in] subnet_handle Subnet Handle.
    \param [in] appkey_handle AppKey Handle.
    \param [in] ttl           Time to Live.
    \param [in] opcode        Access Opcode
    \param [in] data_param    Access parameter, based on the opcode
    \param [in] data_length   Access parameter length, based on the opcode
    \param [in] to_publish    Flag to indicate if the message also to be published

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_access_reply_and_publish
(
    /* IN */ MS_ACCESS_MODEL_HANDLE*   handle,
    /* IN */ MS_NET_ADDR               saddr,
    /* IN */ MS_NET_ADDR               daddr,
    /* IN */ MS_SUBNET_HANDLE          subnet_handle,
    /* IN */ MS_APPKEY_HANDLE          appkey_handle,
    /* IN */ UINT8                     ttl,
    /* IN */ UINT32                    opcode,
    /* IN */ UCHAR*                    data_param,
    /* IN */ UINT16                    data_len,
    /* IN */ UINT8                     to_publish
);

/**
    \brief API to send Access PDUs

    \par Description
    This routine sends transport PDUs to peer device.

    \param [in] saddr         16 bit Source Address.
    \param [in] daddr         16 bit Destination Address.
    \param [in] subnet_handle Subnet Handle.
    \param [in] appkey_handle AppKey Handle.
    \param [in] ttl           Time to Live.
    \param [in] opcode        Access Opcode
    \param [in] data_param    Access parameter, based on the opcode
    \param [in] data_length   Access parameter length, based on the opcode
    \param [in] reliable      If requires lower transport Ack, set reliable as TRUE

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_access_send_pdu
(
    /* IN */ MS_NET_ADDR               saddr,
    /* IN */ MS_NET_ADDR               daddr,
    /* IN */ MS_SUBNET_HANDLE          subnet_handle,
    /* IN */ MS_APPKEY_HANDLE          appkey_handle,
    /* IN */ UINT8                     ttl,
    /* IN */ UINT32                    opcode,
    /* IN */ UCHAR*                    data_param,
    /* IN */ UINT16                    data_length,
    /* IN */ UINT8                     reliable
);

/** TBD: add function header */
API_RESULT MS_access_get_composition_data(/* OUT */ MS_BUFFER* buffer);

/* Configuration Manager related interfaces */

/**
    \brief To reset a node

    \par Description
    This routine resets a node (other than a Provisioner) and removes it from the network.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_access_cm_reset(UINT8 role);

/**
    \brief To get the number of elements in local node

    \par Description
    This routine retrieves the number of elements in local node.

    \param [out] count     Number of elements

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_access_cm_get_element_count
(
    /* OUT */ UINT8*   count
);

/**
    \brief To set primary unicast address

    \par Description
    This routine sets primary unicast address.

    \param [in] addr     Primary Unicast Address to be set

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_access_cm_set_primary_unicast_address
(
    /* IN */ MS_NET_ADDR    addr
);

/**
    \brief To get primary unicast address

    \par Description
    This routine gets primary unicast address.

    \param [out] addr     Memory location where Primary Unicast Address to be filled

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_access_cm_get_primary_unicast_address
(
    /* OUT */ MS_NET_ADDR*     addr
);

/**
    \brief To set default TTL

    \par Description
    This routine sets default TTL.

    \param [in] ttl     Default TTL to be set

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_access_cm_set_default_ttl
(
    /* IN */ UINT8    ttl
);

/**
    \brief To get default TTL

    \par Description
    This routine gets default TTL.

    \param [in] ttl     Memory location where default TTL to be filled

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_access_cm_get_default_ttl
(
    /* IN */ UINT8*     ttl
);

/**
    \brief To set IV Index

    \par Description
    This routine sets IV Index.

    \param [in] iv_index          IV Index to be set
    \param [in] iv_update_flag    IV Update Flag

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_access_cm_set_iv_index
(
    /* IN */ UINT32    iv_index,
    /* IN */ UINT8     iv_update_flag
);

/**
    \brief To get IV Index

    \par Description
    This routine gets IV Index.

    \param [out] iv_index          Memory location where IV Index to be filled
    \param [out] iv_update_flag    Memory location where IV Update Flag to be filled

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_access_cm_get_iv_index
(
    /* OUT */ UINT32*     iv_index,
    /* OUT */ UINT8*      iv_update_flag
);

/**
    \brief To get IV Index by IVI

    \par Description
    This routine gets IV Index based on the IVI in the received packet.

    \param [in]  ivi          Least Significant bit of the IV Index used
                              in the nonce to authenticate and encrypt
                              the Network PDU.
    \param [out] iv_index     Memory location where IV Index to be filled

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_access_cm_get_iv_index_by_ivi
(
    /* IN */  UINT8       ivi,
    /* OUT */ UINT32*     iv_index
);

/**
    \brief To enable/disable a feature

    \par Description
    This routine enables/disables a feature field.

    \param [in] enable     Enable or Disable
    \param [in] feature    Relay, proxy, friend or Low Power

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_access_cm_set_features_field
(
    /* IN */ UINT8    enable,
    /* IN */ UINT8    feature
);

/**
    \brief To get state of a feature

    \par Description
    This routine gets the state of a feature field.

    \param [out] enable     Memory location where Enable or Disable status to be filled.
    \param [in]  feature    Relay, proxy, friend or Low Power

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_access_cm_get_features_field
(
    /* OUT */ UINT8*   enable,
    /* IN */  UINT8    feature
);

/**
    \brief To get state of all features

    \par Description
    This routine gets the state of all features.

    \param [out] features    State of Relay, proxy, friend and Low Power field

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_access_cm_get_features
(
    /* OUT */ UINT8*    features
);

/** Enable Relay Feature */
#define MS_ENABLE_RELAY_FEATURE() \
    MS_access_cm_set_features_field(MS_ENABLE, MS_FEATURE_RELAY)

/** Disable Relay Feature */
#define MS_DISABLE_RELAY_FEATURE() \
    MS_access_cm_set_features_field(MS_DISABLE, MS_FEATURE_RELAY)

/** Enable Proxy Feature */
#define MS_ENABLE_PROXY_FEATURE() \
    MS_access_cm_set_features_field(MS_ENABLE, MS_FEATURE_PROXY)

/** Disable Proxy Feature */
#define MS_DISABLE_PROXY_FEATURE() \
    MS_access_cm_set_features_field(MS_DISABLE, MS_FEATURE_PROXY)

/** Enable Friend Feature */
#define MS_ENABLE_FRIEND_FEATURE() \
    MS_access_cm_set_features_field(MS_ENABLE, MS_FEATURE_FRIEND)

/** Disable Friend Feature */
#define MS_DISABLE_FRIEND_FEATURE() \
    MS_access_cm_set_features_field(MS_DISABLE, MS_FEATURE_FRIEND)

/** Enable Low Power Feature */
#define MS_ENABLE_LPN_FEATURE() \
    MS_access_cm_set_features_field(MS_ENABLE, MS_FEATURE_LPN)

/** Disable Low Power Feature */
#define MS_DISABLE_LPN_FEATURE() \
    MS_access_cm_set_features_field(MS_DISABLE, MS_FEATURE_LPN)

/** Enable Secure Nework Beacon Feature */
#define MS_ENABLE_SNB_FEATURE() \
    MS_access_cm_set_features_field(MS_ENABLE, MS_FEATURE_SEC_NET_BEACON)

/** Disable Secure Nework Beacon Feature */
#define MS_DISABLE_SNB_FEATURE() \
    MS_access_cm_set_features_field(MS_DISABLE, MS_FEATURE_SEC_NET_BEACON)


/**
    \brief To get friendship role of the node

    \par Description
    This routine gets the current friendship role of the node.

    \param [out] frnd_role    Friend role

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_access_cm_get_friendship_role
(
    /* OUT */ UINT8*   frnd_role
);


/**
    \brief To set friendship role of the node

    \par Description
    This routine sets the current friendship role of the node.

    \param [out] frnd_role    Friend role

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_access_cm_set_friendship_role
(
    /* IN */ UINT8   frnd_role
);


/**
    \brief To add Device Key

    \par Description
    This routine adds Device Key entry, along with corresponding
    Primary Device Address and Number of elements.

    \param [in] dev_key        Device Key to be added.
    \param [in] uaddr          Unicast Address of the first element.
    \param [in] num_elements   Number of elements.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_access_cm_add_device_key
(
    /* IN */ UINT8*         dev_key,
    /* IN */ MS_NET_ADDR    uaddr,
    /* IN */ UINT8          num_elements
);

/**
    \brief To get Device Key

    \par Description
    This routine gets Device Key entry.

    \param [in]  dev_key_index    Device Key Index.
    \param [out] dev_key          Pointer to Device Key to be returned.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_access_cm_get_device_key
(
    /* IN */  UINT8     dev_key_index,
    /* OUT */ UINT8**   dev_key
);

/**
    \brief To remove all Device Keys

    \par Description
    This routine removes all Device Keys from table.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_access_cm_remove_all_device_keys(void);

/**
    \brief To get list of Provisioned Device List

    \par Description
    This routine returns list of Provisioned Devices from the Device Key Table.

    \param [in]    prov_dev_list   Provisioned Device List.
    \param [inout] num_entries     Size of the Device Key List provided by the caller.
                                   This routine will return the number of entries
                                   in the Device Key Table.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_access_cm_get_prov_devices_list
(
    /* IN */    MS_PROV_DEV_ENTRY*    prov_dev_list,
    /* OUT */ UINT16*               num_entries,
    /* OUT */ UINT16*               pointer
);

/**
    \brief To get Device Key Handle

    \par Description
    This routine returns Device Key Handle for a given Primary Element Address
    entry in Device Key Table.

    \param [in]  prim_elem_uaddr   Primary element address to be searched
    \param [out] handle            Device Key Table Handle, if match is found.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_access_cm_get_device_key_handle
(
    /* IN */  MS_NET_ADDR                  prim_elem_uaddr,
    /* OUT */ MS_ACCESS_DEV_KEY_HANDLE*    handle
);
/**
    \brief To delete Device Key

    \par Description
    This routine returns status for a given Primary Element Address
    entry in Device Key Table.

    \param [in] handle            Device Key Table Handle, if match is found.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_access_cm_delete_device_key
(
    /* IN */ MS_ACCESS_DEV_KEY_HANDLE       handle
);


/**
    \brief To get AppKey

    \par Description
    This routine gets AppKey along with AID entry.

    \param [in]  appkey_handle    AppKey Handle.
    \param [out] app_key          Pointer to AppKey to be returned.
    \param [out] aid              Pointer to AID to be returned.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_access_cm_get_app_key
(
    /* IN */  MS_APPKEY_HANDLE   appkey_handle,
    /* OUT */ UINT8**            app_key,
    /* OUT */ UINT8*             aid
);

/**
    \brief To add/update NetKey

    \par Description
    This routine adds/updates NetKey entry. Each NetKey is associated with a subnet.

    \param [in] netkey_index     Identifies global Index of NetKey. A 12-bit value.
    \param [in] opcode           To identify Add or Update NetKey
    \param [in] net_key          Associated NetKey to be added/updated.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_access_cm_add_update_netkey
(
    /* IN */ UINT16   netkey_index,
    /* IN */ UINT32   opcode,
    /* IN */ UINT8*   net_key
);

/**
    \brief To add Security Credential of a LPN or the Friend.

    \par Description
    This routine adds NID, privacy and encryption keys associated with a friendship.

    \param [in] subnet_handle    Identifies associated subnet.
    \param [in] friend_offset    Friend Offset.
    \param [in] lpn_addr         Address of the LPN.
    \param [in] friend_addr      Address of the Friend.
    \param [in] lpn_counter      Number of Friend Request messages the LPN has sent.
    \param [in] friend_counter   Number of Friend Offer messages the Friend has sent.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_access_cm_add_friend_sec_credential
(
    /* IN */ MS_SUBNET_HANDLE    subnet_handle,
    /* IN */ UINT16              friend_offset,
    /* IN */ MS_NET_ADDR         lpn_addr,
    /* IN */ MS_NET_ADDR         friend_addr,
    /* IN */ UINT16              lpn_counter,
    /* IN */ UINT16              friend_counter
);

/**
    \brief To delete the Security Credential of a LPN or the Friend.

    \par Description
    This routine deletes NID, privacy and encryption keys associated with a friendship.

    \param [in] subnet_handle    Identifies associated subnet.
    \param [in] friend_index     Friend Index.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_access_cm_delete_friend_sec_credential
(
    /* IN */ MS_SUBNET_HANDLE    subnet_handle,
    /* IN */ UINT16              friend_index
);

/**
    \brief To find a Subnet associated with the NetKey

    \par Description
    This routine finds a Subnet based on the NetKey entry. Each NetKey is associated with a subnet.

    \param [in]  netkey_index     Identifies global Index of NetKey, corresponding Subnet to be returned.
    \param [out] subnet_handle    Memory location to be filled with Subnet Handle, if search is successful.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_access_cm_find_subnet
(
    /* IN */  UINT16               netkey_index,
    /* OUT */ MS_SUBNET_HANDLE*    subnet_handle
);

/**
    \brief To find the Master Subnet associated with the friend security credential, identified by Friend Subnet Handle.

    \par Description
    This routine finds the Master Subnet based on the friend security credential, identified by Friend Subnet Handle.

    \param [out] friend_subnet_handle    Idetifies the Friend Subnet Handle, corresponding to Friend Subnet Handle.
    \param [in]  master_subnet_handle    Memory location to be filled with Master Subnet Handle, if search is successful.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_access_cm_find_master_subnet
(
    /* IN */  MS_SUBNET_HANDLE     friend_subnet_handle,
    /* OUT */ MS_SUBNET_HANDLE*    master_subnet_handle
);

/**
    \brief To delete NetKey

    \par Description
    This routine deletes a NetKey entry. Each NetKey is associated with a subnet.

    \param [in] subnet_handle     Handle of the Subnet for which NetKey to be deleted.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_access_cm_delete_netkey
(
    /* IN */ MS_SUBNET_HANDLE   subnet_handle
);

/**
    \brief To get NetKey

    \par Description
    This routine fetches a NetKey entry. Each NetKey is associated with a subnet.

    \param [in]  subnet_handle     Handle of the Subnet for which NetKey to be deleted.
    \param [out] net_key           Netkey associated with the Subnet.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_access_cm_get_netkey_at_offset
(
    /* IN */  MS_SUBNET_HANDLE    subnet_handle,
    /* IN */  UINT8               offset,
    /* OUT */ UINT8*              net_key
);


/**
    \brief To get list of all known NetKeys

    \par Description
    This routine returns a list of known NetKey Indices.

    \param [inout] netkey_count   Caller fills with maximum number of NetKey Indices
                                  that can be stored in 'netkey_index_list'.
                                  This function will update the value with how many NetKey
                                  Indices has been filled. If the number of available
                                  NetKey Indices is more than that can be returned,
                                  maximum possible Indices will be filled and
                                  an appropriate error values will inform the caller,
                                  there are more NetKey Indices (as an information).
    \param [out] netkey_index_list Memory to be filled with the available NetKey Indices.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_access_cm_get_netkey_index_list
(
    /* INOUT */ UINT16* netkey_count,
    /* OUT */   UINT16* netkey_index_list
);

/**
    \brief To search for NID

    \par Description
    This routine searches for matching NID in subnet table.

    \param [in] nid    NID to be searched in all known subnets for match.
    \param [inout] subnet_handle Same NID can match with multiple subnets.
                                 Caller will fill this value to indicate from which
                                 subnet handle the search to be started. This function
                                 will return the subnet handle, where the match is found
                                 (in case of match). Caller while searching for the same
                                 NID, in the subsequent call can pass the subnet_handle
                                 received in the previous match for the NID.
                                 For the very first call when searching for a NID,
                                 the caller need to use Invalid Subnet Handle
                                 \ref MS_INVALID_SUBNET_HANDLE.
    \param [out] privacy_key    Privacy Key associated with the subnet.
    \param [out] encrypt_key    Encyption Key associated with the subnet.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_access_cm_lookup_nid
(
    /* IN */    UINT8               nid,
    /* INOUT */ MS_SUBNET_HANDLE*   subnet_handle,
    /* OUT */   UINT8*              privacy_key,
    /* OUT */   UINT8*              encrypt_key
);


/**
    \brief To search for Network ID

    \par Description
    This routine searches for matching Network ID in subnet table.

    \param [in] network_id       Network ID to be searched in all known subnets for match.
    \param [inout] subnet_handle Same NID can match with multiple subnets.
                                 Caller will fill this value to indicate from which
                                 subnet handle the search to be started. This function
                                 will return the subnet handle, where the match is found
                                 (in case of match). Caller while searching for the same
                                 NID, in the subsequent call can pass the subnet_handle
                                 received in the previous match for the NID.
                                 For the very first call when searching for a NID,
                                 the caller need to use Invalid Subnet Handle
                                 \ref MS_INVALID_SUBNET_HANDLE.
    \param [out] beacon_key      Beacon Key associated with the subnet.
    \param [out] is_new_key      Flag to indicate if the network ID is associated with
                                 the new Network Key being updated.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_access_cm_lookup_network_id
(
    /* IN */    UINT8*              network_id,
    /* INOUT */ MS_SUBNET_HANDLE*   subnet_handle,
    /* OUT */   UINT8*              beacon_key,
    /* OUT */   UINT8*              is_new_key
);

/**
    \brief To search for AID

    \par Description
    This routine searches for matching NID in subnet table.

    \param [in] aid    AID to be searched in all known AppKeys for match.
    \param [inout] appkey_handle Same AID can match with multiple AppKeys.
                                 Caller will fill this value to indicate from which
                                 AppKey handle the search to be started. This function
                                 will return the AppKey handle, where the match is found
                                 (in case of match). Caller while searching for the same
                                 AID, in the subsequent call can pass the appkey_handle
                                 received in the previous match for the AID.
                                 For the very first call when searching for a AID,
                                 the caller need to use Invalid Subnet Handle
                                 \ref MS_INVALID_APPKEY_HANDLE.
    \param [out] app_key         AppKey associated with the AID.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_access_cm_lookup_aid
(
    /* IN */    UINT8               aid,
    /* INOUT */ MS_APPKEY_HANDLE*   appkey_handle,
    /* OUT */   UINT8*              app_key
);

/**
    \brief Set Provisioning Data

    \par Description
    This routine configures the provisioning data with Access Layer.

    \param prov_data
           Provisioning data received during provisioning procedure.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_access_cm_set_prov_data
(
    /* IN */ PROV_DATA_S*     prov_data
);

API_RESULT MS_access_cm_set_prov_data_provsioner
(
    /* IN */ PROV_DATA_S*     prov_data
);


/**
    \brief To get NID associated with a subnet

    \par Description
    This routine fetches the NID associated with a subnet.

    \param [in]  handle         Handle identifing the subnet.
    \param [out] nid            NID associated with the subnet.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_access_cm_get_subnet_nid
(
    /* IN */  MS_SUBNET_HANDLE   handle,
    /* OUT */ UINT8*             nid
);

/**
    \brief To get Privacy Key associated with a subnet

    \par Description
    This routine fetches the Privacy Key associated with a subnet.

    \param [in]  handle         Handle identifing the subnet.
    \param [out] privacy_key    Privacy Key associated with the subnet.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_access_cm_get_subnet_privacy_key
(
    /* IN */  MS_SUBNET_HANDLE   handle,
    /* OUT */ UINT8*             privacy_key
);

/**
    \brief To get Network ID associated with a subnet

    \par Description
    This routine fetches the Netowrk ID associated with a subnet.

    \param [in]  handle         Handle identifing the subnet.
    \param [out] network_id     Network ID associated with the subnet.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_access_cm_get_subnet_network_id
(
    /* IN */  MS_SUBNET_HANDLE   handle,
    /* OUT */ UINT8*             network_id
);


/**
    \brief To get Beacon Key associated with a subnet

    \par Description
    This routine fetches the Beacon Key associated with a subnet.

    \param [in]  handle         Handle identifing the subnet.
    \param [out] beacon_key     Beacon Key associated with the subnet.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_access_cm_get_subnet_beacon_key
(
    /* IN */  MS_SUBNET_HANDLE  handle,
    /* OUT */ UINT8*              beacon_key
);

/**
    \brief To get Identity Key associated with a subnet

    \par Description
    This routine fetches the Identity Key associated with a subnet.

    \param [in]  handle         Handle identifing the subnet.
    \param [out] identity_key   Identity Key associated with the subnet.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_access_cm_get_subnet_identity_key
(
    /* IN */  MS_SUBNET_HANDLE  handle,
    /* OUT */ UINT8*              identity_key
);

/**
    \brief To get Encryption Key associated with a subnet

    \par Description
    This routine fetches the Encryption Key associated with a subnet.

    \param [in]  handle         Handle identifing the subnet.
    \param [out] encrypt_key    Encyption Key associated with the subnet.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_access_cm_get_subnet_encryption_key
(
    /* IN */  MS_SUBNET_HANDLE   handle,
    /* OUT */ UINT8*             encrypt_key
);

/**
    \brief To get Node Identity

    \par Description
    This routine gets Node Identity State of a node

    \param [in]  subnet_handle    Handle identifing the subnet.
    \param [out] id_state         Memory location where Node Identity state to be filled.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_access_cm_get_node_identity
(
    /* IN */  MS_SUBNET_HANDLE   subnet_handle,
    /* OUT */ UINT8*             id_state
);

/**
    \brief To set Node Identity

    \par Description
    This routine sets Node Identity State of a node

    \param [in] subnet_handle    Handle identifing the subnet.
    \param [in, out] id_state    Node Identity state to be set.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_access_cm_set_node_identity
(
    /* IN */    MS_SUBNET_HANDLE   subnet_handle,
    /* INOUT */ UINT8*               id_state
);


/**
    \brief To get Key Refresh Phase

    \par Description
    This routine gets Key Refresh Phase State of a node

    \param [in]  subnet_handle       Handle identifing the subnet.
    \param [out] key_refresh_state   Memory location where Key Refresh Phase state to be filled.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_access_cm_get_key_refresh_phase
(
    /* IN */  MS_SUBNET_HANDLE   subnet_handle,
    /* OUT */ UINT8*             key_refresh_state
);

/**
    \brief To set Key Refresh Phase

    \par Description
    This routine sets Key Refresh Phase State of a node

    \param [in] subnet_handle    Handle identifing the subnet.
    \param [in, out] key_refresh_state         Key Refresh Phase state to be set.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_access_cm_set_key_refresh_phase
(
    /* IN */ MS_SUBNET_HANDLE   subnet_handle,
    /* IN */ UINT8*             key_refresh_state
);

/**
    \brief To set Network/Relay Transmit state

    \par Description
    This routine sets Network/Relay Transmit state.

    \param [in] tx_state_type   Transmit State Type (Network or Relay)
    \param [in] tx_state        Composite state (3-bits of Tx Count and 5-bits of Tx Interval Steps)

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_access_cm_set_transmit_state
(
    /* IN */ UINT8    tx_state_type,
    /* IN */ UINT8    tx_state
);

/**
    \brief To get Network/Relay Transmit state

    \par Description
    This routine gets Network/Relay Transmit state.

    \param [in]  tx_state_type  Transmit State Type (Network or Relay)
    \param [out] tx_state       Memory location to fill Composite state
                                (3-bits of Tx Count and 5-bits of Tx Interval Steps)

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_access_cm_get_transmit_state
(
    /* IN */  UINT8    tx_state_type,
    /* OUT */ UINT8*   tx_state
);

/**
    \brief To add AppKey

    \par Description
    This routine adds AppKey entry. Each AppKey is associated with a subnet.

    \param [in] subnet_handle    Handle of the Subnet for which AppKey to be added.
    \param [in] appkey_index     Identifies global Index of AppKey. A 12-bit value.
    \param [in] app_key          Associated AppKey to be added.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_access_cm_add_appkey
(
    /* IN */ MS_SUBNET_HANDLE   subnet_handle,
    /* IN */ UINT16             appkey_index,
    /* IN */ UINT8*             app_key
);

/**
    \brief To update/delete AppKey

    \par Description
    This routine updates/deletes AppKey entry. Each AppKey is associated with a subnet.

    \param [in] subnet_handle    Handle of the Subnet for which AppKey to be updated/deleted.
    \param [in] appkey_index     Identifies global Index of AppKey. A 12-bit value.
    \param [in] opcode           To identify Delete or Update NetKey
    \param [in] app_key          Associated AppKey to be updated.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_access_cm_update_delete_appkey
(
    /* IN */ MS_SUBNET_HANDLE   subnet_handle,
    /* IN */ UINT16             appkey_index,
    /* IN */ UINT32             opcode,
    /* IN */ UINT8*             app_key
);

/**
    \brief To update AppKey

    \par Description
    This routine/macro updates AppKey entry. Each AppKey is associated with a subnet.

    \param [in] subnet_handle    Handle of the Subnet for which AppKey to be updated.
    \param [in] appkey_index     Identifies global Index of AppKey. A 12-bit value.
    \param [in] app_key          Associated AppKey to be updated.

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_access_cm_update_appkey(sn, aki, ak) \
    MS_access_cm_update_delete_appkey((sn), (aki), MS_ACCESS_CONFIG_APPKEY_UPDATE_OPCODE, (ak))

/**
    \brief To delete AppKey

    \par Description
    This routine/macro deletes AppKey entry. Each AppKey is associated with a subnet.

    \param [in] subnet_handle    Handle of the Subnet for which AppKey to be deleted.
    \param [in] appkey_index     Identifies global Index of AppKey. A 12-bit value.
    \param [in] app_key          Associated AppKey to be updated.

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_access_cm_delete_appkey(sn, aki, ak) \
    MS_access_cm_update_delete_appkey((sn), (aki), MS_ACCESS_CONFIG_APPKEY_DELETE_OPCODE, (ak))

/**
    \brief To get AppKey Handle for a given AppKey Index

    \par Description
    This routine gets AppKey Handle for a given AppKey Index. Each AppKey is associated with a subnet.

    \param [in]  subnet_handle    Handle of the Subnet for which AppKey to be updated.
    \param [in]  appkey_index     Identifies global Index of AppKey. A 12-bit value.
    \param [in]  app_key          Associated AppKey to be matched.
    \param [out] appkey_handle    Memory to hold the associated AppKey Handle.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_access_cm_get_appkey_handle
(
    /* IN */  MS_SUBNET_HANDLE    subnet_handle,
    /* IN */  UINT16              appkey_index,
    /* IN */  UINT8*              app_key,
    /* OUT */ MS_APPKEY_HANDLE*   appkey_handle
);

/**
    \brief To get list of all known AppKeys

    \par Description
    This routine returns a list of known AppKey Indices associated with a subnet.

    \param [in] subnet_handle     Handle of the Subnet for which AppKey to be returned.
    \param [inout] appkey_count   Caller fills with maximum number of AppKey Indices
                                  that can be stored in 'apptkey_index_list'.
                                  This function will update the value with how many AppKey
                                  Indices has been filled. If the number of available
                                  AppKey Indices is more than that can be returned,
                                  maximum possible Indices will be filled and
                                  an appropriate error values will inform the caller,
                                  there are more NetKey Indices (as an information).
    \param [out] appkey_index_list Memory to be filled with the available AppKey Indices.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_access_cm_get_appkey_index_list
(
    /* IN */    MS_SUBNET_HANDLE     subnet_handle,
    /* INOUT */ UINT16*              appkey_count,
    /* OUT */   UINT16*              appkey_index_list
);

/**
    \brief To bind a model with an AppKey

    \par Description
    This routine binds a model with an AppKey.

    \param [in] model_handle     Model handle identifing the model.
    \param [in] appkey_index     Identifies global Index of AppKey. A 12-bit value.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_access_bind_model_app
(
    /* IN */ MS_ACCESS_MODEL_HANDLE    model_handle,
    /* IN */ UINT16                    appkey_index
);

/**
    \brief To unbind a model with an AppKey

    \par Description
    This routine unbinds a model with an AppKey.

    \param [in] model_handle     Model handle identifing the model.
    \param [in] appkey_index     Identifies global Index of AppKey. A 12-bit value.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_access_unbind_model_app
(
    /* IN */ MS_ACCESS_MODEL_HANDLE    model_handle,
    /* IN */ UINT16                    appkey_index
);

/**
    \brief To get list of all AppKeys associated with a model

    \par Description
    This routine returns a list of known AppKey Indices associated with a model.

    \param [in] model_handle      Handle of the Model for which AppKey to be returned.
    \param [inout] appkey_count   Caller fills with maximum number of AppKey Indices
                                  that can be stored in 'apptkey_index_list'.
                                  This function will update the value with how many AppKey
                                  Indices has been filled. If the number of available
                                  AppKey Indices is more than that can be returned,
                                  maximum possible Indices will be filled and
                                  an appropriate error values will inform the caller,
                                  there are more NetKey Indices (as an information).
    \param [out] appkey_index_list Memory to be filled with the available AppKey Indices.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_access_cm_get_model_app_list
(
    /* IN */    MS_ACCESS_MODEL_HANDLE    model_handle,
    /* INOUT */ UINT16*                   appkey_count,
    /* OUT */   UINT16*                   appkey_index_list
);

/**
    \brief To set Publication information associated with a model

    \par Description
    This routine sets Publication information associated with a model.

    \param [in]    model_handle      Handle of the Model for which Publication info to be set.
    \param [inout] publish_info      Publication Information to be set.
                                     If Label UUID is used, on success corresponding
                                     Virtual Address will be filled and returned.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_access_cm_set_model_publication
(
    /* IN */    MS_ACCESS_MODEL_HANDLE    model_handle,
    /* INOUT */ MS_ACCESS_PUBLISH_INFO*   publish_info
);

/**
    \brief To set Publication Fast Period Divisor information associated with a model

    \par Description
    This routine sets Publication Fast Period Divisor information associated with a model.

    \param [in] model_handle      Handle of the Model for which Publication info to be set.
    \param [in] period_divisor    The value range for the Health Fast Period Divisor state is
                                  0 through 15, all other values are prohibited.
                                  This is used to divide the Health Publish Period by 2^n,
                                  where the n is the value of the Health Fast Period Divisor state.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_access_cm_set_model_publication_period_divisor
(
    /* IN */ MS_ACCESS_MODEL_HANDLE    model_handle,
    /* IN */ UINT8                     period_divisor
);

/**
    \brief To get Publication information associated with a model

    \par Description
    This routine returns Publication information associated with a model.

    \param [in]  model_handle      Handle of the Model for which Publication info to be returned.
    \param [out] publish_info      Memory to be filled with associated Publication info.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_access_cm_get_model_publication
(
    /* IN */  MS_ACCESS_MODEL_HANDLE    model_handle,
    /* OUT */ MS_ACCESS_PUBLISH_INFO*   publish_info
);

/**
    \brief To add an address to a model subscription list

    \par Description
    This routine adds an address to a subscription list of a model

    \param [in] model_handle      Handle of the Model for which address to be added in the subscription list.
    \param [in] sub_addr          Address to be added in subscription list.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_access_cm_add_model_subscription
(
    /* IN */ MS_ACCESS_MODEL_HANDLE    model_handle,
    /* IN */ MS_ACCESS_ADDRESS*        sub_addr
);

/**
    \brief To delete an address to a model subscription list

    \par Description
    This routine deletes an address to a subscription list of a model

    \param [in] model_handle      Handle of the Model for which address to be deleteed in the subscription list.
    \param [in] sub_addr          Address to be deleted from subscription list.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_access_cm_delete_model_subscription
(
    /* IN */ MS_ACCESS_MODEL_HANDLE    model_handle,
    /* IN */ MS_ACCESS_ADDRESS*        sub_addr
);

/**
    \brief To discard a model subscription list

    \par Description
    This routine discards a subscription list of a model

    \param [in] model_handle      Handle of the Model for which the subscription list to be discarded.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_access_cm_delete_all_model_subscription
(
    /* IN */ MS_ACCESS_MODEL_HANDLE    model_handle
);

/**
    \brief To get list of subscription addresses of a model

    \par Description
    This routine returns a list of subscription addresses of a model.

    \param [in] model_handle        Handle of the Model for which the subscription addresses to be returned.
    \param [inout] sub_addr_count   Caller fills with maximum number of subscription addresses
                                    that can be stored in 'sub_addr_list'.
                                    This function will update the value with how many subscription addresses
                                    has been filled. If the number of available subscription addresses is more than that can be returned,
                                    maximum possible addresses will be filled and an appropriate error values will inform the caller,
                                    there are more subscription addresses (as an information).
    \param [out] sub_addr_list      Memory to be filled with the available subscription addresses.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_access_cm_get_model_subscription_list
(
    /* IN */    MS_ACCESS_MODEL_HANDLE    model_handle,
    /* INOUT */ UINT16*                   sub_addr_count,
    /* OUT */   UINT16*                   sub_addr_list
);

/**
    \brief To get list of subscription addresses of all the models

    \par Description
    This routine returns a consolidated list of subscription addresses of all the models.

    \param [inout] sub_addr_count   Caller fills with maximum number of subscription addresses
                                    that can be stored in 'sub_addr_list'.
                                    This function will update the value with how many subscription addresses
                                    has been filled. If the number of available subscription addresses is more than that can be returned,
                                    maximum possible addresses will be filled and an appropriate error values will inform the caller,
                                    there are more subscription addresses (as an information).
    \param [out] sub_addr_list      Memory to be filled with the available subscription addresses.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_access_cm_get_all_model_subscription_list
(
    /* INOUT */ UINT16*                   sub_addr_count,
    /* OUT */   UINT16*                   sub_addr_list
);

/**
    \brief To check if valid element address to receive a packet

    \par Description
    This routine checks if destination address in a received packet matches
    with any of the known element address of local or friend device.

    \param [in] addr     Unicast Address to search

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_access_is_valid_element_address
(
    /* IN */ MS_NET_ADDR    addr
);

/**
    \brief To check if Fixed Group Address in receive packet to be processed

    \par Description
    This routine checks if destination address in a received packet
    as a Fixed Group Address to be processed.

    \param [in] addr     A valid Fixed Group Address, to be checked

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_access_is_fixed_group_addr_to_be_processed
(
    /* IN */ MS_NET_ADDR    addr
);

/**
    \brief To check if valid subscription address to receive a packet

    \par Description
    This routine checks if destination address in a received packet matches
    with any of the known subscription address of local or friend device.

    \param [in] addr     Address to search

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_access_is_valid_subscription_address
(
    /* IN */ MS_NET_ADDR    addr
);

#ifdef MS_STORAGE
/**
    \brief Get Core Modules storage handle and offset from persistent storage.

    \par Description
    This function returns the storage handle and offset for Core Modules.

    \param [out] ps_handle  Persistent Storage Handle.
    \param [out] offset     The memory to be filled with the storage offset information.
*/
API_RESULT MS_access_ps_get_handle_and_offset
(
    /* OUT */ NVSTO_HANDLE* ps_handle,
    /* OUT */ UINT32*        offset
);
#endif /* MS_STORAGE */

#ifdef __cplusplus
};
#endif

/** \} */

/** \} */

#endif /* _H_MS_ACCESS_API_ */

