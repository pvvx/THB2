/*************
 SDK_LICENSE
***************/
/**************************************************************************************************
    Filename:       gapgattserver.c
    Revised:
    Revision:

    Description:    GAP Attribute Server


**************************************************************************************************/

/*********************************************************************
    INCLUDES
*/
#include "bcomdef.h"
#include "config.h"
#include "OSAL.h"
#include "gap.h"
#include "gapgattserver.h"
#include "att.h"
#include "gatt.h"
#include "gatt_uuid.h"
#include "gattservapp.h"
//#include "bleperipheral.h"

/*********************************************************************
    MACROS
*/

/*********************************************************************
    CONSTANTS
*/
// Position of device name in attribute table
#define GAP_DEVICE_NAME_POS         2
#define GAP_APPEARANCE_POS          4
#define GAP_PRIVACY_FLAG_POS        6

/*********************************************************************
    TYPEDEFS
*/

/*********************************************************************
    GLOBAL VARIABLES
*/

/*********************************************************************
    EXTERNAL VARIABLES
*/

/*********************************************************************
    EXTERNAL FUNCTIONS
*/

/*********************************************************************
    LOCAL VARIABLES
*/
static const ggsAppCBs_t* ggs_AppCBs = NULL;

#if defined ( TESTMODES )
    static uint16 paramValue = 0;
#endif

/*********************************************************************
    Profile Attributes - variables
*/

// GAP Service
static CONST gattAttrType_t gapService = { ATT_BT_UUID_SIZE, gapServiceUUID };

// Device Name Characteristic Properties
static uint8 deviceNameCharProps = GATT_PROP_READ;

// Device Name attribute (0 - 248 octets) - extra octet for null-terminate char
static uint8 deviceName[GAP_DEVICE_NAME_LEN+1] = { 0 };

// Appearance Characteristic Properties
static uint8 appearanceCharProps = GATT_PROP_READ;

// Appearance attribute (2-octet enumerated value as defined by Bluetooth Assigned Numbers document)
static uint16 appearance = GAP_APPEARE_GENERIC_THERMOMETER;

#if ( HOST_CONFIG & PERIPHERAL_CFG )

#if defined (GAP_PRIVACY) || defined (GAP_PRIVACY_RECONNECT)

    // Peripheral Privacy Flag Characteristic Properties
    static uint8 periPrivacyFlagCharProps = GATT_PROP_READ;

    // Peripheral Privacy Flag attribute (1 octet)
    static uint8 periPrivacyFlag = GAP_PRIVACY_DISABLED;

#endif // GAP_PRIVACY || GAP_PRIVACY_RECONNECT

#if defined (GAP_PRIVACY_RECONNECT)

// Reconnection Address Characteristic Properties
static uint8 reconnectAddrCharProps = GATT_PROP_WRITE;

// Reconnection Address attribute (6 octets)
static uint8 reconnectAddr[B_ADDR_LEN] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

#endif // GAP_PRIVACY_RECONNECT

// Peripheral Preferred Connection Parameters Characteristic Properties
static uint8 periConnParamCharProps = GATT_PROP_READ;

// Peripheral Preferred Connection Parameters attribute (8 octets)
gapPeriConnectParams_t periConnParameters = 
{ 
#if 1 // FIX_CONN_INTERVAL
	DEFAULT_DESIRED_MIN_CONN_INTERVAL, 
	DEFAULT_DESIRED_MAX_CONN_INTERVAL,
	DEFAULT_DESIRED_SLAVE_LATENCY,
	DEFAULT_DESIRED_CONN_TIMEOUT
#else
	8,80,0,1500
#endif
};

#endif // PERIPHERAL_CFG

/*********************************************************************
    Profile Attributes - Table
*/

