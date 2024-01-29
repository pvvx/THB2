/**
	@headerfile:	thb2_peripheral.h
	$Date:
	$Revision:

	This GAP profile advertises and allows connections.
*/

#ifndef TH_PERIPHERAL_H
#define TH_PERIPHERAL_H


#ifdef __cplusplus
extern "C"
{
#endif

/*	-------------------------------------------------------------------
	INCLUDES
*/
#include "bcomdef.h"

/*	-------------------------------------------------------------------
	CONSTANTS
*/

/** @defgroup GAPROLE_PROFILE_PARAMETERS GAP Role Parameters
	@{
*/
#define GAPROLE_PROFILEROLE			0x300  //!< Reading this parameter will return GAP Role type. Read Only. Size is uint8.
#define GAPROLE_IRK					0x301  //!< Identity Resolving Key. Read/Write. Size is uint8[KEYLEN]. Default is all 0, which means that the IRK will be randomly generated.
#define GAPROLE_SRK					0x302  //!< Signature Resolving Key. Read/Write. Size is uint8[KEYLEN]. Default is all 0, which means that the SRK will be randomly generated.
#define GAPROLE_SIGNCOUNTER			0x303  //!< Sign Counter. Read/Write. Size is uint32. Default is 0.
#define GAPROLE_BD_ADDR				0x304  //!< Device's Address. Read Only. Size is uint8[B_ADDR_LEN]. This item is read from the controller.
#define GAPROLE_ADVERT_ENABLED		0x305  //!< Enable/Disable Advertising. Read/Write. Size is uint8. Default is TRUE=Enabled.
#define GAPROLE_ADVERT_OFF_TIME		0x306  //!< Advertising Off Time for Limited advertisements (in milliseconds). Read/Write. Size is uint16. Default is 30 seconds.
#define GAPROLE_ADVERT_DATA			0x307  //!< Advertisement Data. Read/Write. Max size is uint8[B_MAX_ADV_LEN].  Default is "02:01:01", which means that it is a Limited Discoverable Advertisement.
#define GAPROLE_SCAN_RSP_DATA		0x308  //!< Scan Response Data. Read/Write. Max size is uint8[B_MAX_ADV_LEN]. Defaults to all 0.
#define GAPROLE_ADV_EVENT_TYPE		0x309  //!< Advertisement Type. Read/Write. Size is uint8.	Default is GAP_ADTYPE_ADV_IND (defined in GAP.h).
#define GAPROLE_ADV_DIRECT_TYPE		0x30A  //!< Direct Advertisement Address Type. Read/Write. Size is uint8. Default is ADDRTYPE_PUBLIC (defined in GAP.h).
#define GAPROLE_ADV_DIRECT_ADDR		0x30B  //!< Direct Advertisement Address. Read/Write. Size is uint8[B_ADDR_LEN]. Default is NULL.
#define GAPROLE_ADV_CHANNEL_MAP		0x30C  //!< Which channels to advertise on. Read/Write Size is uint8. Default is GAP_ADVCHAN_ALL (defined in GAP.h)
#define GAPROLE_ADV_FILTER_POLICY	0x30D  //!< Filter Policy. Ignored when directed advertising is used. Read/Write. Size is uint8. Default is GAP_FILTER_POLICY_ALL (defined in GAP.h).
#define GAPROLE_CONNHANDLE			0x30E  //!< Connection Handle. Read Only. Size is uint16.
#define GAPROLE_RSSI_READ_RATE		0x30F  //!< How often to read the RSSI during a connection. Read/Write. Size is uint16. The value is in milliseconds. Default is 0 = OFF.
#define GAPROLE_PARAM_UPDATE_ENABLE 0x310  //!< Slave Connection Parameter Update Enable. Read/Write. Size is uint8. If TRUE then automatic connection parameter update request is sent. Default is FALSE.
#define GAPROLE_MIN_CONN_INTERVAL	0x311  //!< Minimum Connection Interval to allow (n * 1.25ms).	Range: 7.5 msec to 4 seconds (0x0006 to 0x0C80). Read/Write. Size is uint16. Default is 7.5 milliseconds (0x0006).
#define GAPROLE_MAX_CONN_INTERVAL	0x312  //!< Maximum Connection Interval to allow (n * 1.25ms).	Range: 7.5 msec to 4 seconds (0x0006 to 0x0C80). Read/Write. Size is uint16. Default is 4 seconds (0x0C80).
#define GAPROLE_SLAVE_LATENCY		0x313  //!< Update Parameter Slave Latency. Range: 0 - 499. Read/Write. Size is uint16. Default is 0.
#define GAPROLE_TIMEOUT_MULTIPLIER	0x314  //!< Update Parameter Timeout Multiplier (n * 10ms). Range: 100ms to 32 seconds (0x000a - 0x0c80). Read/Write. Size is uint16. Default is 1000.
#define GAPROLE_CONN_BD_ADDR		0x315  //!< Address of connected device. Read only. Size is uint8[B_MAX_ADV_LEN]. Set to all zeros when not connected.
#define GAPROLE_CONN_INTERVAL		0x316  //!< Current connection interval.  Read only. Size is uint16.  Range is 7.5ms to 4 seconds (0x0006 to 0x0C80).  Default is 0 (no connection).
#define GAPROLE_CONN_LATENCY		0x317  //!< Current slave latency.	Read only.	Size is uint16.	 Range is 0 to 499. Default is 0 (no slave latency or no connection).
#define GAPROLE_CONN_TIMEOUT		0x318  //!< Current timeout value.	Read only.	size is uint16.	 Range is 100ms to 32 seconds.	Default is 0 (no connection).
#define GAPROLE_PARAM_UPDATE_REQ	0x319  //!< Slave Connection Parameter Update Request. Write. Size is uint8. If TRUE then connection parameter update request is sent.
#define GAPROLE_STATE				0x31A  //!< Reading this parameter will return GAP Peripheral Role State. Read Only. Size is uint8.

#define GAPROLE_CONNECTION_INTERVAL	 0x31B
#define GAPROLE_CONNECTION_LATENCY 0x31C


/** @} End GAPROLE_PROFILE_PARAMETERS */

#define GAP_EXTADV_PROP_NONCONN_NONSCAN			 0
#define GAP_EXTADV_PROP_CONNECTABLE_NONSCAN		 0x1
#define GAP_EXTADV_PROP_NONCONN_SCANNABLE		 0x2
#define GAP_EXTADV_PROP_LDC_CONNECTABLE			 0x5
#define GAP_EXTADV_PROP_LEGACY_ADV_IND			 LL_EXT_ADV_PROP_ADV_IND
#define GAP_EXTADV_PROP_LEGACY_LDC_ADV			 LL_EXT_ADV_PROP_ADV_LDC_ADV
#define GAP_EXTADV_PROP_LEGACY_HDC_ADV			 LL_EXT_ADV_PROP_ADV_HDC_ADV
#define GAP_EXTADV_PROP_LEGACY_SCAN_IND			 LL_EXT_ADV_PROP_ADV_SCAN_IND
#define GAP_EXTADV_PROP_LEGACY_NOCONN_IND		 LL_EXT_ADV_PROP_ADV_NOCONN_IND

typedef enum
{
	GAP_ADV_SET_CFG_START = 0x350,
	GAP_ADV_SID			  = 0x350,
	GAP_ADV_EVENT_PROP,
	GAP_PRI_CHN_ADV_INT_MIN,
	GAP_PRI_CHN_ADV_INT_MAX,
	GAP_PRI_CHN_ADV_CHNMAP,
	GAP_OWN_ADDR_TYPE,
	GAP_OWN_RANDOM_ADDR,
	GAP_PEER_ADDR_TYPE,
	GAP_PEER_ADDR,
	GAP_WL_POLICY,
	GAP_PRI_ADV_PHY,
	GAP_SEC_ADV_PHY,
	GAP_SEC_ADV_MAX_SKIP,
	GAP_SCANREQ_NOTI_ENABLE,
	GAP_EXT_ADV_DURATION,
	GAP_EXT_ADV_MAX_EVT,
	GAP_EXT_ADV_TX_PWR,
	GAP_EXT_ADV_DATA,
	GAP_EXT_SCAN_RSP_DATA,
	GAP_ADV_SET_MAX_ENUM
} GAP_AdvSetConfig_e;

typedef enum
{
	GAP_ADV_PHY_NONE	  = 0x00,				 // applicable to secondary PHY conf only
	GAP_ADV_PHY_1MBPS	  = 0x01,				 // applicable to both primary & secondary PHY conf
	GAP_ADV_PHY_2MBPS	  = 0x02,				 // applicable to secondary PHY conf only
	GAP_ADV_PHY_CODED	  = 0x03				 // applicable to both primary & secondary PHY conf
} GAP_ExtAdv_PHY_e;
/*	-------------------------------------------------------------------
	TYPEDEFS
*/

/**
	GAP Peripheral Role States.
*/
typedef enum
{
	GAPROLE_INIT = 0,						//!< Waiting to be started
	GAPROLE_STARTED,						//!< Started but not advertising
	GAPROLE_ADVERTISING,					//!< Currently Advertising
	GAPROLE_WAITING,						//!< Device is started but not advertising, is in waiting period before advertising again
	GAPROLE_WAITING_AFTER_TIMEOUT,			//!< Device just timed out from a connection but is not yet advertising, is in waiting period before advertising again
	GAPROLE_CONNECTED,						//!< In a connection
	GAPROLE_CONNECTED_ADV,					//!< In a connection + advertising
	GAPROLE_ERROR							//!< Error occurred - invalid state
} gaprole_States_t;

/**
	Possible actions the peripheral device may take if an unsuccessful parameter
	update is received.

	Parameters for GAPRole_SendUpdateParam() only
*/

#define GAPROLE_NO_ACTION					 0 // Take no action upon unsuccessful parameter updates
#define GAPROLE_RESEND_PARAM_UPDATE			 1 // Continue to resend request until successful update
#define GAPROLE_TERMINATE_LINK				 2 // Terminate link upon unsuccessful parameter updates

/*	-------------------------------------------------------------------
	MACROS
*/

/*	-------------------------------------------------------------------
	Profile Callbacks
*/

/**
	Callback when the connection parameteres are updated.
*/
typedef void (*gapRolesParamUpdateCB_t)( uint16 connInterval,
										 uint16 connSlaveLatency,
										 uint16 connTimeout );

/**
	Callback when the device has been started.	Callback event to
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
	gapRolesStateNotify_t	 pfnStateChange;  //!< Whenever the device changes state
	gapRolesRssiRead_t		 pfnRssiRead;	  //!< When a valid RSSI is read from controller
} gapRolesCBs_t;

/*	-------------------------------------------------------------------
	API FUNCTIONS
*/

/**
	@defgroup GAPROLES_PERIPHERAL_API GAP Peripheral Role API Functions

	@{
*/

/**
	@brief		 Set a GAP Role parameter.

	NOTE: You can call this function with a GAP Parameter ID and it will set the
		  GAP Parameter.  GAP Parameters are defined in (gap.h).  Also,
		  the "len" field must be set to the size of a "uint16" and the
		  "pValue" field must point to a "uint16".

	@param		 param - Profile parameter ID: @ref GAPROLE_PROFILE_PARAMETERS
	@param		 len - length of data to write
	@param		 pValue - pointer to data to write.	 This is dependent on
			the parameter ID and WILL be cast to the appropriate
			data type (example: data type of uint16 will be cast to
			uint16 pointer).

	@return		 SUCCESS or INVALIDPARAMETER (invalid paramID)
*/
extern bStatus_t GAPRole_SetParameter( uint16 param, uint8 len, void* pValue );

/**
	@brief		 Get a GAP Role parameter.

	NOTE: You can call this function with a GAP Parameter ID and it will get a
		  GAP Parameter.  GAP Parameters are defined in (gap.h).  Also, the
		  "pValue" field must point to a "uint16".

	@param		 param - Profile parameter ID: @ref GAPROLE_PROFILE_PARAMETERS
	@param		 pValue - pointer to location to get the value.	 This is dependent on
			the parameter ID and WILL be cast to the appropriate
			data type (example: data type of uint16 will be cast to
			uint16 pointer).

	@return		 SUCCESS or INVALIDPARAMETER (invalid paramID)
*/
extern bStatus_t GAPRole_GetParameter( uint16 param, void* pValue );

/**
	@brief		 Does the device initialization.  Only call this function once.

	@param		 pAppCallbacks - pointer to application callbacks.

	@return		 SUCCESS or bleAlreadyInRequestedMode
*/
extern bStatus_t GAPRole_StartDevice( gapRolesCBs_t* pAppCallbacks );

/**
	@brief		 Terminates the existing connection.

	@return		 SUCCESS or bleIncorrectMode
*/
extern bStatus_t GAPRole_TerminateConnection( void );

/**
	@brief		 Update the parameters of an existing connection

	@param		 connInterval - the new connection interval
	@param		 latency - the new slave latency
	@param		 connTimeout - the new timeout value
	@param		 handleFailure - what to do if the update does not occur.
				Method may choose to terminate connection, try again, or take no action

	@return		 SUCCESS, bleNotConnected or bleInvalidRange
*/
extern bStatus_t GAPRole_SendUpdateParam( uint16 minConnInterval, uint16 maxConnInterval,
										  uint16 latency, uint16 connTimeout, uint8 handleFailure );

/**
	@brief		 Register application's callbacks.

	@param		 pParamUpdateCB - pointer to param update callback.

	@return		 none
*/
extern void GAPRole_RegisterAppCBs( gapRolesParamUpdateCB_t* pParamUpdateCB );

/**
	@} End GAPROLES_PERIPHERAL_API
*/


/*	-------------------------------------------------------------------
	TASK FUNCTIONS - Don't call these. These are system functions.
*/

/**
	@internal

	@brief		 Initialization function for the GAP Role Task.
			This is called during initialization and should contain
			any application specific initialization (ie. hardware
			initialization/setup, table initialization, power up
			notificaiton ... ).

	@param		 the ID assigned by OSAL.  This ID should be
					  used to send messages and set timers.

	@return		 void
*/
extern void GAPRole_Init( uint8 task_id );

/**
	@internal

	@brief		 GAP Role Task event processor.
			This function is called to process all events for the task.
			Events include timers, messages and any other user defined
			events.

	@param	 task_id  - The OSAL assigned task ID.
	@param	 events - events to process.  This is a bit map and can
					 contain more than one event.

	@return		 events not processed
*/
extern uint16 GAPRole_ProcessEvent( uint8 task_id, uint16 events );

/*	-------------------------------------------------------------------
	-------------------------------------------------------------------*/

extern bStatus_t GAPRole_extAdv_ConfigAdvSet( uint8 adv_handle );
extern bStatus_t GAPRole_extAdv_EnableAdvSet( uint8 adv_handle );
extern bStatus_t GAPRole_extAdv_DisableAdvSet( uint8 adv_handle );
extern bStatus_t GAPRole_extAdv_SetRandomAddress( uint8 adv_handle, uint8* random_address);
extern bStatus_t GAPRole_extAdv_RemoveAdvSet( uint8 adv_handle);
extern bStatus_t GAPRole_extAdv_SetParameter( uint8 adv_handler, uint16 param, uint16 len, void* pValue );
extern bStatus_t GAPRole_extAdv_GetParameter( uint8 adv_handler, uint16 param, void* pValue );


// Profile Events
#define START_ADVERTISING_EVT		  0x0001  // Start Advertising
#define RSSI_READ_EVT				  0x0002  // Read RSSI
#define START_CONN_UPDATE_EVT		  0x0004  // Start Connection Update Procedure
#define CONN_PARAM_TIMEOUT_EVT		  0x0008  // Connection Parameters Update Timeout

extern uint16 gapRole_AdvertOffTime;
extern uint8  gapRole_AdvEnabled;
extern uint8  gapRole_AdvertDataLen;
extern uint8  gapRole_AdvertData[B_MAX_ADV_LEN];
extern uint8  gapRole_ScanRspDataLen;
extern uint8  gapRole_ScanRspData[B_MAX_ADV_LEN];
extern uint8  gapRole_AdvDirectAddr[B_ADDR_LEN];
extern uint8  gapRole_AdvEventType;
extern uint8  gapRole_AdvDirectType;
extern uint8  gapRole_AdvChanMap;
extern uint8  gapRole_AdvFilterPolicy;
extern uint8  gapRole_TaskID;
extern gaprole_States_t gapRole_state;
extern uint8  gapRole_ParamUpdateEnable;
extern uint16 gapRole_MinConnInterval;
extern uint16 gapRole_MaxConnInterval;
extern uint16 gapRole_SlaveLatency;
extern uint16 gapRole_TimeoutMultiplier;
extern uint16 gapRole_ConnectionHandle;


#ifdef __cplusplus
}
#endif

#endif /* TH_PERIPHERAL_H */
