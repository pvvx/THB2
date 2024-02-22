/**
    @defgroup Multi GAPRole (Multi)
    @brief This module implements the Multi GAP Role
    For a detailed usage section describing how to send these commands and receive events,
    see the <a href="../ble-stack/gaprole.html">GAPRole Section</a> of the
    User's Guide.
    @{
    @file       multi.h
    @brief      Multi layer interface

     SDK_LICENSE

*/

#ifndef MULTI_H
#define MULTI_H

#ifdef __cplusplus
extern "C"
{
#endif

/*  -------------------------------------------------------------------
    INCLUDES
*/
#include "gap.h"
#include "att.h"

/*  -------------------------------------------------------------------
    CONSTANTS
*/

/** @defgroup Multi_Constants Multi GAPRole Constants
    @{
*/
#define   MAX_CONNECTION_NUM          2		///5//
#define   MAX_CONNECTION_SLAVE_NUM   	1		///3// 
#define   MAX_CONNECTION_MASTER_NUM   ( (MAX_CONNECTION_NUM >= MAX_CONNECTION_SLAVE_NUM)?\
                                        (MAX_CONNECTION_NUM - MAX_CONNECTION_SLAVE_NUM):0)
#if( MAX_CONNECTION_NUM < MAX_CONNECTION_SLAVE_NUM )
#error ("max connection num is less than slave num")
#endif
//// Multi-role Event
#define MULTI_SCHEDULE_EVT              0x0001
#define MULTI_ADV_EVENT_DONE_EVT        0x0002
#define MULTI_PERIOD_EVT                0x0004
#define CONN_TIMEOUT_EVT                0X0008

#define MULTI_PERIOD_TIMING             500     //ms

/** @defgroup Multi_Params Multi GAPRole Parameters
    @{
    Parameters set via @ref GAPRole_SetParameter
*/

/**
    @brief This parameter will return GAP Role type (Read-only)

    size: uint8

    range: @ref GAP_Profile_Roles
*/
#define GAPMULTIROLE_PROFILEROLE         0x300

/**
    @brief Identity Resolving Key (Read/Write) Size is uint8[KEYLEN].

    @note If this is set to all 0x00's, the IRK will be randomly generated

    size: uint8[16]

    default: 0x00000000000000000000000000000000

    range: 0x00000000000000000000000000000000 - 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
*/
#define GAPMULTIROLE_IRK                 0x301

/**
    @brief Signature Resolving Key (Read/Write)

    @note If this is set to all 0x00's, the SRK will be randomly generated

    size: uint8[16]

    default: 0x00000000000000000000000000000000

    range: 0x00000000000000000000000000000000 - 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
*/
#define GAPMULTIROLE_SRK                 0x302

/**
    @brief Sign Counter (Read/Write)

    size: uint32

    default: 0x0000

    range: 0x0000 - 0xFFFF
*/
#define GAPMULTIROLE_SIGNCOUNTER         0x303

/**
    @brief Device Address read from the controller (Read-only)

    The BDADDR is read, in increasing order of priortiy, from the info page,
    secondary address from flash, or set from @ref HCI_ReadBDADDRCmd

    size: uint8[6]

    default: BDADDR from info page

    range: 0x000000000000 - 0xFFFFFFFFFFFE
*/
#define GAPMULTIROLE_BD_ADDR             0x304

/**
    @brief Enable/Disable Connectable Advertising (Read/Write)

    @warning @ref GAPMULTIROLE_ADV_NONCONN_ENABLED must be set to FALSE in order to enable this

    size: uint8

    default: TRUE

    range: TRUE (enabled) or FALSE (disabled)
*/
#define GAPMULTIROLE_ADVERT_ENABLED      0x305

/**
    @brief How long to remain off (in sec) after advertising stops before starting again (Read/Write)

    If set to 0, advertising will not start again.

    size: uint16

    default: 30

    range: 0-65535
*/
#define GAPMULTIROLE_ADVERT_OFF_TIME     0x306