// GAP Attribute Table
static gattAttribute_t gapAttrTbl[] =
{
    // Generic Access Profile
    {
        { ATT_BT_UUID_SIZE, primaryServiceUUID }, /* type */
        GATT_PERMIT_READ,                         /* permissions */
        0,                                        /* handle */
        (uint8*)& gapService                      /* pValue */
    },

    // Characteristic Declaration
    {
        { ATT_BT_UUID_SIZE, characterUUID },
        GATT_PERMIT_READ,
        0,
        &deviceNameCharProps
    },

    // Device Name attribute
    {
        { ATT_BT_UUID_SIZE, deviceNameUUID },
        GATT_PERMIT_READ,
        0,
        deviceName
    },

    // Characteristic Declaration
    {
        { ATT_BT_UUID_SIZE, characterUUID },
        GATT_PERMIT_READ,
        0,
        &appearanceCharProps
    },

    // Icon attribute
    {
        { ATT_BT_UUID_SIZE, appearanceUUID },
        GATT_PERMIT_READ,
        0,
        (uint8*)& appearance
    },

    #if ( HOST_CONFIG & PERIPHERAL_CFG )

    #if defined (GAP_PRIVACY) || defined (GAP_PRIVACY_RECONNECT)

    // Characteristic Declaration
    {
        { ATT_BT_UUID_SIZE, characterUUID },
        GATT_PERMIT_READ,
        0,
        &periPrivacyFlagCharProps
    },

    // Peripheral Privacy Flag attribute
    {
        { ATT_BT_UUID_SIZE, periPrivacyFlagUUID },
        GATT_PERMIT_READ,
        0,
        (uint8*)& periPrivacyFlag
    },

    #endif // GAP_PRIVACY || GAP_PRIVACY_RECONNECT

    #if defined (GAP_PRIVACY_RECONNECT)

    // Characteristic Declaration
    {
        { ATT_BT_UUID_SIZE, characterUUID },
        GATT_PERMIT_READ,
        0,
        &reconnectAddrCharProps
    },

    // Reconnection Address attribute
    {
        { ATT_BT_UUID_SIZE, reconnectAddrUUID },
        GATT_PERMIT_AUTHEN_WRITE,
        0,
        reconnectAddr
    },

    #endif // GAP_PRIVACY_RECONNECT

    // Characteristic Declaration
    {
        { ATT_BT_UUID_SIZE, characterUUID },
        GATT_PERMIT_READ,
        0,
        &periConnParamCharProps
    },

    // Peripheral Preferred Connection Parameters attribute
    {
        { ATT_BT_UUID_SIZE, periConnParamUUID },
        GATT_PERMIT_READ,
        0,
        (uint8*)& periConnParameters
    },

    #endif // PERIPHERAL_CFG
};

/*********************************************************************
    LOCAL FUNCTIONS
*/
static void ggs_SetAttrWPermit( uint8 wPermit, uint8* pPermissions, uint8* pCharProps );

/*********************************************************************
    PUBLIC FUNCTIONS
*/
// GGS Callback functions
static uint8 ggs_ReadAttrCB( uint16 connHandle, gattAttribute_t* pAttr,
                             uint8* pValue, uint16* pLen, uint16 offset, uint8 maxLen );
static bStatus_t ggs_WriteAttrCB( uint16 connHandle, gattAttribute_t* pAttr,
                                  uint8* pValue, uint16 len, uint16 offset );

/*********************************************************************
    PROFILE CALLBACKS
*/
// GAP Service Callbacks
CONST gattServiceCBs_t gapServiceCBs =
{
    ggs_ReadAttrCB,  // Read callback function pointer
    ggs_WriteAttrCB, // Write callback function pointer
    NULL             // Authorization callback function pointer
};

