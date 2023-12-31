
/**
    \file MS_net_api.h

    \brief This file defines the Mesh Network Layer Interface - includes
    Data Structures and Methods.
*/

/*
    Copyright (C) 2017. Mindtree Ltd.
    All rights reserved.
*/

#ifndef _H_MS_NET_API_
#define _H_MS_NET_API_


/* --------------------------------------------- Header File Inclusion */
/* Bearer Layer */
#include "MS_brr_api.h"
#include "MS_prov_api.h"

/* --------------------------------------------- Global Definitions */

/**
    \defgroup net_module NETWORK (Mesh Network Layer)
    \{
    This section describes the interfaces & APIs offered by the EtherMind
    Mesh Network (NETWORK) module to the Application and other upper
    layers of the stack.
*/

/**
    \defgroup net_defines Defines
    \{
    Describes defines for the module.
*/

/**
    \defgroup net_constants Constants
    \{
    Describes Constants defined by the module.
*/

/**
    \defgroup net_addressing Addressing
    \{
    Describes five basic type of addresses, the Network Layer defines.
*/

/**
    Unassigned Address.
    An unassigned address is an address that means that the component of a node
    has not been configured yet or no address has been allocated.

    An unassigned address shall not be used in a source or destination address
    field of a message.

    A Primary Component shall not have an unassigned address.
    A Secondary Component may have an unassigned address.
    A component with an unassigned address is inactive (i.e., it cannot send
    nor receive and process messages).
*/
#define MS_NET_ADDR_UNASSIGNED                  0x0000
/** Network Address Type - Invalid */
#define MS_NET_ADDR_TYPE_INVALID                0x00

/**
    Unicast Address.
    A unicast address is a unique address allocated to each component within a node.
    A unicast address has bit 15 cleared to zero.
    The unicast address shall not have the value 0x0000,
    and therefore can have any value from 0x0001 to 0x7FFF inclusive.

    A unicast address is allocated to a primary component of an unprovisioned
    device by the Provisioner during provisioning for the lifetime of the node
    on the network.

    A unicast address can be allocated to any secondary component within
    an already provisioned node.

    A unicast address shall be used in the source address field of a message
    and can be used in a destination address field of a message.
    A message sent to a unicast address will be processed by at most one node
    or one component within a node.
*/
/** Bitmask for Network Address Type - Unicast */
#define MS_NET_ADDR_UNICAST_BIT_MASK            0x8000
/** Comparison value for Network Address Type - Unicast */
#define MS_NET_ADDR_UNICAST_COMPARE             0x0000
/** Network Address Type - Unicast */
#define MS_NET_ADDR_TYPE_UNICAST                0x01

/**
    Virtual Address.
    A virtual address is an address that is similar to a group address,
    in that multiple devices may know such an address.
    This type of address is used to identify a label to which devices
    may publish or subscribe.
    The label referred to by a given virtual address is uniquely identified
    by a 128-bit UUID, called virtual label UUID.

    The format of a Virtual Address is 10vv vvvv vvvv vvvv.
    The fourteen v-bits are the least significant bits of the following calculation:
       v = AES-CMAC (virtual label UUID, "vtad")[0-13]
*/
/** Bitmask for Network Address Type - Virtual */
#define MS_NET_ADDR_VIRTUAL_BIT_MASK            0xC000
/** Comparison value for Network Address Type - Virtual */
#define MS_NET_ADDR_VIRTUAL_COMPARE             0x8000
/** Network Address Type - Virtual */
#define MS_NET_ADDR_TYPE_VIRTUAL                0x02

/**
    Group Address.
    A group address is an address that is programmed into zero or more nodes or
    components within nodes.
    A group address has bit 15 set to one and bit 14 set to one.
    The group address shall not have the value 0xFFFF, and therefore can have
    any value from 0xC000 to 0xFFFE.
    A group address shall only be used in the destination address field of a message.
    A message sent to a group address will be processed by all the nodes that know
    this group address.
*/
/** Bitmask for Network Address Type - Group */
#define MS_NET_ADDR_GROUP_BIT_MASK              0xC000
/** Comparison value for Network Address Type - Group */
#define MS_NET_ADDR_GROUP_COMPARE               0xC000
/** Network Address Type - Group */
#define MS_NET_ADDR_TYPE_GROUP                  0x03