/**
    @brief Advertisement data (Read/Write)

    @note The third byte sets limited / general advertising as defined in Vol 3, Part C, section 11.1.3
    of the BT 4.2 Core Spec.

    size: a uint8 array of up to 31 bytes

    default: 02:01:01 (general advertising)
*/
#define GAPMULTIROLE_ADVERT_DATA         0x307

/**
    @brief Scan Response Data (Read/Write)

    @note This should be formatted as define d in Vol 3, Part C, section 11.1.3 of the BT 4.2 Core Spec.

    size: a uint8 array of up to 31 bytes

    default: all 0x00's
*/
#define GAPMULTIROLE_SCAN_RSP_DATA       0x308

/**
    @brief Advertisement Types (Read/Write)

    size: uint8

    default: @ref GAP_ADTYPE_ADV_IND

    range: @ref GAP_Adv_Types
*/
#define GAPMULTIROLE_ADV_EVENT_TYPE      0x309

/**
    @brief Direct Advertisement Type (Read/Write)

    size: uint8

    default: @ref ADDRMODE_PUBLIC

    range: @ref Gap_Addr_Modes
*/
#define GAPMULTIROLE_ADV_DIRECT_TYPE     0x30A

/**
    @brief Direct Advertisement Address (Read/Write)

    size: uint8[6]

    default: NULL

    range: 0x000000000000 - 0xFFFFFFFFFFFE
*/
#define GAPMULTIROLE_ADV_DIRECT_ADDR     0x30B

/**
    @brief Which channels to advertise on (Read/Write)

    Multiple channels can be selected by ORing the bit values below.

    size: uint8

    default: @ref GAP_ADVCHAN_ALL

    range: @ref GAP_Adv_Chans
*/
#define GAPMULTIROLE_ADV_CHANNEL_MAP     0x30C

/**
    @brief Policy for filtering advertisements (Read/Write)

    @note This is ignored for direct advertising.

    size: uint8

    default: @ref GAP_FILTER_POLICY_ALL

    range: @ref GAP_Adv_Filter_Policices
*/
#define GAPMULTIROLE_ADV_FILTER_POLICY   0x30D


#define GAPMULTIROLE_PARAM_UPDATE_ENABLE 0x310

/**
    @brief  Minimum connection interval (n * 1.25 ms) to use when performing param update (Read/Write)

    size: uint16

    default: 6

    range: 6 - @ref GAPMULTIROLE_MAX_CONN_INTERVAL
*/
#define GAPMULTIROLE_MIN_CONN_INTERVAL   0x311

/**
    @brief Maximum connection interval (n * 1.25 ms) to use when performing param update (Read/Write)

    size: uint16

    default: 3200

    range: @ref GAPMULTIROLE_MIN_CONN_INTERVAL - 3200
*/
#define GAPMULTIROLE_MAX_CONN_INTERVAL   0x312

/**
    @brief Slave latency to use when performing param update (Read/Write)

    size: uint16

    default: 0

    range: 0 - 499
*/
#define GAPMULTIROLE_SLAVE_LATENCY       0x313

/**
    @brief Supervision timeout (n x 10 ms) to use when performing param update (Read/Write)

    size: uint16

    default: 1000

    range: 10-3200
*/
#define GAPMULTIROLE_TIMEOUT_MULTIPLIER  0x314

/**
    @brief Enable / Disable non-connectable advertising (Read/Write)

    @warning @ref GAPMULTIROLE_ADVERT_ENABLED must be set to FALSE in order to enable this

    size: uint8

    default: FALSE

    range: TRUE (enable) or FALSE (disable)
*/
#define GAPMULTIROLE_ADV_NONCONN_ENABLED 0x31B

