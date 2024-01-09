/**************************************************************************************************
 SDK_LICENSE
**************************************************************************************************/

/*********************************************************************
    INCLUDES
*/
#include "bcomdef.h"
#include "types.h"
#include "OSAL.h"
#include "linkdb.h"
#include "att.h"
#include "gatt.h"
#include "gatt_uuid.h"
#include "gatt_profile_uuid.h"
#include "gattservapp.h"
#include "peripheral.h"
#include "hiddev.h"

#include "battservice.h"
#include "sensor.h"
/*********************************************************************
    MACROS
*/
#define USE_HID_BATT 0
/*********************************************************************
    CONSTANTS
*/

#define BATT_LEVEL_VALUE_IDX        2 // Position of battery level in attribute array
#define BATT_LEVEL_VALUE_CCCD_IDX   3 // Position of battery level CCCD in attribute array

/*********************************************************************
    TYPEDEFS
*/

/*********************************************************************
    GLOBAL VARIABLES
*/
// Battery service
CONST uint8 battServUUID[ATT_BT_UUID_SIZE] =
{
    LO_UINT16(BATT_SERV_UUID), HI_UINT16(BATT_SERV_UUID)
};

// Battery level characteristic
CONST uint8 battLevelUUID[ATT_BT_UUID_SIZE] =
{
    LO_UINT16(BATT_LEVEL_UUID), HI_UINT16(BATT_LEVEL_UUID)
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
static battServiceCB_t battServiceCB;

/*********************************************************************
    Profile Attributes - variables
*/

// Battery Service attribute
static CONST gattAttrType_t battService = { ATT_BT_UUID_SIZE, battServUUID };

// Battery level characteristic
static uint8 battLevelProps = GATT_PROP_READ | GATT_PROP_NOTIFY;

static gattCharCfg_t battLevelClientCharCfg[GATT_MAX_NUM_CONN];

#if USE_HID_BATT	
// HID Report Reference characteristic descriptor, battery level
static uint8 hidReportRefBattLevel[HID_REPORT_REF_LEN] =
{ HID_RPT_ID_BATT_LEVEL_IN, HID_REPORT_TYPE_INPUT };
#endif

/*********************************************************************
    Profile Attributes - Table
*/

static gattAttribute_t battAttrTbl[] =
{
    // Battery Service
    {
        { ATT_BT_UUID_SIZE, primaryServiceUUID }, /* type */
        GATT_PERMIT_READ,                         /* permissions */
        0,                                        /* handle */
        (uint8*)& battService                     /* pValue */
    },

    // Battery Level Declaration
    {
        { ATT_BT_UUID_SIZE, characterUUID },
        GATT_PERMIT_READ,
        0,
        &battLevelProps
    },

    // Battery Level Value
    {
        { ATT_BT_UUID_SIZE, battLevelUUID },
        GATT_PERMIT_READ,
        0,
        &measured_data.battery
    },

    // Battery Level Client Characteristic Configuration
    {
        { ATT_BT_UUID_SIZE, clientCharCfgUUID },
        GATT_PERMIT_READ | GATT_PERMIT_WRITE,
        0,
        (uint8*)& battLevelClientCharCfg
    },

    // HID Report Reference characteristic descriptor, batter level input
#if USE_HID_BATT		
    {
        { ATT_BT_UUID_SIZE, reportRefUUID },
        GATT_PERMIT_READ,
        0,
        hidReportRefBattLevel
    },
#endif		
#ifdef HID_VOICE_SPEC

    // Characteristic Presentation format
    {
        { ATT_BT_UUID_SIZE, charFormatUUID },
        GATT_PERMIT_READ,
        0,
        (uint8_t*)& battLevelPresentation
    },
#endif
};


/*********************************************************************
    LOCAL FUNCTIONS
*/
static uint8 battReadAttrCB( uint16 connHandle, gattAttribute_t* pAttr,
                             uint8* pValue, uint16* pLen, uint16 offset, uint8 maxLen );
static bStatus_t battWriteAttrCB( uint16 connHandle, gattAttribute_t* pAttr,
                                  uint8* pValue, uint16 len, uint16 offset );
static void battNotifyCB( linkDBItem_t* pLinkItem );
//static void battNotifyLevel( void );

/*********************************************************************
    PROFILE CALLBACKS
*/
// Battery Service Callbacks
CONST gattServiceCBs_t battCBs =
{
    battReadAttrCB,  // Read callback function pointer
    battWriteAttrCB, // Write callback function pointer
    NULL             // Authorization callback function pointer
};

/*********************************************************************
    PUBLIC FUNCTIONS
*/

/*********************************************************************
    @fn      Batt_AddService

    @brief   Initializes the Battery Service by registering
            GATT attributes with the GATT server.

    @return  Success or Failure
*/
bStatus_t Batt_AddService( void )
{
    uint8 status = SUCCESS;
    // Initialize Client Characteristic Configuration attributes
    GATTServApp_InitCharCfg( INVALID_CONNHANDLE, battLevelClientCharCfg );
    // Register GATT attribute list and CBs with GATT Server App
    status = GATTServApp_RegisterService( battAttrTbl,
                                          GATT_NUM_ATTRS( battAttrTbl ),
                                          &battCBs );
    return ( status );
}

/*********************************************************************
    @fn      Batt_Register

    @brief   Register a callback function with the Battery Service.

    @param   pfnServiceCB - Callback function.

    @return  None.
*/
extern void Batt_Register( battServiceCB_t pfnServiceCB )
{
    battServiceCB = pfnServiceCB;
}

/*********************************************************************
    @fn          battReadAttrCB

    @brief       Read an attribute.

    @param       connHandle - connection message was received on
    @param       pAttr - pointer to attribute
    @param       pValue - pointer to data to be read
    @param       pLen - length of data to be read
    @param       offset - offset of the first octet to be read
    @param       maxLen - maximum length of data to be read

    @return      Success or Failure
*/
static uint8 battReadAttrCB( uint16 connHandle, gattAttribute_t* pAttr,
                             uint8* pValue, uint16* pLen, uint16 offset, uint8 maxLen )
{
    bStatus_t status = SUCCESS;

    // Make sure it's not a blob operation (no attributes in the profile are long)
    if ( offset > 0 )
    {
        return ( ATT_ERR_ATTR_NOT_LONG );
    }

    uint16 uuid = BUILD_UINT16( pAttr->type.uuid[0], pAttr->type.uuid[1] );

    // Measure battery level if reading level
    if ( uuid == BATT_LEVEL_UUID )
    {
				*pLen = 1;
        pValue[0] = measured_data.battery;
    }
#if USE_HID_BATT			
    else if ( uuid == GATT_REPORT_REF_UUID )
    {
        *pLen = HID_REPORT_REF_LEN;
        osal_memcpy( pValue, pAttr->pValue, HID_REPORT_REF_LEN );
    }
#endif		
    else
    {
        status = ATT_ERR_ATTR_NOT_FOUND;
    }

    return ( status );
}

/*********************************************************************
    @fn      battWriteAttrCB

    @brief   Validate attribute data prior to a write operation

    @param   connHandle - connection message was received on
    @param   pAttr - pointer to attribute
    @param   pValue - pointer to data to be written
    @param   len - length of data
    @param   offset - offset of the first octet to be written

    @return  Success or Failure
*/
static bStatus_t battWriteAttrCB( uint16 connHandle, gattAttribute_t* pAttr,
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

            if ( battServiceCB )
            {
                (*battServiceCB)( (charCfg == GATT_CFG_NO_OPERATION) ?
                                  BATT_LEVEL_NOTI_DISABLED :
                                  BATT_LEVEL_NOTI_ENABLED);
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
    @fn          battNotifyCB

    @brief       Send a notification of the level state characteristic.

    @param       connHandle - linkDB item

    @return      None.
*/
static void battNotifyCB( linkDBItem_t* pLinkItem )
{
    if ( pLinkItem->stateFlags & LINK_CONNECTED )
    {
        uint16 value = GATTServApp_ReadCharCfg( pLinkItem->connectionHandle,
                                                battLevelClientCharCfg );

        if ( value & GATT_CLIENT_CFG_NOTIFY )
        {
            attHandleValueNoti_t noti;
            noti.handle = battAttrTbl[BATT_LEVEL_VALUE_IDX].handle;
            noti.len = 1;
            noti.value[0] = measured_data.battery;
            GATT_Notification( pLinkItem->connectionHandle, &noti, FALSE );
        }
    }
}

/*********************************************************************
    @fn      battNotifyLevelState

    @brief   Send a notification of the battery level state
            characteristic if a connection is established.

    @return  None.
*/
void BattNotifyLevel( void )
{
    // Execute linkDB callback to send notification
    linkDB_PerformFunc( battNotifyCB );
}

/*********************************************************************
    @fn          Batt_HandleConnStatusCB

    @brief       Battery Service link status change handler function.

    @param       connHandle - connection handle
    @param       changeType - type of change

    @return      none
*/
void Batt_HandleConnStatusCB( uint16 connHandle, uint8 changeType )
{
    // Make sure this is not loopback connection
    if ( connHandle != LOOPBACK_CONNHANDLE )
    {
        // Reset Client Char Config if connection has dropped
        if ( ( changeType == LINKDB_STATUS_UPDATE_REMOVED )      ||
                ( ( changeType == LINKDB_STATUS_UPDATE_STATEFLAGS ) &&
                  ( !linkDB_Up( connHandle ) ) ) )
        {
            GATTServApp_InitCharCfg( connHandle, battLevelClientCharCfg );
        }
    }
}


/*********************************************************************
*********************************************************************/
