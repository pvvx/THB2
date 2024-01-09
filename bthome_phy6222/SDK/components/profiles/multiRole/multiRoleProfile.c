/**************************************************************************************************
    Filename:       multiRoleProfile.c
    Revised:
    Revision:

    Description:    This file contains the Simple GATT profile sample GATT service
                  profile for use with the BLE sample application.

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
#include "multiRoleProfile.h"
#include "log.h"
#include "multi.h"


/*********************************************************************
    MACROS
*/

/*********************************************************************
    CONSTANTS
*/

#define SERVAPP_NUM_ATTR_SUPPORTED        8

/*********************************************************************
    TYPEDEFS
*/

/*********************************************************************
    GLOBAL VARIABLES
*/
#if(MAX_CONNECTION_SLAVE_NUM > 0 )
CONST uint8 multiProfileServUUID[ATT_UUID_SIZE] =
{
    0x6E, 0x40, 0x00, 0x01, 0xB5, 0xA3, 0xF3, 0x93, 0xE0, 0xA9, 0xE5, 0x0E, 0x24, 0xDC, 0x41,0x79
};

CONST uint8 multiProfilechar1UUID[ATT_UUID_SIZE] =
{
    0x79, 0x41, 0xDC, 0x24, 0x0E, 0xE5, 0xa9, 0xe0, 0x93, 0xf3, 0xa3, 0xb5, 0x02, 0x00, 0x40,0x6e
};

CONST uint8 multiProfilechar2UUID[ATT_UUID_SIZE] =
{
    0x79, 0x41, 0xDC, 0x24, 0x0E, 0xE5, 0xa9, 0xe0, 0x93, 0xf3, 0xa3, 0xb5, 0x03, 0x00, 0x40,0x6e
};
#endif

/*********************************************************************
    EXTERNAL VARIABLES
*/

/*********************************************************************
    EXTERNAL FUNCTIONS
*/

/*********************************************************************
    LOCAL VARIABLES
*/
#if(MAX_CONNECTION_SLAVE_NUM > 0 )
    static multiProfileCBs_t* multiProfile_AppCBs = NULL;
#endif
/*********************************************************************
    Profile Attributes - variables
*/
#if(MAX_CONNECTION_SLAVE_NUM > 0 )
// multi Profile Service attribute
static CONST gattAttrType_t multiProfileService = { ATT_UUID_SIZE, multiProfileServUUID };

// multi Profile Characteristic 1 Properties
static uint8 multiProfileChar1Props = GATT_PROP_WRITE;

// Characteristic 1 Value
static uint8 multiProfileChar1[ATT_MTU_SIZE];

// multi Profile Characteristic 1 User Description
static uint8 multiProfileChar1UserDesp[] = "Commmon TXRX\0";

// multi Profile Characteristic 1 Properties
static uint8 multiProfileChar2Props = GATT_PROP_READ | GATT_PROP_NOTIFY;

// Characteristic 2 Value
static uint8 multiProfileChar2[ATT_MTU_SIZE];
static uint8 multiChar2NotifyLen = 0;

// multi Profile Characteristic 6 User Description
static uint8 multiProfileChar2UserDesp[] = "NOTIFY\0";
// multi Profile Characteristic 6 Configuration Each client has its own
// instantiation of the Client Characteristic Configuration. Reads of the
// Client Characteristic Configuration only shows the configuration for
// that client and writes only affect the configuration of that client.
static gattCharCfg_t multiProfileChar2Config[MAX_NUM_LL_CONN];
#endif