/**
    @brief Maximmum number of scan reports to store from @ref GAPCentralRole_StartDiscovery (Read/Write)

    size: uint8

    default: 8

    range: 0-256 but this will be constrained by available RAM
*/
#define GAPMULTIROLE_MAX_SCAN_RES        0x404
#define GAPMULTIROLE_SCAN_MODE          0x405
#define GAPMULTIROLE_ACTIVE_SCAN        0x406
#define GAPMULTIROLE_SCAN_WHITELIST     0x407
#define GAPMULTIROLE_LINK_HIGHDUTYCYCLE 0x408
#define GAPMULTIROLE_LINK_WHITELIST     0x409
#define GAPMULTIROLE_ACTION_AFTER_LINK  0x40A



/**
    @brief Advertising off time Units:ms

    size: uint8

    default: 10(ms)

    range: 0-256 :
*/
#define GAPMULTIROLE_ADV_OFF_UNITS

/** @} End Multi_Params */

/** @defgroup Multi_Param_Update_Fail_Actions Failed Parameter Update Actions
    @{
    Possible actions the device may take if an unsuccessful parameter
    update is received.
*/
#define MULTIROLE_NO_ACTION                    0 //!< Take no action upon unsuccessful parameter updates
#define MULTIROLE_RESEND_PARAM_UPDATE          1 //!< Continue to resend request until successful update
#define MULTIROLE_TERMINATE_LINK               2 //!< Terminate link upon unsuccessful parameter updates
/** @} End Multi_Param_Update_Fail_Actions */


/** @defgroup Multi_Param_Update_Options Parameter Update Options
    @{
    Possible actions the device may take when it receives a
    Connection Parameter Update Request.
*/
#define GAPMULTIROLE_LINK_PARAM_UPDATE_ACCEPT       0 //!< Accept all parameter update requests
#define GAPMULTIROLE_LINK_PARAM_UPDATE_REJECT       1 //!< Reject all parameter update requests
#define GAPMULTIROLE_LINK_PARAM_UPDATE_APP_DECIDES  2 //!< Notify app for it to decide
#define GAPMULTIROLE_LINK_PARAM_UPDATE_NUM_OPTIONS  3 //!< Number of options. Used for parameter checking.

/** @} End Multi_Param_Update_Options */


/*  -------------------------------------------------------------------
    macro define -- multi advertising schedule data update info config
    uint8  bit.Number  ��      meaning
                            bit7-bit6:if set means advertising data or
                            sacn response data shall be updated before
                            make discoverable
            7               shall update advertising data
            6       :       shall update scan response data

                            bit1-bit0:if set,means advertising data already updated
            1       :       advertising data already updated,
            0       :       scan response data already updated
*/
#define GAPMULTI_UPDATEADV_FLAG         0x80
#define GAPMULTI_UPDATESRD_FLAG         0x40
#define GAPMULTI_ADV_UPDATED            0x02
#define GAPMULTI_SRD_UPDATED            0x01

#define GAPMULTI_CENTRAL_SMP            0x01

#define GAPMULTI_CENTRAL_MTU_EXCHANGE   0x0001
#define GAPMULTI_CENTRAL_DLE_EXCHANGE   0x0002
#define GAPMULTI_CENTRAL_SDP            0x0004




/** @} End Multi_Constants */


/** @defgroup Multi_Structs Multi GAPRole Structures
    @{
*/


/// @brief Multi GAPRole Event Structure

/**
    GAP multi-gap Role States.
*/
typedef enum
{
    GAPMULTIROLE_INIT = 0,                       //!< Waiting to be started
    GAPMULTIROLE_STARTED,                        //!< Started but not advertising
    GAPMULTIROLE_ADVERTISING,                    //!< Currently Advertising
    GAPMULTIROLE_SCANING,                        //!< Currently scaning
    GAPMULTIROLE_CONNECTING,                     //!< Currently scaning
    GAPMULTIROLE_WAITING,                        //!< Device is started but not advertising, is in waiting period before advertising again
    GAPMULTIROLE_WAITING_AFTER_TIMEOUT,          //!< Device just timed out from a connection but is not yet advertising, is in waiting period before advertising again
    GAPMULTIROLE_CONNECTED,                      //!< In a connection
    GAPMULTIROLE_TERMINATED,
    GAPMULTIROLE_CONNECTED_ADV,                  //!< In a connection + advertising
    GAPMULTIROLE_CONNECTED_SCAN,                 //!< In a connection + scan
    GAPMULTIROLE_ERROR                           //!< Error occurred - invalid state
} GAPMultiRole_states_t;