/**
    Fixed Group Addresses are all-proxies, all-friends, all-relays and all-nodes.

    Note: Fixed Group Addresses in the range 0xFF00 - 0xFFFB are Reserved for Future.
*/
/** Fixed Group Address - All-Proxies */
#define MS_NET_ADDR_ALL_PROXIES                 0xFFFC
/** Fixed Group Address - All-Friends */
#define MS_NET_ADDR_ALL_FRIENDS                 0xFFFD
/** Fixed Group Address - All-Relays */
#define MS_NET_ADDR_ALL_RELAYS                  0xFFFE
/** Fixed Group Address - All-Nodes */
#define MS_NET_ADDR_ALL_NODES                   0xFFFF

/**
    Address Validity

    | Address Type | Valid in Source Address Field  | Valid in Destination Address Field |
    | :----------: | :----------------------------: | :--------------------------------: |
    | Unassigned   | No                             | No                                 |
    | Unicast      | Yes                            | Yes                                |
    | Virtual      | No                             | Yes                                |
    | Group        | No                             | Yes                                |
    | Broadcast    | No                             | Yes                                |
*/

/** Network Layer Feature Idenfiers */
/** Network Layer Feature - Proxy */
#define MS_NET_FEATURE_PROXY      0x00
/** Network Layer Feature - Relay */
#define MS_NET_FEATURE_RELAY      0x01

/** Primary Subnet - NetKey Index is 0x000 */
#define MS_PRIMARY_SUBNET         0x000

/** Invalid Subnet Handle */
#define MS_INVALID_SUBNET_HANDLE  0xFFFF

/** Invalid AppKey Handle */
#define MS_INVALID_APPKEY_HANDLE  0xFFFF

/** \} */

/**
    \defgroup net_proxy Proxy
    \{
    Describes Network Layer Proxy Feature related defines.
*/
/** GATT Proxy Filter Types */
/** GATT Proxy Filter Type - Whitelist */
#define MS_PROXY_WHITELIST_FILTER        0x00
/** GATT Proxy Filter Type - Blacklist */
#define MS_PROXY_BLACKLIST_FILTER        0x01

/** GATT Proxy Configuration Opcodes */
/** GATT Proxy Configuration - Set Filter Opcode */
#define MS_PROXY_SET_FILTER_OPCODE       0x00
/** GATT Proxy Configuration - Add to Filter Opcode */
#define MS_PROXY_ADD_TO_FILTER_OPCODE    0x01
/** GATT Proxy Configuration - Remove From Filter Opcode */
#define MS_PROXY_REM_FROM_FILTER_OPCODE  0x02
/** GATT Proxy Configuration - Filter Status Opcode */
#define MS_PROXY_FILTER_STATUS_OPCODE    0x03

/** GATT Proxy ADV Modes */
/** Network ID Type */
#define MS_PROXY_NET_ID_ADV_MODE         0x01
/** Node Idetity Type */
#define MS_PROXY_NODE_ID_ADV_MODE        0x02

/** \} */

/** \} */

/**
    \defgroup net_events Events
    \{
    This section lists the Asynchronous Events notified to Application by the
    Module.
*/

/** GATT Proxy Events */
/** GATT Proxy Event - Interface UP */
#define MS_PROXY_UP_EVENT                0x00
/** GATT Proxy Event - Interface Down */
#define MS_PROXY_DOWN_EVENT              0x01
/** GATT Proxy Event - Status */
#define MS_PROXY_STATUS_EVENT            0x02

/** \} */

/**
    \defgroup net_proxy states
    \{
    This section lists the various states of Proxy Module exposed by it to
    other Modules.
*/

/**
    GATT Proxy States.

    | Proxy Callback   |  Proxy Iface |  Error Code
    |------------------|--------------|-------------------
    |  NULL            |  Down        | MS_PROXY_NULL
    |  NULL            |  Up          | MS_PROXY_NULL
    |  !NULL           |  Down        | MS_PROXY_READY
    |  !NULL           |  UP          | MS_PROXY_CONNECTED
*/
/** GATT Proxy State - Invalid/Not Initialized */
#define MS_PROXY_NULL                    0x00
/** GATT Proxy State - Ready/Initialized */
#define MS_PROXY_READY                   0x01
/** GATT Proxy State - Connected */
#define MS_PROXY_CONNECTED               0x02


