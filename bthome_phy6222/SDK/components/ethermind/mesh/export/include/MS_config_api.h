
/**
    \file MS_config_api.h

    \brief This file defines the Mesh Configuration Foundation Model Application Interface
    - includes Data Structures and Methods for both Server and Client.
*/

/*
    Copyright (C) 2017. Mindtree Ltd.
    All rights reserved.
*/

#ifndef _H_MS_CONFIG_API_
#define _H_MS_CONFIG_API_


/* --------------------------------------------- Header File Inclusion */
/* Access Layer */
#include "MS_access_api.h"


/* --------------------------------------------- Global Definitions */

/**
    \defgroup config_module CONFIG (Mesh Configuration Model)
    \{
    This section describes the interfaces & APIs offered by the EtherMind
    Mesh Configuration Model (CONFIG) module to the Application.
*/

/**
    \defgroup config_defines Defines
    \{
    Describes defines for the module.
*/

/**
    \defgroup config_constants Constants
    \{
    Describes Constants defined by the module.
*/

/**
    \defgroup config_status Status Codes
    \{
    This section lists the Status Codes applicable at the Configuration Model.
*/

/** \} */

/** \} */

/** \} */

/**
    \defgroup config_events Events
    \{
    This section lists the Asynchronous Events notified to Application by the
    Module.
*/

/** \} */

/**
    \defgroup config_marcos Utility Macros
    \{
    This section defines the utility macros for use by the application.

*/

/** \} */

/* --------------------------------------------- Data Types/ Structures */

/**
    \defgroup config_cb Application Callback
    \{
    This Section Describes the module Notification Callback interface offered
    to the application
*/
/**
    Configuration Client application Asynchronous Notification Callback.

    Configuration Client calls the registered callback to indicate events occurred to the
    application.

    \param handle        Model Handle.
    \param opcode        Opcode.
    \param data_param    Data associated with the event if any or NULL.
    \param data_len      Size of the event data. 0 if event data is NULL.
*/
typedef API_RESULT (* MS_CONFIG_MODEL_CB)
(
    MS_ACCESS_MODEL_HANDLE* handle,
    UINT32                   opcode,
    UCHAR*                   data_param,
    UINT16                   data_len
) DECL_REENTRANT;
/** \} */

/**
    \defgroup config_structures Structures
    \{
*/

/**
    \defgroup config_cli_structs Configuration Client Data Structures
    \{
    This section describes the data structures for use in Configuration Client APIs.
*/

/**
    Beacon Set parameter structure
*/
typedef struct _ACCESS_CONFIG_BEACON_SET_PARAM
{
    /** New Secure Network Beacon state */
    UCHAR beacon;

} ACCESS_CONFIG_BEACON_SET_PARAM;

/**
    Composition Data Get parameter structure
*/
typedef struct _ACCESS_CONFIG_COMPDATA_GET_PARAM
{
    /** Page number of the Composition Data */
    UCHAR page;

} ACCESS_CONFIG_COMPDATA_GET_PARAM;

/**
    Default TTL Set parameter structure
*/
typedef struct _ACCESS_CONFIG_DEFAULT_TTL_SET_PARAM
{
    /** New Default TTL value */
    UCHAR ttl;

} ACCESS_CONFIG_DEFAULT_TTL_SET_PARAM;

/**
    GATT Proxy Set parameter structure
*/
typedef struct _ACCESS_CONFIG_GATT_PROXY_SET_PARAM
{
    /** New GATT Proxy state */
    UCHAR proxy;

} ACCESS_CONFIG_GATT_PROXY_SET_PARAM;

/**
    Relay Set parameter structure
*/
typedef struct _ACCESS_CONFIG_RELAY_SET_PARAM
{
    /** Relay */
    UCHAR relay;

    /**
        Number of retransmissions on advertising bearer for
        each Network PDU relayed by the node
        - 3 bits validity
    */
    UCHAR relay_rtx_count;

    /**
        Number of 10-millisecond steps between retransmissions
        - 5 bits validity
    */
    UCHAR relay_rtx_interval_steps;

} ACCESS_CONFIG_RELAY_SET_PARAM;

/**
    Model Publication Get parameter structure
*/
typedef struct _ACCESS_CONFIG_MODELPUB_GET_PARAM
{
    /** Address of the element */
    UINT16 element_address;

    /** SIG Model ID or Vendor Model ID */
    MS_ACCESS_MODEL_ID model;

} ACCESS_CONFIG_MODELPUB_GET_PARAM;

/**
    Model Publication Set parameter structure
*/
typedef struct _ACCESS_CONFIG_MODELPUB_SET_PARAM
{
    /** Address of the element */
    UINT16 element_address;

    /** Value of the publish address */
    UINT16 publish_address;

    /**
        Index of the application key
        - 12 bits validity
    */
    UINT16 appkey_index;

    /**
        Value of the Friendship Credential Flag
        - 1 bit validity
    */
    UCHAR credential_flag;

    /** Default TTL value for the outgoing messages */
    UCHAR publish_ttl;

    /** Period for periodic status publishing */
    UCHAR publish_period;

    /**
        Number of retransmissions for each published message
        - 3 bits validity
    */
    UCHAR publish_rtx_count;

    /**
        Number of 50-millisecond steps between retransmissions
        - 5 bits validity
    */
    UCHAR publish_rtx_interval_steps;

    /** SIG Model ID or Vendor Model ID */
    MS_ACCESS_MODEL_ID model;

} ACCESS_CONFIG_MODELPUB_SET_PARAM;

/**
    Model Publication Virtual Address Set parameter structure
*/
typedef struct _ACCESS_CONFIG_MODELPUB_VADDR_SET_PARAM
{
    /** Address of the element */
    UINT16 element_address;

    /** Value of the Label UUID publish address */
    UCHAR publish_address[MS_ACCESS_VADDR_LABEL_UUID_SIZE];

    /**
        Index of the application key
        - 12 bits validity
    */
    UINT16 appkey_index;

    /**
        Value of the Friendship Credential Flag
        - 1 bit validity
    */
    UCHAR credential_flag;

    /** Default TTL value for the outgoing messages */
    UCHAR publish_ttl;

    /** Period for periodic status publishing */
    UCHAR publish_period;

    /**
        Number of retransmissions for each published message
        - 3 bits validity
    */
    UCHAR publish_rtx_count;

    /**
        Number of 50-millisecond steps between retransmissions
        - 5 bits validity
    */
    UCHAR publish_rtx_interval_steps;

    /** SIG Model ID or Vendor Model ID */
    MS_ACCESS_MODEL_ID model;

} ACCESS_CONFIG_MODELPUB_VADDR_SET_PARAM;