/*********************************************************************
    @fn      GGS_SetParameter

    @brief   Set a GAP GATT Server parameter.

    @param   param - Profile parameter ID
    @param   len - length of data to right
    @param   value - pointer to data to write.  This is dependent on
            the parameter ID and WILL be cast to the appropriate
            data type (example: data type of uint16 will be cast to
            uint16 pointer).

    @return  bStatus_t
*/
bStatus_t GGS_SetParameter( uint8 param, uint8 len, void* value )
{
    bStatus_t ret = SUCCESS;

    switch ( param )
    {
    case GGS_DEVICE_NAME_ATT:

        // Always leave room for null-terminate char
        if ( len <= GAP_DEVICE_NAME_LEN )
        {
            VOID osal_memset( deviceName, 0, GAP_DEVICE_NAME_LEN+1 );
            VOID osal_memcpy( deviceName, value, len );
        }
        else
        {
            ret = bleInvalidRange;
        }

        break;

    case GGS_APPEARANCE_ATT:
        if ( len == sizeof ( uint16 ) )
        {
            appearance = *((uint16*)value);
        }
        else
        {
            ret = bleInvalidRange;
        }

        break;
        #if ( HOST_CONFIG & PERIPHERAL_CFG )
        #if defined (GAP_PRIVACY) || defined (GAP_PRIVACY_RECONNECT)

    case GGS_PERI_PRIVACY_FLAG_ATT:
        if ( len == sizeof ( uint8 ) )
        {
            periPrivacyFlag = *((uint8*)value);
        }
        else
        {
            ret = bleInvalidRange;
        }

        break;

    case GGS_PERI_PRIVACY_FLAG_PROPS:
        if ( len == sizeof ( uint8 ) )
        {
            periPrivacyFlagCharProps = *((uint8*)value);
        }
        else
        {
            ret = bleInvalidRange;
        }

        break;

    case GGS_W_PERMIT_PRIVACY_FLAG_ATT:
        if ( len == sizeof ( uint8 ) )
        {
            uint8 wPermit = *(uint8*)value;

            // Optionally Writeable with Authentication
            if ( !gattPermitWrite( wPermit ) )
            {
                ggs_SetAttrWPermit( wPermit,
                                    &(gapAttrTbl[GAP_PRIVACY_FLAG_POS].permissions),
                                    gapAttrTbl[GAP_PRIVACY_FLAG_POS-1].pValue );
            }
            else
            {
                ret = bleInvalidRange;
            }
        }
        else
        {
            ret = bleInvalidRange;
        }

        break;
        #endif // GAP_PRIVACY || GAP_PRIVACY_RECONNECT
        #if defined (GAP_PRIVACY_RECONNECT)

    case GGS_RECONNCT_ADDR_ATT:
        if ( len == B_ADDR_LEN )
        {
            VOID osal_memcpy( reconnectAddr, value, len );
        }
        else
        {
            ret = bleInvalidRange;
        }

        break;
        #endif // GAP_PRIVACY_RECONNECT

    case GGS_PERI_CONN_PARAM_ATT:
        if ( len == sizeof(gapPeriConnectParams_t) )
        {
            periConnParameters = *((gapPeriConnectParams_t*)(value));
        }
        else
        {
            ret = bleInvalidRange;
        }

        break;
        #endif // PERIPHERAL_CFG

    case GGS_W_PERMIT_DEVICE_NAME_ATT:
        if ( len == sizeof ( uint8 ) )
        {
            // Optionally Writeable
            ggs_SetAttrWPermit( *(uint8*)value,
                                &(gapAttrTbl[GAP_DEVICE_NAME_POS].permissions),
                                gapAttrTbl[GAP_DEVICE_NAME_POS-1].pValue );
        }
        else
        {
            ret = bleInvalidRange;
        }

        break;

    case GGS_W_PERMIT_APPEARANCE_ATT:
        if ( len == sizeof ( uint8 ) )
        {
            // Optionally Writeable
            ggs_SetAttrWPermit( *(uint8*)value,
                                &(gapAttrTbl[GAP_APPEARANCE_POS].permissions),
                                gapAttrTbl[GAP_APPEARANCE_POS-1].pValue );
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
    @fn      GGS_GetParameter

    @brief   Get a GAP GATT Server parameter.

    @param   param - Profile parameter ID
    @param   value - pointer to data to put.  This is dependent on
            the parameter ID and WILL be cast to the appropriate
            data type (example: data type of uint16 will be cast to
            uint16 pointer).

    @return  bStatus_t
*/
bStatus_t GGS_GetParameter( uint8 param, void* value )
{
    bStatus_t ret = SUCCESS;

    switch ( param )
    {
    case GGS_DEVICE_NAME_ATT:
        VOID osal_memcpy( value, deviceName, GAP_DEVICE_NAME_LEN );
        break;

    case GGS_APPEARANCE_ATT:
        *((uint16*)value) = appearance;
        break;
#if ( HOST_CONFIG & PERIPHERAL_CFG )
#if defined (GAP_PRIVACY) || defined (GAP_PRIVACY_RECONNECT)

    case GGS_PERI_PRIVACY_FLAG_ATT:
        *((uint8*)value) = periPrivacyFlag;
        break;

    case GGS_PERI_PRIVACY_FLAG_PROPS:
        *((uint8*)value) = periPrivacyFlagCharProps;
        break;
#endif // GAP_PRIVACY || GAP_PRIVACY_RECONNECT
#if defined (GAP_PRIVACY_RECONNECT)

    case GGS_RECONNCT_ADDR_ATT:
        VOID osal_memcpy( value, reconnectAddr, B_ADDR_LEN );
        break;
#endif // GAP_PRIVACY_RECONNECT

    case GGS_PERI_CONN_PARAM_ATT:
        *((gapPeriConnectParams_t*)(value)) = periConnParameters;
        break;
#endif // PERIPHERAL_CFG

    default:
        ret = INVALIDPARAMETER;
        break;
    }

    return ( ret );
}

/*********************************************************************
    @fn      GGS_SetParamValue

    @brief   Set a GGS Parameter value. Use this function to change
            the default GGS parameter values.

    @param   value - new GGS param value

    @return  void
*/
void GGS_SetParamValue( uint16 value )
{
    #if defined ( TESTMODES )
    uint8 wpermit;
    paramValue = value;

    switch ( value )
    {
    case GGS_TESTMODE_OFF:
        wpermit = 0;
        VOID GGS_SetParameter( GGS_W_PERMIT_DEVICE_NAME_ATT, sizeof( uint8 ), (void*)&wpermit );
        VOID GGS_SetParameter( GGS_W_PERMIT_APPEARANCE_ATT, sizeof( uint8 ), (void*)&wpermit );
        VOID GGS_SetParameter( GGS_W_PERMIT_PRIVACY_FLAG_ATT, sizeof( uint8 ), (void*)&wpermit );
        break;

    case GGS_TESTMODE_W_PERMIT_DEVICE_NAME:
        wpermit = GATT_PERMIT_WRITE;
        VOID GGS_SetParameter( GGS_W_PERMIT_DEVICE_NAME_ATT, sizeof( uint8 ), (void*)&wpermit );
        break;

    case GGS_TESTMODE_W_PERMIT_APPEARANCE:
        wpermit = GATT_PERMIT_WRITE;
        VOID GGS_SetParameter( GGS_W_PERMIT_APPEARANCE_ATT, sizeof( uint8 ), (void*)&wpermit );
        break;

    case GGS_TESTMODE_W_PERMIT_PRIVACY_FLAG:
        wpermit = GATT_PERMIT_AUTHEN_WRITE;
        VOID GGS_SetParameter( GGS_W_PERMIT_PRIVACY_FLAG_ATT, sizeof( uint8 ), (void*)&wpermit );
        break;

    default:
        break;
    }

    #else
    VOID value;
    #endif
}

/*********************************************************************
    @fn      GGS_GetParamValue

    @brief   Get a GGS Parameter value.

    @param   none

    @return  GGS Parameter value
*/
uint16 GGS_GetParamValue( void )
{
    #if defined ( TESTMODES )
    return ( paramValue );
    #else
    return ( 0 );
    #endif
}

/*********************************************************************
    LOCAL FUNCTION PROTOTYPES
*/

/*********************************************************************
    @fn      GGS_AddService

    @brief   Add function for the GAP GATT Service.

    @param   services - services to add. This is a bit map and can
                       contain more than one service.

    @return  SUCCESS: Service added successfully.
            INVALIDPARAMETER: Invalid service field.
            FAILURE: Not enough attribute handles available.
            bleMemAllocError: Memory allocation error occurred.
*/
bStatus_t GGS_AddService( uint32 services )
{
    uint8 status = SUCCESS;

    if ( services & GAP_SERVICE )
    {
        // Register GAP attribute list and CBs with GATT Server Server App
        status = GATTServApp_RegisterService( gapAttrTbl, GATT_NUM_ATTRS( gapAttrTbl ),
                                              &gapServiceCBs );
    }

    return ( status );
}

/******************************************************************************
    @fn      GATTServApp_DelService

    @brief   Delete function for the GAP GATT Service.

    @param   services - services to delete. This is a bit map and can
                       contain more than one service.

    @return  SUCCESS: Service deleted successfully.
            FAILURE: Service not found.
*/
bStatus_t GGS_DelService( uint32 services )
{
    uint8 status = SUCCESS;

    if ( services & GAP_SERVICE )
    {
        // Deregister GAP attribute list and CBs from GATT Server Application
        status = GATTServApp_DeregisterService( GATT_SERVICE_HANDLE( gapAttrTbl ), NULL );
    }

    return ( status );
}

/*********************************************************************
    @fn      GGS_RegisterAppCBs

    @brief   Registers the application callback function.

            Note: Callback registration is needed only when the
                  Device Name is made writable. The application
                  will be notified when the Device Name is changed
                  over the air.

    @param   appCallbacks - pointer to application callbacks.

    @return  none
*/
void GGS_RegisterAppCBs( ggsAppCBs_t* appCallbacks )
{
    ggs_AppCBs = appCallbacks;
}

/*********************************************************************
    @fn          ggs_SetAttrWPermit

    @brief       Update attribute Write access permissions and characteristic
                properties for over-the-air write operations.

    @param       wPermit - write acces permissions
    @param       pPermissions - pointer to attribute permissions
    @param       pCharProps - pointer to characteristic properties

    @return      none
*/
static void ggs_SetAttrWPermit( uint8 wPermit, uint8* pPermissions, uint8* pCharProps )
{
    // Update attribute Write access permissions
    if ( gattPermitWrite( wPermit ) )
    {
        *pPermissions |= GATT_PERMIT_WRITE;
    }
    else
    {
        *pPermissions &= ~GATT_PERMIT_WRITE;
    }

    if ( gattPermitAuthenWrite( wPermit ) )
    {
        *pPermissions |= GATT_PERMIT_AUTHEN_WRITE;
    }
    else
    {
        *pPermissions &= ~GATT_PERMIT_AUTHEN_WRITE;
    }

    if ( gattPermitAuthorWrite( wPermit ) )
    {
        *pPermissions |= GATT_PERMIT_AUTHOR_WRITE;
    }
    else
    {
        *pPermissions &= ~GATT_PERMIT_AUTHOR_WRITE;
    }

    // Update attribute Write characteristic properties
    if ( gattPermitWrite( wPermit )       ||
            gattPermitAuthenWrite( wPermit ) ||
            gattPermitAuthorWrite( wPermit ) )
    {
        *pCharProps |= (GATT_PROP_WRITE_NO_RSP | GATT_PROP_WRITE);
    }
    else if ( wPermit == 0 )
    {
        // Attribute not Writable
        *pCharProps &= ~(GATT_PROP_WRITE_NO_RSP | GATT_PROP_WRITE);
    }
}

/*********************************************************************
    @fn          ggs_ReadAttrCB

    @brief       Read an attribute.

    @param       connHandle - connection message was received on
    @param       pAttr - pointer to attribute
    @param       pValue - pointer to data to be read
    @param       pLen - length of data to be read
    @param       offset - offset of the first octet to be read
    @param       maxLen - maximum length of data to be read

    @return      Success or Failure
*/
static uint8 ggs_ReadAttrCB( uint16 connHandle, gattAttribute_t* pAttr,
                             uint8* pValue, uint16* pLen, uint16 offset, uint8 maxLen )
{
    uint16 uuid;
    bStatus_t status = SUCCESS;
    VOID connHandle; // Not needed for now!

    // Make sure it's not a blob operation
    if ( offset > 0 )
    {
        return ( ATT_ERR_ATTR_NOT_LONG );
    }

    if ( pAttr->type.len == ATT_BT_UUID_SIZE )
    {
        // 16-bit UUID
        uuid = BUILD_UINT16( pAttr->type.uuid[0], pAttr->type.uuid[1]);

        switch ( uuid )
        {
        case DEVICE_NAME_UUID:
        {
            uint8 len = osal_strlen( (char*)(pAttr->pValue) );

            // If the attribute value is longer than maxLen then maxLen
            // octets shall be included in this response.
            if ( len > maxLen )
            {
                len = maxLen;
            }

            *pLen = len;
            VOID osal_memcpy( pValue, pAttr->pValue, len );
        }
        break;

        case APPEARANCE_UUID:
        {
            uint16 value = *((uint16*)(pAttr->pValue));
            *pLen = 2;
            pValue[0] = LO_UINT16( value );
            pValue[1] = HI_UINT16( value );
        }
        break;

        case RECONNECT_ADDR_UUID:
            *pLen = B_ADDR_LEN;
            VOID osal_memcpy( pValue, pAttr->pValue, B_ADDR_LEN );
            break;

        case PERI_PRIVACY_FLAG_UUID:
            *pLen = 1;
            *pValue = *pAttr->pValue;
            break;

        case PERI_CONN_PARAM_UUID:
            if ( pAttr->pValue != NULL )
            {
                gapPeriConnectParams_t* pConnectParam = (gapPeriConnectParams_t*)(pAttr->pValue);
                *pLen = 8;
                pValue[0] = LO_UINT16( pConnectParam->intervalMin );
                pValue[1] = HI_UINT16( pConnectParam->intervalMin );
                pValue[2] = LO_UINT16( pConnectParam->intervalMax );
                pValue[3] = HI_UINT16( pConnectParam->intervalMax );
                pValue[4] = LO_UINT16( pConnectParam->latency );
                pValue[5] = HI_UINT16( pConnectParam->latency );
                pValue[6] = LO_UINT16( pConnectParam->timeout );
                pValue[7] = HI_UINT16( pConnectParam->timeout );
            }
            else
            {
                *pLen = 0;
            }

            break;

        default:
            // Should never get here!
            *pLen = 0;
            status = ATT_ERR_INVALID_HANDLE;
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
    @fn      ggs_ValidateWriteAttrCB

    @brief   Validate and Write attribute data

    @param   connHandle - connection message was received on
    @param   pAttr - pointer to attribute
    @param   pValue - pointer to data to be written
    @param   len - length of data
    @param   offset - offset of the first octet to be written

    @return  Success or Failure
*/
static bStatus_t ggs_WriteAttrCB( uint16 connHandle, gattAttribute_t* pAttr,
                                  uint8* pValue, uint16 len, uint16 offset )
{
    bStatus_t status = SUCCESS;
    VOID connHandle; // Not needed for now!

    if ( pAttr->type.len == ATT_BT_UUID_SIZE )
    {
        // 16-bit UUID
        uint16 uuid = BUILD_UINT16( pAttr->type.uuid[0], pAttr->type.uuid[1]);

        switch ( uuid )
        {
        case DEVICE_NAME_UUID:
            // Validate the long value
        {
            uint8 curLen = osal_strlen( (char*)(pAttr->pValue) );

            // If the value offset is greater than the current length of the
            // attribute value then an Error Response shall be sent with the
            // error code Invalid Offset.
            if ( offset <= curLen )
            {
                // Always leave room for null-terminate char
                if ( ( offset + len ) > GAP_DEVICE_NAME_LEN )
                {
                    // Appliction error
                    status = ATT_ERR_INVALID_VALUE_SIZE;
                }
            }
            else
            {
                status = ATT_ERR_INVALID_OFFSET;
            }
        }

            // Write the long value
        if ( status == SUCCESS )
        {
            VOID osal_memcpy( &(pAttr->pValue[offset]), pValue, len );
            offset += len;
            pAttr->pValue[offset] = '\0';

            // Notify application
            if ( ggs_AppCBs && ggs_AppCBs->pfnAttrValueChange )
            {
                ggs_AppCBs->pfnAttrValueChange( GGS_DEVICE_NAME_ID );
            }
        }

        break;

        case APPEARANCE_UUID:

            // Validate the value
            if ( offset == 0 )
            {
                if ( len != 2 )
                {
                    status = ATT_ERR_INVALID_VALUE_SIZE;
                }
            }
            else
            {
                status = ATT_ERR_ATTR_NOT_LONG;
            }

            // Write the value
            if ( status == SUCCESS )
            {
                uint16* pCurValue = (uint16*)pAttr->pValue;
                *pCurValue = BUILD_UINT16( pValue[0], pValue[1] );

                // Notify application
                if ( ggs_AppCBs && ggs_AppCBs->pfnAttrValueChange )
                {
                    ggs_AppCBs->pfnAttrValueChange( GGS_APPEARANCE_ID );
                }
            }

            break;

        case RECONNECT_ADDR_UUID:
            // Validate the value - writable by a bonded device
            #if ( HOST_CONFIG & PERIPHERAL_CFG )
            #if defined (GAP_PRIVACY) || defined (GAP_PRIVACY_RECONNECT)
            if ( periPrivacyFlag == GAP_PRIVACY_DISABLED )
            {
                status = ATT_ERR_WRITE_NOT_PERMITTED;
            }
            else
            #endif // GAP_PRIVACY || GAP_PRIVACY_RECONNECT
            #endif // PERIPHERAL_CFG
                if ( offset == 0 )
                {
                    if ( len != B_ADDR_LEN )
                    {
                        status = ATT_ERR_INVALID_VALUE_SIZE;
                    }
                }
                else
                {
                    status = ATT_ERR_ATTR_NOT_LONG;
                }

            // Write the value
            if ( status == SUCCESS )
            {
                VOID osal_memcpy( pAttr->pValue, pValue, B_ADDR_LEN );
            }

            break;

        case PERI_PRIVACY_FLAG_UUID:
            // Validate the value - writable by a bonded device
            #if ( HOST_CONFIG & PERIPHERAL_CFG )
            #if defined (GAP_PRIVACY) || defined (GAP_PRIVACY_RECONNECT)
            if ( (periPrivacyFlagCharProps & GATT_PROP_WRITE) == 0 )
            {
                status = ATT_ERR_WRITE_NOT_PERMITTED;
            }
            else
            #endif // GAP_PRIVACY || GAP_PRIVACY_RECONNECT
            #endif // PERIPHERAL_CFG
                if ( offset == 0 )
                {
                    if ( len == 1 )
                    {
                        // Validate characteristic configuration bit field
                        if ( ( *pValue != GAP_PRIVACY_DISABLED ) &&
                                ( *pValue != GAP_PRIVACY_ENABLED ) )
                        {
                            status = ATT_ERR_INVALID_VALUE;
                        }
                    }
                    else
                    {
                        status = ATT_ERR_INVALID_VALUE_SIZE;
                    }
                }
                else
                {
                    status = ATT_ERR_ATTR_NOT_LONG;
                }

            // Write the value
            if ( status == SUCCESS )
            {
                *pAttr->pValue = *pValue;
            }

            break;

        default:
            // Should never get here!
            status = ATT_ERR_INVALID_HANDLE;
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
*********************************************************************/