/* Secure Beacon Network Timer (minimum of 10 s) */
extern EM_timer_handle ms_snb_timer_handle;

extern EM_timer_handle ms_iv_update_timer_handle;

extern EM_timer_handle net_key_refresh_timer_handle;

/* Secure Beacon Network Beacon Timeout value - default 10s */
#define MS_SNB_TIMEOUT     (10 * 1000) /* in ms */


/** \} */
/** \} */

/**
    \defgroup net_marcos Utility Macros
    \{
    Initialization and other Utility Macros offered by the module.
*/

/** \} */

/* --------------------------------------------- Data Types/ Structures */

/**
    \addtogroup net_defines Defines
    \{
*/

/**
    \addtogroup net_structures Structures
    \{
*/

/** Network Address Type */
typedef UINT16 MS_NET_ADDR;

/** Subnet Handle */
typedef UINT16 MS_SUBNET_HANDLE;

/** AppKey Handle */
typedef UINT16 MS_APPKEY_HANDLE;

/** Address Type */
typedef UCHAR MS_NET_ADDR_TYPE;

/** Network Header Type */
typedef struct _MS_NET_HEADER
{
    /** Least significant bit of IV Index - 1 bit */
    UINT8 ivi;

    /**
        Value derived from the NetKey used to identify
        the Encrytion Key and Privacy Key used to secure
        this PDU - 7 bits
    */
    UINT8 nid;

    /** Network Control - 1 bit */
    UINT8 ctl;

    /** Time To Live - 7 bits */
    UINT8 ttl;

    /** 16 Bit Source Address */
    MS_NET_ADDR saddr;

    /** 16 Bit Destination Address */
    MS_NET_ADDR daddr;

    /** 24 bit sequence number - currently filled only in recption path */
    UINT32 seq_num;

} MS_NET_HEADER;

/** Data structures for filter type and address list */
typedef UCHAR PROXY_FILTER_TYPE;

/** Proxy Address */
typedef MS_NET_ADDR PROXY_ADDR;

/** Network Interface Handle */
typedef UINT8    NETIF_HANDLE;

/** Current Sequence Number and Block State */
typedef struct _NET_SEQ_NUMBER_STATE
{
    /** Current Sequence Number */
    UINT32 seq_num;

    /** Block Sequence number - maximum available */
    UINT32  block_seq_num_max;

} NET_SEQ_NUMBER_STATE;

/** \} */

/** \} */

/**
    \defgroup net_cb Application Callback
    \{
    This Section Describes the module Notification Callback interface offered
    to the application
*/
/**
    NETWORK Application Asynchronous Notification Callback.

    NETWORK calls the registered callback to indicate events occurred to the
    application.

    \param brr_type          Bearer Type.
    \param net_hdr           Network Header.
    \param subnet_handle     Associated Subnet Handle.
    \param data_param        Data associated with the event if any or NULL.
    \param data_len          Size of the event data. 0 if event data is NULL.

    \return
    - \ref NET_POST_PROCESS_RX_PKT: To inform Network Layer if the packet to be
          further processed, e.g. to be relayed or proxied etc.

    - Any Other Result/Error Code defined in MS_error.h: Ignored by Network Layer.
*/
typedef API_RESULT (*NET_NTF_CB)
(
    MS_NET_HEADER*     net_hdr,
    MS_SUBNET_HANDLE   subnet_handle,
    UCHAR*             data_param,
    UINT16             data_len
) DECL_REENTRANT;

/**
    Network Proxy Application Asynchronous Notification Callback.

    NETWORK PROXY calls the registered callback to indicate events occurred to the
    application.

    \param handle            Network Interface Handle.
    \param p_evt             Proxy Event.
    \param data_param        Data associated with the event if any or NULL.
    \param data_len          Size of the event data. 0 if event data is NULL.
*/
typedef void (*PROXY_NTF_CB)
(
    NETIF_HANDLE*        handle,
    UCHAR                p_evt,
    UCHAR*               data_param,
    UINT16               data_len
) DECL_REENTRANT;
/** \} */

/**
    \defgroup net_api_defs API Definitions
    \{
    This section describes the EtherMind Mesh Network Layer APIs.
*/

