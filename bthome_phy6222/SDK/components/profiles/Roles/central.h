/*************
 SDK_LICENSE
***************/
#ifndef CENTRAL_H
#define CENTRAL_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
    INCLUDES
*/
#include "bcomdef.h"
#include "OSAL.h"
#include "gap.h"

/*********************************************************************
    CONSTANTS
*/

/** @defgroup GAPCENTRALROLE_PROFILE_PARAMETERS GAP Central Role Parameters
    @{
*/
#define GAPCENTRALROLE_IRK                 0x400  //!< Identity Resolving Key. Read/Write. Size is uint8[KEYLEN]. Default is all 0, which means that the IRK will be randomly generated.
#define GAPCENTRALROLE_SRK                 0x401  //!< Signature Resolving Key. Read/Write. Size is uint8[KEYLEN]. Default is all 0, which means that the SRK will be randomly generated.
#define GAPCENTRALROLE_SIGNCOUNTER         0x402  //!< Sign Counter. Read/Write. Size is uint32. Default is 0.
#define GAPCENTRALROLE_BD_ADDR             0x403  //!< Device's Address. Read Only. Size is uint8[B_ADDR_LEN]. This item is read from the controller.
#define GAPCENTRALROLE_MAX_SCAN_RES        0x404  //!< Maximum number of discover scan results to receive. Default is 0 = unlimited.
/** @} End GAPCENTRALROLE_PROFILE_PARAMETERS */

/**
    Number of simultaneous links with periodic RSSI reads
*/
#ifndef GAPCENTRALROLE_NUM_RSSI_LINKS
#define GAPCENTRALROLE_NUM_RSSI_LINKS     4
#endif

/*********************************************************************
    VARIABLES
*/

/*********************************************************************
    MACROS
*/

/*********************************************************************
    TYPEDEFS
*/

/**
    Central Event Structure
*/
typedef union
{
    gapEventHdr_t             gap;                //!< GAP_MSG_EVENT and status.
    gapDeviceInitDoneEvent_t  initDone;           //!< GAP initialization done.
    gapDeviceInfoEvent_t      deviceInfo;         //!< Discovery device information event structure.
    gapDevDiscEvent_t         discCmpl;           //!< Discovery complete event structure.
    gapEstLinkReqEvent_t      linkCmpl;           //!< Link complete event structure.
    gapLinkUpdateEvent_t      linkUpdate;         //!< Link update event structure.
    gapTerminateLinkEvent_t   linkTerminate;      //!< Link terminated event structure.
} gapCentralRoleEvent_t;

/**
    RSSI Read Callback Function
*/
typedef void (*pfnGapCentralRoleRssiCB_t)
(
    uint16 connHandle,                    //!< Connection handle.
    int8  rssi                            //!< New RSSI value.
);

/**
    Central Event Callback Function
*/
typedef void (*pfnGapCentralRoleEventCB_t)
(
    gapCentralRoleEvent_t* pEvent         //!< Pointer to event structure.
);

/**
    Central Callback Structure
*/
typedef struct
{
    pfnGapCentralRoleRssiCB_t   rssiCB;   //!< RSSI callback.
    pfnGapCentralRoleEventCB_t  eventCB;  //!< Event callback.
} gapCentralRoleCB_t;

/*********************************************************************
    VARIABLES
*/

/*********************************************************************
    API FUNCTIONS
*/

/*  -------------------------------------------------------------------
    Central Profile Public APIs
*/

/**
    @defgroup CENTRAL_PROFILE_API Central Profile API Functions

    @{
*/

/**
    @brief   Start the device in Central role.  This function is typically
            called once during system startup.

    @param   pAppCallbacks - pointer to application callbacks

    @return  SUCCESS: Operation successful.<BR>
            bleAlreadyInRequestedMode: Device already started.<BR>
*/
extern bStatus_t GAPCentralRole_StartDevice( gapCentralRoleCB_t* pAppCallbacks );

/**
    @brief   Set a parameter in the Central Profile.

    @param   param - profile parameter ID: @ref GAPCENTRALROLE_PROFILE_PARAMETERS
    @param   len - length of data to write
    @param   pValue - pointer to data to write.  This is dependent on
            the parameter ID and WILL be cast to the appropriate
            data type.

    @return  SUCCESS: Operation successful.<BR>
            INVALIDPARAMETER: Invalid parameter ID.<BR>
*/
extern bStatus_t GAPCentralRole_SetParameter( uint16 param, uint8 len, void* pValue );

