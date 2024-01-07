
/**
    \file MS_brr_api.h

    \brief This file defines the Mesh Bearer Application Interface - includes
    Data Structures and Methods.
*/

/*
    Copyright (C) 2017. Mindtree Ltd.
    All rights reserved.
*/

#ifndef _H_MS_BRR_API_
#define _H_MS_BRR_API_


/* --------------------------------------------- Header File Inclusion */
#include "MS_common.h"

/* --------------------------------------------- Global Definitions */

/**
    \defgroup brr_module BEARER (Mesh Bearer Layer)
    \{
    This section describes the interfaces & APIs offered by the EtherMind
    Mesh Bearer (BEARER) module to the Application and other upper
    layers of the stack.
*/

/**
    \defgroup brr_defines Defines
    \{
    Describes defines for the module.
*/

/**
    \defgroup brr_constants Constants
    \{
    Describes Constants defined by the module.
*/

/** Invalid Bearer handle identifier */
#define BRR_HANDLE_INVALID                          0xFF

/** Bearer Interface events to the above layer */
#define BRR_IFACE_DOWN                              0x00
#define BRR_IFACE_UP                                0x01
#define BRR_IFACE_DATA                              0x02
#define BRR_IFACE_PROXY_DATA                        0x03

/** Bearer beacon types - Connectable (Active) and Non-Connectable (Passive) */
#define BRR_BCON_PASSIVE                            0x00
#define BRR_BCON_ACTIVE                             0x01

/** Bearer Beacon Operations - Broadcast/Observe */
#define BRR_BROADCAST                               0x00
#define BRR_OBSERVE                                 0x01

/** Bearer Beacon Actions - Enable/Disable */
#define BRR_DISABLE                                 0x00
#define BRR_ENABLE                                  0x01

/** Maximum PDU size for data received over bearer */
#define BRR_MAX_PDU_SIZE                            65

/** Bearer Server Client Roles */
#define BRR_CLIENT_ROLE                             0x00
#define BRR_SERVER_ROLE                             0x01
#define BRR_INVALID_ROLE                            0xFF

/** Bearer Transmit and Receive operation modes */
#define BRR_TX                                      0x01
#define BRR_RX                                      0x02

/** \} */

/** \} */

/**
    \defgroup brr_events Events
    \{
    This section lists the Asynchronous Events notified to Application by the
    Module.
*/

/**
    \defgroup brr_marcos Utility Macros
    \{
    Initialization and other Utility Macros offered by the module.
*/

/** \} */
/** \} */

/* --------------------------------------------- Data Types/ Structures */

/**
    \addtogroup brr_defines Defines
    \{
*/
/** Bearer handle identifier */
typedef UCHAR           BRR_HANDLE;

/** Bearer Type definitions */
typedef enum _BRR_TYPE
{
    /** Beacon Bearer */
    BRR_TYPE_BCON,

    /** Advertising Bearer */
    BRR_TYPE_ADV,

    /** Provisioning Advertising Bearer */
    BRR_TYPE_PB_ADV,

    /** GATT Bearer */
    BRR_TYPE_GATT,

    /** Provisioning GATT Bearer */
    BRR_TYPE_PB_GATT,

    /** Number of bearers supported */
    BRR_COUNT

} BRR_TYPE;

/* GATT Bearer Message Type Masks */
#define BRR_SUBTYPE_GATT_T_MASK_BIT_OFFSET      6
#define BRR_SUBTYPE_GATT_T_MASK                 (0xC0)
#define BRR_SUBTYPE_GATT_NETWORK_T_MASK         ((MESH_GATT_TYPE_NETWORK << BRR_SUBTYPE_GATT_T_MASK_BIT_OFFSET) & (BRR_SUBTYPE_GATT_T_MASK))
#define BRR_SUBTYPE_GATT_BEACON_T_MASK          ((MESH_GATT_TYPE_BEACON << BRR_SUBTYPE_GATT_T_MASK_BIT_OFFSET) & (BRR_SUBTYPE_GATT_T_MASK))
#define BRR_SUBTYPE_GATT_PROXY_T_MASK           ((MESH_GATT_TYPE_PROXY << BRR_SUBTYPE_GATT_T_MASK_BIT_OFFSET) & (BRR_SUBTYPE_GATT_T_MASK))
#define BRR_SUBTYPE_GATT_PROV_T_MASK            ((MESH_GATT_TYPE_PROV << BRR_SUBTYPE_GATT_T_MASK_BIT_OFFSET) & (BRR_SUBTYPE_GATT_T_MASK))

