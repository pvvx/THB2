/*
 thservice.c
*/

/*********************************************************************
	INCLUDES
*/
#include "types.h"
#include "config.h"
#if (DEV_SERVICES & SERVICE_THS)
#include "bcomdef.h"
#include "OSAL.h"
#include "linkdb.h"
#include "att.h"
#include "gatt.h"
#include "gatt_uuid.h"
#include "gatt_profile_uuid.h"
#include "gattservapp.h"
#include "peripheral.h"
//#include "hiddev.h"

#include "thservice.h"
#include "sensors.h"
/*********************************************************************
	MACROS
*/
/*********************************************************************
	CONSTANTS
*/

#define TEMP_LEVEL_VALUE_IDX		2 // Position of temp level in attribute array
#define TEMP_LEVEL_VALUE_CCCD_IDX	3 // Position of temp level CCCD in attribute array
#define HUMI_LEVEL_VALUE_IDX		5 // Position of humi level in attribute array
#define HUMI_LEVEL_VALUE_CCCD_IDX	6 // Position of humi level CCCD in attribute array

/*********************************************************************
	TYPEDEFS
*/

/*********************************************************************
	GLOBAL VARIABLES
*/
// Battery service
CONST uint8 temphumServUUID[ATT_BT_UUID_SIZE] =
{
	LO_UINT16(ENV_SENSING_SERV_UUID), HI_UINT16(ENV_SENSING_SERV_UUID)
};

// Temperatyre level characteristic
CONST uint8 tempLevelUUID[ATT_BT_UUID_SIZE] =
{
	LO_UINT16(TEMPERATYRE_UUID), HI_UINT16(TEMPERATYRE_UUID)
};

// Humidity level characteristic
CONST uint8 humiLevelUUID[ATT_BT_UUID_SIZE] =
{
	LO_UINT16(HUMIDITY_UUID), HI_UINT16(HUMIDITY_UUID)
};


/*********************************************************************
	EXTERNAL VARIABLES
*/

/*********************************************************************
	EXTERNAL FUNCTIONS
*/

/*********************************************************************
	LOCAL VARIABLES
*/

// Application callback
static thServiceCB_t thServiceCB;

/*********************************************************************
	Profile Attributes - variables
*/

// TH Service attribute
static CONST gattAttrType_t thService = { ATT_BT_UUID_SIZE, temphumServUUID };

// TH level characteristic
static uint8 thProps = GATT_PROP_READ | GATT_PROP_NOTIFY;

static gattCharCfg_t tempLevelClientCharCfg[GATT_MAX_NUM_CONN];
static gattCharCfg_t humiLevelClientCharCfg[GATT_MAX_NUM_CONN];


/*********************************************************************
	Profile Attributes - Table
*/

static gattAttribute_t thAttrTbl[] =
{
	// 0 TH Service
	{
		{ ATT_BT_UUID_SIZE, primaryServiceUUID }, /* type */
		GATT_PERMIT_READ,						  /* permissions */
		0,										  /* handle */
		(uint8*)& thService						  /* pValue */
	},

	// 1 Temp Level Declaration
	{
		{ ATT_BT_UUID_SIZE, characterUUID },
		GATT_PERMIT_READ,
		0,
		&thProps
	},

	// 2 Temp Level Value
	{
		{ ATT_BT_UUID_SIZE, tempLevelUUID },
		GATT_PERMIT_READ,
		0,
		(uint8_t*)&measured_data.temp
	},

	// 3 Temp Level Client Characteristic Configuration
	{
		{ ATT_BT_UUID_SIZE, clientCharCfgUUID },
		GATT_PERMIT_READ | GATT_PERMIT_WRITE,
		0,
		(uint8*)& tempLevelClientCharCfg
	},

	// 4 Humi Level Declaration
	{
		{ ATT_BT_UUID_SIZE, characterUUID },
		GATT_PERMIT_READ,
		0,
		&thProps
	},

	// 5 Humi Level Value
	{
		{ ATT_BT_UUID_SIZE, humiLevelUUID },
		GATT_PERMIT_READ,
		0,
		(uint8_t*)&measured_data.humi
	},

	// 6 Humi Level Client Characteristic Configuration
	{
		{ ATT_BT_UUID_SIZE, clientCharCfgUUID },
		GATT_PERMIT_READ | GATT_PERMIT_WRITE,
		0,
		(uint8*)& humiLevelClientCharCfg
	},

};