/**
    @brief   Get a parameter in the Central Profile.

    @param   param - profile parameter ID: @ref GAPCENTRALROLE_PROFILE_PARAMETERS
    @param   pValue - pointer to buffer to contain the read data

    @return  SUCCESS: Operation successful.<BR>
            INVALIDPARAMETER: Invalid parameter ID.<BR>
*/
extern bStatus_t GAPCentralRole_GetParameter( uint16 param, void* pValue );

/**
    @brief   Terminate a link.

    @param   connHandle - connection handle of link to terminate
            or @ref GAP_CONN_HANDLE_DEFINES

    @return  SUCCESS: Terminate started.<BR>
            bleIncorrectMode: No link to terminate.<BR>
*/
extern bStatus_t GAPCentralRole_TerminateLink( uint16 connHandle );

/**
    @brief   Establish a link to a peer device.

    @param   highDutyCycle -  TRUE to high duty cycle scan, FALSE if not
    @param   whiteList - determines use of the white list: @ref GAP_WHITELIST_DEFINES
    @param   addrTypePeer - address type of the peer device: @ref GAP_ADDR_TYPE_DEFINES
    @param   peerAddr - peer device address

    @return  SUCCESS: started establish link process.<BR>
            bleIncorrectMode: invalid profile role.<BR>
            bleNotReady: a scan is in progress.<BR>
            bleAlreadyInRequestedMode: can?t process now.<BR>
            bleNoResources: too many links.<BR>
*/
extern bStatus_t GAPCentralRole_EstablishLink( uint8 highDutyCycle, uint8 whiteList,
                                               uint8 addrTypePeer, uint8* peerAddr );

/**
    @brief   Update the link connection parameters.

    @param   connHandle - connection handle
    @param   connIntervalMin - minimum connection interval in 1.25ms units
    @param   connIntervalMax - maximum connection interval in 1.25ms units
    @param   connLatency - number of LL latency connection events
    @param   connTimeout - connection timeout in 10ms units

    @return  SUCCESS: Connection update started started.<BR>
            bleIncorrectMode: No connection to update.<BR>
*/
extern bStatus_t GAPCentralRole_UpdateLink( uint16 connHandle, uint16 connIntervalMin,
                                            uint16 connIntervalMax, uint16 connLatency,
                                            uint16 connTimeout );
/**
    @brief   Start a device discovery scan.

    @param   mode - discovery mode: @ref GAP_DEVDISC_MODE_DEFINES
    @param   activeScan - TRUE to perform active scan
    @param   whiteList - TRUE to only scan for devices in the white list

    @return  SUCCESS: Discovery scan started.<BR>
            bleIncorrectMode: Invalid profile role.<BR>
            bleAlreadyInRequestedMode: Not available.<BR>
*/
extern bStatus_t GAPCentralRole_StartDiscovery( uint8 mode, uint8 activeScan, uint8 whiteList );

/**
    @brief   Cancel a device discovery scan.

    @return  SUCCESS: Cancel started.<BR>
            bleInvalidTaskID: Not the task that started discovery.<BR>
            bleIncorrectMode: Not in discovery mode.<BR>
*/
extern bStatus_t GAPCentralRole_CancelDiscovery( void );

/**
    @brief   Start periodic RSSI reads on a link.

    @param   connHandle - connection handle of link
    @param   period - RSSI read period in ms

    @return  SUCCESS: Terminate started.<BR>
            bleIncorrectMode: No link.<BR>
            bleNoResources: No resources.<BR>
*/
extern bStatus_t GAPCentralRole_StartRssi( uint16 connHandle, uint16 period );

/**
    @brief   Cancel periodic RSSI reads on a link.

    @param   connHandle - connection handle of link

    @return  SUCCESS: Operation successful.<BR>
            bleIncorrectMode: No link.<BR>
*/
extern bStatus_t GAPCentralRole_CancelRssi(uint16 connHandle );

/**
    @}
*/

/*  -------------------------------------------------------------------
    TASK API - These functions must only be called by OSAL.
*/

/**
    @internal

    @brief   Central Profile Task initialization function.

    @param   taskId - Task ID.

    @return  void
*/
extern void GAPCentralRole_Init( uint8 taskId );

/**
    @internal

    @brief   Central Profile Task event processing function.

    @param   taskId - Task ID
    @param   events - Events.

    @return  events not processed
*/
extern uint16 GAPCentralRole_ProcessEvent( uint8 taskId, uint16 events );

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* CENTRAL_H */
