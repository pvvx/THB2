
/**
    \file net_internal.h

    Module Internal Header File contains structure definitions including tables
    maintained by the module
*/

/*
    Copyright (C) 2016. Mindtree Ltd.
    All rights reserved.
*/

#ifndef _H_NET_INTERNAL_
#define _H_NET_INTERNAL_

/* --------------------------------------------- Header File Inclusion */
#include "MS_common.h"
#include "MS_net_api.h"
#include "MS_brr_api.h"

/* --------------------------------------------- Global Definitions */

#ifdef NET_NO_DEBUG
    #define NET_ERR          EM_debug_null
#else /* NET_NO_DEBUG */
    #ifdef VAR_ARG_IN_MACRO_NOT_SUPPORTED
        #define NET_ERR
    #else
        #define NET_ERR(...)     EM_debug_error(MS_MODULE_ID_NET, __VA_ARGS__)
    #endif /* VAR_ARG_IN_MACRO_NOT_SUPPORTED */
#endif /* NET_NO_DEBUG */

#ifdef NET_DEBUG
    #ifdef VAR_ARG_IN_MACRO_NOT_SUPPORTED
        #define NET_TRC
        #define NET_INF

        #define NET_debug_dump_bytes(data, datalen)

    #else
        #define NET_TRC(...)     EM_debug_trace(MS_MODULE_ID_NET,__VA_ARGS__)
        #define NET_INF(...)     EM_debug_info(MS_MODULE_ID_NET,__VA_ARGS__)

        #define NET_debug_dump_bytes(data, datalen) EM_debug_dump_bytes(MS_MODULE_ID_NET, (data), (datalen))

    #endif /* VAR_ARG_IN_MACRO_NOT_SUPPORTED */
#else /* NET_DEBUG */
    #define NET_TRC          EM_debug_null
    #define NET_INF          EM_debug_null

    #define NET_debug_dump_bytes(data, datalen)

#endif /* NET_DEBUG */

/**
    Locks the NET Mutex which prevents any global variable being
    overwritten by any function. It returns an error if mutex lock fails.
*/
#define NET_LOCK()\
    MS_MUTEX_LOCK(net_mutex, NET)

/**
    Locks the NET_mutex which prevents any global variable being
    overwritten by any function. To be used in void function as it
    returns no error.
*/
#define NET_LOCK_VOID()\
    MS_MUTEX_LOCK_VOID(net_mutex, NET)

/**
    Unlocks the NET_mutex which realeses the global variables
    to be written into. It returns an error if mutex unlock fails.
*/
#define NET_UNLOCK()\
    MS_MUTEX_UNLOCK(net_mutex, NET)

/**
    Unlocks the NET_mutex which realeses the global variables
    to be written into. To be used in void functions as it returns
    no error.
*/
#define NET_UNLOCK_VOID()\
    MS_MUTEX_UNLOCK_VOID(net_mutex, NET)


#define net_alloc_mem(size)\
    EM_alloc_mem(size)

#define net_free_mem(ptr)\
    EM_free_mem(ptr)

#ifndef NET_NO_NULL_PARAM_CHECK
/** Null Check of Network API Parameters */
#define NET_NULL_CHECK(x) \
    if (NULL == (x)) \
    { \
        NET_ERR(  \
                  "[NET] NULL Pointer detected. Referrence Impossible\n"); \
        return NET_NULL_PARAMETER_NOT_ALLOWED; \
    }
#else
#define NET_NULL_CHECK(x)
#endif /* NET_NO_NULL_PARAM_CHECK */

#ifndef MS_NET_NO_RANGE_CHECK
/** Range Check for Network API Parameters */
#define NET_RANGE_CHECK_START(param, start) \
    if ( ! ((param) >= (start)) ) \
    { \
        NET_ERR( \
                 "[NET] NET Range Check FAILED\n"); \
        return NET_PARAMETER_OUTSIDE_RANGE; \
    }