/**
    Model Subscription Add parameter structure
*/
typedef struct _ACCESS_CONFIG_MODELSUB_ADD_PARAM
{
    /** Address of the element */
    UINT16 element_address;

    /** Value of the address */
    UINT16 address;

    /** SIG Model ID or Vendor Model ID */
    MS_ACCESS_MODEL_ID model;

} ACCESS_CONFIG_MODELSUB_ADD_PARAM;

/**
    Model Subscription Virtual Address Add parameter structure
*/
typedef struct _ACCESS_CONFIG_MODELSUB_VADDR_ADD_PARAM
{
    /** Address of the element */
    UINT16 element_address;

    /** Value of the Label UUID */
    UCHAR label[MS_ACCESS_VADDR_LABEL_UUID_SIZE];

    /** SIG Model ID or Vendor Model ID */
    MS_ACCESS_MODEL_ID model;

} ACCESS_CONFIG_MODELSUB_VADDR_ADD_PARAM;

/**
    Model Subscription Delete parameter structure
*/
typedef struct _ACCESS_CONFIG_MODELSUB_DEL_PARAM
{
    /** Address of the element */
    UINT16 element_address;

    /** Value of the address */
    UINT16 address;

    /** SIG Model ID or Vendor Model ID */
    MS_ACCESS_MODEL_ID model;

} ACCESS_CONFIG_MODELSUB_DEL_PARAM;

/**
    Model Subscription Virtual Address Delete parameter structure
*/
typedef struct _ACCESS_CONFIG_MODELSUB_VADDR_DEL_PARAM
{
    /** Address of the element */
    UINT16 element_address;

    /** Value of the Label UUID */
    UCHAR label[MS_ACCESS_VADDR_LABEL_UUID_SIZE];

    /** SIG Model ID or Vendor Model ID */
    MS_ACCESS_MODEL_ID model;

} ACCESS_CONFIG_MODELSUB_VADDR_DEL_PARAM;

/**
    Model Subscription Overwrite parameter structure
*/
typedef struct _ACCESS_CONFIG_MODELSUB_OVERWRITE_PARAM
{
    /** Address of the element */
    UINT16 element_address;

    /** Value of the address */
    UINT16 address;

    /** SIG Model ID or Vendor Model ID */
    MS_ACCESS_MODEL_ID model;

} ACCESS_CONFIG_MODELSUB_OVERWRITE_PARAM;

/**
    Model Subscription Virtual Address Overwrite parameter structure
*/
typedef struct _ACCESS_CONFIG_MODELSUB_VADDR_OVERWRITE_PARAM
{
    /** Address of the element */
    UINT16 element_address;

    /** Value of the Label UUID */
    UCHAR label[MS_ACCESS_VADDR_LABEL_UUID_SIZE];

    /** SIG Model ID or Vendor Model ID */
    MS_ACCESS_MODEL_ID model;

} ACCESS_CONFIG_MODELSUB_VADDR_OVERWRITE_PARAM;

/**
    Model Subscription Delete All parameter structure
*/
typedef struct _ACCESS_CONFIG_MODELSUB_DELETEALL_PARAM
{
    /** Address of the element */
    UINT16 element_address;

    /** SIG Model ID or Vendor Model ID */
    MS_ACCESS_MODEL_ID model;

} ACCESS_CONFIG_MODELSUB_DELETEALL_PARAM;

/**
    SIG Model Subscription Get parameter structure
*/
typedef struct _ACCESS_CONFIG_SIGMODELSUB_GET_PARAM
{
    /** Address of the element */
    UINT16 element_address;

    /** SIG Model ID */
    MS_ACCESS_MODEL_ID_SIG model_id;

} ACCESS_CONFIG_SIGMODELSUB_GET_PARAM;

/**
    Vendor Model Subscription Get parameter structure
*/
typedef struct _ACCESS_CONFIG_VENDORMODELSUB_GET_PARAM
{
    /** Address of the element */
    UINT16 element_address;

    /** Vendor Model ID */
    MS_ACCESS_MODEL_ID_VENDOR model_id;

} ACCESS_CONFIG_VENDORMODELSUB_GET_PARAM;

/**
    Netkey Add parameter structure
*/
typedef struct _ACCESS_CONFIG_NETKEY_ADD_PARAM
{
    /** Netkey */
    UCHAR netkey[MS_ACCESS_NETKEY_SIZE];

    /** Netkey Index */
    UINT16 netkey_index;

} ACCESS_CONFIG_NETKEY_ADD_PARAM;

/**
    Netkey Update parameter structure
*/
typedef struct _ACCESS_CONFIG_NETKEY_UPDATE_PARAM
{
    /** New Netkey */
    UCHAR netkey[MS_ACCESS_NETKEY_SIZE];

    /** Netkey Index */
    UINT16 netkey_index;

} ACCESS_CONFIG_NETKEY_UPDATE_PARAM;

/**
    Netkey Delete parameter structure
*/
typedef struct _ACCESS_CONFIG_NETKEY_DELETE_PARAM
{
    /** Netkey Index */
    UINT16 netkey_index;

} ACCESS_CONFIG_NETKEY_DELETE_PARAM;

/**
    Appkey Add parameter structure
*/
typedef struct _ACCESS_CONFIG_APPKEY_ADD_PARAM
{
    /** Appkey value */
    UCHAR appkey[MS_ACCESS_APPKEY_SIZE];

    /**
        Index of the NetKey and index of the AppKey
        - 24 bits valid
    */
    UINT16 netkey_index;
    UINT16 appkey_index;

} ACCESS_CONFIG_APPKEY_ADD_PARAM;

/**
    Appkey Update parameter structure
*/
typedef struct _ACCESS_CONFIG_APPKEY_UPDATE_PARAM
{
    /** New Appkey value */
    UCHAR appkey[MS_ACCESS_APPKEY_SIZE];

    /**
        Index of the NetKey and index of the AppKey
        - 24 bits valid
    */
    UINT16 netkey_index;
    UINT16 appkey_index;

} ACCESS_CONFIG_APPKEY_UPDATE_PARAM;

