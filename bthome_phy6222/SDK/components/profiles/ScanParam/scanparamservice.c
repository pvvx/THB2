/*************
 SDK_LICENSE
***************/
/*********************************************************************
    INCLUDES
*/
#include "bcomdef.h"
#include "OSAL.h"
#include "att.h"
#include "gatt.h"
#include "gatt_uuid.h"
#include "gattservapp.h"
#include "gatt_profile_uuid.h"
#include "gapbondmgr.h"
#include "linkdb.h"
#include "scanparamservice.h"

/*********************************************************************
    MACROS
*/

/*********************************************************************
    CONSTANTS
*/

/*********************************************************************
    TYPEDEFS
*/

/*********************************************************************
    GLOBAL VARIABLES
*/
// Scan parameters service
CONST uint8 scanParamServUUID[ATT_BT_UUID_SIZE] =
{
    LO_UINT16(SCAN_PARAM_SERV_UUID), HI_UINT16(SCAN_PARAM_SERV_UUID)
};

// Scan interval window characteristic
CONST uint8 scanIntervalWindowUUID[ATT_BT_UUID_SIZE] =
{
    LO_UINT16(SCAN_INTERVAL_WINDOW_UUID), HI_UINT16(SCAN_INTERVAL_WINDOW_UUID)
};