#define NET_RANGE_CHECK_END(param, end) \
    if ( ! ((param) <= (end)) ) \
    { \
        NET_ERR( \
                 "[NET] NET Range Check FAILED\n"); \
        return NET_PARAMETER_OUTSIDE_RANGE; \
    }

#define NET_RANGE_CHECK(param, start, end) \
    if ( ! ( ((param) >= (start)) && ((param) <= (end)) ) ) \
    { \
        NET_ERR( \
                 "[NET] NET Range Check FAILED\n"); \
        return NET_PARAMETER_OUTSIDE_RANGE; \
    }

#else
#define NET_RANGE_CHECK_START(param, start)
#define NET_RANGE_CHECK_END(param, end)
#define NET_RANGE_CHECK(param, start, end)
#endif /* NET_NO_RANGE_CHECK */

/** Network Header Size */
#define NET_HDR_SIZE           9

/** Maximum Network Payload Size - TransportPDU can be 8 to 128 bits */
#define NET_MAX_PAYLOAD_SIZE   16

/** Network MIC Size - 32 or 64 bits. */
#define NET_MIN_MIC_SIZE       4
#define NET_MAX_MIC_SIZE       8

/** Sequence Number related macro defines */
#define NET_INIT_SEQ_NUM_STATE() \
    net_seq_number_state.seq_num = 0; \
    net_seq_number_state.block_seq_num_max = 0

/**
    (all-nodes address) OR
    ((all-relays address) AND (Relay funcationality is enabled))
    ((all-friends address) AND (Friend funcationality is enabled))
    ((all-proxies address) AND (Proxy funcationality is enabled))
*/
#define NET_ADDR_FROM_FIXED_GROUP(addr) \
    (((MS_NET_ADDR_ALL_NODES == (addr)) ||\
      (MS_NET_ADDR_ALL_RELAYS == (addr)) ||\
      (MS_NET_ADDR_ALL_FRIENDS == (addr)) ||\
      (MS_NET_ADDR_ALL_PROXIES == (addr))) ? MS_TRUE: MS_FALSE)

/**
    RFU: 0xFF00 - 0xFFFB
*/
#define NET_ADDR_FROM_RFU(addr) \
    (((0xFF00 <= (addr)) && (0xFFFB >= (addr))) ? MS_TRUE : MS_FALSE)

/**
    Network Proxy Filter List Count is ONE less that configured network interface.
    First (0-th) network interface is used for Advertising Channel.
*/
#define PROXY_FILTER_LIST_COUNT (MS_CONFIG_LIMITS(MS_NUM_NETWORK_INTERFACES) - 1)


/* --------------------------------------------- Data Types/ Structures */
/** Network Cache Data Structure */
typedef struct _NET_CACHE_ELEMENT
{
    /** Least significant bit of IV Index - 1 bit */
    UINT8 ivi;

    /**
        Value derived from the NetKey used to identify
        the Encrytion Key and Privacy Key used to secure
        this PDU - 7 bits
    */
    UINT8 nid;

    /** 16 Bit Source Address */
    MS_NET_ADDR saddr;

    /** 24 bit sequence number */
    UINT32 seq_num;

} NET_CACHE_ELEMENT;

/** Network Interface Packet Type - Network Packet or Proxy Packet */
typedef UINT8    NETIF_PKT_T;

/**
    NOTE: The below define values of NETIF_PKT_T_NETWORK and NETIF_PKT_T_RELAY
         'SHALL' not be modified. The Network logic is dependent on these
         values.
*/
#define NETIF_PKT_T_NETWORK     0x00
#define NETIF_PKT_T_RELAY       0x01
#define NETIF_PKT_T_PROXY       0x02

#define NETIF_PKT_T_RELAY_MASK  NETIF_PKT_T_RELAY