/**
    Appkey Delete parameter structure
*/
typedef struct _ACCESS_CONFIG_APPKEY_DELETE_PARAM
{
    /**
        Index of the NetKey and index of the AppKey
        - 24 bits valid
     * */
    UINT16 netkey_index;
    UINT16 appkey_index;

} ACCESS_CONFIG_APPKEY_DELETE_PARAM;

/**
    Appkey Get parameter structure
*/
typedef struct _ACCESS_CONFIG_APPKEY_GET_PARAM
{
    /** Index of the NetKey */
    UINT16 netkey_index;

} ACCESS_CONFIG_APPKEY_GET_PARAM;

/**
    Node Identity Get parameter structure
*/
typedef struct _ACCESS_CONFIG_NODEID_GET_PARAM
{
    /** Index of the NetKey */
    UINT16 netkey_index;

} ACCESS_CONFIG_NODEID_GET_PARAM;

/**
    Node Identity Set parameter structure
*/
typedef struct _ACCESS_CONFIG_NODEID_SET_PARAM
{
    /** Index of the NetKey */
    UINT16 netkey_index;

    /** New Node Identity state */
    UCHAR identity;

} ACCESS_CONFIG_NODEID_SET_PARAM;

/**
    Model App Bind parameter structure
*/
typedef struct _ACCESS_CONFIG_MODEL_APP_BIND_PARAM
{
    /** Address of the element */
    UINT16 element_address;

    /** Index of the AppKey */
    UINT16 appkey_index;

    /** SIG Model ID or Vendor Model ID */
    MS_ACCESS_MODEL_ID model;

    /**
        Local SIG Model ID or Vendor Model ID.
        Used only for MS_config_client_model_app_bind().
    */
    MS_ACCESS_MODEL_ID client_model;

} ACCESS_CONFIG_MODEL_APP_BIND_PARAM;

/**
    Model App Unbind parameter structure
*/
typedef struct _ACCESS_CONFIG_MODEL_APP_UNBIND_PARAM
{
    /** Address of the element */
    UINT16 element_address;

    /** Index of the AppKey */
    UINT16 appkey_index;

    /** SIG Model ID or Vendor Model ID */
    MS_ACCESS_MODEL_ID model;

    /**
        Local SIG Model ID or Vendor Model ID.
        Used only for MS_config_client_model_app_unbind().
    */
    MS_ACCESS_MODEL_ID client_model;

} ACCESS_CONFIG_MODEL_APP_UNBIND_PARAM;

/**
    SIG Model App Get parameter structure
*/
typedef struct _ACCESS_CONFIG_SIG_MODEL_APP_GET_PARAM
{
    /** Address of the element */
    UINT16 element_address;

    /** SIG Model ID */
    MS_ACCESS_MODEL_ID_SIG model_id;

} ACCESS_CONFIG_SIG_MODEL_APP_GET_PARAM;

/**
    Vendor Model App Get parameter structure
*/
typedef struct _ACCESS_CONFIG_VENDOR_MODEL_APP_GET_PARAM
{
    /** Address of the element */
    UINT16 element_address;

    /** Vendor Model ID */
    MS_ACCESS_MODEL_ID_VENDOR model_id;

} ACCESS_CONFIG_VENDOR_MODEL_APP_GET_PARAM;

/**
    Friend Set parameter structure
*/
typedef struct _ACCESS_CONFIG_FRIEND_SET_PARAM
{
    /** New Friend state */
    UCHAR friend;

} ACCESS_CONFIG_FRIEND_SET_PARAM;

/**
    Key Refresh Phase Get parameter structure
*/
typedef struct _ACCESS_CONFIG_KEYREFRESH_PHASE_GET_PARAM
{
    /** Netkey Index */
    UINT16 netkey_index;

} ACCESS_CONFIG_KEYREFRESH_PHASE_GET_PARAM;

/**
    Key Refresh Phase Set parameter structure
*/
typedef struct _ACCESS_CONFIG_KEYREFRESH_PHASE_SET_PARAM
{
    /** Netkey Index */
    UINT16 netkey_index;

    /** New Key Refresh Phase Transition */
    UCHAR transition;

} ACCESS_CONFIG_KEYREFRESH_PHASE_SET_PARAM;

/**
    Heartbeat Publication Set parameter structure
*/
typedef struct _ACCESS_CONFIG_HEARTBEATPUB_SET_PARAM
{
    /** Destination address for Heartbeat messages */
    UINT16 destination;

    /** Number of Heartbeat messages to be sent */
    UCHAR countlog;

    /** Period for sending Heartbeat messages */
    UCHAR periodlog;

    /** TTL to be used when sending Heartbeat messages */
    UCHAR ttl;

    /**
        Bit field indicating features that trigger
        Heartbeat messages when changed
    */
    UINT16 features;

    /** Netkey Index */
    UINT16 netkey_index;

} ACCESS_CONFIG_HEARTBEATPUB_SET_PARAM;

/**
    Heartbeat Subscription Set parameter structure
*/
typedef struct _ACCESS_CONFIG_HEARTBEATSUB_SET_PARAM
{
    /** Source address for Heartbeat messages */
    UINT16 source;

    /** Destination address for Heartbeat messages */
    UINT16 destination;

    /** Period for receiving Heartbeat messages */
    UCHAR periodlog;

} ACCESS_CONFIG_HEARTBEATSUB_SET_PARAM;

/**
    Low Power Node PollTimeout Get parameter structure
*/
typedef struct _ACCESS_CONFIG_LPNPOLLTIMEOUT_GET_PARAM
{
    /** The unicast address of the Low Power node */
    UINT16 lpn_address;

} ACCESS_CONFIG_LPNPOLLTIMEOUT_GET_PARAM;

/**
    Network Transmit Set parameter structure
*/
typedef struct _ACCESS_CONFIG_NETWORK_TRANSMIT_SET_PARAM
{
    /**
        Number of transmissions for each Network PDU
        originating from the node
        - 3 bits validity
    */
    UCHAR net_tx_count;

    /**
        Number of 10-millisecond steps between transmissions
        - 5 bits validity
    */
    UCHAR net_tx_interval_steps;

} ACCESS_CONFIG_NETWORK_TRANSMIT_SET_PARAM;

/** \} */

/** \} */


/* --------------------------------------------- Function */

/**
    \defgroup config_api_defs API Definitions
    \{
    This section describes the EtherMind Mesh Config Model APIs.
*/

/**
    \defgroup config_cli_api_defs Configuration Client API Definitions
    \{
    This section describes the Configuration Client APIs.
*/

