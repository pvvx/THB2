/**************************************************************************************************
    Filename:       AudioGATTprofile.c
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

#include "AudioGATTprofile.h"

#include "log.h"
//#include "common.h"
#include "peripheral.h"

#include "hidkbd.h"




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

#define AUDIO_BASE_UUID_128( uuid )  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xB0, \
    0x00, 0x40, 0x51, 0x04, LO_UINT16( uuid ), HI_UINT16( uuid ), 0x00, 0xF0

/*********************************************************************
    GLOBAL VARIABLES
*/


uint8 char1Tx_length=0;
uint8 char2Tx_length=0;

// Audio GATT Profile Service UUID: 0xFF01

CONST uint8 AudioProfileServUUID[ATT_UUID_SIZE] =
{
    AUDIO_BASE_UUID_128(AUDIOPROFILE_SERV_UUID)
};

// Characteristic 1 UUID: 0xFFF1
CONST uint8 AudioProfilechar1UUID[ATT_UUID_SIZE] =
{
    AUDIO_BASE_UUID_128(AUDIOPROFILE_CHAR1_UUID)
};

// Characteristic 2 UUID: 0xFFF2
CONST uint8 AudioProfilechar2UUID[ATT_UUID_SIZE] =
{
    AUDIO_BASE_UUID_128(AUDIOPROFILE_CHAR2_UUID)
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

static AudioProfileCBs_t* AudioProfile_AppCBs = NULL;

/*********************************************************************
    Profile Attributes - variables
*/

// Simple Profile Service attribute
static CONST gattAttrType_t AudioProfileService = { ATT_UUID_SIZE, AudioProfileServUUID };


// Audio Profile Characteristic 1 Properties
static uint8 AudioProfileChar1Props =  GATT_PROP_NOTIFY|GATT_PROP_READ;

// Characteristic 1 Value
uint8 AudioProfileChar1[AUDIOPROFILE_CHAR1_LEN];

// Audio Profile Characteristic 1 Configuration Each client has its own
// instantiation of the Client Characteristic Configuration. Reads of the
// Client Characteristic Configuration only shows the configuration for
// that client and writes only affect the configuration of that client.
static gattCharCfg_t AudioProfileChar1Config[GATT_MAX_NUM_CONN];


// Audio Profile Characteristic 2 User Description
//static uint8 AudioProfileChar1UserDesp[] = "RX CHAR\0";



// Audio Profile Characteristic 1 Properties
static uint8 AudioProfileChar2Props = GATT_PROP_NOTIFY|GATT_PROP_READ;

// Characteristic 1 Value
uint8 AudioProfileChar2[AUDIOPROFILE_CHAR2_LEN];

// Audio Profile Characteristic 1 User Description
//static uint8 AudioProfileChar2UserDesp[] = "TX CHAR\0";

// Audio Profile Characteristic 1 Configuration Each client has its own
// instantiation of the Client Characteristic Configuration. Reads of the
// Client Characteristic Configuration only shows the configuration for
// that client and writes only affect the configuration of that client.
static gattCharCfg_t AudioProfileChar2Config[GATT_MAX_NUM_CONN];


/*********************************************************************
    Profile Attributes - Table
*/

static gattAttribute_t AudioProfileAttrTbl[] =
{
    // =========== Simple Profile Service
    {
        { ATT_BT_UUID_SIZE, primaryServiceUUID }, /* type */
        GATT_PERMIT_READ,                         /* permissions */
        0,                                        /* handle */
        (uint8*)& AudioProfileService            /* pValue */
    },

    // ----------------------------------------------------------------------
    // Characteristic 1 Declaration,
    {
        { ATT_BT_UUID_SIZE, characterUUID },
        GATT_PERMIT_READ,
        0,
        &AudioProfileChar1Props
    },

    // Characteristic Value 1
    {
        { ATT_UUID_SIZE, AudioProfilechar1UUID },
        GATT_PERMIT_READ | GATT_PERMIT_WRITE,
        0,
        (uint8*)& AudioProfileChar1
    },

    // Characteristic 1 User Description, this field is optional
//   {
//       { ATT_BT_UUID_SIZE, charUserDescUUID },
//       GATT_PERMIT_READ,
//       0,
//        AudioProfileChar1UserDesp
//    },

    // Characteristic 1 configuration
    {
        { ATT_BT_UUID_SIZE, clientCharCfgUUID },
        GATT_PERMIT_READ | GATT_PERMIT_WRITE,
        0,
        (uint8*)AudioProfileChar1Config
    },

    // ----------------------------------------------------------------------
    // Characteristic 2 Declaration,
    {
        { ATT_BT_UUID_SIZE, characterUUID },
        GATT_PERMIT_READ,
        0,
        &AudioProfileChar2Props
    },

    // Characteristic Value 2
    {
        { ATT_UUID_SIZE, AudioProfilechar2UUID },
        GATT_PERMIT_READ,
        0,
        AudioProfileChar2
    },

    // Characteristic 2 configuration
    {
        { ATT_BT_UUID_SIZE, clientCharCfgUUID },
        GATT_PERMIT_READ | GATT_PERMIT_WRITE,
        0,
        (uint8*)AudioProfileChar2Config
    },

    // Characteristic 2 User Description
    //  {
//       { ATT_BT_UUID_SIZE, charUserDescUUID },
//       GATT_PERMIT_READ,
//       0,
//       AudioProfileChar2UserDesp
//    },


};


/*********************************************************************
    LOCAL FUNCTIONS
*/
static uint8 AudioProfile_ReadAttrCB( uint16 connHandle, gattAttribute_t* pAttr,
                                      uint8* pValue, uint16* pLen, uint16 offset, uint8 maxLen );
static bStatus_t AudioProfile_WriteAttrCB( uint16 connHandle, gattAttribute_t* pAttr,
                                           uint8* pValue, uint16 len, uint16 offset );

static void AudioProfile_HandleConnStatusCB( uint16 connHandle, uint8 changeType );


/*********************************************************************
    PROFILE CALLBACKS
*/
// Audio Profile Service Callbacks
CONST gattServiceCBs_t AudioProfileCBs =
{
    AudioProfile_ReadAttrCB,  // Read callback function pointer
    AudioProfile_WriteAttrCB, // Write callback function pointer
    NULL                       // Authorization callback function pointer
};

/*********************************************************************
    PUBLIC FUNCTIONS
*/

/*********************************************************************
    @fn      AudioProfile_AddService

    @brief   Initializes the Simple Profile service by registering
            GATT attributes with the GATT server.

    @param   services - services to add. This is a bit map and can
                       contain more than one service.

    @return  Success or Failure
*/
bStatus_t AudioProfile_AddService( uint32 services )
{
    uint8 status = SUCCESS;
    // Initialize Client Characteristic Configuration attributes
    GATTServApp_InitCharCfg( INVALID_CONNHANDLE, AudioProfileChar1Config );
    GATTServApp_InitCharCfg( INVALID_CONNHANDLE, AudioProfileChar2Config );
    // Register with Link DB to receive link status change callback
    VOID linkDB_Register(AudioProfile_HandleConnStatusCB );

    if ( services & AUDIOPROFILE_SERVICE )
    {
        // Register GATT attribute list and CBs with GATT Server App
        status = GATTServApp_RegisterService( AudioProfileAttrTbl,
                                              GATT_NUM_ATTRS( AudioProfileAttrTbl ),
                                              &AudioProfileCBs );
    }

    return ( status );
}


/*********************************************************************
    @fn      AudioProfile_RegisterAppCBs

    @brief   Registers the application callback function. Only call
            this function once.

    @param   callbacks - pointer to application callbacks.

    @return  SUCCESS or bleAlreadyInRequestedMode
*/
bStatus_t AudioProfile_RegisterAppCBs( AudioProfileCBs_t* appCallbacks )
{
    if ( appCallbacks )
    {
        AudioProfile_AppCBs = appCallbacks;
        return ( SUCCESS );
    }
    else
    {
        return ( bleAlreadyInRequestedMode );
    }
}

/*********************************************************************
    @fn      AudioProfile_SetParameter

    @brief   Set a Simple Profile parameter.

    @param   param - Profile parameter ID
    @param   len - length of data to right
    @param   value - pointer to data to write.  This is dependent on
            the parameter ID and WILL be cast to the appropriate
            data type (example: data type of uint16 will be cast to
            uint16 pointer).

    @return  bStatus_t
*/
bStatus_t AudioProfile_SetParameter( uint8 param, uint8 len, void* value )
{
    bStatus_t ret = SUCCESS;

    switch ( param )
    {
    case AUDIOPROFILE_CHAR1:
        if ( len >0)
        {
            VOID osal_memcpy( AudioProfileChar1, value, len );
            char1Tx_length=len;
            ret=GATTServApp_ProcessCharCfg( AudioProfileChar1Config, AudioProfileChar1, FALSE,
                                            AudioProfileAttrTbl, GATT_NUM_ATTRS( AudioProfileAttrTbl ),
                                            INVALID_TASK_ID );

            if(ret!=SUCCESS)
            {
                // LOG("Notify_error:%d\n\r",ret);
            }
        }
        else
        {
            ret = bleInvalidRange;
        }

        break;

    case AUDIOPROFILE_CHAR2:
        if ( len >0)
        {
            VOID osal_memcpy( AudioProfileChar2, value, len );
            char2Tx_length=len;
            ret=GATTServApp_ProcessCharCfg( AudioProfileChar2Config, AudioProfileChar2, FALSE,
                                            AudioProfileAttrTbl, GATT_NUM_ATTRS( AudioProfileAttrTbl ),
                                            INVALID_TASK_ID );

            if(ret!=SUCCESS)
            {
                // LOG("Notify_error:%d\nr",ret);
            }
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
    @fn      AudioProfile_GetParameter

    @brief   Get a Audio Profile parameter.

    @param   param - Profile parameter ID
    @param   value - pointer to data to put.  This is dependent on
            the parameter ID and WILL be cast to the appropriate
            data type (example: data type of uint16 will be cast to
            uint16 pointer).

    @return  bStatus_t
*/
bStatus_t AudioProfile_GetParameter( uint8 param, void* value )
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
    @fn          AudioProfile_ReadAttrCB

    @brief       Read an attribute.

    @param       connHandle - connection message was received on
    @param       pAttr - pointer to attribute
    @param       pValue - pointer to data to be read
    @param       pLen - length of data to be read
    @param       offset - offset of the first octet to be read
    @param       maxLen - maximum length of data to be read

    @return      Success or Failure
*/
static uint8 AudioProfile_ReadAttrCB( uint16 connHandle, gattAttribute_t* pAttr,
                                      uint8* pValue, uint16* pLen, uint16 offset, uint8 maxLen )
{
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

    if ( pAttr->type.len == ATT_BT_UUID_SIZE||pAttr->type.len == ATT_UUID_SIZE )
    {
        AudioProfile_Read(connHandle,pAttr,pValue,pLen,offset,maxLen);
    }

    return ( status );
}

/*********************************************************************
    @fn      AudioProfile_WriteAttrCB

    @brief   Validate attribute data prior to a write operation

    @param   connHandle - connection message was received on
    @param   pAttr - pointer to attribute
    @param   pValue - pointer to data to be written
    @param   len - length of data
    @param   offset - offset of the first octet to be written

    @return  Success or Failure
*/
static bStatus_t AudioProfile_WriteAttrCB( uint16 connHandle, gattAttribute_t* pAttr,
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

    if ( pAttr->type.len == ATT_BT_UUID_SIZE )
    {
        // 16-bit UUID
        uint16 uuid = BUILD_UINT16( pAttr->type.uuid[0], pAttr->type.uuid[1]);

        switch ( uuid )
        {
        case GATT_CLIENT_CHAR_CFG_UUID:
            status = GATTServApp_ProcessCCCWriteReq( connHandle, pAttr, pValue, len,
                                                     offset, GATT_CLIENT_CFG_NOTIFY );

            if ( status == SUCCESS )
            {
                // uint16 charCfg = BUILD_UINT16( pValue[0], pValue[1] );
                if(pAttr->handle==AudioProfileAttrTbl[3].handle)
                {
                    LOG("audio start cmd enable\n\r");
                }
                else if(pAttr->handle==AudioProfileAttrTbl[6].handle)
                {
                    LOG("audio data transf enable\n\r");
                    // if(charCfg==0x0001)
                    //osal_start_timerEx(hidKbdTaskId, HID_KEY_TEST_EVT, 8000);
                }
            }

            break;

        default:
            // Should never get here! (characteristics 2 and 4 do not have write permissions)
            status = ATT_ERR_ATTR_NOT_FOUND;
            break;
        }
    }
    else
    {
        // 128-bit UUID
        status = ATT_ERR_INVALID_HANDLE;
    }

    // If a charactersitic value changed then callback function to notify application of change
    if ( (notifyApp != 0xFF ) && AudioProfile_AppCBs && AudioProfile_AppCBs->pfnAudioProfileChange )
    {
        AudioProfile_AppCBs->pfnAudioProfileChange( notifyApp );
    }

    return ( status );
}

/*********************************************************************
    @fn          AudioProfile_HandleConnStatusCB

    @brief       Audio Profile link status change handler function.

    @param       connHandle - connection handle
    @param       changeType - type of change

    @return      none
*/
static void AudioProfile_HandleConnStatusCB( uint16 connHandle, uint8 changeType )
{
    // Make sure this is not loopback connection
    if ( connHandle != LOOPBACK_CONNHANDLE )
    {
        // Reset Client Char Config if connection has dropped
        if ( ( changeType == LINKDB_STATUS_UPDATE_REMOVED )      ||
                ( ( changeType == LINKDB_STATUS_UPDATE_STATEFLAGS ) &&
                  ( !linkDB_Up( connHandle ) ) ) )
        {
            GATTServApp_InitCharCfg( connHandle, AudioProfileChar1Config );
            GATTServApp_InitCharCfg( connHandle, AudioProfileChar2Config );
        }
    }
}

bStatus_t AudioProfile_Notify( uint8 param, uint8 len, void* value )
{
    bStatus_t ret = SUCCESS;

    switch ( param )
    {
    case AUDIOPROFILE_CHAR2:
        VOID osal_memcpy( AudioProfileChar2, value, len );
        GATTServApp_ProcessCharCfg( AudioProfileChar2Config, AudioProfileChar2, FALSE,
                                    AudioProfileAttrTbl, GATT_NUM_ATTRS( AudioProfileAttrTbl ),
                                    INVALID_TASK_ID );
        break;

    default:
        ret = INVALIDPARAMETER;
        break;
    }

    return ( ret );
}

bStatus_t AudioProfile_Write( uint16 connHandle, gattAttribute_t* pAttr,
                              uint8* pValue, uint8 len, uint16 offset )
{
    bStatus_t status = SUCCESS;
    // 16-bit UUID
    uint16 uuid = BUILD_UINT16( pAttr->type.uuid[0], pAttr->type.uuid[1]);

    switch(uuid)
    {
    case AUDIOPROFILE_CHAR1_UUID:
        if ( offset != 0 )
        {
            status = ATT_ERR_ATTR_NOT_LONG;
        }

        if ( status == SUCCESS )
        {
            uint8* pCurValue = (uint8*)pAttr->pValue;
            VOID osal_memcpy( pCurValue, pValue, len );
            //rxdata_process(pCurValue,len);
        }

        break;

    case GATT_CLIENT_CHAR_CFG_UUID:
        status = GATTServApp_ProcessCCCWriteReq( connHandle, pAttr, pValue, len,
                                                 offset, GATT_CLIENT_CFG_NOTIFY );
        break;

    default:
        status = ATT_ERR_ATTR_NOT_FOUND;
        break;
    }

    return status;
}

bStatus_t AudioProfile_Read( uint16 connHandle, gattAttribute_t* pAttr,
                             uint8* pValue, uint16* pLen, uint16 offset, uint8 maxLen )
{
    bStatus_t status = SUCCESS;

    if(pAttr->type.len == ATT_BT_UUID_SIZE)
    {
        // 16-bit UUID
        uint16 uuid = BUILD_UINT16( pAttr->type.uuid[0], pAttr->type.uuid[1]);
        LOG("16 uuid:%X\n\r",uuid);

        switch ( uuid )
        {
        case AUDIOPROFILE_CHAR1_UUID:
            *pLen = char1Tx_length;
            VOID osal_memcpy( pValue, pAttr->pValue, *pLen );
            break;

        case AUDIOPROFILE_CHAR2_UUID:
            *pLen=char2Tx_length;
            VOID osal_memcpy( pValue, pAttr->pValue, *pLen );
            break;

        default:
            // Should never get here! (characteristics 3 and 4 do not have read permissions)
            *pLen = 0;
            status = ATT_ERR_ATTR_NOT_FOUND;
            LOG("uuid not find\n\r");
            break;
        }
    }
    else
    {
        uint16 uuid = BUILD_UINT16( pAttr->type.uuid[12], pAttr->type.uuid[13]);

        //LOG("128 uuid:%X\n\r",uuid);
        switch ( uuid )
        {
        case AUDIOPROFILE_CHAR1_UUID:
            *pLen = char1Tx_length;
            VOID osal_memcpy( pValue, pAttr->pValue, *pLen );
            break;

        case AUDIOPROFILE_CHAR2_UUID:
            *pLen=char2Tx_length;
            VOID osal_memcpy( pValue, pAttr->pValue, *pLen );
            break;

        default:
            // Should never get here! (characteristics 3 and 4 do not have read permissions)
            *pLen = 0;
            status = ATT_ERR_ATTR_NOT_FOUND;
            LOG("uuid not find\n\r");
            break;
        }
    }

    return status;
}


/*********************************************************************
*********************************************************************/
