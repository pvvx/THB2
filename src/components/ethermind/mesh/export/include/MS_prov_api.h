
/**
    \file MS_prov_api.h

    \brief This file defines the Mesh Provisioning Application Interface - includes
    Data Structures and Methods.
*/

/*
    Copyright (C) 2017. Mindtree Ltd.
    All rights reserved.
*/

#ifndef _H_MS_PROV_API_
#define _H_MS_PROV_API_


/* --------------------------------------------- Header File Inclusion */
/* Bearer Layer */
#include "MS_brr_api.h"

/* --------------------------------------------- Global Definitions */

/**
    \defgroup prov_module PROVISIONING (Mesh Provisioning Layer)
    \{
    This section describes the interfaces & APIs offered by the EtherMind
    Mesh Provisioning module to the Application and other upper
    layers of the stack.
*/

/**
    \defgroup prov_defines Defines
    \{
    Describes defines for the module.
*/

/**
    \defgroup prov_constants Constants
    \{
    Describes Constants defined by the module.
*/

/** Provisiong Roles */
#define PROV_ROLE_DEVICE                    0x01
#define PROV_ROLE_PROVISIONER               0x02

/** Provisioning Bearer Types */
#define PROV_BRR_ADV                        0x01
#define PROV_BRR_GATT                       0x02

/** Provisioning PDU Types */
#define PROV_PDU_TYPE_INVITE                0x00
#define PROV_PDU_TYPE_CAPAB                 0x01
#define PROV_PDU_TYPE_START                 0x02
#define PROV_PDU_TYPE_PUBKEY                0x03
#define PROV_PDU_TYPE_INPUT_CMPLT           0x04
#define PROV_PDU_TYPE_CONF                  0x05
#define PROV_PDU_TYPE_RAND                  0x06
#define PROV_PDU_TYPE_DATA                  0x07
#define PROV_PDU_TYPE_COMPLETE              0x08
#define PROV_PDU_TYPE_FAILED                0x09

/** Provisioning algorithm values */
#define PROV_ALGO_EC_FIPS_P256              0x00

/** Provisioning public key values */
#define PROV_PUBKEY_NO_OOB                  0x00
#define PROV_PUBKEY_OOB                     0x01

/** Provisioning authentication method values */
#define PROV_AUTH_OOB_NONE                  0x00
#define PROV_AUTH_OOB_STATIC                0x01
#define PROV_AUTH_OOB_OUTPUT                0x02
#define PROV_AUTH_OOB_INPUT                 0x03

/** Provisioning Output OOB action values */
#define PROV_OOOB_ACTION_BLINK              0x00
#define PROV_OOOB_ACTION_BEEP               0x01
#define PROV_OOOB_ACTION_VIBRATE            0x02
#define PROV_OOOB_ACTION_NUMERIC            0x03
#define PROV_OOOB_ACTION_ALPHANUMERIC       0x04

/** Provisioning Input OOB action values */
#define PROV_IOOB_ACTION_PUSH               0x00
#define PROV_IOOB_ACTION_TWIST              0x01
#define PROV_IOOB_ACTION_NUMERIC            0x02
#define PROV_IOOB_ACTION_ALPHANUMERIC       0x03

/** Provisioning algorithm support masks */
#define PROV_MASK_ALGO_EC_FIPS_P256         (1 << 0)

/** Provisioning public key supported type masks */
#define PROV_MASK_PUBKEY_OOBINFO            (1 << 0)

/** Provisioning static oob supported type masks */
#define PROV_MASK_STATIC_OOBINFO            (1 << 0)

/** Output OOB actions supported masks */
#define PROV_MASK_OOOB_ACTION_BLINK         (1 << 0)
#define PROV_MASK_OOOB_ACTION_BEEP          (1 << 1)
#define PROV_MASK_OOOB_ACTION_VIBRATE       (1 << 2)
#define PROV_MASK_OOOB_ACTION_NUMERIC       (1 << 3)
#define PROV_MASK_OOOB_ACTION_ALPHANUMERIC  (1 << 4)