/**
    \brief API to initialize Configuration Client model

    \par Description
    This is to initialize Configuration Client model and to register with Acess layer.

    \param [in] element_handle
                Element identifier to be associated with the model instance.

    \param [in, out] model_handle
                     Model identifier associated with the model instance on successful initialization.
                     After power cycle of an already provisioned node, the model handle will have
                     valid value and the same will be reused for registration.

    \param [in] appl_cb    Application Callback to be used by the Configuration Client.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_config_client_init
(
    /* IN */    MS_ACCESS_ELEMENT_HANDLE    element_handle,
    /* INOUT */ MS_ACCESS_MODEL_HANDLE*     model_handle,
    /* IN */    MS_CONFIG_MODEL_CB          appl_cb
);

API_RESULT MS_config_client_set_publish_address
(
    /* IN */ MS_NET_ADDR    pub_addr
);


/**
    \brief API to set configuration server

    \par Description
    This is to sets the information about server which is to be configured.

    \param [in] server_addr   Address of Configuration Server.
    \param [in] dev_key       Device Key of Configuration Server.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_config_client_set_server
(
    /* IN */ MS_NET_ADDR    server_addr,
    /* IN */ UCHAR*         dev_key
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
API_RESULT MS_config_client_send_reliable_pdu
(
    /* IN */ UINT32    req_opcode,
    /* IN */ void*     param,
    /* IN */ UINT32    rsp_opcode
);