/**
    Macro to check if Unicast Address
*/
#define MS_IS_UNICAST_ADDR(addr) \
    (((MS_NET_ADDR_UNASSIGNED != (addr)) && \
      (MS_NET_ADDR_UNICAST_COMPARE == ((addr) & MS_NET_ADDR_UNICAST_BIT_MASK))) \
     ? MS_TRUE : MS_FALSE)

/**
    Macro to check if Virtual Address
*/
#define MS_IS_VIRTUAL_ADDR(addr) \
    ((MS_NET_ADDR_VIRTUAL_COMPARE == ((addr) & MS_NET_ADDR_VIRTUAL_BIT_MASK)) \
     ? MS_TRUE : MS_FALSE)

/**
    Macro to check if Group Address
*/
#define MS_IS_GROUP_ADDR(addr) \
    ((MS_NET_ADDR_GROUP_COMPARE == ((addr) & MS_NET_ADDR_GROUP_BIT_MASK)) \
     ? MS_TRUE : MS_FALSE)

/**
    Source address shall be Unicast Address.
*/
#define NET_IS_VALID_SRC_ADDR(addr) \
    MS_IS_UNICAST_ADDR(addr)

/**
    Destination address
    - Shall not be Unassigned Address.
    - Control Message shall not be a Virtual Address.
*/
#define NET_IS_VALID_DST_ADDR(addr, ctl) \
    (((MS_NET_ADDR_UNASSIGNED != (addr)) && \
      ((0x01 != (ctl)) || (MS_FALSE == MS_IS_VIRTUAL_ADDR(addr)))) \
     ? MS_TRUE : MS_FALSE)

#ifdef MS_PROXY_CLIENT
/* GATT Proxy Client Related defines */
/**
    \brief Set Proxy WhiteList Filter.

    \par Description This function is used by the Proxy Client
    to set the filter type on the Proxy Server to \ref MS_PROXY_WHITELIST_FILTER.

    \param [in] nh Network Interface Handle
    \param [in] sh Subnet Handle

    \note This API will be used by the Proxy Client only.

    \return API_SUCCESS or Error Code on failure
*/
#define MS_proxy_set_whitelist_filter(nh,sh)    \
    MS_proxy_set_filter                     \
    (                                       \
                                            (nh),                               \
                                            (sh),                               \
                                            MS_PROXY_WHITELIST_FILTER           \
    );

/**
    \brief Set Proxy BlackList Filter.

    \par Description This function is used by the Proxy Client
    to set the filter type on the Proxy Server to \ref MS_PROXY_BLACKLIST_FILTER.

    \param [in] nh Network Interface Handle
    \param [in] sh Subnet Handle

    \note This API will be used by the Proxy Client only.

    \return API_SUCCESS or Error Code on failure
*/
#define MS_proxy_set_blacklist_filter(nh,sh)    \
    MS_proxy_set_filter                     \
    (                                       \
                                            (nh),                               \
                                            (sh),                               \
                                            MS_PROXY_BLACKLIST_FILTER           \
    );

/**
    \brief Add addressess to Proxy Filter List.

    \par Description This function is used by the Proxy Client
    to add Addressess to the Proxy Server's filter List.

    \param [in] nh Network Interface Handle
    \param [in] sh Subnet Handle
    \param [in] a  Pointer to List of Address to be added
    \param [in] c  Count of Addressess present in the provided List

    \note This API will be used by the Proxy Client only.

    \return API_SUCCESS or Error Code on failure
*/
#define MS_proxy_add_to_list(nh,sh,a,c)         \
    MS_proxy_filter_op                      \
    (                                       \
                                            (nh),                               \
                                            (sh),                               \
                                            MS_PROXY_ADD_TO_FILTER_OPCODE,      \
                                            (a),                                \
                                            (c)                                 \
    );

/**
    \brief Delete addresses from Proxy Filter List.

    \par Description This function is used by the Proxy Client
    to delete/remove Addresses from the Proxy Server's filter List.

    \param [in] nh Network Interface Handle
    \param [in] sh Subnet Handle
    \param [in] a  Pointer to List of Address to be deleted/removed
    \param [in] c  Count of Addressess present in the provided List

    \note This API will be used by the Proxy Client only.

    \return API_SUCCESS or Error Code on failure
*/
#define MS_proxy_del_from_list(nh,sh,a,c)       \
    MS_proxy_filter_op                      \
    (                                       \
                                            (nh),                               \
                                            (sh),                               \
                                            MS_PROXY_REM_FROM_FILTER_OPCODE,    \
                                            (a),                                \
                                            (c)                                 \
    );