/*********************************************************************
	LOCAL FUNCTIONS
*/
static uint8 thReadAttrCB( uint16 connHandle, gattAttribute_t* pAttr,
							 uint8* pValue, uint16* pLen, uint16 offset, uint8 maxLen );
static bStatus_t thWriteAttrCB( uint16 connHandle, gattAttribute_t* pAttr,
								  uint8* pValue, uint16 len, uint16 offset );
static void thNotifyCB( linkDBItem_t* pLinkItem );

/*********************************************************************
	PROFILE CALLBACKS
*/
// TH Service Callbacks
CONST gattServiceCBs_t thCBs =
{
	thReadAttrCB,	// Read callback function pointer
	thWriteAttrCB,	// Write callback function pointer
	NULL			// Authorization callback function pointer
};

/*********************************************************************
	PUBLIC FUNCTIONS
*/

/*********************************************************************
	@fn		 TH_AddService

	@brief	 Initializes the Battery Service by registering
			GATT attributes with the GATT server.

	@return	 Success or Failure
*/
bStatus_t TH_AddService( void )
{
	uint8 status = SUCCESS;
	// Initialize Client Characteristic Configuration attributes
	GATTServApp_InitCharCfg( INVALID_CONNHANDLE, tempLevelClientCharCfg );
	GATTServApp_InitCharCfg( INVALID_CONNHANDLE, humiLevelClientCharCfg );
	// Register GATT attribute list and CBs with GATT Server App
	status = GATTServApp_RegisterService( thAttrTbl,
										  GATT_NUM_ATTRS( thAttrTbl ),
										  &thCBs );
	return ( status );
}

/*********************************************************************
	@fn		 Batt_Register

	@brief	 Register a callback function with the Battery Service.

	@param	 pfnServiceCB - Callback function.

	@return	 None.
*/
extern void TH_Register( thServiceCB_t pfnServiceCB )
{
	thServiceCB = pfnServiceCB;
}

/*********************************************************************
	@fn			 thReadAttrCB

	@brief		 Read an attribute.

	@param		 connHandle - connection message was received on
	@param		 pAttr - pointer to attribute
	@param		 pValue - pointer to data to be read
	@param		 pLen - length of data to be read
	@param		 offset - offset of the first octet to be read
	@param		 maxLen - maximum length of data to be read

	@return		 Success or Failure
*/
static uint8 thReadAttrCB( uint16 connHandle, gattAttribute_t* pAttr,
							 uint8* pValue, uint16* pLen, uint16 offset, uint8 maxLen )
{
	(void)connHandle;
	(void)maxLen;
	bStatus_t status = SUCCESS;

	// Make sure it's not a blob operation (no attributes in the profile are long)
	if ( offset > 0 )
	{
		return ( ATT_ERR_ATTR_NOT_LONG );
	}

	uint16 uuid = BUILD_UINT16( pAttr->type.uuid[0], pAttr->type.uuid[1] );

	// Measure temp level if reading level
	if ( uuid == TEMPERATYRE_UUID )
	{
		*pLen = 2;
		pValue[0] = measured_data.temp;
		pValue[1] = measured_data.temp >> 8;
	}
	// Measure humi level if reading level
	else if ( uuid == HUMIDITY_UUID)
	{
		*pLen = 2;
		pValue[0] = measured_data.humi;
		pValue[1] = measured_data.humi >> 8;
	}
	else
	{
		status = ATT_ERR_ATTR_NOT_FOUND;
	}
	return ( status );
}

