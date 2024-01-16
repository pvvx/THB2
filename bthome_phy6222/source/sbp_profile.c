/**************************************************************************************************
	Filename:		sbpProfile.c
	Revised:
	Revision:
	Description:	This file contains the Simple GATT profile sample GATT service
				  profile for use with the BLE sample application.
**************************************************************************************************/

/*********************************************************************
	INCLUDES
*/
#include "bcomdef.h"
#include "config.h"
#include "OSAL.h"
#include "linkdb.h"
#include "att.h"
#include "gatt.h"
#include "gatt_uuid.h"
#include "gattservapp.h"
#include "gapbondmgr.h"
//#include "log.h"
#include "thb2_peripheral.h"
#include "bleperipheral.h"
#include "sbp_profile.h"
#include "cmd_parcer.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */
/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */
// Simple GATT Profile Service UUID: 0xFFF0
CONST uint8_t simpleProfileServUUID[ATT_BT_UUID_SIZE] =
{
	LO_UINT16(SIMPLEPROFILE_SERV_UUID), HI_UINT16(SIMPLEPROFILE_SERV_UUID)
};

// Characteristic 1 UUID: 0xFFF3
CONST uint8_t simpleProfilechar1UUID[ATT_BT_UUID_SIZE] =
{
	LO_UINT16(SIMPLEPROFILE_CHAR1_UUID), HI_UINT16(SIMPLEPROFILE_CHAR1_UUID)
};

// Characteristic 2 UUID: 0xFFF4
CONST uint8_t simpleProfilechar2UUID[ATT_BT_UUID_SIZE] =
{
	LO_UINT16(SIMPLEPROFILE_CHAR2_UUID), HI_UINT16(SIMPLEPROFILE_CHAR2_UUID)
};

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

static simpleProfileCBs_t *simpleProfile_AppCBs = NULL;
//static	uint8_t	ReadNotify_Len		=	0;

/*********************************************************************
 * Profile Attributes - variables
 */

// Simple Profile Service attribute 0xFFF0
static CONST gattAttrType_t simpleProfileService = { ATT_BT_UUID_SIZE, simpleProfileServUUID };


// Simple Profile Characteristic 1 Properties
static CONST uint8_t simpleProfileChar1Props			=	GATT_PROP_READ | GATT_PROP_WRITE | GATT_PROP_WRITE_NO_RSP;
//static CONST uint8_t simpleProfileChar1UserDesp[]		=	"OTA\0";	// Simple Profile Characteristic 1 User Description

static uint8_t ota_in_buffer[20];			// Characteristic 1 Value
static uint8_t ota_in_len;

// Simple Profile Characteristic 2 Properties
static CONST uint8_t simpleProfileChar2Props			=	GATT_PROP_READ | GATT_PROP_WRITE | GATT_PROP_WRITE_NO_RSP | GATT_PROP_NOTIFY;
//static CONST uint8_t simpleProfileChar2UserDesp[]		=	"CMD\0";	// Simple Profile Characteristic 2 User Description
static gattCharCfg_t simpleProfileChar2Config[GATT_MAX_NUM_CONN];		//

static uint8_t cmd_in_buffer[32];			// Characteristic 2 Value
static uint8_t cmd_in_len;							// Characteristic 2 Value

/*********************************************************************
 * Profile Attributes - Table
 */

#define SERVAPP_NUM_ATTR_SUPPORTED		  6
#define OTA_DATA_ATTR_IDX				  2 // Position of OTA in attribute array
#define CDM_DATA_ATTR_IDX				  4 // Position of CMD in attribute array

static gattAttribute_t simpleProfileAttrTbl[SERVAPP_NUM_ATTR_SUPPORTED] =
{
	// Simple Profile Service
	{
			{ ATT_BT_UUID_SIZE, primaryServiceUUID },	/* type */
			GATT_PERMIT_READ,							/* permissions */
			0,											/* handle */
			(uint8_t *)&simpleProfileService			/* pValue */
	},

	// Characteristic 1 Declaration
	{
			{ ATT_BT_UUID_SIZE, characterUUID },
			GATT_PERMIT_READ,
			0,
			(uint8_t *)&simpleProfileChar1Props
	},
	// Characteristic Value 1
	{
			{ ATT_BT_UUID_SIZE, simpleProfilechar1UUID },
			GATT_PERMIT_READ | GATT_PERMIT_WRITE,
			0,
			(uint8_t *)&ota_in_buffer[0]
	},
#if 0
	// Characteristic 1 User Description
	{
			{ ATT_BT_UUID_SIZE, charUserDescUUID },
			GATT_PERMIT_READ,
			0,
			(uint8_t *)simpleProfileChar1UserDesp
	},
#endif
	// Characteristic 2 Declaration
	{
			{ ATT_BT_UUID_SIZE, characterUUID },
			GATT_PERMIT_READ,
			0,
			(uint8_t *)&simpleProfileChar2Props
	},
	// Characteristic Value 2
	{
			{ ATT_BT_UUID_SIZE, simpleProfilechar2UUID },
			GATT_PERMIT_READ | GATT_PERMIT_WRITE,
			0,
			(uint8_t *)&cmd_in_buffer[0]
	},
	// Characteristic 2 configuration
	{
			{ ATT_BT_UUID_SIZE, clientCharCfgUUID },
			GATT_PERMIT_READ | GATT_PERMIT_WRITE,
			0,
			(uint8_t *)simpleProfileChar2Config
	},
#if 0
	// Characteristic 2 User Description
	{
			{ ATT_BT_UUID_SIZE, charUserDescUUID },
			GATT_PERMIT_READ,
			0,
			(uint8_t *)simpleProfileChar2UserDesp
	},
#endif
};