/** Input OOB actions supported masks */
#define PROV_MASK_IOOB_ACTION_PUSH          (1 << 0)
#define PROV_MASK_IOOB_ACTION_TWIST         (1 << 1)
#define PROV_MASK_IOOB_ACTION_NUMERIC       (1 << 2)
#define PROV_MASK_IOOB_ACTION_ALPHANUMERIC  (1 << 3)

/**
    \defgroup prov_control Control Constants
    \{
    Control constants defined by the specification.
*/

/** Number of Fragments - Start of New Transaction */
#define PROV_PCF_NUM_FRGMNTS                0x00

/** Control Message */
#define PROV_PCF_CTRL_MSG                   0x01

/** Fragment Index - Continuation of a transaction */
#define PROV_PCF_CONTINU_FRGMNT             0x02

/** Transport Specific Messaging (only used by PB ADV) */
#define PROV_PCF_TX_SPECIFIC                0x03

/** \} */

/**
    \defgroup prov_advtxopcodes ADV Transport Opcodes
    \{
    Specification defined transport Opcodes for PB-ADV bearer.
*/

/* Link Open Request */
#define PROV_PB_ADV_OPEN_REQ                0x00

/* Link Open Confirm */
#define PROV_PB_ADV_OPEN_CNF                0x01

/* Link Close Indication */
#define PROV_PB_ADV_CLOSE_IND               0x02

/** \} */

/**
    \defgroup prov_gatttxopcodes GATT Transport Opcodes
    \{
    Implementation specific transport Opcodes for PB-GATT bearer.
*/

/* Link Open Indication */
#define PROV_PB_GATT_OPEN_IND               0xF1

/* Link Close Indication */
#define PROV_PB_GATT_CLOSE_IND              0xF0

/** \} */

/**
    \defgroup prov_errorcode Error Codes
    \{
    Describes Error Codes defined by the specification.
*/

/** Provisioning Failure Error Codes */
#define PROV_ERR_PROHIBITED                 0x00
#define PROV_ERR_INVALID_PDU                0x01
#define PROV_ERR_INVALID_FORMAT             0x02
#define PROV_ERR_UNEXPECTED_PDU             0x03
#define PROV_ERR_CONFIRMATION_FAILED        0x04
#define PROV_ERR_OUT_OF_RESOURCES           0x05
#define PROV_ERR_DECRYPTION_FAILED          0x06
#define PROV_ERR_UNEXPECTED_ERROR           0x07
#define PROV_ERR_CANNOT_ASSIGN_ADDRESS      0x08

/** Provisioning LinkClose Error codes */
#define PROV_CLOSE_REASON_SUCCESS           0x00
#define PROV_CLOSE_REASON_TIMEOUT           0x01
#define PROV_CLOSE_REASON_FAIL              0x02
/** \} */

/** Provisioning array size requirements */
#define PROV_KEY_NETKEY_SIZE                16
#define PROV_OOB_VALUE_SIZE                 16
#define PROV_URI_HASH_SIZE                  4

/** Provisioning OOB type masks for ADV data */
#define PROV_OOB_TYPE_OTHER                 (1 << 0)
#define PROV_OOB_TYPE_URI                   (1 << 1)
#define PROV_OOB_TYPE_2DMRC                 (1 << 2)
#define PROV_OOB_TYPE_BARCODE               (1 << 3)
#define PROV_OOB_TYPE_NFC                   (1 << 4)
#define PROV_OOB_TYPE_NUMBER                (1 << 5)
#define PROV_OOB_TYPE_STRING                (1 << 6)
#define PROV_OOB_TYPE_ONBOX                 (1 << 11)
#define PROV_OOB_TYPE_INSIDEBOX             (1 << 12)
#define PROV_OOB_TYPE_ONPIECEOFPAPER        (1 << 13)
#define PROV_OOB_TYPE_INSIDEMANUAL          (1 << 14)
#define PROV_OOB_TYPE_ONDEVICE              (1 << 15)