#if ((NETIF_PKT_T_NETWORK != 0x00) || (NETIF_PKT_T_RELAY != 0x01) || (NETIF_PKT_T_RELAY_MASK != NETIF_PKT_T_RELAY))
    #error "NETIF_PKT_T defines modified !!"
#endif /* ((NETIF_PKT_T_NETWORK != 0x00) || (NETIF_PKT_T_RELAY != 0x01) || (NETIF_PKT_T_RELAY_MASK != NETIF_PKT_T_RELAY)) */

/** Network Tx Queue */
typedef struct _NET_TX_Q_ELEMENT
{
    /* "Allocated" Data Pointer */
    UINT8*   allocated_data;

    /*
        Data Length. If data length is zero, the element is considered
        invalid.
        If data_length is 0, element is not valid.
    */
    UINT16 data_length;

    /* Transmission Count */
    UINT8 tx_count;

    /* Transmission Interval */
    UINT8 tx_interval;

    /* Type of the Packet */
    NETIF_PKT_T type;

    /* Destination Address */
    MS_NET_ADDR d_addr;

    /* Check  Transmission Type(seg)*/
    UINT8 unsegment;

    /* Transmission Flag*/
    UINT8 tx_flag;

} NET_TX_Q_ELEMENT;

/** Proxy Filter List */
typedef struct _PROXY_FILTER_LIST
{
    /* Proxy Address List */
    MS_DEFINE_GLOBAL_ARRAY(PROXY_ADDR, p_addr, MS_CONFIG_LIMITS(MS_PROXY_FILTER_LIST_SIZE));
    MS_DEFINE_GLOBAL_ARRAY(PROXY_ADDR, v_addr, MS_CONFIG_LIMITS(MS_PROXY_FILTER_LIST_SIZE));

    /* Proxy Address List Active Count */
    UINT16             p_count;

    /* Proxy Address List Active Count */
    UINT16             v_count;

    /* Proxy List Filter Type */
    PROXY_FILTER_TYPE  type;

    /* Proxy Server/Client Role */
    UCHAR              role;

} PROXY_FILTER_LIST;

//DECL_STATIC MS_DEFINE_GLOBAL_ARRAY(PROXY_FILTER_LIST, net_proxy_list, PROXY_FILTER_LIST_COUNT);


//UINT16 proxy_filter_table[MS_PROXY_FILTER_LIST_SIZE];


/* TODO: Move to Limits.h */
/* NET TX Q */
//#define NET_TX_QUEUE_SIZE             30    //20 ->30 by ZQ
#define NET_TX_QUEUE_SIZE             40    //30 ->40 by HQ


/* TODO: To be dynamically adjusted based on the type of packet and corresponding configuration redefine by hq*/
#define NET_TX_TIMEOUT                (EM_TIMEOUT_MILLISEC | 15) /* Millisecond */

MS_DECLARE_GLOBAL_ARRAY(NET_CACHE_ELEMENT, net_cache, MS_CONFIG_LIMITS(MS_NET_CACHE_SIZE));


extern UINT16 net_cache_start;
extern UINT16 net_cache_size;



/* --------------------------------------------- Functions */
/**
    \par Description
    This function handles the incoming data received over a network interface.

    \param [in] type
           Network Inteface Packet Type
    \param [in] handle
           Network Interface Handle
    \param [in] pdata
           The incoming Data Packet
    \param [in] pdatalen
           Size of the incoming Data Packet
*/
void net_pkt_in
(
    /* IN */ NETIF_PKT_T       type,
    /* IN */ NETIF_HANDLE*     handle,
    /* IN */ UCHAR*            pdata,
    /* IN */ UINT16            pdatalen
);

API_RESULT netif_adv_recv_cb (BRR_HANDLE* handle, UCHAR pevent, UCHAR* pdata, UINT16 pdatalen);
API_RESULT netif_gatt_recv_cb (BRR_HANDLE* handle, UCHAR pevent, UCHAR* pdata, UINT16 pdatalen);

/* Initialize network cache */
void net_init_cache (void);