/** Bearer Beacon type definitions */
typedef enum _BRR_BCON_TYPE
{
    /* Unprovisioned Device Beacon */
    BRR_BCON_TYPE_UNPROV_DEVICE,

    /* Secure Network Beacon */
    BRR_BCON_TYPE_SECURE_NET,

    /* Proxy beacon with Network ID */
    BRR_BCON_TYPE_PROXY_NETID,

    /* Proxy beacon with Node Identity */
    BRR_BCON_TYPE_PROXY_NODEID,

    /** Number of Beacon types */
    BRR_BCON_COUNT

} BRR_BCON_TTYPE;

/**
    \addtogroup brr_structures Structures
    \{
*/

/** Bearer information to register */
typedef struct _BRR_BEARER_INFO
{
    /** Bearer Information */
    MS_BUFFER* binfo;

    /** Data Send routine */
    API_RESULT (*bearer_send)(BRR_HANDLE*, UCHAR, void*, UINT16);

    /** Data Receive routine */
    void       (*bearer_recv)(BRR_HANDLE*, UCHAR*, UINT16, MS_BUFFER* info);

    /** Bearer Sleep Interface */
    void       (*bearer_sleep)(BRR_HANDLE*);

    /** Bearer Wakeup Interface */
    void       (*bearer_wakeup)(BRR_HANDLE*, UINT8 mode);

} BRR_BEARER_INFO;

/** Bearer Beacon type data structure */
typedef struct _BRR_BEACON_INFO
{
    /**
        Beacon Action
        - Lower Nibble:
         > BRR_OBSERVE
         > BRR_BROADCAST

        - Higher Nibble:
         > BRR_ENABLE
         > BRR_DISABLE
    */
    UCHAR action;

    /**
        Beacon type
        - Lower Nibble:
         > BRR_BCON_PASSIVE - Non Connectable beacon
         > BRR_BCON_ACTIVE  - Connectable beacon

        - Higher Nibble (Valid only when Passive)
         > BRR_BCON_TYPE_UNPROV_DEVICE
         > BRR_BCON_TYPE_SECURE_NET
    */
    UCHAR type;

    /** Beacon Broadcast Data */
    UCHAR* bcon_data;

    /** Beacon Broadcast Data length */
    UINT16 bcon_datalen;

    /** URI information in case of Unprovisioned Beacons */
    MS_BUFFER* uri;

} BRR_BEACON_INFO;

/** Bearer GATT Channel informartion related data structure */
typedef struct _BRR_BEARER_CH_INFO
{
    /** Identifies the MTU for the Bearer Channel */
    UINT16   mtu;

    /** Identifies the role for the Bearer channel */
    UCHAR   role;

} BRR_BEARER_CH_INFO;

/** \} */
/** \} */

/**
    \defgroup brr_cb Application Callback
    \{
    This Section Describes the module Notification Callback interface offered
    to the application
*/
/**
    BEARER Application Asynchronous Notification Callback.

    BEARER calls the registered callback to indicate events occurred to the
    application.

    \param brr_type Bearer Type.
    \param data Data associated with the event if any or NULL.
    \param data_len Size of the event data. 0 if event data is NULL.
*/
typedef API_RESULT (*BRR_NTF_CB)
(
    BRR_HANDLE*     brr_handle,
    UCHAR           brr_event,
    UCHAR*          data_param,
    UINT16          data_len
) DECL_REENTRANT;