/*********************************************************************
    Profile Attributes - Table
*/
#if(MAX_CONNECTION_SLAVE_NUM > 0 )
static gattAttribute_t multiProfileAttrTbl[SERVAPP_NUM_ATTR_SUPPORTED] =
{
    // multi Profile Service
    {
        { ATT_BT_UUID_SIZE, primaryServiceUUID }, /* type */
        GATT_PERMIT_READ,                         /* permissions */
        0,                                        /* handle */
        (uint8*)& multiProfileService            /* pValue */
    },

    // Characteristic 1 Declaration
    {
        { ATT_BT_UUID_SIZE, characterUUID },
        GATT_PERMIT_READ,
        0,
        &multiProfileChar1Props
    },

    // Characteristic Value 1
    {
        { ATT_UUID_SIZE, multiProfilechar1UUID },
        GATT_PERMIT_READ | GATT_PERMIT_WRITE,
        0,
        &multiProfileChar1[0]
    },

    // Characteristic 1 User Description
    {
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ,
        0,
        multiProfileChar1UserDesp
    },

    // ----------------------------------------------------------------------
    // Characteristic 2 Declaration, NOTify
    {
        { ATT_BT_UUID_SIZE, characterUUID },
        GATT_PERMIT_READ,
        0,
        &multiProfileChar2Props
    },

    // Characteristic Value 2
    {
        { ATT_UUID_SIZE, multiProfilechar2UUID },
        GATT_PERMIT_READ,
        0,
        (uint8*)& multiProfileChar2
    },

    // Characteristic 2 configuration
    {
        { ATT_BT_UUID_SIZE, clientCharCfgUUID },
        GATT_PERMIT_READ | GATT_PERMIT_WRITE,
        0,
        (uint8*)multiProfileChar2Config
    },

    // Characteristic 2 User Description
    {
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ,
        0,
        multiProfileChar2UserDesp
    },
};

/*********************************************************************
    LOCAL FUNCTIONS
*/
static uint8 multiProfile_ReadAttrCB( uint16 connHandle, gattAttribute_t* pAttr,
                                      uint8* pValue, uint16* pLen, uint16 offset, uint8 maxLen );
static bStatus_t multiProfile_WriteAttrCB( uint16 connHandle, gattAttribute_t* pAttr,
                                           uint8* pValue, uint16 len, uint16 offset );

static void multiProfile_HandleConnStatusCB( uint16 connHandle, uint8 changeType );


/*********************************************************************
    PROFILE CALLBACKS
*/
// multi Profile Service Callbacks
CONST gattServiceCBs_t multiProfileCBs =
{
    multiProfile_ReadAttrCB,  // Read callback function pointer
    multiProfile_WriteAttrCB, // Write callback function pointer
    NULL                       // Authorization callback function pointer
};
#endif

/*********************************************************************
    PUBLIC FUNCTIONS
*/

/*********************************************************************
    @fn      multiProfile_AddService

    @brief   Initializes the multi Profile service by registering
            GATT attributes with the GATT server.

    @param   services - services to add. This is a bit map and can
                       contain more than one service.

    @return  Success or Failure
*/
#if(MAX_CONNECTION_SLAVE_NUM > 0 )
bStatus_t MultiProfile_AddService( uint32 services )
{
    uint8 status = SUCCESS;
    // Initialize Client Characteristic Configuration attributes
    GATTServApp_InitCharCfg( INVALID_CONNHANDLE, multiProfileChar2Config );
    // Register with Link DB to receive link status change callback
    linkDB_Register( multiProfile_HandleConnStatusCB );
    // Register GATT attribute list and CBs with GATT Server App
    status = GATTServApp_RegisterService( multiProfileAttrTbl,
                                          GATT_NUM_ATTRS( multiProfileAttrTbl ),
                                          &multiProfileCBs );
    return ( status );
}


/*********************************************************************
    @fn      multiProfile_RegisterAppCBs

    @brief   Registers the application callback function. Only call
            this function once.

    @param   callbacks - pointer to application callbacks.

    @return  SUCCESS or bleAlreadyInRequestedMode
*/
bStatus_t MultiProfile_RegisterAppCBs( multiProfileCBs_t* appCallbacks )
{
    if ( appCallbacks )
    {
        multiProfile_AppCBs = appCallbacks;
        return ( SUCCESS );
    }
    else
    {
        return ( bleAlreadyInRequestedMode );
    }
}