typedef union
{
    gapEventHdr_t             gap;                //!< @ref GAP_MSG_EVENT and status.
    gapDeviceInitDoneEvent_t  initDone;           //!< GAP initialization done.
    gapDeviceInfoEvent_t      deviceInfo;         //!< Discovery device information event structure.
    gapDevDiscEvent_t         discCmpl;           //!< Discovery complete event structure.
    gapEstLinkReqEvent_t      linkCmpl;           //!< Link complete event structure.
    gapLinkUpdateEvent_t      linkUpdate;         //!< Link update event structure.
    gapTerminateLinkEvent_t   linkTerminate;      //!< Link terminated event structure.
} gapMultiRoleEvent_t;

/// @brief Multi GAPRole Parameter Update Structure
typedef struct
{
    uint8   paramUpdateEnable;            //!< @ref Multi_Param_Update_Options
    uint16  connHandle;                  //!< connection handle
    uint16  minConnInterval;             //!< minimum connection interval
    uint16  maxConnInterval;             //!< maximum connection interval
    uint16  slaveLatency;                //!< slave latency
    uint16  timeoutMultiplier;           //!< supervision timeout
} gapRole_updateConnParams_t;


// Service and Characteristic
typedef struct Characteristic
{
    uint16  charHandle;
    uint8   Properties;
    uint16  valueHandle;
    uint8   uuid_Len;
    uint8   uuid[16];
    struct Characteristic* next;
} Characteristic;
typedef struct GATTReadGroupRsp
{
    uint16          StHandle;      // start handler
    uint16          EGHandle;      // End_Group_Handle
    uint8           uuid_Len;
    uint8           uuid[16];
    struct GATTReadGroupRsp* next;
} GATTReadGroupRsp;
typedef struct
{
    GATTReadGroupRsp*    PrimServ;              //ServerGroupService
    Characteristic*      Charac;
} GattMultiRoleScanServeice;

// Discovery states
typedef enum
{
    DISC_STATE_IDLE,
    DISC_STATE_SVC,
    DISC_STATE_CHAR,                 // Service discovery
    DISC_STATE_DONE
} MultiRole_discState_t;

typedef struct centralSDP
{
    uint16      connHandle;
    uint16      sdpCharHandle;
    // exchange
    MultiRole_discState_t    state;
    // service
    GattMultiRoleScanServeice service;
    struct centralSDP* next;
} GAPMultiRole_CentralSDP_t;

/*********************************************************************
    TYPEDEFS
*/
typedef struct
{
    struct
    {
        uint8   profileRole;
        uint8   IRK[KEYLEN];
        uint8   SRK[KEYLEN];
        uint32  signCounter;
        uint8   bdAddr[B_ADDR_LEN];

        // expiry connection parameter
        uint16  ExConnIntvMIN;
        uint16  ExConnIntvMAX;
        uint16  ExLatency;
        uint16  ExTimeOut;
    } common;
#if( MAX_CONNECTION_SLAVE_NUM > 0 )
    struct
    {
        uint8   EventType;
        uint8   ChanMap;
        uint8   FilterPolicy;
        uint8   UpdateEnable;
    } adv;
#endif
#if( MAX_CONNECTION_MASTER_NUM > 0 )
    struct
    {
        uint8 maxScanRes;
        uint8 scanMode;
        uint8 activeScan;
        uint8 whitelist;
    } scan;
    struct
    {
        uint8 highDutyCycle;
        uint8 whitelist;

        // bit value
        uint16 actionAfterLink;
    } link;
#endif
} GAPMultiRoleParam_t;

typedef enum
{
    Idle_Role = 0xFF,
    Slave_Role = 0x0,
    Master_Role = 0x3
} GAPMultiRole_State_t;