/*********************************************************************
 * LOCAL FUNCTIONS
 */
static bStatus_t	simpleProfile_ReadAttrCB( uint16_t connHandle, gattAttribute_t *pAttr,uint8_t *pValue, uint16_t *pLen, uint16_t offset, uint8_t maxLen );
static bStatus_t	simpleProfile_WriteAttrCB( uint16_t connHandle, gattAttribute_t *pAttr,uint8_t *pValue, uint16_t len, uint16_t offset );
static void			simpleProfile_HandleConnStatusCB( uint16_t connHandle, uint8_t changeType );
/*********************************************************************
 * PROFILE CALLBACKS
 */
// Simple Profile Service Callbacks
CONST gattServiceCBs_t simpleProfileCBs =
{
	simpleProfile_ReadAttrCB,  // Read callback function pointer
	simpleProfile_WriteAttrCB, // Write callback function pointer
	NULL					   // Authorization callback function pointer
};

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn		SimpleProfile_AddService
 *
 * @brief	Initializes the Simple Profile service by registering
 *			GATT attributes with the GATT server.
 *
 * @param	services - services to add. This is a bit map and can
 *					   contain more than one service.
 *
 * @return	Success or Failure
 */
bStatus_t SimpleProfile_AddService( uint32_t services )
{
	uint8_t status = SUCCESS;

	// Initialize Client Characteristic Configuration attributes
	GATTServApp_InitCharCfg( INVALID_CONNHANDLE, simpleProfileChar2Config );

	// Register with Link DB to receive link status change callback
	VOID linkDB_Register( simpleProfile_HandleConnStatusCB );

	if ( services & SIMPLEPROFILE_SERVICE )
	{
		// Register GATT attribute list and CBs with GATT Server App
		status = GATTServApp_RegisterService( simpleProfileAttrTbl,
					GATT_NUM_ATTRS( simpleProfileAttrTbl ),
					&simpleProfileCBs );
	}

	return ( status );
}


/*********************************************************************
 * @fn		SimpleProfile_RegisterAppCBs
 *
 * @brief	Registers the application callback function. Only call
 *			this function once.
 *
 * @param	callbacks - pointer to application callbacks.
 *
 * @return	SUCCESS or bleAlreadyInRequestedMode
 */
bStatus_t SimpleProfile_RegisterAppCBs( simpleProfileCBs_t *appCallbacks )
{
	if ( appCallbacks ){
		simpleProfile_AppCBs = appCallbacks;
		return ( SUCCESS );
	}else{
		return ( bleAlreadyInRequestedMode );
	}
}


/*********************************************************************
 * @fn			simpleProfile_ReadAttrCB
 *
 * @brief		Read an attribute.
 *
 * @param		connHandle - connection message was received on
 * @param		pAttr - pointer to attribute
 * @param		pValue - pointer to data to be read
 * @param		pLen - length of data to be read
 * @param		offset - offset of the first octet to be read
 * @param		maxLen - maximum length of data to be read
 *
 * @return		Success or Failure
 */
static bStatus_t simpleProfile_ReadAttrCB( uint16_t connHandle, gattAttribute_t *pAttr,
							uint8_t *pValue, uint16_t *pLen, uint16_t offset, uint8_t maxLen )
{
	(void)connHandle;
	(void)maxLen;
	bStatus_t status = SUCCESS;

	// If attribute permissions require authorization to read, return error
	if ( gattPermitAuthorRead( pAttr->permissions ) )
	{
		// Insufficient authorization
		return ( ATT_ERR_INSUFFICIENT_AUTHOR );
	}

	// Make sure it's not a blob operation (no attributes in the profile are long)
	if ( offset > 0 )
	{
		return ( ATT_ERR_ATTR_NOT_LONG );
	}

	if ( pAttr->type.len == ATT_BT_UUID_SIZE )
	{
		// 16-bit UUID
		uint16_t uuid = BUILD_UINT16( pAttr->type.uuid[0], pAttr->type.uuid[1]);
		switch ( uuid )
		{
			// No need for "GATT_SERVICE_UUID" or "GATT_CLIENT_CHAR_CFG_UUID" cases;
			// gattserverapp handles those reads
			case SIMPLEPROFILE_CHAR1_UUID:
				*pLen = sizeof(ota_in_buffer);
				osal_memcpy( pValue, pAttr->pValue, *pLen );
				LOG("Read_UUID1:\n");
				break;
			case SIMPLEPROFILE_CHAR2_UUID:
				*pLen = sizeof(cfg);
				osal_memcpy( pValue, &cfg, *pLen );
				LOG("Read_UUID2:\n");
				break;
			default:
				// Should never get here! (characteristics 3 and 4 do not have read permissions)
				*pLen = 0;
				status = ATT_ERR_ATTR_NOT_FOUND;
			break;
		}
	} else {
		// 128-bit UUID
		*pLen = 0;
		status = ATT_ERR_INVALID_HANDLE;
	}
	return ( status );
}