/*********************************************************************
    @fn      multiProfile_SetParameter

    @brief   Set a multi Profile parameter.

    @param   param - Profile parameter ID
    @param   len - length of data to right
    @param   value - pointer to data to write.  This is dependent on
            the parameter ID and WILL be cast to the appropriate
            data type (example: data type of uint16 will be cast to
            uint16 pointer).

    @return  bStatus_t
*/
bStatus_t MultiProfile_SetParameter( uint8 param, uint8 len, void* value )
{
    bStatus_t ret = SUCCESS;

    switch ( param )
    {
    case MULTIPROFILE_CHAR1:
    {
        if ( len <= ATT_MTU_SIZE )
        {
            osal_memcpy(multiProfileChar1, value, len);
        }
        else
        {
            ret = bleInvalidRange;
        }
    }
    break;

    case MULTIPROFILE_CHAR2:
    {
        if ( len <= ATT_MTU_SIZE )
        {
            osal_memcpy(multiProfileChar2, value, len);
        }
        else
        {
            ret = bleInvalidRange;
        }
    }
    break;

    default:
        ret = INVALIDPARAMETER;
        break;
    }

    return ( ret );
}

/*********************************************************************
    @fn      multiProfile_GetParameter

    @brief   Get a multi Profile parameter.

    @param   param - Profile parameter ID
    @param   value - pointer to data to put.  This is dependent on
            the parameter ID and WILL be cast to the appropriate
            data type (example: data type of uint16 will be cast to
            uint16 pointer).

    @return  bStatus_t
*/
bStatus_t MultiProfile_GetParameter( uint16 connHandle,uint8 param, void* value )
{
    bStatus_t ret = SUCCESS;

    switch ( param )
    {
    case MULTIPROFILE_CHAR1:
        VOID osal_memcpy( value, multiProfileChar1, ATT_GetCurrentMTUSize( connHandle )-3 );
        break;

    case MULTIPROFILE_CHAR2:
        VOID osal_memcpy( value, multiProfileChar2, ATT_GetCurrentMTUSize( connHandle )-3 );
        break;

    default:
        ret = INVALIDPARAMETER;
        break;
    }

    return ( ret );
}

/*********************************************************************
    @fn          multiProfile_ReadAttrCB

    @brief       Read an attribute.

    @param       connHandle - connection message was received on
    @param       pAttr - pointer to attribute
    @param       pValue - pointer to data to be read
    @param       pLen - length of data to be read
    @param       offset - offset of the first octet to be read
    @param       maxLen - maximum length of data to be read

    @return      Success or Failure
*/
static uint8 multiProfile_ReadAttrCB( uint16 connHandle, gattAttribute_t* pAttr,
                                      uint8* pValue, uint16* pLen, uint16 offset, uint8 maxLen )
{
    bStatus_t status = SUCCESS;

//  LOG("%s connHandle %d,pAttr->type.len %d\n",__func__,connHandle,pAttr->type.len);
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

    // 16B UUID
    if ( pAttr->type.len == ATT_UUID_SIZE )
    {
        switch ( pAttr->type.uuid[ ATT_UUID_SIZE - 4 ] )
        {
        // No need for "GATT_SERVICE_UUID" or "GATT_CLIENT_CHAR_CFG_UUID" cases;
        // gattserverapp handles those reads
        case MULTIPROFILE_CHAR1:
        case MULTIPROFILE_CHAR2:
            *pLen = multiChar2NotifyLen;
            VOID osal_memcpy( pValue, pAttr->pValue, ATT_MTU_SIZE );
            break;

        default:
            // Should never get here! (characteristics 3 and 4 do not have read permissions)
            *pLen = 0;
            status = ATT_ERR_ATTR_NOT_FOUND;
            break;
        }
    }

    return ( status );
}