typedef struct link_T
{
    uint16  connectionHandle;
    GAPMultiRole_State_t    RoleState;
    uint8   peerDevAddrType;
    uint8   peerDevAddr[B_ADDR_LEN];
    uint16  connInterval;
    uint16  connLatency;
    uint16  connTimeout;
    struct  link_T*  next;
} GAPMultiRoleLinkCtrl_t;

typedef struct multiScan
{
    int8  rssi;
    uint8 addrtype;
    uint8 addr[B_ADDR_LEN];
    uint8 advDatalen;
    uint8* advData;
    uint8 scanRsplen;
    uint8* rspData;
    struct multiScan* next;
} GAPMultiRolScanner_t;

typedef struct mCentralDev
{
    uint8 addrType;
    uint8 addr[B_ADDR_LEN];
    struct mCentralDev* next;
} GAPMultiRoleCentralDev_t;

typedef struct mCentralAction
{
    uint16 connHandle;
    uint16 action;
    uint8  busy;
    struct mCentralAction* next;
} GAPMultiRoleCentralAction_t;

typedef enum
{
    MULTI_SCAN_MODE = 0,
    MULTI_CONFIG_SLAVE_DEV_MODE,
    MULTI_CONFIG_MASTER_LINKING_MODE,
    MULTI_CONFIG_MASTER_ACTION_MODE,
    MULTI_CENTRAL_ACTION_TIMER_MODE,
    MULTI_CENTRAL_SDP_MODE,
    MULTI_CENTRAL_SDP_PRIMSERV_MODE,
    MULTI_CENTRAL_SDP_CHARAC_MODE
} GAPMultiListMode_t;

typedef union
{
    uint16 info;
    struct
    {
        uint16 connHandle   : 4;
        uint16 role         : 4;
        uint16 perIdx       : 4;
        uint16 rfu          : 4;
    } value;
} GAPMultiLinkInfo_t;

#define GAPMULTILIST_MULTI_SCAN_MODE    GAPMultiRolScanner_t
#define GAPMULTILIST_MULTI_CONFIG_SLAVE_DEV_MODE    GAPMultiRoleCentralDev_t

#define _GAPMULTILIST_mode(x)   GAPMULTILIST_##x
#define GAPMULTILIST_mode(x)    _GAPMULTILIST_mode(x)
/** @} End Multi_Structs */

/*  -------------------------------------------------------------------
    MACROS
*/

/*  -------------------------------------------------------------------
    Profile Callbacks
*/

/**
    Callback when the device has been started.  Callback event to
    the Notify of a state change.
*/
typedef void (*gapRolesEachScan_t)( gapDeviceInfoEvent_t* pPkt );
typedef void (*gapRolesScanDone_t)(GAPMultiRolScanner_t* pPkt);
typedef void (*gapRolesEstablish_t)( uint8 status,uint16 connHandle,GAPMultiRole_State_t role,uint8 perIdx,uint8* addr );
typedef void (*gapRolesTerminate_t)( uint16 connHandle,GAPMultiRole_State_t role,uint8 perIdx,uint8 reason );

/**
    Callback when the device has read an new RSSI value during a connection.
*/
typedef void (*gapRolesRssiRead_t)( uint16 connHandle,int8 newRSSI );

/**
    Callback when SDP done during a connection.
*/
typedef void (*gapRolesSDPNotify_t)( void* msg);

typedef void (*gapRolesDataNotify_t)( uint16 connHandle,uint16 len,uint8* data);

typedef struct
{
    #if( MAX_CONNECTION_MASTER_NUM > 0)
    gapRolesEachScan_t      pfnEachScan;
    gapRolesScanDone_t      pfnScanDone;
    gapRolesSDPNotify_t     SDPNotify;      //!< Event callback.
    gapRolesDataNotify_t    dataNotify;
    #endif
    gapRolesEstablish_t     pfnEstablish;  //!< Whenever the device changes state
    gapRolesTerminate_t     pfnTerminate;
} gapMultiRolesCBs_t;