/* Is already in cache? */
API_RESULT net_is_in_cache
(
    /* IN */ MS_NET_HEADER*  hdr
);

/* Add to cache */
API_RESULT net_add_to_cache
(
    /* IN */ MS_NET_HEADER*  hdr
);

/**
    Routine to decode a received Network PDU,
    based on a specific network Key from the store.
    This function returns status of the decode operation.
    If success, return the decoded network header and PDU.
*/
API_RESULT net_decode_frame
(
    /* IN */  NETIF_PKT_T           type,
    /* IN */  UCHAR*                pdata,
    /* IN */  UINT16                pdatalen,
    /* OUT */ MS_SUBNET_HANDLE*     subnet_handle,
    /* OUT */ MS_NET_HEADER*        net_hdr,
    /* OUT */ UCHAR*                trn_pdu,
    /* OUT */ UINT16*               trn_pdu_len
);

/**
    \par Description
    This function obfuscates or de-obfuscates a network header.

    \param [in] network_pkt
           Network Packet
    \param [in] privacy_key
           Privacy Key
    \param [in] pecb_input
           PECB Input
    \param [out] pecb_output
           PECB Output. Can not be same pointer as PECB Input.
*/
void net_obfuscate
(
    /* IN */  UINT8*   network_pkt,
    /* IN */  UINT8*   privacy_key,
    /* IN */  UINT8*   pecb_input,
    /* OUT */ UINT8*   pecb_output
);

/**
    \par Description
    This macro de-obfuscates a network header.

    \param [in] network_pkt
           Network Packet
    \param [in] privacy_key
           Privacy Key
    \param [in] pecb_input
           PECB Input
    \param [out] pecb_output
           PECB Output. Can not be same pointer as PECB Input.
*/
#define net_de_obfuscate(network_pkt, privacy_key, pecb_input, pecb_output) \
    net_obfuscate((network_pkt), (privacy_key), (pecb_input), (pecb_output))

/**
    \par Description
    This function creaets input bytes to be used for PECB.

    \param [in] iv_index
           Network IV Index
    \param [in] network_pkt
           Network Packet
    \param [out] pecb_input
           Buffer where PECB input bytes to be prepared
*/
void net_create_pecb_input
(
    /* IN */   UINT32   iv_index,
    /* IN */   UINT8*   network_pkt,
    /* OUT */  UINT8*   pecb_input
);

/**
    \par Description
    This function sends data over a network interface.

    \param [in] hdr
           Network Header for the transmit packet
    \param [in] subnet_handle
           Handle identifying associated subnet on which the packet to be transmitted
    \param [in] buffer
           Outgoing Data Packet
    \param [in] is_relay
           Flag
           \ref MS_TRUE : If the packet to be tagged as relay
           \ref MS_FALSE: Otherwise
*/
API_RESULT net_pkt_send
(
    /* IN */ MS_NET_HEADER*     hdr,
    /* IN */ MS_SUBNET_HANDLE   subnet_handle,
    /* IN */ MS_BUFFER*         buffer,
    /* IN */ UINT8              is_relay,
    /* IN */ UINT8              is_seg
);

/* netif interfaces */
API_RESULT netif_init (void);
API_RESULT netif_deinit (void);
API_RESULT netif_send (NETIF_PKT_T type, MS_NET_ADDR d_addr, UCHAR* pdata, UINT16 pdatalen,UINT8 unsegment_flag,UINT8 tx_flag);

/* Network Tx Queue routines */
void net_tx_queue_init(void);
API_RESULT net_tx_enqueue
(
    /* IN */ NETIF_PKT_T            type,
    /* IN */ MS_NET_ADDR            d_addr,
    /* IN */ UINT8*                 buffer,
    /* IN */ UINT16                 buffer_len,
    /* IN */ UINT8                  is_seg
);
API_RESULT net_trigger_tx(void);
void net_tx_timeout_handler(void* args, UINT16 size);