/*********************************************************************
    @fn      multiProfile_WriteAttrCB

    @brief   Validate attribute data prior to a write operation

    @param   connHandle - connection message was received on
    @param   pAttr - pointer to attribute
    @param   pValue - pointer to data to be written
    @param   len - length of data
    @param   offset - offset of the first octet to be written

    @return  Success or Failure
*/
// TODO: test this function
static bStatus_t multiProfile_WriteAttrCB( uint16 connHandle, gattAttribute_t* pAttr,
                                           uint8* pValue, uint16 len, uint16 offset )
{
    bStatus_t status = SUCCESS;
    uint8 notifyApp = 0xFF;

    // If attribute permissions require authorization to write, return error
    if ( gattPermitAuthorWrite( pAttr->permissions ) )
    {
        // Insufficient authorization
        return ( ATT_ERR_INSUFFICIENT_AUTHOR );
    }

    if ( pAttr->type.len == ATT_UUID_SIZE )
    {
        switch ( pAttr->type.uuid[ ATT_UUID_SIZE - 4 ] )
        {
        case MULTIPROFILE_CHAR1:
            osal_memcpy(multiProfileChar1, pValue, len);
            notifyApp = MULTIPROFILE_CHAR1;
            break;
        }
    }
    else if( pAttr->type.len == ATT_BT_UUID_SIZE )
    {
        // 16-bit UUID
        uint16 uuid = BUILD_UINT16( pAttr->type.uuid[0], pAttr->type.uuid[1]);

        switch ( uuid )
        {
        case GATT_CLIENT_CHAR_CFG_UUID:
        {
            status = GATTServApp_ProcessCCCWriteReq( connHandle, pAttr, pValue, len,
                                                     offset, GATT_CLIENT_CFG_NOTIFY );
            osal_memcpy(multiProfileChar2, pValue, len);

            if ( status == SUCCESS )
            {
                notifyApp = MULTIPROFILE_CHAR2;
            }
        }
        break;
        }
    }

    // If a charactersitic value changed then callback function to notify application of change
    if ( (notifyApp != 0xFF ) && multiProfile_AppCBs && multiProfile_AppCBs->pfnMultiProfileChange )
    {
        multiProfile_AppCBs->pfnMultiProfileChange(connHandle,notifyApp,len );
    }

    return status;
}

/*********************************************************************
    @fn          multiProfile_HandleConnStatusCB

    @brief       multi Profile link status change handler function.

    @param       connHandle - connection handle
    @param       changeType - type of change

    @return      none
*/
static void multiProfile_HandleConnStatusCB( uint16 connHandle, uint8 changeType )
{
    // Make sure this is not loopback connection
    if ( connHandle != LOOPBACK_CONNHANDLE )
    {
        // Reset Client Char Config if connection has dropped
        if ( ( changeType == LINKDB_STATUS_UPDATE_REMOVED )      ||
                ( ( changeType == LINKDB_STATUS_UPDATE_STATEFLAGS ) &&
                  ( !linkDB_Up( connHandle ) ) ) )
        {
            GATTServApp_InitCharCfg( connHandle, multiProfileChar2Config );
        }
    }
}


bStatus_t MultiProfile_Notify(uint16 connHandle,uint8 param, uint16 len, void* value )
{
    bStatus_t ret = SUCCESS;
    uint16 notfEnable = FALSE;
    uint16 notfConnHandle = INVALID_CONNHANDLE;

    switch ( param )
    {
    case MULTIPROFILE_CHAR2:
    {
        for(uint8 i=0; i<MAX_NUM_LL_CONN; i++)
        {
            if( connHandle == multiProfileChar2Config[i].connHandle )
            {
                notfEnable = GATTServApp_ReadCharCfg( connHandle, &multiProfileChar2Config[i] );
                notfConnHandle = i;
                break;
            }
        }

//      LOG("%p,enable %d\n",&multiProfileChar2Config[notfConnHandle],notfEnable);
//      LOG("multiProfileChar2Config NOT follow with connHandle \n");
//      LOG("multiProfileChar2Config[%d].connHandle %d,value %d\n",notfConnHandle,multiProfileChar2Config[notfConnHandle].connHandle,\
        multiProfileChar2Config[notfConnHandle].value);

        // If notifications enabled
        if ( notfEnable & GATT_CLIENT_CFG_NOTIFY )
    {
        VOID osal_memcpy( multiProfileChar2, value, len );
            multiChar2NotifyLen = len;
            ret=GATTServApp_ProcessCharCfg( &multiProfileChar2Config[notfConnHandle], multiProfileChar2, FALSE,
//          ret=GATTServApp_ProcessCharCfg( multiProfileChar2Config, multiProfileChar2, FALSE,
                                            multiProfileAttrTbl, GATT_NUM_ATTRS( multiProfileAttrTbl ),
                                            INVALID_TASK_ID );
        }
        else
        {
            ret = bleNotReady;
        }
    }
    break;

    default:
        ret = INVALIDPARAMETER;
        break;
    }

    return ( ret );
}
#endif


/*********************************************************************
*********************************************************************/