/** @defgroup Multi_CBs Multi GAPRole Callbacks
    @{
    These are functions whose pointers are passed from the application
    to the GAPRole so that the GAPRole can send events to the application
*/

/**
    @brief Multi Event Callback Function

    This callback is used by the Multi GAPRole to forward GAP_Events to the
    application.

    If the message is successfully queued to the application for later processing,
    FALSE is returned because the application deallocates it later. Consider the
    following state change event from multi_role as an example of this:

    @code{.c}
    static void multi_role_processAppMsg(mrEvt_t *pMsg)
    {
     switch (pMsg->event)
     {
     case MR_STATE_CHANGE_EVT:
       multi_role_processStackMsg((ICall_Hdr *)pMsg->pData);
       // Free the stack message
       ICall_freeMsg(pMsg->pData);
       break;
    @endcode

    If the message is not successfully queued to the application, TRUE is returned
    so that the GAPRole can deallocate the message. If the heap has enough room,
    the message must always be successfully enqueued.

    @param pEvent Pointer to event structure

    @return  TRUE if safe to deallocate event message
    @return  FALSE otherwise
*/
typedef uint8 (*passThroughToApp_t)
(
    gapMultiRoleEvent_t* pEvent
);

/**
    @brief Callback for the app to decide on a parameter update request

    This callback will be used if the @ref GAP_UPDATE_LINK_PARAM_REQ_EVENT parameter
    is set to @ref GAPMULTIROLE_LINK_PARAM_UPDATE_APP_DECIDES

    @param pReq Pointer to param update request
    @param pRsp Pointer to param update response.
*/
typedef void (*paramUpdateAppDecision_t)
(
    gapUpdateLinkParamReq_t* pReq
//  gapUpdateLinkParamReqReply_t *pRsp
);

/**
    @brief Multi GAPRole Callback structure

    This must be setup by the application and passed to the GAPRole when
    @ref GAPRole_StartDevice is called.
*/
typedef struct
{
    passThroughToApp_t        pfnPassThrough;             //!< When the event should be processed by the app instead of the GAP Role
    paramUpdateAppDecision_t  pfnParamUpdateAppDecision;  //!< When the app should decide on a param update request
} gapRolesCBs_t;

/** @} End Multi_Structs */

/*  -------------------------------------------------------------------
    API FUNCTIONS
*/

/**
    @brief       Set a GAP Role parameter.

    @note
          The "len" field must be set to the size of a "uint16" and the
          "pValue" field must point to a "uint16".

    @param param     @ref Multi_Params
    @param len       length of data to write
    @param pValue    pointer to data to write.  This is dependent on
     the parameter ID and will be cast to the appropriate
     data type (example: data type of uint16 will be cast to
     uint16 pointer).
    @param           connHandle connection handle

    @return @ref SUCCESS
    @return @ref INVALIDPARAMETER
    @return  @ref bleInvalidRange : len is invalid for the given param
*/
extern bStatus_t GAPMultiRole_SetParameter(uint16 param, uint8 len, void* pValue);

/**
    @brief       Get a GAP Role parameter.

    @note
          The "pValue" field must point to a "uint16".

    @param param     @ref Multi_Params
    @param pValue    pointer to location to get the value.  This is dependent on
     the parameter ID and will be cast to the appropriate
     data type (example: data type of uint16 will be cast to
     uint16 pointer).
    @param           connHandle connection handle

    @return @ref SUCCESS
    @return @ref INVALIDPARAMETER
*/
extern bStatus_t GAPMultiRole_GetParameter(uint16 param, void* pValue);

/**
    @brief Initialize the GAP layer.

    @warning Only call this function once.

    @param       pAppCallbacks @ref gapRolesCBs_t
    @param       numConns a pointer to the desired number of connections that the
                application wants is passed in with this parameter. the GAPRole
                will use this value to negotiate with the amount of connections
                that the stack supports and place the negotiated value in this
                memory location for return to the app.

    @return      @ref SUCCESS
    @return  @ref bleAlreadyInRequestedMode : Device already started.
*/
bStatus_t GAPMultiRole_StartDevice(gapMultiRolesCBs_t* pAppCallbacks, uint8* numConns);