/*********************************************************************
 * @fn		simpleProfile_WriteAttrCB
 *
 * @brief	Validate attribute data prior to a write operation
 *
 * @param	connHandle - connection message was received on
 * @param	pAttr - pointer to attribute
 * @param	pValue - pointer to data to be written
 * @param	len - length of data
 * @param	offset - offset of the first octet to be written
 *
 * @return	Success or Failure
 */
 static bStatus_t simpleProfile_WriteAttrCB( uint16_t connHandle, gattAttribute_t *pAttr,
								 uint8_t *pValue, uint16_t len, uint16_t offset )
{
	bStatus_t status = SUCCESS;

	// If attribute permissions require authorization to write, return error
	if ( gattPermitAuthorWrite( pAttr->permissions ) )
	{
		// Insufficient authorization
		return ( ATT_ERR_INSUFFICIENT_AUTHOR );
	}

	if ( pAttr->type.len == ATT_BT_UUID_SIZE )
	{
		// 16-bit UUID
		uint16_t uuid = BUILD_UINT16( pAttr->type.uuid[0], pAttr->type.uuid[1]);
		switch ( uuid )
		{
			case SIMPLEPROFILE_CHAR1_UUID:
	            //Validate the value
	            // Make sure it's not a blob oper
	            if ( offset == 0 ) {
					if ( len > sizeof(ota_in_buffer))
						status = ATT_ERR_INVALID_VALUE_SIZE;
	            } else
	                status = ATT_ERR_ATTR_NOT_LONG;

	            //Write the value
	            if ( status == SUCCESS) {
	            	osal_memcpy( pAttr->pValue, pValue, len );
	            	ota_in_len = len;
	            }
				break;

			case SIMPLEPROFILE_CHAR2_UUID:
				// Validate the value
				// Make sure it's not a blob oper
				if ( offset == 0 ) {
					if ( len > sizeof(cmd_in_buffer))
						status = ATT_ERR_INVALID_VALUE_SIZE;
				} else
					status = ATT_ERR_ATTR_NOT_LONG;
				// Write the value
				if ( status == SUCCESS ) {
					osal_memcpy(pAttr->pValue, pValue, len );
					cmd_in_len = len;
					LOG("receive data = 0x ");
					LOG_DUMP_BYTE(pCurValue, len);
					osal_set_event(simpleBLEPeripheral_TaskID, SBP_DEALDATA);
				}
				break;

			case GATT_CLIENT_CHAR_CFG_UUID:
				LOG("Enable/Disable Notity\n");
				status = GATTServApp_ProcessCCCWriteReq( connHandle, pAttr, pValue, len,
													 offset, GATT_CLIENT_CFG_NOTIFY );
			break;

			default:
				// Should never get here! (characteristics 2 and 4 do not have write permissions)
				status = ATT_ERR_ATTR_NOT_FOUND;
			break;
		}
	}else{
		// 128-bit UUID
		status = ATT_ERR_INVALID_HANDLE;
	}
	return ( status );
}

/*********************************************************************
 * @fn			simpleProfile_HandleConnStatusCB
 *
 * @brief		Simple Profile link status change handler function.
 *
 * @param		connHandle - connection handle
 * @param		changeType - type of change
 *
 * @return		none
 */
static void simpleProfile_HandleConnStatusCB( uint16_t connHandle, uint8_t changeType )
{
	// Make sure this is not loopback connection
	if ( connHandle != LOOPBACK_CONNHANDLE ){
		// Reset Client Char Config if connection has dropped
		if ( ( changeType == LINKDB_STATUS_UPDATE_REMOVED )||( ( changeType == LINKDB_STATUS_UPDATE_STATEFLAGS ) &&( !linkDB_Up( connHandle ) ) ) ){
			GATTServApp_InitCharCfg( connHandle, simpleProfileChar2Config );
		}
	}
}


void new_cmd(void) {
	attHandleValueNoti_t noti;
	noti.handle = simpleProfileAttrTbl[CDM_DATA_ATTR_IDX].handle;
	noti.len = cmd_parser(noti.value, cmd_in_buffer, cmd_in_len);
	if(noti.len) {
		GATT_Notification(gapRole_ConnectionHandle, &noti, FALSE );
	}
}