/**
    BEARER Application Asynchronous Notification Callback for Beacons.

    Application registers callback for beacon notification with bearer.

    \param brr_type Bearer Type.
    \param data Data associated with the event if any or NULL.
    \param data_len Size of the event data. 0 if event data is NULL.
*/
typedef void (*BRR_BCON_CB)
(
    UCHAR*          data_param,
    UINT16          data_len
) DECL_REENTRANT;
/** \} */

/**
    \addtogroup brr_defines Defines
    \{
*/

/**
    \addtogroup brr_structures Structures
    \{
*/


/** \} */

/** \} */

/* --------------------------------------------- Function */

/**
    \defgroup brr_api_defs API Definitions
    \{
    This section describes the EtherMind Mesh Bearer Layer APIs.
*/
#ifdef __cplusplus
extern "C" {
#endif

/**
    \brief Register Interface with Bearer Layer

    \par Description
    This routine registers interface with the Bearer Layer.
    Bearer Layer supports single Application, hence this rouine shall be called once.

    \param [in] brr_type
           Bearer Type

    \param [in] brr_cb
           Details describing Application Notification Callback

    \return API_SUCCESS or an error code indicating reason for failure

*/
API_RESULT MS_brr_register
(
    /* IN */ BRR_TYPE        brr_type,
    /* IN */ BRR_NTF_CB      brr_cb
);

/**
    \brief Register Beacon Interface with Bearer Layer

    \par Description
    This routine registers interface with the Bearer Layer to process Beacons.
    Bearer Layer supports single Application, hence this rouine shall be called once.
    1
    \param [in] bcon_type
           Beacon type - Unprovisioned Device or Secure Network.

    \param [in] bcon_handler
           Callback handler to be registered for the given beacon type.

    \return API_SUCCESS or an error code indicating reason for failure

*/
API_RESULT MS_brr_register_beacon_handler
(
    /* IN */ UCHAR   bcon_type,
    /* IN */ void    (*bcon_handler) (UCHAR* data, UINT16 datalen)
);

/**
    \brief Add a bearer to Bearer Layer

    \par Description
    This routine adds a bearer that is setup by the application
    for use by the Mesh Stack. Bearer Layer supports single Application,
    hence this rouine shall be called once.

    \param [in] brr_type
           Bearer Type

    \param [in] brr_info
           Details describing the Bearer being added

    \param [out] brr_handle
           Handle to the bearer that is added. Used in data APIs.

    \return API_SUCCESS or an error code indicating reason for failure

*/
API_RESULT MS_brr_add_bearer
(
    /* IN */  BRR_TYPE            brr_type,
    /* IN */  BRR_BEARER_INFO*    brr_info,
    /* OUT */ BRR_HANDLE*         brr_handle
);

/**
    \brief Remove a bearer from Bearer Layer

    \par Description
    This routine removes a bearer from the Mesh Stack. Bearer Layer
    supports single Application, hence this rouine shall be called once.

    \param [in] brr_type
           Bearer Type

    \param [out] brr_handle
           Handle to the bearer that is added. Used in data APIs.

    \return API_SUCCESS or an error code indicating reason for failure

*/
API_RESULT MS_brr_remove_bearer
(
    /* IN */ BRR_TYPE            brr_type,
    /* IN */ BRR_HANDLE*         brr_handle
);

/**
    \brief Observe ON/OFF for the beacon type

    \par Description
    This routine sends enables/disables the observation procedure
    for the given beacon type.

    \param [in] bcon_type
           Type of beacon to observe - Active/Passive.

    \param [in] enable
           Enable or Disable the observation procedure.

    \return API_SUCCESS or an error code indicating reason for failure

*/
API_RESULT MS_brr_observe_beacon
(
    /* IN */ UCHAR    bcon_type,
    /* IN */ UCHAR    enable
);


/**
    \brief API to send Unprovisioned Device Beacon

    \par Description
    This routine sends Unprovisioned Device Beacon

    \param [in] type
           Active or Passive Broadcast type.

    \param [in] dev_uuid
           Device UUID uniquely identifying this device.

    \param [in] oob_info
           OOB Information

    \param [in] uri
           Optional Parameter. NULL if not present.
           Points to the length and payload pointer of the URI string to be
           advertised interleaving with the unprovisioned beacon.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_brr_bcast_unprovisioned_beacon
(
    /* IN */ UCHAR       type,
    /* IN */ UCHAR*      dev_uuid,
    /* IN */ UINT16      oob_info,
    /* IN */ MS_BUFFER* uri
);