/** Invalid Provisioning Handle */
#define PROV_HANDLE_INVALID                 0xFF

/** \} */

/**
    \defgroup prov_events Events
    \{
    This section lists the Asynchronous Events notified to Application by the
    Module.
*/

/**
    This event indicates the availability of an unprovisioned device beacon,
    with the following values as parameters in the
    \ref PROV_UI_NOTIFY_CB callback.

    \param [in] phandle Pointer to the Provisioning context handle
    \param [in] event_type \ref PROV_EVT_UNPROVISIONED_BEACON.
    \param [in] event_result  \ref API_SUCCESS.
    \param [in] event_data Pointer to the array with the UUID of the device.
    \param [in] event_datalen MS_DEVICE_UUID_SIZE

    \note This event is received by the Provisioner application. On reception of
    this event, the application shall make use of the MS_prov_bind() to initiate
    the provisioning procedure.

    \return \ref API_SUCCESS (always)
*/
#define PROV_EVT_UNPROVISIONED_BEACON           0x01

/**
    This event indicates that the provisioning procedure capability exchange is
    complete, with the following values as parameters in the
    \ref PROV_UI_NOTIFY_CB callback.

    \param [in] phandle Pointer to the Provisioning context handle
    \param [in] event_type \ref PROV_EVT_PROVISIONING_SETUP.
    \param [in] event_result  \ref API_SUCCESS on successful procedure completion, else
    an Error Code.
    \param [in] event_data When local provisioner, this contains peer device
    capabilities and when local device, this contains the attention timeout
    value.
    \param [in] event_datalen When local provisioner, sizeof(\ref
    PROV_CAPABILITIES_S) and when local device, sizeof(UINT32).

    \note When local provisioner, the appliation shall select the required
    capability from the received capabilities and choose to start the procedure
    by calling \ref MS_prov_start().

    \return \ref API_SUCCESS (always)
*/
#define PROV_EVT_PROVISIONING_SETUP             0x02

/**
    This event indicates to the application the OOB random data that is to be
    displayed on the UI via the \ref PROV_UI_NOTIFY_CB callback.

    \param [in] phandle Pointer to the Provisioning context handle
    \param [in] event_type \ref PROV_EVT_OOB_DISPLAY.
    \param [in] event_result  \ref API_SUCCESS on successful procedure completion, else
    an Error Code.
    \param [in] event_data Pointer to OOB information as in \ref PROV_OOB_TYPE_S.
    \param [in] event_datalen sizeof (\ref PROV_OOB_TYPE_S).

    \return \ref API_SUCCESS (always)
*/
#define PROV_EVT_OOB_DISPLAY                    0x03

/**
    This event indicates to the application requesting for OOB random data that
    is to be used in the procedure via the \ref PROV_UI_NOTIFY_CB callback.

    \param [in] phandle Pointer to the Provisioning context handle
    \param [in] event_type \ref PROV_EVT_OOB_ENTRY.
    \param [in] event_result  \ref API_SUCCESS on successful procedure completion, else
    an Error Code.
    \param [in] event_data \ref Pointer to OOB information as in \ref PROV_OOB_TYPE_S.
    \param [in] event_datalen sizeof (\ref PROV_OOB_TYPE_S).

    \return \ref API_SUCCESS (always)
*/
#define PROV_EVT_OOB_ENTRY                      0x04

/**
    This event indicates to the application that the peer device has completed the
    Input of OOB when this capability is negotiated via the
    \ref PROV_UI_NOTIFY_CB callback.

    \param [in] phandle Pointer to the Provisioning context handle
    \param [in] event_type \ref PROV_EVT_DEVINPUT_COMPLETE.
    \param [in] event_result  \ref API_SUCCESS on successful procedure completion, else
    an Error Code.
    \param [in] event_data NULL.
    \param [in] event_datalen 0.

    \note This event is generated only for the provisioner application.

    \return \ref API_SUCCESS (always)
*/
#define PROV_EVT_DEVINPUT_COMPLETE              0x05