/**
    @brief       Terminates the existing connection.

    @param       connHandle handle of connection to terminate

    @return      @ref SUCCESS
    @return      @ref bleIncorrectMode
    @return      @ref HCI_ERROR_CODE_CONTROLLER_BUSY : terminate procedure has already started
*/
extern bStatus_t GAPMultiRole_TerminateConnection(uint16 connHandle);

/**
    @brief   Start a device discovery scan.

    @param   mode discovery mode: @ref GAP_Discovery
    @param   activeScan TRUE to perform active scan
    @param   whiteList TRUE to only scan for devices in the white list

    @return  @ref SUCCESS : Discovery discovery has started.
    @return  @ref bleIncorrectMode : Invalid profile role.
    @return  @ref bleAlreadyInRequestedMode : Device discovery already started
    @return  @ref HCI_ERROR_CODE_INVALID_HCI_CMD_PARAMS : bad parameter
*/
extern bStatus_t GAPMultiRole_StartDiscovery(uint8 mode, uint8 activeScan, uint8 whiteList);

/**
    @brief   Cancel a device discovery scan.

    @return  @ref SUCCESS : Cancel started.
    @return  @ref bleInvalidTaskID : Not the task that started discovery.
    @return  @ref bleIncorrectMode : Not in discovery mode.
*/
extern bStatus_t GAPMultiRole_CancelDiscovery(void);

/**
    @brief   Establish a link to a peer device.

    @param   highDutyCycle  TRUE to high duty cycle scan, FALSE if not
    @param   whiteList determines use of the white list: @ref GAP_Whitelist
    @param   addrTypePeer @ref Addr_type
    @param   peerAddr peer device address

    @return  @ref SUCCESS : started establish link process
    @return  @ref bleIncorrectMode : invalid profile role
    @return  @ref bleNotReady : a scan is in progress
    @return  @ref bleAlreadyInRequestedMode : can�t process now
    @return  @ref bleNoResources : too many links
*/
extern bStatus_t GAPMultiRole_EstablishLink(uint8 highDutyCycle, uint8 whiteList,
                                            uint8 addrTypePeer, uint8* peerAddr);


/**
    @brief   Send a connection parameter update to a connected device

    @param   handleFailure @ref Multi_Param_Update_Fail_Actions
    @param   pConnParams pointer to connection parameters

    @return  @ref SUCCESS : operation was successful.
    @return  @ref INVALIDPARAMETER : Data can not fit into one packet.
    @return  @ref MSG_BUFFER_NOT_AVAIL : No HCI buffer is available.
    @return  @ref bleInvalidRange : params do not satisfy spec
    @return  @ref bleIncorrectMode : invalid profile role.
    @return  @ref bleAlreadyInRequestedMode : already updating link parameters.
    @return  @ref bleNotConnected : Connection is down
    @return  @ref bleMemAllocError : Memory allocation error occurred.
    @return  @ref bleNoResources : No available resource
*/
extern bStatus_t GAPMultiRole_connUpdate(uint8 handleFailure,
                                         gapRole_updateConnParams_t* pConnParams);


// === from peripheral.c
/**
    @brief       Update the parameters of an existing connection

    @param       connInterval - the new connection interval
    @param       latency - the new slave latency
    @param       connTimeout - the new timeout value
    @param       handleFailure - what to do if the update does not occur.
                Method may choose to terminate connection, try again, or take no action

    @return      SUCCESS, bleNotConnected or bleInvalidRange
*/
extern bStatus_t GAPMultiRole_SendUpdateParam( uint16 minConnInterval, uint16 maxConnInterval,
                                               uint16 latency, uint16 connTimeout, uint8 handleFailure );
/// @cond NODOC

/*  -------------------------------------------------------------------
    TASK FUNCTIONS - Don't call these. These are system functions.
*/