/**
    \brief API to broadcast a beacon

    \par Description
    This routine sends the beacon of given type on Adv and GATT bearers

    \param [in] type
           The type of beacon

    \param [in] packet
           Beacon data

    \param [in] length
           Beacon data length

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_brr_broadcast_beacon
(
    /* IN */ UCHAR     type,
    /* IN */ UCHAR*    packet,
    /* IN */ UINT16    length
);

/**
    \brief API to send Proxy Device ADV

    \par Description
    This routine sends Proxy Device ADV

    \param [in] type
           Proxy ADV Type:
           0x00 - Network ID
           0x01 - Node Identity

    \param [in] data
           Data to be advertised by Proxy.
           If the "type" is:
           0x00 - Network ID    - 8 Bytes of Network ID
           0x01 - Node Identity - 8 Bytes Hash, 8 Bytes Random num

    \param [in] datalen
           Length of the data to be advertised by Proxy.
           If the "type" is:
           0x00 - Network ID    - 8 Bytes of Network ID
           0x01 - Node Identity - 8 Bytes Hash, 8 Bytes Random num

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_brr_start_proxy_adv
(
    /* IN */ UCHAR      type,
    /* IN */ UCHAR*     data,
    /* IN */ UINT16     datalen
);

/**
    \brief API to disable broadcast of given beacon type

    \par Description
    This routine stops advertising of given beacon type.

    \param [in] bcon
           The Bearer Beacon (Unprovisioned/Secure Network).

    \param [in] type
           The type of braodcast beacon (Active/Passive).

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_brr_bcast_end
(
    /* IN */ UCHAR    bcon,
    /* IN */ UCHAR    type
);


/**
    \brief Send a bearer PDU

    \par Description
    This routine sends a PDU from the Mesh stack to over the bearer
    indicated by the bearer handle.

    \param [in] brr_handle
           Bearer handle on which PDU is to be sent.

    \param [in] brr_type
           Type of Bearer as in \ref BRR_TYPE.

    \param [in] buffer
           PDU data to be sent.

    \return API_SUCCESS or an error code indicating reason for failure

*/
API_RESULT MS_brr_send_pdu
(
    /* IN */ BRR_HANDLE*     brr_handle,
    /* IN */ BRR_TYPE        brr_type,
    /* IN */ MS_BUFFER*      buffer
);

/**
    \brief Get the RSSI of current received packet being processed.

    \par Description
    This routine returns the RSSI value of the received packet in its
    context when called from the Mesh stack.

    \return RSSI value of the current packet in context.

    \note This applies only when the packet is received over ADV bearer

*/
UCHAR MS_brr_get_packet_rssi(void);

/**
    \brief Put the bearer to sleep.

    \par Description
    This routine requests the underlying bearer interface to sleep.
    Default bearer interface is that of advertising bearer.

    \return API_SUCCESS

*/
API_RESULT MS_brr_sleep(void);

/**
    \brief Wakeup the bearer.

    \par Description
    This routine requests the underlying bearer interface to wakeup.
    Default bearer interface is that of advertising bearer.

    \param mode
           Identifies the mode (BRR_TX/BRR_RX) for which bearer is requested
           for wakeup.

    \return API_SUCCESS

*/
API_RESULT MS_brr_wakeup(UINT8 mode);

#ifdef __cplusplus
};
#endif

/** \} */

/** \} */

#endif /* _H_MS_BRR_API_ */

