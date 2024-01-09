/*************
 SDK_LICENSE
***************/
#ifndef OBSERVER_H
#define OBSERVER_H

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

/** @defgroup GAPOBSERVERROLE_PROFILE_PARAMETERS GAP Observer Role Parameters
    @{
*/
#define GAPOBSERVERROLE_BD_ADDR        0x400  //!< Device's Address. Read Only. Size is uint8[B_ADDR_LEN]. This item is read from the controller.
#define GAPOBSERVERROLE_MAX_SCAN_RES   0x401  //!< Maximum number of discover scan results to receive. Default is 0 = unlimited.
/** @} End GAPOBSERVERROLE_PROFILE_PARAMETERS */

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
    Observer Event Structure
*/
typedef union
{
    gapEventHdr_t             gap;                //!< GAP_MSG_EVENT and status.
    gapDeviceInitDoneEvent_t  initDone;           //!< GAP initialization done.
    gapDeviceInfoEvent_t      deviceInfo;         //!< Discovery device information event structure.
    gapDevDiscEvent_t         discCmpl;           //!< Discovery complete event structure.
} gapObserverRoleEvent_t;

/**
    RSSI Read Callback Function
*/
typedef void (*pfnGapObserverRoleRssiCB_t)
(
    uint16 connHandle,                    //!< Connection handle.
    int8  rssi                            //!< New RSSI value.
);

/**
    Observer Event Callback Function
*/
typedef void (*pfnGapObserverRoleEventCB_t)
(
    gapObserverRoleEvent_t* pEvent         //!< Pointer to event structure.
);

/**
    Observer Callback Structure
*/
typedef struct
{
    pfnGapObserverRoleRssiCB_t   rssiCB;   //!< RSSI callback.
    pfnGapObserverRoleEventCB_t  eventCB;  //!< Event callback.
} gapObserverRoleCB_t;

/*********************************************************************
    VARIABLES
*/

/*********************************************************************
    API FUNCTIONS
*/

/*  -------------------------------------------------------------------
    Observer Profile Public APIs
*/

/**
    @defgroup OBSERVER_PROFILE_API Observer Profile API Functions

    @{
*/

/**
    @brief   Start the device in Observer role.  This function is typically
            called once during system startup.

    @param   pAppCallbacks - pointer to application callbacks

    @return  SUCCESS: Operation successful.<BR>
            bleAlreadyInRequestedMode: Device already started.<BR>
*/
extern bStatus_t GAPObserverRole_StartDevice( gapObserverRoleCB_t* pAppCallbacks );

/**
    @brief   Set a parameter in the Observer Profile.

    @param   param - profile parameter ID: @ref GAPOBSERVERROLE_PROFILE_PARAMETERS
    @param   len - length of data to write
    @param   pValue - pointer to data to write.  This is dependent on
            the parameter ID and WILL be cast to the appropriate
            data type.

    @return  SUCCESS: Operation successful.<BR>
            INVALIDPARAMETER: Invalid parameter ID.<BR>
*/
extern bStatus_t GAPObserverRole_SetParameter( uint16 param, uint8 len, void* pValue );

/**
    @brief   Get a parameter in the Observer Profile.

    @param   param - profile parameter ID: @ref GAPOBSERVERROLE_PROFILE_PARAMETERS
    @param   pValue - pointer to buffer to contain the read data

    @return  SUCCESS: Operation successful.<BR>
            INVALIDPARAMETER: Invalid parameter ID.<BR>
*/
extern bStatus_t GAPObserverRole_GetParameter( uint16 param, void* pValue );

/**
    @brief   Start a device discovery scan.

    @param   mode - discovery mode: @ref GAP_DEVDISC_MODE_DEFINES
    @param   activeScan - TRUE to perform active scan
    @param   whiteList - TRUE to only scan for devices in the white list

    @return  SUCCESS: Discovery scan started.<BR>
            bleIncorrectMode: Invalid profile role.<BR>
            bleAlreadyInRequestedMode: Not available.<BR>
*/
extern bStatus_t GAPObserverRole_StartDiscovery( uint8 mode, uint8 activeScan, uint8 whiteList );

/**
    @brief   Cancel a device discovery scan.

    @return  SUCCESS: Cancel started.<BR>
            bleInvalidTaskID: Not the task that started discovery.<BR>
            bleIncorrectMode: Not in discovery mode.<BR>
*/
extern bStatus_t GAPObserverRole_CancelDiscovery( void );

/**
    @}
*/

/*  -------------------------------------------------------------------
    TASK API - These functions must only be called by OSAL.
*/

/**
    @internal

    @brief   Observer Profile Task initialization function.

    @param   taskId - Task ID.

    @return  void
*/
extern void GAPObserverRole_Init( uint8 taskId );

/**
    @internal

    @brief   Observer Profile Task event processing function.

    @param   taskId - Task ID
    @param   events - Events.

    @return  events not processed
*/
extern uint16 GAPObserverRole_ProcessEvent( uint8 taskId, uint16 events );

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* OBSERVER_H */