/**
    @internal

    @brief       Initialization function for the GAP Role Task.
            This is called during initialization and should contain
            any application specific initialization (ie. hardware
            initialization/setup, table initialization, power up
            notificaiton ... ).

    @param       the ID assigned by OSAL.  This ID should be
                      used to send messages and set timers.

    @return      void
*/
extern void GAPMultiRole_Init( uint8 task_id );

/**
    @internal

    @brief       GAP Role Task event processor.
            This function is called to process all events for the task.
            Events include timers, messages and any other user defined
            events.

    @param   task_id  - The OSAL assigned task ID.
    @param   events - events to process.  This is a bit map and can
                     contain more than one event.

    @return      events not processed
*/
extern uint16 GAPMultiRole_ProcessEvent( uint8 task_id, uint16 events );

/// @endcond // NODOC

/*  -------------------------------------------------------------------
    -------------------------------------------------------------------*/

/**

    @brief       GAP MultiRole Init peer address to link

    @param       idx - Index of peer address
                pAddr-peer device Address
                addrType - peer device address type (Public address ...)

    @return      None
*/
extern char* bdAddr2Str( uint8* pAddr );
//extern uint8 MultiRole_addPeerAddr(uint8* pAddr,uint8 addrType,uint8 en_connect );
//extern uint8 MultiRole_delPeerAddr(uint8* pAddr );
//extern uint8 MultiRole_ConnDevLen();
//extern uint8 MultiRole_isAddr_inlist(uint8* pAddr);

//extern void  GAPMultiRole_enConnPeerAddr(uint8 *pAddr,uint8 en);
//extern GattScanServer* GAPMultiRole_GetSDPIdx( uint16 connHandle );

//extern void gapRole_setEvent(uint32 event);

/*******************************************************************************************************
 *******************************************************************************************************
    @ Description    :    according macro define MAX_CONNECTION_SLAVE_NUM and
                        MAX_CONNECTION_MASTER_NUM initialization list MultiScehdule *mSche
    @ Parameters     :
                   :  [IN]  pNum:according MAX_CONNECTION_SLAVE_NUM , max peripheral num
                            cNum:according MAX_CONNECTION_MASTER_NUM, max central num
                   :  [OUT] void
    @ Return         :  None
    @ Other          :  1.list case :     adv  --> adv ---
                                        adv  --> conn --> adv --> conn ---
                                        adv+adv --> adv+adv ---
                                        adv+scan --> adv+scan ---

                                        adv+adv+scan --> adv+adv+scan ---
                      2.list add priority: adv0 --> adv1 --> scanner --> initiator
    Modification History
    DATE        DESCRIPTION
    2020-11-26   [A] first add
    2020-11-27   [A] add create node priority : advertiser --> scanner --> initiator
    ------------------------------------------------------------------------------
    @author          :
 *******************************************************************************************************
 *******************************************************************************************************/
//uint8 multiSchedule_Config(uint8 Mode,uint8 flag, uint8 idx );
//uint8 multiScan_Config(uint8 flag);
//uint8 multiInitiator_Config(uint8 flag);
//
#if( MAX_CONNECTION_MASTER_NUM > 0 )
void multiGetScanStrategy(uint8* param,uint8 len);
void multiGetLinkStrategy(uint8* param,uint8 len);
uint8 multiConfigSlaveList( uint8 addrType,uint8* addr);
uint8 multiClearSlaveList( uint8 addrType,uint8* addr);
GAPMultiRoleCentralDev_t* multiGetConfigSlaveList(void);
void multiClearAllSlaveList(void);
uint8 multiDevInConfSlaveList(uint8* addr);
uint8 multi_devInLinkList(uint8* addr);
void multiAddSlaveConnList( uint8 addrType,uint8* addr);
GAPMultiRoleCentralDev_t* multiGetSlaveConnList(void);
void multiDelCurrentConnNode(void);

#endif

#ifdef __cplusplus
}
#endif

#endif /* MULTI_H */

/** @} End Multi */