#endif /* MS_PROXY_CLIENT */

/* --------------------------------------------- Function */

#ifdef __cplusplus
extern "C" {
#endif

/**
    \brief Register Inerface with NETWORK Layer

    \par Description
    This routine registers interface with the NETWORK Layer.
    NETWORK Layer supports only one upper layer, hence this routine shall be called once.

    \param [in] net_cb
           Upper Layer Notification Callback

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_net_register
(
    /* IN */ NET_NTF_CB    net_cb
);

/**
    \brief API to send Secure Network Beacon

    \par Description
    This routine sends Secure Network Beacon for the
    given subnet handle

    \param [in] subnet_handle
           Subnet handle of the network to be broadcasted.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_net_broadcast_secure_beacon
(
    /* IN */ MS_SUBNET_HANDLE    subnet_handle
);

/**
    \brief API to send NETWORK PDUs

    \par Description
    This routine sends NETWORK PDUs to peer device.

    \param [in] hdr
           Network Header

    \param [in] subnet_handle
           Subnet Handle

    \param [in] buffer
           Lower Transport Payload

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_net_send_pdu
(
    /* IN */ MS_NET_HEADER*     hdr,
    /* IN */ MS_SUBNET_HANDLE   subnet_handle,
    /* IN */ MS_BUFFER*         buffer,
    /* IN */ UINT8              is_seg
);

/**
    \brief To get address type.

    \par Description
    This routine is to get address type for a given address.

    \param [in] addr            Input Network Address

    \return One of the following address type
            \ref MS_NET_ADDR_TYPE_INVALID
            \ref MS_NET_ADDR_TYPE_UNICAST
            \ref MS_NET_ADDR_TYPE_VIRTUAL
            \ref MS_NET_ADDR_TYPE_GROUP
*/
MS_NET_ADDR_TYPE MS_net_get_address_type
(
    /* IN */ MS_NET_ADDR addr
);

/**
    \brief Register Interface with NETWORK PROXY Layer

    \par Description
    This routine registers interface with the NETWORK PROXY Layer.
    NETWORK PROXY Layer supports only one upper layer, hence this rouine shall be called once.

    \param [in] proxy_cb
           Upper Layer Notification Callback

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_proxy_register
(
    /* IN */ PROXY_NTF_CB    proxy_cb
);

/**
    \brief Check if the Proxy Module is ready to handle Proxy Messages/Events

    \par Description
    This routine returns the current state of the Proxy. The valid states of
    proxy are:
    1. MS_PROXY_NULL      - If no callback registered by Upper Layers
    2. MS_PROXY_READY     - If callback registered and Proxy not connected
    3. MS_PROXY_CONNECTED - if callback registered and Proxy connected

    \param [out] proxy_state returns the current state of the Proxy

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_proxy_fetch_state (UCHAR* proxy_state);

#ifdef MS_PROXY_CLIENT
/**
    \cond ignore_this Ignore this block while generating doxygen document
*/
/**
    \brief Set Proxy Server's Filter Type.

    \par Description This function is used by the Proxy Client
    to set the filter type on the Proxy Server.

    \param [in] handle Network Interface Handle
    \param [in] subnet_handle Subnet Handle
    \param [in] type Type of the Proxy Filter to be set. Either
                \ref MS_PROXY_WHITELIST_FILTER or
                \ref MS_PROXY_BLACKLIST_FILTER

    \note This API will be used by the Proxy Client only.

    \return API_SUCCESS or Error Code on failure
*/
API_RESULT MS_proxy_set_filter
(
    /* IN */ NETIF_HANDLE*        handle,
    /* IN */ MS_SUBNET_HANDLE   subnet_handle,
    /* IN */ PROXY_FILTER_TYPE type
);