#ifdef MS_PROXY_SERVER
/* GATT Proxy Server Related defines */
#define net_proxy_server_add_to_list(h,a,c,f)     \
    net_proxy_server_filter_op              \
    (                                       \
                                            (h),                                \
                                            MS_PROXY_ADD_TO_FILTER_OPCODE,    \
                                            (a),                                \
                                            (c),                                \
                                            (f)                                 \
    );

#define net_proxy_server_del_from_list(h,a,c,f)   \
    net_proxy_server_filter_op              \
    (                                       \
                                            (h),                                \
                                            MS_PROXY_REM_FROM_FILTER_OPCODE,  \
                                            (a),                                \
                                            (c),                                 \
                                            (f)                                 \
    );
#endif /* MS_PROXY_SERVER */

/* TODO: Protect by Proxy Feature */
/* Init functons */
API_RESULT net_proxy_init(void);

void net_proxy_iface_up  (NETIF_HANDLE* handle, UCHAR role);
void net_proxy_iface_down(NETIF_HANDLE* handle);

/**
    Interface for Network layer to inform Proxy of the received
    Packets which are intended for the Proxy[Configuration Messages].

    \param net_hdr           Network Header.
    \param netif_handle      Network Interface Handle.
    \param subnet_handle     Associated Subnet Handle.
    \param data_param        Data associated with the event if any or NULL.
    \param data_len          Size of the event data. 0 if event data is NULL.
*/
void net_proxy_recv
(
    MS_NET_HEADER*     net_hdr,
    NETIF_HANDLE*      handle,
    MS_SUBNET_HANDLE   subnet_handle,
    UCHAR*             data_param,
    UINT16             data_len
);

/**
    \brief Send a Proxy PDU

    \par Description
    This routine sends a PDU from the Mesh stack to over the Proxy
    indicated by the Proxy Type.

    \param [in] subnet_handle
           subnet_handle on which PDU is to be sent.

    \param [in] pdu
           PDU data to be sent.

    \param [in] pdu_len
           length of PDU data to be sent.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT net_proxy_send
(
    /* IN */ MS_SUBNET_HANDLE   subnet_handle,
    /* IN */ UCHAR*             pdu,
    /* IN */ UINT16             pdu_len
);

#ifdef MS_PROXY_SERVER
/* Interface to set, and get current proxy filter types */
API_RESULT net_proxy_server_set_filter
(
    /* IN */ NETIF_HANDLE*       handle,
    /* IN */ PROXY_FILTER_TYPE   type
);

/* Interface to ADD, Remove to Filter List */
API_RESULT net_proxy_server_filter_op
(
    /* IN */ NETIF_HANDLE*   handle,
    /* IN */ UCHAR           opcode,
    /* IN */ UCHAR*          pdu,
    /* IN */ UINT16          pdu_len,
    /* IN */ UCHAR           proxy_fitlter_flg
);

/* Interface to send List Filter Status */
void net_proxy_send_filter_status
(
    NETIF_HANDLE*        handle,
    MS_SUBNET_HANDLE     subnet_handle
);

API_RESULT net_proxy_process_first_pkt
(
    /* IN */ NETIF_HANDLE*    handle,
    /* IN */ PROXY_ADDR       src_addr
);
#endif /* MS_PROXY_SERVER */

API_RESULT net_proxy_filter_check_forwarding
(
    NETIF_HANDLE*        handle,
    NETIF_PKT_T          pkt_sub_type,
    PROXY_ADDR           d_addr
);

void net_handle_secure_beacon(UCHAR* pdata, UINT16 pdatalen);
API_RESULT net_proxy_nodeid_adv(MS_SUBNET_HANDLE handle);
void net_proxy_netid_timeout_handler(void* args, UINT16 size);
void net_proxy_nodeid_timeout_handler(void* args, UINT16 size);

#endif /* _H_NET_INTERNAL_ */