/**
    \brief API to get the secure network beacon state

    \par Description
    The Config Beacon Get is an acknowledged message used to get the current
    Secure Network Beacon state of a node.

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_config_client_beacon_get() \
    MS_config_client_send_reliable_pdu \
    (\
     MS_ACCESS_CONFIG_BEACON_GET_OPCODE,\
     NULL,\
     MS_ACCESS_CONFIG_BEACON_STATUS_OPCODE \
    )

/**
    \brief API to set the secure network beacon state

    \par Description
    The Config Beacon Set is an acknowledged message used to set the current
    Secure Network Beacon state of a node.

    \param [in] param
           Pointer to the structure populated as in \ref ACCESS_CONFIG_BEACON_SET_PARAM

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_config_client_beacon_set(param)\
    MS_config_client_send_reliable_pdu \
    (\
     MS_ACCESS_CONFIG_BEACON_SET_OPCODE,\
     (void *)param, \
     MS_ACCESS_CONFIG_BEACON_STATUS_OPCODE \
    )

/**
    \brief API to get the composition data state

    \par Description
    The Config Composition Data Get is an acknowledged message used to read
    one page of the Composition Data.

    \param [in] param
           Pointer to the structure populated as in \ref ACCESS_CONFIG_COMPDATA_GET_PARAM

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_config_client_composition_data_get(param)\
    MS_config_client_send_reliable_pdu \
    (\
     MS_ACCESS_CONFIG_COMPOSITION_DATA_GET_OPCODE,\
     (void *)param, \
     MS_ACCESS_CONFIG_COMPOSITION_DATA_STATUS_OPCODE \
    )

/**
    \brief API to get the default TTL state

    \par Description
    Config Default TTL Get is an acknowledged message used to get the current
    Default TTL state of a node.

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_config_client_default_ttl_get()\
    MS_config_client_send_reliable_pdu \
    (\
     MS_ACCESS_CONFIG_DEFAULT_TTL_GET_OPCODE,\
     NULL, \
     MS_ACCESS_CONFIG_DEFAULT_TTL_STATUS_OPCODE \
    )

/**
    \brief API to set the default TTL state

    \par Description
    The Config Default TTL Set is an acknowledged message used to set the
    Default TTL state of a node.

    \param [in] param
           Pointer to the structure populated as in \ref ACCESS_CONFIG_DEFAULT_TTL_SET_PARAM

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_config_client_default_ttl_set(param)\
    MS_config_client_send_reliable_pdu \
    (\
     MS_ACCESS_CONFIG_DEFAULT_TTL_SET_OPCODE,\
     (void *)param, \
     MS_ACCESS_CONFIG_DEFAULT_TTL_STATUS_OPCODE \
    )

/**
    \brief API to get the GATT proxy state

    \par Description
    The Config GATT Proxy Get is an acknowledged message used to get the GATT
    Proxy state of a node.

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_config_client_gatt_proxy_get()\
    MS_config_client_send_reliable_pdu \
    (\
     MS_ACCESS_CONFIG_GATT_PROXY_GET_OPCODE,\
     NULL, \
     MS_ACCESS_CONFIG_GATT_PROXY_STATUS_OPCODE \
    )

/**
    \brief API to set the GATT Proxy state

    \par Description
    The Config GATT Proxy Set is an acknowledged message used to set the
    GATT Proxy state of a node.

    \param [in] param
           Pointer to the structure populated as in \ref ACCESS_CONFIG_GATT_PROXY_SET_PARAM

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_config_client_gatt_proxy_set(param)\
    MS_config_client_send_reliable_pdu \
    (\
     MS_ACCESS_CONFIG_GATT_PROXY_SET_OPCODE,\
     (void *)param, \
     MS_ACCESS_CONFIG_GATT_PROXY_STATUS_OPCODE \
    )

/**
    \brief API to get the relay state

    \par Description
    The Config Relay Get is an acknowledged message used to get the current
    Relay and Relay Retransmit states of a node.

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_config_client_relay_get()\
    MS_config_client_send_reliable_pdu \
    (\
     MS_ACCESS_CONFIG_RELAY_GET_OPCODE,\
     NULL, \
     MS_ACCESS_CONFIG_RELAY_STATUS_OPCODE \
    )

/**
    \brief API to set the relay state

    \par Description
    The Config Relay Set is an acknowledged message used to set the current
    Relay and Relay Retransmit states of a node.

    \param [in] param
           Pointer to the structure populated as in \ref ACCESS_CONFIG_RELAY_SET_PARAM

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_config_client_relay_set(param)\
    MS_config_client_send_reliable_pdu \
    (\
     MS_ACCESS_CONFIG_RELAY_SET_OPCODE,\
     (void *)param, \
     MS_ACCESS_CONFIG_RELAY_STATUS_OPCODE \
    )

/**
    \brief API to get the model publication state

    \par Description
    The Config Model Publication Get is an acknowledged message used to get
    the publish address and parameters of an outgoing message that originates
    from a model.

    \param [in] param
           Pointer to the structure populated as in \ref ACCESS_CONFIG_MODELPUB_GET_PARAM

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_config_client_model_publication_get(param)\
    MS_config_client_send_reliable_pdu \
    (\
     MS_ACCESS_CONFIG_MODEL_PUBLICATION_GET_OPCODE,\
     (void *)param, \
     MS_ACCESS_CONFIG_MODEL_PUBLICATION_STATUS_OPCODE \
    )

/**
    \brief API to set the model publication state

    \par Description
    The Config Model Publication Set is an acknowledged message used to set
    the Model Publication state of an outgoing message that
    originates from a model.

    \param [in] param
           Pointer to the structure populated as in \ref ACCESS_CONFIG_MODELPUB_SET_PARAM

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_config_client_model_publication_set(param)\
    MS_config_client_send_reliable_pdu \
    (\
     MS_ACCESS_CONFIG_MODEL_PUBLICATION_SET_OPCODE,\
     (void *)param, \
     MS_ACCESS_CONFIG_MODEL_PUBLICATION_STATUS_OPCODE \
    )

/**
    \brief API to set the model publication state

    \par Description
    The Config Model Publication Set is an acknowledged message used to set
    the Model Publication state of an outgoing message that
    originates from a model.

    \param [in] param
           Pointer to the structure populated as in \ref ACCESS_CONFIG_MODELPUB_VADDR_SET_PARAM

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_config_client_model_publication_vaddr_set(param)\
    MS_config_client_send_reliable_pdu \
    (\
     MS_ACCESS_CONFIG_MODEL_PUBLICATION_VIRTUAL_ADDRESS_SET_OPCODE,\
     (void *)param, \
     MS_ACCESS_CONFIG_MODEL_PUBLICATION_STATUS_OPCODE \
    )

/**
    \brief API to add subscription address

    \par Description
    The Config Model Subscription Add is an acknowledged message used to
    add an address to a Subscription List of a model.

    \param [in] param
           Pointer to the structure populated as in \ref ACCESS_CONFIG_MODELSUB_ADD_PARAM

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_config_client_model_subscription_add(param)\
    MS_config_client_send_reliable_pdu \
    (\
     MS_ACCESS_CONFIG_MODEL_SUBSCRIPTION_ADD_OPCODE,\
     (void *)param, \
     MS_ACCESS_CONFIG_MODEL_SUBSCRIPTION_STATUS_OPCODE \
    )

/**
    \brief API to add subscription address

    \par Description
    The Config Model Subscription Add is an acknowledged message used to
    add an address to a Subscription List of a model.

    \param [in] param
           Pointer to the structure populated as in \ref ACCESS_CONFIG_MODELSUB_VADDR_ADD_PARAM

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_config_client_model_subscription_vaddr_add(param)\
    MS_config_client_send_reliable_pdu \
    (\
     MS_ACCESS_CONFIG_MODEL_SUBSCRIPTION_VIRTUAL_ADDRESS_ADD_OPCODE,\
     (void *)param, \
     MS_ACCESS_CONFIG_MODEL_SUBSCRIPTION_STATUS_OPCODE \
    )

/**
    \brief API to delete subscription address

    \par Description
    The Config Model Subscription Delete is an acknowledged message used to
    delete a subscription address from the Subscription List of a model.

    \param [in] param
           Pointer to the structure populated as in \ref ACCESS_CONFIG_MODELSUB_DEL_PARAM

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_config_client_model_subscription_delete(param)\
    MS_config_client_send_reliable_pdu \
    (\
     MS_ACCESS_CONFIG_MODEL_SUBSCRIPTION_DELETE_OPCODE,\
     (void *)param, \
     MS_ACCESS_CONFIG_MODEL_SUBSCRIPTION_STATUS_OPCODE \
    )

/**
    \brief API to delete subscription address

    \par Description
    The Config Model Subscription Delete is an acknowledged message used to
    delete a subscription address from the Subscription List of a model.

    \param [in] param
           Pointer to the structure populated as in \ref ACCESS_CONFIG_MODELSUB_VADDR_DEL_PARAM

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_config_client_model_subscription_vaddr_delete(param)\
    MS_config_client_send_reliable_pdu \
    (\
     MS_ACCESS_CONFIG_MODEL_SUBSCRIPTION_VIRTUAL_ADDRESS_DELETE_OPCODE,\
     (void *)param, \
     MS_ACCESS_CONFIG_MODEL_SUBSCRIPTION_STATUS_OPCODE \
    )

/**
    \brief API to add subscription address to cleared list

    \par Description
    The Config Model Subscription Overwrite is an acknowledged message used
    to discard the Subscription List and add an address to the cleared
    Subscription List of a model.

    \param [in] param
           Pointer to the structure populated as in \ref ACCESS_CONFIG_MODELSUB_OVERWRITE_PARAM

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_config_client_model_subscription_overwrite(param)\
    MS_config_client_send_reliable_pdu \
    (\
     MS_ACCESS_CONFIG_MODEL_SUBSCRIPTION_OVERWRITE_OPCODE,\
     (void *)param, \
     MS_ACCESS_CONFIG_MODEL_SUBSCRIPTION_STATUS_OPCODE \
    )

/**
    \brief API to add subscription address to cleared list

    \par Description
    The Config Model Subscription Overwrite is an acknowledged message used
    to discard the Subscription List and add an address to the cleared
    Subscription List of a model.

    \param [in] param
           Pointer to the structure populated as in \ref ACCESS_CONFIG_MODELSUB_VADDR_OVERWRITE_PARAM

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_config_client_model_subscription_vaddr_overwrite(param)\
    MS_config_client_send_reliable_pdu \
    (\
     MS_ACCESS_CONFIG_MODEL_SUBSCRIPTION_VIRTUAL_ADDRESS_OVERWRITE_OPCODE,\
     (void *)param, \
     MS_ACCESS_CONFIG_MODEL_SUBSCRIPTION_STATUS_OPCODE \
    )

/**
    \brief API to discard subscription list

    \par Description
    The Config Model Subscription Delete All is an acknowledged message
    used to discard the Subscription List of a model.

    \param [in] param
           Pointer to the structure populated as in \ref ACCESS_CONFIG_MODELSUB_DELETEALL_PARAM

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_config_client_model_subscription_delete_all(param)\
    MS_config_client_send_reliable_pdu \
    (\
     MS_ACCESS_CONFIG_MODEL_SUBSCRIPTION_DELETE_ALL_OPCODE,\
     (void *)param, \
     MS_ACCESS_CONFIG_MODEL_SUBSCRIPTION_STATUS_OPCODE \
    )

/**
    \brief API to get subscription list

    \par Description
    The Config SIG Model Subscription Get is an acknowledged message used to
    get the list of subscription addresses of a model within the element.
    This message is only for SIG Models.

    \param [in] param
           Pointer to the structure populated as in \ref ACCESS_CONFIG_SIGMODELSUB_GET_PARAM

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_config_client_sig_model_subscription_get(param)\
    MS_config_client_send_reliable_pdu \
    (\
     MS_ACCESS_CONFIG_SIG_MODEL_SUBSCRIPTION_GET_OPCODE,\
     (void *)param, \
     MS_ACCESS_CONFIG_SIG_MODEL_SUBSCRIPTION_LIST_OPCODE \
    )

/**
    \brief API to get subscription list

    \par Description
    The Config SIG Model Subscription Get is an acknowledged message used to
    get the list of subscription addresses of a model within the element.
    This message is only for Vendor Models.

    \param [in] param
           Pointer to the structure populated as in \ref ACCESS_CONFIG_VENDORMODELSUB_GET_PARAM

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_config_client_vendor_model_subscription_get(param)\
    MS_config_client_send_reliable_pdu \
    (\
     MS_ACCESS_CONFIG_VENDOR_MODEL_SUBSCRIPTION_GET_OPCODE,\
     (void *)param, \
     MS_ACCESS_CONFIG_VENDOR_MODEL_SUBSCRIPTION_LIST_OPCODE \
    )

/**
    \brief API to add to Netkey list

    \par Description
    The Config NetKey Add is an acknowledged message used to add a NetKey
    to a NetKey List on a node. The added NetKey is then used by the node to
    authenticate and decrypt messages it receives, as well as authenticate and
    encrypt messages it sends.

    \param [in] param
           Pointer to the structure populated as in \ref ACCESS_CONFIG_NETKEY_ADD_PARAM

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_config_client_netkey_add(param)\
    MS_config_client_send_reliable_pdu \
    (\
     MS_ACCESS_CONFIG_NETKEY_ADD_OPCODE,\
     (void *)param, \
     MS_ACCESS_CONFIG_NETKEY_STATUS_OPCODE \
    )

/**
    \brief API to update to Netkey list

    \par Description
    The Config NetKey Update is an acknowledged message used to update a NetKey
    on a node. The updated NetKey is then used by the node to authenticate and
    decrypt messages it receives, as well as authenticate and encrypt messages
    it sends, as defined by the Key Refresh procedure.

    \param [in] param
           Pointer to the structure populated as in \ref ACCESS_CONFIG_NETKEY_UPDATE_PARAM

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_config_client_netkey_update(param)\
    MS_config_client_send_reliable_pdu \
    (\
     MS_ACCESS_CONFIG_NETKEY_UPDATE_OPCODE,\
     (void *)param, \
     MS_ACCESS_CONFIG_NETKEY_STATUS_OPCODE \
    )

/**
    \brief API to delete from Netkey list

    \par Description
    The Config NetKey Delete is an acknowledged message used to
    delete a NetKey on a NetKey List from a node.

    \param [in] param
           Pointer to the structure populated as in \ref ACCESS_CONFIG_NETKEY_DELETE_PARAM

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_config_client_netkey_delete(param)\
    MS_config_client_send_reliable_pdu \
    (\
     MS_ACCESS_CONFIG_NETKEY_DELETE_OPCODE,\
     (void *)param, \
     MS_ACCESS_CONFIG_NETKEY_STATUS_OPCODE \
    )

/**
    \brief API to get Netkey list

    \par Description
    The Config NetKey Get is an acknowledged message used to report
    all NetKeys known to the node.

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_config_client_netkey_get()\
    MS_config_client_send_reliable_pdu \
    (\
     MS_ACCESS_CONFIG_NETKEY_GET_OPCODE,\
     NULL, \
     MS_ACCESS_CONFIG_NETKEY_LIST_OPCODE \
    )

/**
    \brief API to add to Appkey list

    \par Description
    The Config AppKey Add is an acknowledged message used to add an AppKey to
    the AppKey List on a node and bind it to the NetKey identified by
    NetKeyIndex. The added AppKey can be used by the node only as a pair with
    the specified NetKey. The AppKey is used to authenticate and decrypt
    messages it receives, as well as authenticate and encrypt messages it sends.

    \param [in] param
           Pointer to the structure populated as in \ref ACCESS_CONFIG_APPKEY_ADD_PARAM

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_config_client_appkey_add(param)\
    MS_config_client_send_reliable_pdu \
    (\
     MS_ACCESS_CONFIG_APPKEY_ADD_OPCODE,\
     (void *)param, \
     MS_ACCESS_CONFIG_APPKEY_STATUS_OPCODE \
    )

/**
    \brief API to update to Appkey list

    \par Description
    The Config AppKey Update is an acknowledged message used to update an AppKey value
    on the AppKey List on a node. The updated AppKey is used by the node to authenticate
    and decrypt messages it receives, as well as authenticate and encrypt messages it
    sends, as defined by the Key Refresh procedure.

    \param [in] param
           Pointer to the structure populated as in \ref ACCESS_CONFIG_APPKEY_UPDATE_PARAM

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_config_client_appkey_update(param)\
    MS_config_client_send_reliable_pdu \
    (\
     MS_ACCESS_CONFIG_APPKEY_UPDATE_OPCODE,\
     (void *)param, \
     MS_ACCESS_CONFIG_APPKEY_STATUS_OPCODE \
    )

/**
    \brief API to delete from Appkey list

    \par Description
    The Config AppKey Delete is an acknowledged message used to delete an AppKey
    from the AppKey List on a node

    \param [in] param
           Pointer to the structure populated as in \ref ACCESS_CONFIG_APPKEY_DELETE_PARAM

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_config_client_appkey_delete(param)\
    MS_config_client_send_reliable_pdu \
    (\
     MS_ACCESS_CONFIG_APPKEY_DELETE_OPCODE,\
     (void *)param, \
     MS_ACCESS_CONFIG_APPKEY_STATUS_OPCODE \
    )

/**
    \brief API to get the Appkey list

    \par Description
    The AppKey Get is an acknowledged message used to report all AppKeys
    bound to the NetKey.

    \param [in] param
           Pointer to the structure populated as in \ref ACCESS_CONFIG_APPKEY_GET_PARAM

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_config_client_appkey_get(param)\
    MS_config_client_send_reliable_pdu \
    (\
     MS_ACCESS_CONFIG_APPKEY_GET_OPCODE,\
     (void *)param, \
     MS_ACCESS_CONFIG_APPKEY_LIST_OPCODE \
    )

/**
    \brief API to get the Node Identity state

    \par Description
    The Config Node Identity Get is an acknowledged message used to get the
    current Node Identity state for a subnet.

    \param [in] param
           Pointer to the structure populated as in \ref ACCESS_CONFIG_NODEID_GET_PARAM

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_config_client_node_identity_get(param)\
    MS_config_client_send_reliable_pdu \
    (\
     MS_ACCESS_CONFIG_NODE_IDENTITY_GET_OPCODE,\
     (void *)param, \
     MS_ACCESS_CONFIG_NODE_IDENTITY_STATUS_OPCODE \
    )

/**
    \brief API to set the Node Identity state

    \par Description
    The Config Node Identity Set is an acknowledged message used to set the
    current Node Identity state for a subnet.

    \param [in] param
           Pointer to the structure populated as in \ref ACCESS_CONFIG_NODEID_SET_PARAM

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_config_client_node_identity_set(param)\
    MS_config_client_send_reliable_pdu \
    (\
     MS_ACCESS_CONFIG_NODE_IDENTITY_SET_OPCODE,\
     (void *)param, \
     MS_ACCESS_CONFIG_NODE_IDENTITY_STATUS_OPCODE \
    )

/**
    \brief API to bind Appkey to model

    \par Description
    The Config Model App Bind is an acknowledged message used to bind an
    AppKey to a model.

    \param [in] param
           Pointer to the structure populated as in \ref ACCESS_CONFIG_MODEL_APP_BIND_PARAM

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_config_client_model_app_bind(param)\
    MS_config_client_send_reliable_pdu \
    (\
     MS_ACCESS_CONFIG_MODEL_APP_BIND_OPCODE,\
     (void *)param, \
     MS_ACCESS_CONFIG_MODEL_APP_STATUS_OPCODE \
    )

/**
    \brief API to unbind Appkey to model

    \par Description
    The Config Model App Unbind is an acknowledged message used to remove the
    binding between an AppKey and a model.

    \param [in] param
           Pointer to the structure populated as in \ref ACCESS_CONFIG_MODEL_APP_UNBIND_PARAM

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_config_client_model_app_unbind(param)\
    MS_config_client_send_reliable_pdu \
    (\
     MS_ACCESS_CONFIG_MODEL_APP_UNBIND_OPCODE,\
     (void *)param, \
     MS_ACCESS_CONFIG_MODEL_APP_STATUS_OPCODE \
    )

/**
    \brief API to get all SIG model Appkeys

    \par Description
    The Config SIG Model App Get is an acknowledged message used to request
    report of all AppKeys bound to the SIG Model.

    \param [in] param
           Pointer to the structure populated as in \ref ACCESS_CONFIG_SIG_MODEL_APP_GET_PARAM

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_config_client_sig_model_app_get(param)\
    MS_config_client_send_reliable_pdu \
    (\
     MS_ACCESS_CONFIG_SIG_MODEL_APP_GET_OPCODE,\
     (void *)param, \
     MS_ACCESS_CONFIG_SIG_MODEL_APP_LIST_OPCODE \
    )

/**
    \brief API to get all Vendor model Appkeys

    \par Description
    The Config Vendor Model App Get is an acknowledged message used to request
    report of all AppKeys bound to the Vendor Model.

    \param [in] param
           Pointer to the structure populated as in \ref ACCESS_CONFIG_VENDOR_MODEL_APP_GET_PARAM

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_config_client_vendor_model_app_get(param)\
    MS_config_client_send_reliable_pdu \
    (\
     MS_ACCESS_CONFIG_VENDOR_MODEL_APP_GET_OPCODE,\
     (void *)param, \
     MS_ACCESS_CONFIG_VENDOR_MODEL_APP_LIST_OPCODE \
    )

/**
    \brief API to reset a node

    \par Description
    The Config Node Reset is an acknowledged message used to reset a node
    (other than a Provisioner) and remove it from the network.

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_config_client_node_reset()\
    MS_config_client_send_reliable_pdu \
    (\
     MS_ACCESS_CONFIG_NODE_RESET_OPCODE,\
     NULL, \
     MS_ACCESS_CONFIG_NODE_RESET_STATUS_OPCODE \
    )

/**
    \brief API to get friend state

    \par Description
    The Config Friend Get is an acknowledged message used to get the
    current Friend state of a node.

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_config_client_friend_get()\
    MS_config_client_send_reliable_pdu \
    (\
     MS_ACCESS_CONFIG_FRIEND_GET_OPCODE,\
     NULL, \
     MS_ACCESS_CONFIG_FRIEND_STATUS_OPCODE \
    )

/**
    \brief API to set friend state

    \par Description
    The Config Friend Set is an acknowledged message used to set the
    Friend state of a node.

    \param [in] param
           Pointer to the structure populated as in \ref ACCESS_CONFIG_FRIEND_SET_PARAM

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_config_client_friend_set(param)\
    MS_config_client_send_reliable_pdu \
    (\
     MS_ACCESS_CONFIG_FRIEND_SET_OPCODE,\
     (void *)param, \
     MS_ACCESS_CONFIG_FRIEND_STATUS_OPCODE \
    )

/**
    \brief API to get key refresh phase state

    \par Description
    The Config Key Refresh Phase Get is an acknowledged message used
    to get the current Key Refresh Phase state of the identified network key.

    \param [in] param
           Pointer to the structure populated as in \ref ACCESS_CONFIG_KEYREFRESH_PHASE_GET_PARAM

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_config_client_keyrefresh_phase_get(param)\
    MS_config_client_send_reliable_pdu \
    (\
     MS_ACCESS_CONFIG_KEY_REFRESH_PHASE_GET_OPCODE,\
     (void *)param, \
     MS_ACCESS_CONFIG_KEY_REFRESH_PHASE_STATUS_OPCODE \
    )

/**
    \brief API to set key refresh phase state

    \par Description
    The Config Key Refresh Phase Set is an acknowledged message used
    to set the current Key Refresh Phase state of the identified network key.

    \param [in] param
           Pointer to the structure populated as in \ref ACCESS_CONFIG_KEYREFRESH_PHASE_SET_PARAM

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_config_client_keyrefresh_phase_set(param)\
    MS_config_client_send_reliable_pdu \
    (\
     MS_ACCESS_CONFIG_KEY_REFRESH_PHASE_SET_OPCODE,\
     (void *)param, \
     MS_ACCESS_CONFIG_KEY_REFRESH_PHASE_STATUS_OPCODE \
    )

/**
    \brief API to get heartbeat publication state

    \par Description
    The Config Heartbeat Publication Get is an acknowledged message used to get
    the current Heartbeat Publication state of an element.

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_config_client_heartbeat_publication_get()\
    MS_config_client_send_reliable_pdu \
    (\
     MS_ACCESS_CONFIG_HEARTBEAT_PUBLICATION_GET_OPCODE,\
     NULL, \
     MS_ACCESS_CONFIG_HEARTBEAT_PUBLICATION_STATUS_OPCODE \
    )

/**
    \brief API to set heartbeat publication state

    \par Description
    The Config Heartbeat Publication Set is an acknowledged message
    used to set the current Heartbeat Publication state of an element.

    \param [in] param
           Pointer to the structure populated as in \ref ACCESS_CONFIG_HEARTBEATPUB_SET_PARAM

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_config_client_heartbeat_publication_set(param)\
    MS_config_client_send_reliable_pdu \
    (\
     MS_ACCESS_CONFIG_HEARTBEAT_PUBLICATION_SET_OPCODE,\
     (void *)param, \
     MS_ACCESS_CONFIG_HEARTBEAT_PUBLICATION_STATUS_OPCODE \
    )

/**
    \brief API to get heartbeat subscription state

    \par Description
    The Config Heartbeat Subscription Get is an acknowledged message used
    to get the current Heartbeat Subscription state of an element.

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_config_client_heartbeat_subscription_get()\
    MS_config_client_send_reliable_pdu \
    (\
     MS_ACCESS_CONFIG_HEARTBEAT_SUBSCRIPTION_GET_OPCODE,\
     NULL, \
     MS_ACCESS_CONFIG_HEARTBEAT_SUBSCRIPTION_STATUS_OPCODE \
    )

/**
    \brief API to set heartbeat subscription state

    \par Description
    The Config Heartbeat Publication Set is an acknowledged message
    used to set the current Heartbeat Subscription state of an element.

    \param [in] param
           Pointer to the structure populated as in \ref ACCESS_CONFIG_HEARTBEATSUB_SET_PARAM

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_config_client_heartbeat_subscription_set(param)\
    MS_config_client_send_reliable_pdu \
    (\
     MS_ACCESS_CONFIG_HEARTBEAT_SUBSCRIPTION_SET_OPCODE,\
     (void *)param, \
     MS_ACCESS_CONFIG_HEARTBEAT_SUBSCRIPTION_STATUS_OPCODE \
    )

/**
    \brief API to get LPN Polltimeout state

    \par Description
    The Config Low Power Node PollTimeout Get is an acknowledged message used
    to get the current value of PollTimeout timer of the Low Power node within
    a Friend node. The message is sent to a Friend node that has claimed to be
    handling messages by sending ACKs On Behalf Of (OBO) the indicated Low
    Power node. This message should only be sent to a node that has the Friend
    feature supported and enabled.

    \param [in] param
           Pointer to the structure populated as in \ref ACCESS_CONFIG_LPNPOLLTIMEOUT_GET_PARAM

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_config_client_lpn_polltimeout_get(param)\
    MS_config_client_send_reliable_pdu \
    (\
     MS_ACCESS_CONFIG_LOW_POWER_NODE_POLLTIMEOUT_GET_OPCODE,\
     (void *)param, \
     MS_ACCESS_CONFIG_LOW_POWER_NODE_POLLTIMEOUT_STATUS_OPCODE \
    )

/**
    \brief API to get Network transmit state

    \par Description
    The Config Network Transmit Get is an acknowledged message used to get
    the current Network Transmit state of a node.

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_config_client_network_transmit_get()\
    MS_config_client_send_reliable_pdu \
    (\
     MS_ACCESS_CONFIG_NETWORK_TRANSMIT_GET_OPCODE,\
     NULL, \
     MS_ACCESS_CONFIG_NETWORK_TRANSMIT_STATUS_OPCODE \
    )

/**
    \brief API to set Network transmit state

    \par Description
    The Config Network Transmit Set is an acknowledged message used to set
    the current Network Transmit state of a node.

    \param [in] param
           Pointer to the structure populated as in \ref ACCESS_CONFIG_NETWORK_TRANSMIT_SET_PARAM

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_config_client_network_transmit_set(param)\
    MS_config_client_send_reliable_pdu \
    (\
     MS_ACCESS_CONFIG_NETWORK_TRANSMIT_SET_OPCODE,\
     (void *)param, \
     MS_ACCESS_CONFIG_NETWORK_TRANSMIT_STATUS_OPCODE \
    )

/** \} */