/**
    \brief Add or Delete/Remove addresses to/from Proxy Filter List.

    \par Description This function is used by the Proxy Client
    to add/delete Addresses to/from the Proxy Server's filter List.

    \param [in] handle Network Interface Handle
    \param [in] subnet_handle Subnet Handle
    \param [in] opcode Operation to be performed. Either
                \ref MS_PROXY_ADD_TO_FILTER_OPCODE or
                \ref MS_PROXY_REM_FROM_FILTER_OPCODE
    \param [in] addr        Pointer to List of Address to be added/deleted
    \param [in] addr_count  Count of Addresses present in the provided List

    \note This API will be used by the Proxy Client only.

    \return API_SUCCESS or Error Code on failure
*/
API_RESULT MS_proxy_filter_op
(
    /* IN */ NETIF_HANDLE*        handle,
    /* IN */ MS_SUBNET_HANDLE     subnet_handle,
    /* IN */ UCHAR                opcode,
    /* IN */ PROXY_ADDR*          addr,
    /* IN */ UINT16               addr_count
);
/**
    \endcond
*/
#endif /* MS_PROXY_CLIENT */

/* Function to Start ADV using Proxy */
#ifdef MS_PROXY_SERVER
/**
    \brief Start Connectable Advertisements for a Proxy Server.

    \par Description This function is used by the Proxy Server
    to start Connectable Undirected Advertisements.

    \param [in] subnet_handle  Subnet Handle which the Proxy Server is
                part of.
    \param [in] proxy_adv_mode Mode of Proxy Advertisements. This could
                be of two types
                \ref MS_PROXY_NET_ID_ADV_MODE or
                \ref MS_PROXY_NODE_ID_ADV_MODE

    \note This API will be used by the Proxy Server only.

    \return API_SUCCESS or Error Code on failure
*/
API_RESULT MS_proxy_server_adv_start
(
    /* IN */ MS_SUBNET_HANDLE   subnet_handle,
    /* IN */ UCHAR              proxy_adv_mode
);

API_RESULT MS_proxy_server_stop_timer(void);


/**
    \brief Stop Connectable Advertisements for a Proxy Server.

    \par Description This function is used by the Proxy Server
    to stop Connectable Undirected Advertisements.

    \note This API will be used by the Proxy Server only.

    \return API_SUCCESS or Error Code on failure
*/
API_RESULT MS_proxy_server_adv_stop (void);
#endif /* MS_PROXY_SERVER */

/**
    \brief To allocate Sequence Number.

    \par Description This function is used to allocate
    Sequence Number.

    \param [out] seq_num   Location where SeqNum to be filled.

    \return API_SUCCESS or Error Code on failure
*/
API_RESULT MS_net_alloc_seq_num(/* OUT */ UINT32*    seq_num);

API_RESULT MS_net_start_iv_update_timer(UINT8 timer_flag,UINT8 reset_en);

void MS_net_stop_iv_update_timer(void);

/**
    \brief To get current Sequence Number state.

    \par Description This function is used to get current
    Sequence Number state.

    \param [out] seq_num_state  Location where Seq Number state to be filled.

    \return API_SUCCESS or Error Code on failure
*/
API_RESULT MS_net_get_seq_num_state(/* OUT */ NET_SEQ_NUMBER_STATE* seq_num_state);

/**
    \brief To set current Sequence Number state.

    \par Description This function is used to set current
    Sequence Number state.

    \param [in] seq_num_state  Location from where Seq Number state to be taken.

    \return API_SUCCESS or Error Code on failure
*/
API_RESULT MS_net_set_seq_num_state(/* IN */ NET_SEQ_NUMBER_STATE* seq_num_state);

/* Start Secure Network Beacon Timer */
void MS_net_start_snb_timer
(
    /* IN */    MS_SUBNET_HANDLE         subnet_handle
);

/* Stop Timer */
void MS_net_stop_snb_timer
(
    /* IN */ MS_SUBNET_HANDLE         subnet_handle
);

void MS_net_iv_update_rcv_pro
(
    /* IN */ UINT32    iv_index,
    /* IN */ UINT8     iv_update_flag
);

void MS_net_key_refresh
(
    /* IN */ MS_NET_ADDR*            key_refresh_whitelist,
    /* IN */ UINT16                 num,
    /* IN */ UINT8*                  new_net_key
);


/* Start Secure Network Beacon Timer */
void MS_net_start_key_refresh_timer
(
    /* IN */    UINT16              context,
    /* IN */    UINT32              time
);

/* Stop Key refresh Timer */
void MS_net_stop_key_refresh_timer
(
    void
);


void MS_net_key_refresh_init(void);




#ifdef __cplusplus
};
#endif

/** \} */

/** \} */

#endif /* _H_MS_NET_API_ */

