/*************
 SDK_LICENSE
***************/
#ifndef BROADCASTER_H
#define BROADCASTER_H

#ifdef __cplusplus
extern "C"
{
#endif

/*  -------------------------------------------------------------------
    INCLUDES
*/

/*  -------------------------------------------------------------------
    CONSTANTS
*/

/** @defgroup GAPROLE_PROFILE_PARAMETERS GAP Role Parameters
    @{
*/
#define GAPROLE_PROFILEROLE         0x300  //!< Reading this parameter will return GAP Role type. Read Only. Size is uint8.
#define GAPROLE_BD_ADDR             0x301  //!< Device's Address. Read Only. Size is uint8[B_ADDR_LEN]. This item is read from the controller.
#define GAPROLE_ADVERT_ENABLED      0x302  //!< Enable/Disable Advertising. Read/Write. Size is uint8. Default is TRUE=Enabled.
#define GAPROLE_ADVERT_OFF_TIME     0x303  //!< Advertising Off Time for Limited advertisements (in milliseconds). Read/Write. Size is uint16. Default is 30 seconds.
#define GAPROLE_ADVERT_DATA         0x304  //!< Advertisement Data. Read/Write. Size is uint8[B_MAX_ADV_LEN].  Default is "02:01:01", which means that it is a Limited Discoverable Advertisement.
#define GAPROLE_SCAN_RSP_DATA       0x305  //!< Scan Response Data. Read/Write. Size is uint8[B_MAX_ADV_LEN]. Defaults to all 0.
#define GAPROLE_ADV_EVENT_TYPE      0x306  //!< Advertisement Type. Read/Write. Size is uint8.  Default is GAP_ADTYPE_ADV_IND (defined in GAP.h).
#define GAPROLE_ADV_DIRECT_TYPE     0x307  //!< Direct Advertisement Address Type. Ready/Write. Size is uint8. Default is ADDRTYPE_PUBLIC (defined in GAP.h).
#define GAPROLE_ADV_DIRECT_ADDR     0x308  //!< Direct Advertisement Address. Read/Write. Size is uint8[B_ADDR_LEN]. Default is NULL.
#define GAPROLE_ADV_CHANNEL_MAP     0x309  //!< Which channels to advertise on. Read/Write Size is uint8. Default is GAP_ADVCHAN_ALL (defined in GAP.h)
#define GAPROLE_ADV_FILTER_POLICY   0x30A  //!< Filter Policy. Ignored when directed advertising is used. Read/Write. Size is uint8. Default is GAP_FILTER_POLICY_ALL (defined in GAP.h).
/** @} End GAPROLE_PROFILE_PARAMETERS */

/*  -------------------------------------------------------------------
    TYPEDEFS
*/

/**
    GAP Broadcaster Role States.
*/
typedef enum
{
    GAPROLE_INIT = 0,                       //!< Waiting to be started
    GAPROLE_STARTED,                        //!< Started but not advertising
    GAPROLE_ADVERTISING,                    //!< Currently Advertising
    GAPROLE_WAITING,                        //!< Device is started but not advertising, is in waiting period before advertising again
    GAPROLE_ERROR                           //!< Error occurred - invalid state
} gaprole_States_t;

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
typedef void (*gapRolesStateNotify_t)( gaprole_States_t newState );

/**
    Callback when the device has read an new RSSI value during a connection.
*/
typedef void (*gapRolesRssiRead_t)( int8 newRSSI );

/**
    Callback structure - must be setup by the application and used when gapRoles_StartDevice() is called.
*/
typedef struct
{
    gapRolesStateNotify_t    pfnStateChange;  //!< Whenever the device changes state
    gapRolesRssiRead_t       pfnRssiRead;     //!< When a valid RSSI is read from controller
} gapRolesCBs_t;

/*  -------------------------------------------------------------------
    API FUNCTIONS
*/

/**
    @defgroup GAPROLES_BROADCASTER_API GAP Broadcaster Role API Functions

    @{
*/

/**
    @brief       Set a GAP Role parameter.

    NOTE: You can call this function with a GAP Parameter ID and it will set the
          GAP Parameter.  GAP Parameters are defined in (gap.h).  Also,
          the "len" field must be set to the size of a "uint16" and the
          "pValue" field must point to a "uint16".

    @param       param - Profile parameter ID: @ref GAPROLE_PROFILE_PARAMETERS
    @param       len - length of data to write
    @param       pValue - pointer to data to write.  This is dependent on
            the parameter ID and WILL be cast to the appropriate
            data type (example: data type of uint16 will be cast to
            uint16 pointer).

    @return      SUCCESS or INVALIDPARAMETER (invalid paramID)
*/
extern bStatus_t GAPRole_SetParameter( uint16 param, uint8 len, void* pValue );

/**
    @brief       Get a GAP Role parameter.

    NOTE: You can call this function with a GAP Parameter ID and it will get a
          GAP Parameter.  GAP Parameters are defined in (gap.h).  Also, the
          "pValue" field must point to a "uint16".

    @param       param - Profile parameter ID: @ref GAPROLE_PROFILE_PARAMETERS
    @param       pValue - pointer to location to get the value.  This is dependent on
            the parameter ID and WILL be cast to the appropriate
            data type (example: data type of uint16 will be cast to
            uint16 pointer).

    @return      SUCCESS or INVALIDPARAMETER (invalid paramID)
*/
extern bStatus_t GAPRole_GetParameter( uint16 param, void* pValue );

/**
    @brief       Does the device initialization.  Only call this function once.

    @param       pAppCallbacks - pointer to application callbacks.

    @return      SUCCESS or bleAlreadyInRequestedMode
*/
extern bStatus_t GAPRole_StartDevice( gapRolesCBs_t* pAppCallbacks );

/**
    @} End GAPROLES_BROADCASTER_API
*/


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
extern void GAPRole_Init( uint8 task_id );

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
extern uint16 GAPRole_ProcessEvent( uint8 task_id, uint16 events );

/*  -------------------------------------------------------------------
    -------------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif /* BROADCASTER_H */