/**
    This event indicates to the application requesting for Provisional data to be
    sent to the peer device via the \ref PROV_UI_NOTIFY_CB callback.

    \param [in] phandle Pointer to the Provisioning context handle
    \param [in] event_type \ref PROV_EVT_PROVDATA_INFO_REQ.
    \param [in] event_result  \ref API_SUCCESS on successful procedure completion, else
    an Error Code.
    \param [in] event_data NULL.
    \param [in] event_datalen 0.

    \note This event is generated only for the provisioner application.

    \return \ref API_SUCCESS (always)
*/
#define PROV_EVT_PROVDATA_INFO_REQ              0x06

/**
    This event indicates to the application the Provisional data received
    from the Provisioner via the \ref PROV_UI_NOTIFY_CB callback.

    \param [in] phandle Pointer to the Provisioning context handle
    \param [in] event_type \ref PROV_EVT_PROVDATA_INFO.
    \param [in] event_result  \ref API_SUCCESS on successful procedure completion, else
    an Error Code.
    \param [in] event_data Pointer to the provisioning data \ref PROV_DATA_S.
    \param [in] event_datalen sizeof(\ref PROV_DATA_S).

    \note This event is generated only for the device application.

    \return \ref API_SUCCESS (always)
*/
#define PROV_EVT_PROVDATA_INFO                  0x07

/**
    This event indicates to the application that the provisioning procedure
    is complete via the \ref PROV_UI_NOTIFY_CB callback.

    \param [in] phandle Pointer to the Provisioning context handle
    \param [in] event_type \ref PROV_EVT_PROVISIONING_COMPLETE.
    \param [in] event_result  \ref API_SUCCESS on successful procedure completion, else
    an Error Code.
    \param [in] event_data NULL.
    \param [in] event_datalen 0.

    \return \ref API_SUCCESS (always)
*/
#define PROV_EVT_PROVISIONING_COMPLETE          0x08

/** \} */

/** \} */

/* --------------------------------------------- Structures/Data Types */

/**
    \addtogroup prov_defines Defines
    \{
*/

/**
    \defgroup prov_structures Structures
    \{
*/

/** Role of the device */
typedef UCHAR       PROV_ROLE;

/** Bearer for the provisioning session */
typedef UCHAR       PROV_BRR;

/** Handle to reference the active provisioning context */
typedef UCHAR       PROV_HANDLE;

/** Device Information used for Provisioning */
typedef struct _PROV_DEVICE_S
{
    /**
        Device UUID -
        Used in unprovisioned device beacon and Provisioning Invite
    */
    UCHAR uuid[MS_DEVICE_UUID_SIZE];

    /** OOB Information */
    UINT16 oob;

    /** URI if any, to be given in encoded form */
    MS_BUFFER* uri;

} PROV_DEVICE_S;

/** OOB type for provisioning */
typedef struct _PROV_OOB_TYPE_S
{
    /** OOB Action information */
    UINT16 action;

    /** OOB Size information */
    UCHAR size;

} PROV_OOB_TYPE_S;

/** Device capabilities used for Provisioning */
typedef struct _PROV_CAPABILITIES_S
{
    /** Number of Elements */
    UCHAR num_elements;

    /** Supported algorithms */
    UINT16 supp_algorithms;

    /** Public key type */
    UCHAR supp_pubkey;

    /** Static OOB type */
    UCHAR supp_soob;

    /** Output OOB information */
    PROV_OOB_TYPE_S ooob;

    /** Input OOB information */
    PROV_OOB_TYPE_S ioob;

} PROV_CAPABILITIES_S;

