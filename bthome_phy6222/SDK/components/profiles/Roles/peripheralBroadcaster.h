/*************
 SDK_LICENSE
***************/

#ifndef PERIPHERAL_BROADCASTER_H
#define PERIPHERAL_BROADCASTER_H

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
#define GAPROLE_IRK                 0x301  //!< Identity Resolving Key. Read/Write. Size is uint8[KEYLEN]. Default is all 0, which means that the IRK will be randomly generated.
#define GAPROLE_SRK                 0x302  //!< Signature Resolving Key. Read/Write. Size is uint8[KEYLEN]. Default is all 0, which means that the SRK will be randomly generated.
#define GAPROLE_SIGNCOUNTER         0x303  //!< Sign Counter. Read/Write. Size is uint32. Default is 0.
#define GAPROLE_BD_ADDR             0x304  //!< Device's Address. Read Only. Size is uint8[B_ADDR_LEN]. This item is read from the controller.
#define GAPROLE_ADVERT_ENABLED      0x305  //!< Enable/Disable Advertising. Read/Write. Size is uint8. Default is TRUE=Enabled.  During a connection, this will enable and display advertising (non-connectable undirected).  Setting this variable during a connection does not change the permanent state of the parameter.
#define GAPROLE_ADVERT_OFF_TIME     0x306  //!< Advertising Off Time for Limited advertisements (in milliseconds). Read/Write. Size is uint16. Default is 30 seconds.
#define GAPROLE_ADVERT_DATA         0x307  //!< Advertisement Data. Read/Write. Max size is uint8[B_MAX_ADV_LEN].  Default is "02:01:01", which means that it is a Limited Discoverable Advertisement.
#define GAPROLE_SCAN_RSP_DATA       0x308  //!< Scan Response Data. Read/Write. Max size is uint8[B_MAX_ADV_LEN]. Defaults to all 0.
#define GAPROLE_ADV_EVENT_TYPE      0x309  //!< Advertisement Type. Read/Write. Size is uint8.  Default is GAP_ADTYPE_ADV_IND (defined in GAP.h).
#define GAPROLE_ADV_DIRECT_TYPE     0x30A  //!< Direct Advertisement Address Type. Read/Write. Size is uint8. Default is ADDRTYPE_PUBLIC (defined in GAP.h).
#define GAPROLE_ADV_DIRECT_ADDR     0x30B  //!< Direct Advertisement Address. Read/Write. Size is uint8[B_ADDR_LEN]. Default is NULL.
#define GAPROLE_ADV_CHANNEL_MAP     0x30C  //!< Which channels to advertise on. Read/Write Size is uint8. Default is GAP_ADVCHAN_ALL (defined in GAP.h)
#define GAPROLE_ADV_FILTER_POLICY   0x30D  //!< Filter Policy. Ignored when directed advertising is used. Read/Write. Size is uint8. Default is GAP_FILTER_POLICY_ALL (defined in GAP.h).
#define GAPROLE_CONNHANDLE          0x30E  //!< Connection Handle. Read Only. Size is uint16.
#define GAPROLE_RSSI_READ_RATE      0x30F  //!< How often to read the RSSI during a connection. Read/Write. Size is uint16. The value is in milliseconds. Default is 0 = OFF.
#define GAPROLE_PARAM_UPDATE_ENABLE 0x310  //!< Slave Connection Parameter Update Enable. Read/Write. Size is uint8. If TRUE then automatic connection parameter update request is sent. Default is FALSE.
#define GAPROLE_MIN_CONN_INTERVAL   0x311  //!< Minimum Connection Interval to allow (n * 1.25ms).  Range: 7.5 msec to 4 seconds (0x0006 to 0x0C80). Read/Write. Size is uint16. Default is 7.5 milliseconds (0x0006).
#define GAPROLE_MAX_CONN_INTERVAL   0x312  //!< Maximum Connection Interval to allow (n * 1.25ms).  Range: 7.5 msec to 4 seconds (0x0006 to 0x0C80). Read/Write. Size is uint16. Default is 4 seconds (0x0C80).
#define GAPROLE_SLAVE_LATENCY       0x313  //!< Update Parameter Slave Latency. Range: 0 - 499. Read/Write. Size is uint16. Default is 0.
#define GAPROLE_TIMEOUT_MULTIPLIER  0x314  //!< Update Parameter Timeout Multiplier (n * 10ms). Range: 100ms to 32 seconds (0x000a - 0x0c80). Read/Write. Size is uint16. Default is 1000.
#define GAPROLE_CONN_BD_ADDR        0x315  //!< Address of connected device. Read only. Size is uint8[B_MAX_ADV_LEN]. Set to all zeros when not connected.
/** @} End GAPROLE_PROFILE_PARAMETERS */

/*  -------------------------------------------------------------------
    TYPEDEFS
*/

/**
    GAP Peripheral + Broadcaster Role States.
*/
typedef enum
{
    GAPROLE_INIT = 0,                         //!< Waiting to be started
    GAPROLE_STARTED,                          //!< Started but not advertising
    GAPROLE_ADVERTISING,                      //!< Currently Advertising
    GAPROLE_WAITING,                          //!< Device is started but not advertising, is in waiting period before advertising again
    GAPROLE_WAITING_AFTER_TIMEOUT,            //!< Device just timed out from a connection but is not yet advertising, is in waiting period before advertising again
    GAPROLE_CONNECTED,                        //!< In a connection
    GAPROLE_CONNECTED_ADV,                    //!< In a connection and advertising
    GAPROLE_ERROR                             //!< Error occurred - invalid state
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
    @defgroup GAPROLES_PERIPHERAL_BROADCASTER_API GAP Peripheral + Broadcaster Role API Functions

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
    @brief       Terminates the existing connection.

    @return      SUCCESS or bleIncorrectMode
*/
extern bStatus_t GAPRole_TerminateConnection( void );

/**
    @} End GAPROLES_PERIPHERAL_BROADCASTER_API
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

#endif /* PERIPHERAL_BROADCASTER_H */