// Scan parameter refresh characteristic
CONST uint8 scanParamRefreshUUID[ATT_BT_UUID_SIZE] =
{
    LO_UINT16(SCAN_REFRESH_UUID), HI_UINT16(SCAN_REFRESH_UUID)
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
static scanParamServiceCB_t scanParamServiceCB;

/*********************************************************************
    Profile Attributes - variables
*/

// Scan Parameters Service attribute
static CONST gattAttrType_t scanParamService = { ATT_BT_UUID_SIZE, scanParamServUUID };

// Scan Interval Window characteristic
static uint8 scanIntervalWindowProps = GATT_PROP_WRITE_NO_RSP;
static uint8 scanIntervalWindow[SCAN_INTERVAL_WINDOW_CHAR_LEN];

// Scan Parameter Refresh characteristic
static uint8 scanParamRefreshProps = GATT_PROP_NOTIFY;
static uint8 scanParamRefresh[SCAN_PARAM_REFRESH_LEN];
static gattCharCfg_t scanParamRefreshClientCharCfg[GATT_MAX_NUM_CONN];

/*********************************************************************
    Profile Attributes - Table
*/

static gattAttribute_t scanParamAttrTbl[] =
{
    // Scan Parameters Service attribute
    {
        { ATT_BT_UUID_SIZE, primaryServiceUUID }, /* type */
        GATT_PERMIT_READ,                         /* permissions */
        0,                                        /* handle */
        (uint8*)& scanParamService                /* pValue */
    },

    // Scan Interval Window declaration
    {
        { ATT_BT_UUID_SIZE, characterUUID },
        GATT_PERMIT_READ,
        0,
        &scanIntervalWindowProps
    },

    // Scan Interval Window characteristic
    {
        { ATT_BT_UUID_SIZE, scanIntervalWindowUUID },
        GATT_PERMIT_ENCRYPT_WRITE,
        0,
        scanIntervalWindow
    },

    // Scan Parameter Refresh declaration
    {
        { ATT_BT_UUID_SIZE, characterUUID },
        GATT_PERMIT_READ,
        0,
        &scanParamRefreshProps
    },

    // Scan Parameter Refresh characteristic
    {
        { ATT_BT_UUID_SIZE, scanParamRefreshUUID },
        0,
        0,
        scanParamRefresh
    },

    // Scan Parameter Refresh characteristic client characteristic configuration
    {
        { ATT_BT_UUID_SIZE, clientCharCfgUUID },
        GATT_PERMIT_READ | GATT_PERMIT_ENCRYPT_WRITE,
        0,
        (uint8*)& scanParamRefreshClientCharCfg
    }
};

// Attribute index enumeration-- these indexes match array elements above
enum
{
    SCAN_PARAM_SERVICE_IDX,           // Scan Parameters Service
    SCAN_PARAM_INTERVAL_DECL_IDX,     // Scan Interval Window declaration
    SCAN_PARAM_INTERVAL_IDX,          // Scan Interval Window characteristic
    SCAN_PARAM_REFRESH_DECL_IDX,      // Scan Parameter Refresh declaration
    SCAN_PARAM_REFRESH_IDX,           // Scan Parameter Refresh characteristic
    SCAN_PARAM_REFRESH_CCCD_IDX       // Scan Parameter Refresh characteristic client characteristic configuration
};


/*********************************************************************
    LOCAL FUNCTIONS
*/
static bStatus_t scanParamWriteAttrCB( uint16 connHandle, gattAttribute_t* pAttr,
                                       uint8* pValue, uint16 len, uint16 offset );
static uint8 scanParamReadAttrCB( uint16 connHandle, gattAttribute_t* pAttr,
                                  uint8* pValue, uint16* pLen, uint16 offset, uint8 maxLen );

/*********************************************************************
    PROFILE CALLBACKS
*/

// Service Callbacks
CONST gattServiceCBs_t scanParamCBs =
{
    scanParamReadAttrCB,  // Read callback function pointer
    scanParamWriteAttrCB, // Write callback function pointer
    NULL                  // Authorization callback function pointer
};

/*********************************************************************
    PUBLIC FUNCTIONS
*/

/*********************************************************************
    @fn      ScanParam_AddService

    @brief   Initializes the Battery Service by registering
            GATT attributes with the GATT server.

    @return  Success or Failure
*/
bStatus_t ScanParam_AddService( void )
{
    uint8 status = SUCCESS;
    // Initialize Client Characteristic Configuration attributes
    GATTServApp_InitCharCfg( INVALID_CONNHANDLE, scanParamRefreshClientCharCfg );
    // Register GATT attribute list and CBs with GATT Server App
    status = GATTServApp_RegisterService( scanParamAttrTbl, GATT_NUM_ATTRS( scanParamAttrTbl ),
                                          &scanParamCBs );
    return ( status );
}

/*********************************************************************
    @fn      ScanParam_Register

    @brief   Register a callback function with the Battery Service.

    @param   pfnServiceCB - Callback function.

    @return  None.
*/
extern void ScanParam_Register( scanParamServiceCB_t pfnServiceCB )
{
    scanParamServiceCB = pfnServiceCB;
}

/*********************************************************************
    @fn      ScanParam_SetParameter

    @brief   Set a Battery Service parameter.

    @param   param - Profile parameter ID
    @param   len - length of data to right
    @param   value - pointer to data to write.  This is dependent on
            the parameter ID and WILL be cast to the appropriate
            data type (example: data type of uint16 will be cast to
            uint16 pointer).

    @return  bStatus_t
*/
bStatus_t ScanParam_SetParameter( uint8 param, uint8 len, void* value )
{
    bStatus_t ret = SUCCESS;

    switch ( param )
    {
    default:
        ret = INVALIDPARAMETER;
        break;
    }

    return ( ret );
}

/*********************************************************************
    @fn      ScanParam_GetParameter

    @brief   Get a Battery Service parameter.

    @param   param - Profile parameter ID
    @param   value - pointer to data to get.  This is dependent on
            the parameter ID and WILL be cast to the appropriate
            data type (example: data type of uint16 will be cast to
            uint16 pointer).

    @return  bStatus_t
*/
bStatus_t ScanParam_GetParameter( uint8 param, void* value )
{
    bStatus_t ret = SUCCESS;

    switch ( param )
    {
    case SCAN_PARAM_PARAM_INTERVAL:
        *((uint16*)value) = BUILD_UINT16(scanIntervalWindow[0],
                                         scanIntervalWindow[1]);
        break;

    case SCAN_PARAM_PARAM_WINDOW:
        *((uint16*)value) = BUILD_UINT16(scanIntervalWindow[2],
                                         scanIntervalWindow[3]);
        break;

    default:
        ret = INVALIDPARAMETER;
        break;
    }

    return ( ret );
}

/*********************************************************************
    @fn      ScanParam_RefreshNotify

    @brief   Notify the peer to refresh the scan parameters.

    @param   connHandle - connection handle

    @return  None
*/
void ScanParam_RefreshNotify( uint16 connHandle )
{
    attHandleValueNoti_t  noti;
    uint16 value;
    value  = GATTServApp_ReadCharCfg( connHandle, scanParamRefreshClientCharCfg );

    if ( value & GATT_CLIENT_CFG_NOTIFY )
    {
        // send notification
        noti.handle = scanParamAttrTbl[SCAN_PARAM_REFRESH_CCCD_IDX].handle;
        noti.len = SCAN_PARAM_REFRESH_LEN;
        noti.value[0] = SCAN_PARAM_REFRESH_REQ;
        GATT_Notification( connHandle, &noti, FALSE );
    }
}

/*********************************************************************
    @fn          scanParamReadAttrCB

    @brief       GATT read callback.

    @param       connHandle - connection message was received on
    @param       pAttr - pointer to attribute
    @param       pValue - pointer to data to be read
    @param       pLen - length of data to be read
    @param       offset - offset of the first octet to be read
    @param       maxLen - maximum length of data to be read

    @return      Success or Failure
*/
static uint8 scanParamReadAttrCB( uint16 connHandle, gattAttribute_t* pAttr,
                                  uint8* pValue, uint16* pLen, uint16 offset, uint8 maxLen )
{
    bStatus_t   status = SUCCESS;
    return ( status );
}

/*********************************************************************
    @fn      scanParamWriteAttrCB

    @brief   Validate attribute data prior to a write operation

    @param   connHandle - connection message was received on
    @param   pAttr - pointer to attribute
    @param   pValue - pointer to data to be written
    @param   len - length of data
    @param   offset - offset of the first octet to be written

    @return  Success or Failure
*/
static bStatus_t scanParamWriteAttrCB( uint16 connHandle, gattAttribute_t* pAttr,
                                       uint8* pValue, uint16 len, uint16 offset )
{
    bStatus_t status = SUCCESS;

    // Make sure it's not a blob operation (no attributes in the profile are long)
    if ( offset > 0 )
    {
        return ( ATT_ERR_ATTR_NOT_LONG );
    }

    uint16 uuid = BUILD_UINT16( pAttr->type.uuid[0], pAttr->type.uuid[1]);

    // Only one writeable attribute
    if ( uuid == SCAN_INTERVAL_WINDOW_UUID )
    {
        if ( len == SCAN_INTERVAL_WINDOW_CHAR_LEN )
        {
            uint16 interval = BUILD_UINT16( pValue[0], pValue[1] );
            uint16 window = BUILD_UINT16( pValue[0], pValue[1] );

            // Validate values
            if ( window <= interval )
            {
                osal_memcpy( pAttr->pValue, pValue, len );
                (*scanParamServiceCB)( SCAN_INTERVAL_WINDOW_SET );
            }
            else
            {
                status = ATT_ERR_INVALID_VALUE;
            }
        }
        else
        {
            status = ATT_ERR_INVALID_VALUE_SIZE;
        }
    }
    else if ( uuid == GATT_CLIENT_CHAR_CFG_UUID )
    {
        status = GATTServApp_ProcessCCCWriteReq( connHandle, pAttr, pValue, len,
                                                 offset, GATT_CLIENT_CFG_NOTIFY );
    }
    else
    {
        status = ATT_ERR_ATTR_NOT_FOUND;
    }

    return ( status );
}

/*********************************************************************
    @fn          ScanParam_HandleConnStatusCB

    @brief       Service link status change handler function.

    @param       connHandle - connection handle
    @param       changeType - type of change

    @return      none
*/
void ScanParam_HandleConnStatusCB( uint16 connHandle, uint8 changeType )
{
    // Make sure this is not loopback connection
    if ( connHandle != LOOPBACK_CONNHANDLE )
    {
        // Reset Client Char Config if connection has dropped
        if ( ( changeType == LINKDB_STATUS_UPDATE_REMOVED )      ||
                ( ( changeType == LINKDB_STATUS_UPDATE_STATEFLAGS ) &&
                  ( !linkDB_Up( connHandle ) ) ) )
        {
            GATTServApp_InitCharCfg( connHandle, scanParamRefreshClientCharCfg );
        }
    }
}