/**
    \defgroup config_svr_api_defs Configuration Server API Definitions
    \{
    This section describes the Configuration Server APIs.
*/

/**
    \brief API to initialize configuration server model

    \par Description
    This is to initialize configuration server model and to register with Acess layer.

    \param [in] element_handle
                Element identifier to be associated with the model instance.

    \param [in, out] model_handle
                     Model identifier associated with the model instance on successful initialization.
                     After power cycle of an already provisioned node, the model handle will have
                     valid value and the same will be reused for registration.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_config_server_init
(
    /* IN */    MS_ACCESS_ELEMENT_HANDLE    element_handle,
    /* INOUT */ MS_ACCESS_MODEL_HANDLE*     model_handle
);


typedef API_RESULT ( *APP_config_server_CB_t )(
    /* IN */ MS_ACCESS_MODEL_HANDLE*,
    /* IN */ MS_NET_ADDR,
    /* IN */ MS_NET_ADDR,
    /* IN */ MS_SUBNET_HANDLE,
    /* IN */ MS_APPKEY_HANDLE,
    /* IN */ UINT32,
    /* IN */ UCHAR*,
    /* IN */ UINT16,
    /* IN */ API_RESULT,
    /* IN */ UINT32,
    /* IN */ UCHAR*,
    /* IN */ UINT16                    );

void APP_config_server_CB_init(APP_config_server_CB_t appConfigServerCB);

/** \} */

/** \} */

/** \} */

#endif /* _H_MS_CONFIG_API_ */