/*********************************************************************
	@fn		thWriteAttrCB

	@brief	 Validate attribute data prior to a write operation

	@param	 connHandle - connection message was received on
	@param	 pAttr - pointer to attribute
	@param	 pValue - pointer to data to be written
	@param	 len - length of data
	@param	 offset - offset of the first octet to be written

	@return	 Success or Failure
*/
static bStatus_t thWriteAttrCB( uint16 connHandle, gattAttribute_t* pAttr,
								  uint8* pValue, uint16 len, uint16 offset )
{
	bStatus_t status = SUCCESS;
	uint16 uuid = BUILD_UINT16( pAttr->type.uuid[0], pAttr->type.uuid[1]);

	switch ( uuid )
	{
	case GATT_CLIENT_CHAR_CFG_UUID:
		status = GATTServApp_ProcessCCCWriteReq( connHandle, pAttr, pValue, len,
												 offset, GATT_CLIENT_CFG_NOTIFY );

		if ( status == SUCCESS )
		{
			uint16 charCfg = BUILD_UINT16( pValue[0], pValue[1] );

			if ( thServiceCB )
			{
				(*thServiceCB)( (charCfg == GATT_CFG_NO_OPERATION) ?
								  TEMP_LEVEL_NOTI_DISABLED :
								  TEMP_LEVEL_NOTI_ENABLED);
			}
		}

		break;

	default:
		status = ATT_ERR_ATTR_NOT_FOUND;
		break;
	}

	return ( status );
}

/*********************************************************************
	@fn			 thNotifyCB

	@brief		 Send a notification of the level state characteristic.

	@param		 connHandle - linkDB item

	@return		 None.
*/
static void thNotifyCB( linkDBItem_t* pLinkItem )
{
	attHandleValueNoti_t noti;

	if ( pLinkItem->stateFlags & LINK_CONNECTED )
	{
		if ( GATTServApp_ReadCharCfg( pLinkItem->connectionHandle,
												tempLevelClientCharCfg )
				  & GATT_CLIENT_CFG_NOTIFY )
		{
			noti.handle = thAttrTbl[TEMP_LEVEL_VALUE_IDX].handle;
			noti.len = 2;
			noti.value[0] = measured_data.temp;
			noti.value[1] = measured_data.temp >> 8;
			GATT_Notification( pLinkItem->connectionHandle, &noti, FALSE );
		}

		if ( GATTServApp_ReadCharCfg( pLinkItem->connectionHandle,
												humiLevelClientCharCfg )
					& GATT_CLIENT_CFG_NOTIFY )
		{
			noti.handle = thAttrTbl[HUMI_LEVEL_VALUE_IDX].handle;
			noti.len = 2;
			noti.value[0] = measured_data.humi;
			noti.value[1] = measured_data.humi >> 8;
			GATT_Notification( pLinkItem->connectionHandle, &noti, FALSE );
		}
	}
}

/*********************************************************************
	@fn		 thNotifyLevelState

	@brief	 Send a notification of the th state
			characteristic if a connection is established.

	@return	 None.
*/
void TH_NotifyLevel(void)
{
	// Execute linkDB callback to send notification
	linkDB_PerformFunc( thNotifyCB );
}

/*********************************************************************
	@fn			 TH_HandleConnStatusCB

	@brief		 TH Service link status change handler function.

	@param		 connHandle - connection handle
	@param		 changeType - type of change

	@return		 none
*/
void TH_HandleConnStatusCB( uint16 connHandle, uint8 changeType )
{
	// Make sure this is not loopback connection
	if ( connHandle != LOOPBACK_CONNHANDLE )
	{
		// Reset Client Char Config if connection has dropped
		if ( ( changeType == LINKDB_STATUS_UPDATE_REMOVED )		 ||
				( ( changeType == LINKDB_STATUS_UPDATE_STATEFLAGS ) &&
				  ( !linkDB_Up( connHandle ) ) ) )
		{
			GATTServApp_InitCharCfg( connHandle, tempLevelClientCharCfg );
			GATTServApp_InitCharCfg( connHandle, humiLevelClientCharCfg );
		}
	}
}

#endif // (DEV_SERVICES & SERVICE_THS)