/** Provisioning method information */
typedef struct _PROV_METHOD_S
{
    /** Algorithm selected */
    UCHAR algorithm;

    /** Public key usage */
    UCHAR pubkey;

    /** Authentication type */
    UCHAR auth;

    /** OOB type */
    PROV_OOB_TYPE_S oob;

} PROV_METHOD_S;

/** Data exchanged during Provisiong procedure */
typedef struct _PROV_DATA_S
{
    /** NetKey */
    UCHAR netkey[PROV_KEY_NETKEY_SIZE];

    /** Index of the NetKey */
    UINT16 keyid;

    /**
        Flags bitmask
        bit 0: Key Refresh Flag.
              0: Not-In-Phase2
              1: In-Phase2
        bit 1: IV Update Flag
              0: Normal operation
              1: IV Update active

        bits 2-7: RFU
    */
    UCHAR flags;

    /** Current value of the IV index */
    UINT32 ivindex;

    /** Unicast address of the primary element */
    UINT16 uaddr;

} PROV_DATA_S;

/** \} */
/** \} */

/**
    \defgroup prov_cb Application Callback
    \{
    This Section Describes the module Notification Callback interface offered
    to the application
*/

/**
    Provisioning Application Asynchronous Notification Callback.

    Provisioning calls the registered callback to indicate events occurred to the
    application.

    \param phandle Handle that identifies the provisioning context.
    \param event_type Event type from any of the events in \ref prov_events.
    \param event_result API_SUCCESS or any error code.
    \param event_data Data associated with the event if any or NULL.
    \param event_datalen Size of the event data. 0 if event data is NULL.
*/
typedef API_RESULT (* PROV_UI_NOTIFY_CB)
(
    PROV_HANDLE* phandle,
    UCHAR         event_type,
    API_RESULT    event_result,
    void*         event_data,
    UINT16        event_datalen
);

/** \} */

/* --------------------------------------------- Macros */
/**
    \defgroup prov_marcos Utility Macros
    \{
    Initialization and other Utility Macros offered by the module.
*/

/**
    \brief Setup the Device for provisioning

    \par Description This function configures the application as an Unprovisioned
    device over Advertising channel bearer and sets it ready for provisioning.

    \param [in]  pdev  Pointer to the device strcuture \ref PROV_DEVICE_S.
    \param [in]  tmo   The setup timeout value
    \param [out] phdl  The handle to the context setup on successful allocation

    \return API_SUCCESS or Error Code on failure
*/
#define MS_prov_setup_device_pbadv(pdev, tmo, phdl) \
    MS_prov_setup (PROV_ROLE_DEVICE, PROV_BRR_ADV, (pdev), (tmo), (phdl))

/**
    \brief Setup the Provisioner for provisioning

    \par Description This function configures the application as a Provisioner
    over Advertising channel bearer and sets it ready for provisioning.

    \param [in]  tmo   The setup timeout value
    \param [out] phdl  The handle to the context setup on successful allocation

    \return API_SUCCESS or Error Code on failure
*/
#define MS_prov_setup_provisioner_pbadv(tmo, phdl) \
    MS_prov_setup (PROV_ROLE_PROVISIONER, PROV_BRR_ADV, NULL, (tmo), (phdl))

/**
    \brief Setup the device for provisioning

    \par Description This function configures the application as an Unprovisioned
    device over GATT channel bearer and sets it ready for provisioning.

    \param [in]  pdev  Pointer to the device strcuture \ref PROV_DEVICE_S.
    \param [in]  tmo   The setup timeout value
    \param [out] phdl  The handle to the context setup on successful allocation

    \return API_SUCCESS or Error Code on failure
*/
#define MS_prov_setup_device_pbgatt(pdev, tmo, phdl) \
    MS_prov_setup (PROV_ROLE_DEVICE, PROV_BRR_GATT, (pdev), (tmo), (phdl))

