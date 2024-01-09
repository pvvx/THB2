/**************************************************************************************************
    Filename:       simplekey.c
    Revised:
    Revision:

    Description:    Simple Keys Profile

 SDK_LICENSE

**************************************************************************************************/

/*********************************************************************
    INCLUDES
*/
#include "bcomdef.h"
#include "OSAL.h"
#include "linkdb.h"
#include "att.h"
#include "gatt.h"
#include "gatt_uuid.h"
#include "gattservapp.h"
#include "gapbondmgr.h"

#include "simplekeys.h"

/*********************************************************************
    MACROS
*/

/*********************************************************************
    CONSTANTS
*/

#define SERVAPP_NUM_ATTR_SUPPORTED        5

/*********************************************************************
    TYPEDEFS
*/

/*********************************************************************
    GLOBAL VARIABLES
*/
// SK Service UUID: 0x1800
CONST uint8 skServUUID[ATT_BT_UUID_SIZE] =
{
    LO_UINT16(SK_SERV_UUID), HI_UINT16(SK_SERV_UUID)
};

// Key Pressed UUID: 0x1801
CONST uint8 keyPressedUUID[ATT_BT_UUID_SIZE] =
{
    LO_UINT16(SK_KEYPRESSED_UUID), HI_UINT16(SK_KEYPRESSED_UUID)
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

/*********************************************************************
    Profile Attributes - variables
*/

// SK Service attribute
static CONST gattAttrType_t skService = { ATT_BT_UUID_SIZE, skServUUID };

// Keys Pressed Characteristic Properties
static uint8 skCharProps = GATT_PROP_NOTIFY;

// Key Pressed State Characteristic
static uint8 skKeyPressed = 0;

// Key Pressed Characteristic Configs
static gattCharCfg_t skConfig[GATT_MAX_NUM_CONN];

// Key Pressed Characteristic User Description
static uint8 skCharUserDesp[16] = "Key Press State\0";


/*********************************************************************
    Profile Attributes - Table
*/

static gattAttribute_t simplekeysAttrTbl[SERVAPP_NUM_ATTR_SUPPORTED] =
{
    // Simple Keys Service
    {
        { ATT_BT_UUID_SIZE, primaryServiceUUID }, /* type */
        GATT_PERMIT_READ,                         /* permissions */
        0,                                        /* handle */
        (uint8*)& skService                       /* pValue */
    },

    // Characteristic Declaration for Keys
    {
        { ATT_BT_UUID_SIZE, characterUUID },
        GATT_PERMIT_READ,
        0,
        &skCharProps
    },

    // Characteristic Value- Key Pressed
    {
        { ATT_BT_UUID_SIZE, keyPressedUUID },
        0,
        0,
        &skKeyPressed
    },

    // Characteristic configuration
    {
        { ATT_BT_UUID_SIZE, clientCharCfgUUID },
        GATT_PERMIT_READ | GATT_PERMIT_WRITE,
        0,
        (uint8*)skConfig
    },

    // Characteristic User Description
    {
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ,
        0,
        skCharUserDesp
    },
};


/*********************************************************************
    LOCAL FUNCTIONS
*/
static uint8 sk_ReadAttrCB( uint16 connHandle, gattAttribute_t* pAttr,
                            uint8* pValue, uint16* pLen, uint16 offset, uint8 maxLen );
static bStatus_t sk_WriteAttrCB( uint16 connHandle, gattAttribute_t* pAttr,
                                 uint8* pValue, uint16 len, uint16 offset );

static void sk_HandleConnStatusCB( uint16 connHandle, uint8 changeType );

/*********************************************************************
    PROFILE CALLBACKS
*/
// SK Service Callbacks
CONST gattServiceCBs_t skCBs =
{
    sk_ReadAttrCB,  // Read callback function pointer
    sk_WriteAttrCB, // Write callback function pointer
    NULL            // Authorization callback function pointer
};

/*********************************************************************
    PUBLIC FUNCTIONS
*/

/*********************************************************************
    @fn      SK_AddService

    @brief   Initializes the Simple Key service by registering
            GATT attributes with the GATT server.

    @param   services - services to add. This is a bit map and can
                       contain more than one service.

    @return  Success or Failure
*/
bStatus_t SK_AddService( uint32 services )
{
    uint8 status = SUCCESS;
    // Initialize Client Characteristic Configuration attributes
    GATTServApp_InitCharCfg( INVALID_CONNHANDLE, skConfig );
    // Register with Link DB to receive link status change callback
    VOID linkDB_Register( sk_HandleConnStatusCB );

    if ( services & SK_SERVICE )
    {
        // Register GATT attribute list and CBs with GATT Server App
        status = GATTServApp_RegisterService( simplekeysAttrTbl,
                                              GATT_NUM_ATTRS( simplekeysAttrTbl ),
                                              &skCBs );
    }

    return ( status );
}

/*********************************************************************
    @fn      SK_SetParameter

    @brief   Set a Simple Key Profile parameter.

    @param   param - Profile parameter ID
    @param   len - length of data to right
    @param   pValue - pointer to data to write.  This is dependent on
            the parameter ID and WILL be cast to the appropriate
            data type (example: data type of uint16 will be cast to
            uint16 pointer).

    @return  bStatus_t
*/
bStatus_t SK_SetParameter( uint8 param, uint8 len, void* pValue )
{
    bStatus_t ret = SUCCESS;

    switch ( param )
    {
    case SK_KEY_ATTR:
        if ( len == sizeof ( uint8 ) )
        {
            skKeyPressed = *((uint8*)pValue);
            // See if Notification/Indication has been enabled
            GATTServApp_ProcessCharCfg( skConfig, &skKeyPressed, FALSE,
                                        simplekeysAttrTbl, GATT_NUM_ATTRS( simplekeysAttrTbl ),
                                        INVALID_TASK_ID );
        }
        else
        {
            ret = bleInvalidRange;
        }

        break;

    default:
        ret = INVALIDPARAMETER;
        break;
    }

    return ( ret );
}

/*********************************************************************
    @fn      SK_GetParameter

    @brief   Get a Simple Key Profile parameter.

    @param   param - Profile parameter ID
    @param   pValue - pointer to data to put.  This is dependent on
            the parameter ID and WILL be cast to the appropriate
            data type (example: data type of uint16 will be cast to
            uint16 pointer).

    @return  bStatus_t
*/
bStatus_t SK_GetParameter( uint8 param, void* pValue )
{
    bStatus_t ret = SUCCESS;

    switch ( param )
    {
    case SK_KEY_ATTR:
        *((uint8*)pValue) = skKeyPressed;
        break;

    default:
        ret = INVALIDPARAMETER;
        break;
    }

    return ( ret );
}

/*********************************************************************
    @fn          sk_ReadAttrCB

    @brief       Read an attribute.

    @param       connHandle - connection message was received on
    @param       pAttr - pointer to attribute
    @param       pValue - pointer to data to be read
    @param       pLen - length of data to be read
    @param       offset - offset of the first octet to be read
    @param       maxLen - maximum length of data to be read

    @return      Success or Failure
*/
static uint8 sk_ReadAttrCB( uint16 connHandle, gattAttribute_t* pAttr,
                            uint8* pValue, uint16* pLen, uint16 offset, uint8 maxLen )
{
    bStatus_t status = SUCCESS;

    // Make sure it's not a blob operation (no attributes in the profile are long
    if ( offset > 0 )
    {
        return ( ATT_ERR_ATTR_NOT_LONG );
    }

    if ( pAttr->type.len == ATT_BT_UUID_SIZE )
    {
        // 16-bit UUID
        uint16 uuid = BUILD_UINT16( pAttr->type.uuid[0], pAttr->type.uuid[1]);

        switch ( uuid )
        {
        // No need for "GATT_SERVICE_UUID" or "GATT_CLIENT_CHAR_CFG_UUID" cases;
        // gattserverapp handles this type for reads

        // simple keys characteristic does not have read permissions, but because it
        //   can be sent as a notification, it must be included here
        case SK_KEYPRESSED_UUID:
            *pLen = 1;
            pValue[0] = *pAttr->pValue;
            break;

        default:
            // Should never get here!
            *pLen = 0;
            status = ATT_ERR_ATTR_NOT_FOUND;
            break;
        }
    }
    else
    {
        // 128-bit UUID
        *pLen = 0;
        status = ATT_ERR_INVALID_HANDLE;
    }

    return ( status );
}

/*********************************************************************
    @fn      sk_WriteAttrCB

    @brief   Validate attribute data prior to a write operation

    @param   connHandle - connection message was received on
    @param   pAttr - pointer to attribute
    @param   pValue - pointer to data to be written
    @param   len - length of data
    @param   offset - offset of the first octet to be written

    @return  Success or Failure
*/
static bStatus_t sk_WriteAttrCB( uint16 connHandle, gattAttribute_t* pAttr,
                                 uint8* pValue, uint16 len, uint16 offset )
{
    bStatus_t status = SUCCESS;

    if ( pAttr->type.len == ATT_BT_UUID_SIZE )
    {
        // 16-bit UUID
        uint16 uuid = BUILD_UINT16( pAttr->type.uuid[0], pAttr->type.uuid[1]);

        switch ( uuid )
        {
        case GATT_CLIENT_CHAR_CFG_UUID:
            status = GATTServApp_ProcessCCCWriteReq( connHandle, pAttr, pValue, len,
                                                     offset, GATT_CLIENT_CFG_NOTIFY );
            break;

        default:
            // Should never get here!
            status = ATT_ERR_ATTR_NOT_FOUND;
            break;
        }
    }
    else
    {
        // 128-bit UUID
        status = ATT_ERR_INVALID_HANDLE;
    }

    return ( status );
}

/*********************************************************************
    @fn          sk_HandleConnStatusCB

    @brief       Simple Keys Profile link status change handler function.

    @param       connHandle - connection handle
    @param       changeType - type of change

    @return      none
*/
static void sk_HandleConnStatusCB( uint16 connHandle, uint8 changeType )
{
    // Make sure this is not loopback connection
    if ( connHandle != LOOPBACK_CONNHANDLE )
    {
        // Reset Client Char Config if connection has dropped
        if ( ( changeType == LINKDB_STATUS_UPDATE_REMOVED )      ||
                ( ( changeType == LINKDB_STATUS_UPDATE_STATEFLAGS ) &&
                  ( !linkDB_Up( connHandle ) ) ) )
        {
            GATTServApp_InitCharCfg( connHandle, skConfig );
        }
    }
}


/*********************************************************************
*********************************************************************/