/**
    \brief Setup the Provisioner for provisioning

    \par Description This function configures the application as a Provisioner
    over GATT channel bearer and sets it ready for provisioning.

    \param [in]  tmo   The setup timeout value
    \param [out] phdl  The handle to the context setup on successful allocation

    \return API_SUCCESS or Error Code on failure
*/
#define MS_prov_setup_provisioner_pbgatt(tmo, phdl) \
    MS_prov_setup (PROV_ROLE_PROVISIONER, PROV_BRR_GATT, NULL, (tmo), (phdl))

/**
    \brief Start Provisioning on the select set of device capabilities.

    \par Description This function is used by the provisioner application to start
    the procedure using the selected set of capabilities of the unprovisioned
    device.

    \param [in] phandle Provisioning context to be used.
    \param [in] pmethod Pointer to the structure with selected method.

    \note This API will be used by the provisioner only, in response to the
    \ref PROV_EVT_PROVISIONING_SETUP event

    \return API_SUCCESS or Error Code on failure
*/
#define MS_prov_start(phandle, pmethod) \
    MS_prov_send_pdu ((phandle), PROV_PDU_TYPE_START, (pmethod), sizeof(PROV_METHOD_S))

/**
    \brief Input Authentication value from Device/Provisioner application.

    \par Description This function is used to receive the Authval from the
    application and use it for the algorithm. When the application is that of a
    device, it also sends the input complete message to the provisioner in the
    given context.

    \param [in] phandle Provisioning context to be used.
    \param [in] pauth Authentication Value (UINT32 *) or (UCHAR *)
    \param [in] size Size of pauth

    \note This API will be used in response to the
    \ref PROV_EVT_OOB_ENTRY event

    \return API_SUCCESS or Error Code on failure
*/
#define MS_prov_input_authval(phandle, pauth, size) \
    MS_prov_send_pdu ((phandle), PROV_PDU_TYPE_INPUT_CMPLT, (pauth), (size))

/**
    \brief Send provisioning data to the device.

    \par Description This function is used by the provisioning application to send
    the provisioning data to the device after authentication is complete.

    \param [in] phandle Provisioning context to be used.
    \param [in] pdata Pointer to the Provisioning data structre as in \ref
    PROV_DATA_S

    \note This API will be used by the provisioner only, in response to the
    \ref PROV_EVT_PROVDATA_INFO_REQ event

    \return API_SUCCESS or Error Code on failure
*/
#define MS_prov_data(phandle, pdata) \
    MS_prov_send_pdu ((phandle), PROV_PDU_TYPE_DATA, (pdata), sizeof (PROV_DATA_S))

/** \} */

/* --------------------------------------------- External Global Definitions */
/** Global Provisioning Role */
extern UCHAR prov_role;


/* --------------------------------------------- API Declarations */

/**
    \defgroup prvsng_api_defs API Definitions
    \{
    This section describes the EtherMind Mesh Provisioning Layer APIs.
*/

#ifdef __cplusplus
extern "C" {
#endif

/**
    \brief Register provisioning capabilities and callback

    \par Description This function registers the provisioning capabilities of the
    application along with the application callback to notify events during the
    provisioning procedure.

    \param [in] pcapab Pointer to the provisioning capabilities structure \ref PROV_CAPABILITIES_S
    \param [in] cb Application callback function pointer

    \return API_SUCCESS or Error Code on failure
*/
API_RESULT MS_prov_register
(
    /* IN */ PROV_CAPABILITIES_S* pcapab,
    /* IN */ PROV_UI_NOTIFY_CB     cb
);

API_RESULT MS_prov_stop_interleave_timer
(
    void
);


/**
    \brief Setup the device for provisioning

    \par Description This function configures the device to get in a provisionable
    state by specifying the role, bearer and creating a context.

    \param [in] role Provisioniong role to be setup - Device or Provisioner.
    \param [in] bearer Provisioning bearer to be setup - PB-ADV or PB-GATT
    \param [in] pdevice Pointer to the device strcuture \ref PROV_DEVICE_S
    containing the UUID to be beaconed. This parameter is used only when the
    role is PROV_ROLE_DEVICE and ignored otherwise.
    \param [in] timeout The time period for which the setup shall be active.

    \return API_SUCCESS or Error Code on failure
*/
API_RESULT MS_prov_setup
(
    /* IN */  PROV_ROLE       role,
    /* IN */  PROV_BRR        bearer,
    /* IN */  PROV_DEVICE_S* pdevice,
    /* IN */  UINT16          gatt_timeout,
    /* IN */  UINT16          adv_timeout
);

/**
    \brief Bind to the peer device for provisioning

    \par Description This function establishes a provisioning link with the peer device
    and exchanges the capabilities for provisioning.

    \param [in] bearer Provisioning bearer on which to be bound - PB-ADV or PB-GATT
    \param [in] pdevice Pointer to the device strcuture \ref PROV_DEVICE_S.
    \param [in] attention The attention duration in seconds to be configured by the
    device. This parameter is dont care if the role is PROV_ROLE_DEVICE.
    \param [out] phandle The handle to the context setup on successful allocation.

    \note This API is for use by the Provisioner application only upon
    reception of an Unprovisioned Device Beacon.

    \return API_SUCCESS or Error Code on failure
*/
API_RESULT MS_prov_bind
(
    /* IN */  PROV_BRR        bearer,
    /* IN */  PROV_DEVICE_S* pdevice,
    /* IN */  UCHAR           attention,
    /* OUT */ PROV_HANDLE*    phandle
);

/**
    \brief Send provisioning PDUs to the peer.

    \par Description This function is used by the provisioning application to send
    the provisioning PDUs to the peer device during the procedure.

    \param [in] phandle Provisioning context to be used.
    \param [in] pdu Following PDU types are handled -
                    PROV_PDU_TYPE_START
                    PROV_PDU_TYPE_INPUT_CMPLT
                    PROV_PDU_TYPE_DATA
    \param [in] pdata Pointer to the data corresponding to the above PDUs
    \param [in] datalen Size of the pdata

    \return API_SUCCESS or Error Code on failure
*/
API_RESULT MS_prov_send_pdu
(
    /* IN */ PROV_HANDLE* phandle,
    /* IN */ UCHAR         pdu,
    /* IN */ void*         pdata,
    /* IN */ UINT16        datalen
);

/**
    \brief Set the display authval.

    \par Description This function shall be used by the provisioning application
    to set the authval being displayed to the user on receiving \ref
    PROV_EVT_OOB_DISPLAY event with the respective OOB Action and Size.

    \param [in] phandle Provisioning context to be used.
    \param [in] pdata Pointer to the Authval (UINT32 *) or (UCHAR *)
    \param [in] datalen Size of the pdata

    \return API_SUCCESS or Error Code on failure
*/
API_RESULT MS_prov_set_authval
(
    /* IN */ PROV_HANDLE* phandle,
    /* IN */ void*         pdata,
    /* IN */ UINT16        datalen
);

/**
    \brief Abort the provisioning procedure

    \par Description
    This API can be used by the application to abort the ongoing provisioning
    procedure. This routine closes the provisioning link with the reason as
    specified.

    \param [in] phandle Provisioning context to be used.
    \param [in] reason Reason for termination.

    \return API_SUCCESS or Error Code on failure
*/
API_RESULT MS_prov_abort
(
    PROV_HANDLE* phandle,
    UCHAR reason
);

/**
    \brief Utility API to fetch current ECDH Public Key to be used for
           Provisioning

    \par Description
    This API can be used by the application to fetch the current ECDH P256
    Public Key which is to be used for the Provisioning Procedure.

    \param [out] public_key to a pointer of \ref UCHAR array of length
                 \ref PROV_PUBKEY_SIZE

    \return API_SUCCESS or Error Code on failure
*/
API_RESULT MS_prov_get_local_public_key
(
    /* OUT */ UCHAR*   public_key
);

#ifdef __cplusplus
};
#endif

/** \} */

/** \} */

#endif /* _H_MS_PROV_API_ */

