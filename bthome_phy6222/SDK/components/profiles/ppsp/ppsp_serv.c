
/**************************************************************************************************
    Filename:       ppsp_serv.c
    Revised:
    Revision:

    Description:    This file contains the Simple GATT profile sample GATT service
                  profile for use with the BLE sample application.


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

#include "ppsp_serv.h"
#include "ppsp_impl.h"
#include "log.h"


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


// Simple GATT Profile Service UUID: 0xFFF0
CONST uint8 __ppsp_serv_serv_uuid[ATT_BT_UUID_SIZE] =
{
    LO_UINT16(PPSP_SERV_CFGS_SERV_FEB3_UUID), HI_UINT16(PPSP_SERV_CFGS_SERV_FEB3_UUID)
};

// Characteristic 1 UUID: 0xFFF1
CONST uint8 __ppsp_serv_char_ffd4_uuid[ATT_BT_UUID_SIZE] =
{
    LO_UINT16(PPSP_SERV_CFGS_CHAR_FED4_UUID), HI_UINT16(PPSP_SERV_CFGS_CHAR_FED4_UUID)
};

// Characteristic 2 UUID: 0xFFF2
CONST uint8 __ppsp_serv_char_ffd5_uuid[ATT_BT_UUID_SIZE] =
{
    LO_UINT16(PPSP_SERV_CFGS_CHAR_FED5_UUID), HI_UINT16(PPSP_SERV_CFGS_CHAR_FED5_UUID)
};

// Characteristic 3 UUID: 0xFFF3
CONST uint8 __ppsp_serv_char_ffd6_uuid[ATT_BT_UUID_SIZE] =
{
    LO_UINT16(PPSP_SERV_CFGS_CHAR_FED6_UUID), HI_UINT16(PPSP_SERV_CFGS_CHAR_FED6_UUID)
};

// Characteristic 4 UUID: 0xFFF4
CONST uint8 __ppsp_serv_char_ffd7_uuid[ATT_BT_UUID_SIZE] =
{
    LO_UINT16(PPSP_SERV_CFGS_CHAR_FED7_UUID), HI_UINT16(PPSP_SERV_CFGS_CHAR_FED7_UUID)
};

// Characteristic 5 UUID: 0xFFF5
CONST uint8 __ppsp_serv_char_ffd8_uuid[ATT_BT_UUID_SIZE] =
{
    LO_UINT16(PPSP_SERV_CFGS_CHAR_FED8_UUID), HI_UINT16(PPSP_SERV_CFGS_CHAR_FED8_UUID)
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

static ppsp_serv_appl_CBs_t* simpleProfile_AppCBs = NULL;

/*********************************************************************
    Profile Attributes - variables
*/

// Simple Profile Service attribute
static CONST gattAttrType_t
__ppsp_serv_serv = { ATT_BT_UUID_SIZE, __ppsp_serv_serv_uuid };


// Simple Profile Characteristic 1 Properties
static uint8
__ppsp_serv_char_ffd4_prop = GATT_PROP_READ;

// Characteristic 1 Value
static uint8
__ppsp_serv_char_ffd4_para[PPSP_SERV_CFGS_CHAR_FED4_DLEN];
static uint16
__ppsp_serv_char_ffd4_para_size = 0;
// Simple Profile Characteristic 1 User Description
// static uint8
// __ppsp_serv_char_ffd4_desc[] = "READ\0";


// Simple Profile Characteristic 2 Properties
static uint8
__ppsp_serv_char_ffd5_prop = GATT_PROP_WRITE;

// Characteristic 2 Value
static uint8
__ppsp_serv_char_ffd5_para[PPSP_SERV_CFGS_CHAR_FED5_DLEN];

// Simple Profile Characteristic 2 User Description
// static uint8
// __ppsp_serv_char_ffd5_desc[] = "WRITE\0";


// Simple Profile Characteristic 3 Properties
static uint8
__ppsp_serv_char_ffd6_prop = GATT_PROP_READ | GATT_PROP_INDICATE;

// Characteristic 3 Value
static uint8
__ppsp_serv_char_ffd6_para[PPSP_SERV_CFGS_CHAR_FED6_DLEN];
static uint16
__ppsp_serv_char_ffd6_para_size = 0;
// Simple Profile Characteristic 4 Configuration Each client has its own
// instantiation of the Client Characteristic Configuration. Reads of the
// Client Characteristic Configuration only shows the configuration for
// that client and writes only affect the configuration of that client.
static gattCharCfg_t
__ppsp_serv_char_ffd6_cfgs[GATT_MAX_NUM_CONN];

// Simple Profile Characteristic 3 User Description
// static uint8
// __ppsp_serv_char_ffd6_desc[] = "INDICATE\0";


// Simple Profile Characteristic 4 Properties
static uint8
__ppsp_serv_char_ffd7_prop = GATT_PROP_WRITE_NO_RSP;

// Characteristic 4 Value
static uint8
__ppsp_serv_char_ffd7_para[PPSP_SERV_CFGS_CHAR_FED7_DLEN];

// Simple Profile Characteristic 4 User Description
// static uint8
// __ppsp_serv_char_ffd7_desc[] = "WRITE_WITH_NO_RESP\0";


// Simple Profile Characteristic 5 Properties
static uint8
__ppsp_serv_char_ffd8_prop = GATT_PROP_READ | GATT_PROP_NOTIFY;   // to change to write only, HZF

// Characteristic 5 Value
static uint8
__ppsp_serv_char_ffd8_para[PPSP_SERV_CFGS_CHAR_FED8_DLEN];
static uint16
__ppsp_serv_char_ffd8_para_size = 0;
// Simple Profile Characteristic 1 Configuration Each client has its own
// instantiation of the Client Characteristic Configuration. Reads of the
// Client Characteristic Configuration only shows the configuration for
// that client and writes only affect the configuration of that client.
static gattCharCfg_t __ppsp_serv_char_ffd8_cfgs[GATT_MAX_NUM_CONN];

// Simple Profile Characteristic 5 User Description
// static uint8
// __ppsp_serv_char_ffd8_desc[] = "NOTIFY\0";



/*********************************************************************
    Profile Attributes - Table
*/

static gattAttribute_t simpleProfileAttrTbl[] =
{
    // Simple Profile Service
    {
        { ATT_BT_UUID_SIZE, primaryServiceUUID }, /* type */
        GATT_PERMIT_READ,                         /* permissions */
        0,                                        /* handle */
        (uint8*)& __ppsp_serv_serv                /* pValue */
    },

    // Characteristic 1 Declaration
    {
        { ATT_BT_UUID_SIZE, characterUUID },
        GATT_PERMIT_READ,
        0,
        &__ppsp_serv_char_ffd4_prop
    },
    // Characteristic Value 1
    {
        { ATT_BT_UUID_SIZE, __ppsp_serv_char_ffd4_uuid },
        GATT_PERMIT_READ | GATT_PERMIT_WRITE,
        0,
        __ppsp_serv_char_ffd4_para
    },
    // Characteristic 1 User Description
    /*  {
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ,
        0,
        __ppsp_serv_char_FFD4_desc
        }, */

    // Characteristic 2 Declaration
    {
        { ATT_BT_UUID_SIZE, characterUUID },
        GATT_PERMIT_READ,
        0,
        &__ppsp_serv_char_ffd5_prop
    },
    // Characteristic Value 2
    {
        { ATT_BT_UUID_SIZE, __ppsp_serv_char_ffd5_uuid },
        GATT_PERMIT_READ | GATT_PERMIT_WRITE,
        0,
        __ppsp_serv_char_ffd5_para
    },
    // Characteristic 2 User Description
    /*  {
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ,
        0,
        __ppsp_serv_char_FFD5_desc
        }, */

    // Characteristic 3 Declaration
    {
        { ATT_BT_UUID_SIZE, characterUUID },
        GATT_PERMIT_READ,
        0,
        &__ppsp_serv_char_ffd6_prop
    },
    // Characteristic Value 3
    {
        { ATT_BT_UUID_SIZE, __ppsp_serv_char_ffd6_uuid },
        GATT_PERMIT_READ | GATT_PERMIT_WRITE,
        0,
        __ppsp_serv_char_ffd6_para
    },
    // Characteristic 3 configuration
    {
        { ATT_BT_UUID_SIZE, clientCharCfgUUID },
        GATT_PERMIT_READ | GATT_PERMIT_WRITE,
        0,
        (uint8*)__ppsp_serv_char_ffd6_cfgs
    },
    // Characteristic 3 User Description
    /*  {
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ,
        0,
        __ppsp_serv_char_FFD6_desc
        }, */

    // Characteristic 4 Declaration
    {
        { ATT_BT_UUID_SIZE, characterUUID },
        GATT_PERMIT_READ,
        0,
        &__ppsp_serv_char_ffd7_prop
    },
    // Characteristic Value 4
    {
        { ATT_BT_UUID_SIZE, __ppsp_serv_char_ffd7_uuid },
        GATT_PERMIT_READ | GATT_PERMIT_WRITE,
        0,
        __ppsp_serv_char_ffd7_para
    },
    // Characteristic 4 User Description
    /*  {
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ,
        0,
        __ppsp_serv_char_FFD7_desc
        }, */

    // Characteristic 5 Declaration
    {
        { ATT_BT_UUID_SIZE, characterUUID },
        GATT_PERMIT_READ,
        0,
        &__ppsp_serv_char_ffd8_prop
    },
    // Characteristic Value 5
    {
        { ATT_BT_UUID_SIZE, __ppsp_serv_char_ffd8_uuid },
        GATT_PERMIT_READ | GATT_PERMIT_WRITE,
        0,
        __ppsp_serv_char_ffd8_para
    },
    // Characteristic 5 configuration
    {
        { ATT_BT_UUID_SIZE, clientCharCfgUUID },
        GATT_PERMIT_READ | GATT_PERMIT_WRITE,
        0,
        (uint8*)__ppsp_serv_char_ffd8_cfgs
    },
    // Characteristic 5 User Description
    /*  {
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ,
        0,
        __ppsp_serv_char_FFD8_desc
        }, */
};


/*********************************************************************
    LOCAL FUNCTIONS
*/
static uint8
ppsp_serv_get_attr( uint16 connHandle, gattAttribute_t* pAttr,
                    uint8* pValue, uint16* pLen, uint16 offset, uint8 maxLen );
static bStatus_t
ppsp_serv_set_attr( uint16 connHandle, gattAttribute_t* pAttr,
                    uint8* pValue, uint16 len, uint16 offset );
static void
ppsp_serv_hdlr_conn_stat( uint16 connHandle, uint8 changeType );

// static bStatus_t send_notity_indication( gattCharCfg_t *charCfgTbl, uint8 *pValue,
//                                       uint8 authenticated, gattAttribute_t *attrTbl,
//                                       uint16 numAttrs, uint8 taskId,uint8 dlen);


/*********************************************************************
    @fn          simpleProfile_ReadAttrCB

    @brief       Read an attribute.

    @param       connHandle - connection message was received on
    @param       pAttr - pointer to attribute
    @param       pValue - pointer to data to be read
    @param       pLen - length of data to be read
    @param       offset - offset of the first octet to be read
    @param       maxLen - maximum length of data to be read

    @return      Success or Failure
*/
static uint8 ppsp_serv_get_attr( uint16 connHandle, gattAttribute_t* pAttr,
                                 uint8* pValue, uint16* pLen, uint16 offset, uint8 maxLen )
{
    bStatus_t status = SUCCESS;

    // LOG("[PANDA][ENT] %s len:%d, offs:%d, max:%d \r\n", __func__, *pLen, offset, maxLen);
    // printf("[PANDA][INF] mtu:%d", ATT_GetCurrentMTUSize());

    // If attribute permissions require authorization to read, return error
    if ( gattPermitAuthorRead( pAttr->permissions ) )
    {
        // Insufficient authorization
        return ( ATT_ERR_INSUFFICIENT_AUTHOR );
    }

    // Make sure it's not a blob operation (no attributes in the profile are long)
    // if ( 0 <  offset ) {
    //   return ( ATT_ERR_ATTR_NOT_LONG );
    // }

    if ( pAttr->type.len == ATT_BT_UUID_SIZE )
    {
        // 16-bit UUID
        uint16 uuid = BUILD_UINT16( pAttr->type.uuid[0], pAttr->type.uuid[1]);
        // LOG("[PANDA][INF] uuid:%x \r\n", uuid);
        uint16 gsiz = offset + 1; // size of read
        uint16 ssiz = 0;          // size of left

        switch ( uuid )
        {
        // No need for "GATT_SERVICE_UUID" or "GATT_CLIENT_CHAR_CFG_UUID" cases;
        // gattserverapp handles those reads
        case PPSP_SERV_CFGS_CHAR_FED4_UUID:
        {
            ssiz = PPSP_SERV_CFGS_CHAR_FED4_DLEN - gsiz;
            *pLen = ssiz > maxLen ? maxLen : ssiz;
            VOID osal_memcpy( pValue, pAttr->pValue + offset, *pLen );
        }
        break;

        case PPSP_SERV_CFGS_CHAR_FED5_UUID:
        {
            ssiz = PPSP_SERV_CFGS_CHAR_FED5_DLEN - gsiz;
            *pLen = ssiz > maxLen ? maxLen : ssiz;
            VOID osal_memcpy( pValue, pAttr->pValue + offset, *pLen );
        }
        break;

        case PPSP_SERV_CFGS_CHAR_FED6_UUID:
        {
            ssiz = PPSP_SERV_CFGS_CHAR_FED6_DLEN - gsiz;
            *pLen = ssiz > maxLen ? maxLen : ssiz;
            //osal_memcpy( pValue, pAttr->pValue + offset, *pLen );
            osal_memcpy( pValue, pAttr->pValue, *pLen = __ppsp_serv_char_ffd6_para_size );
        }
        break;

        case PPSP_SERV_CFGS_CHAR_FED7_UUID:
        {
            ssiz = PPSP_SERV_CFGS_CHAR_FED7_DLEN - gsiz;
            *pLen = ssiz > maxLen ? maxLen : ssiz;
            VOID osal_memcpy( pValue, pAttr->pValue + offset, *pLen );
        }
        break;

        case PPSP_SERV_CFGS_CHAR_FED8_UUID:
        {
            ssiz = PPSP_SERV_CFGS_CHAR_FED8_DLEN - gsiz;
            *pLen = ssiz > maxLen ? maxLen : ssiz;
            // VOID osal_memcpy( pValue, pAttr->pValue + offset, *pLen );
            osal_memcpy( pValue, pAttr->pValue, *pLen = __ppsp_serv_char_ffd8_para_size );
        }
        break;

        default:
            // Should never get here! (characteristics 3 and 4 do not have read permissions)
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
    @fn      simpleProfile_WriteAttrCB

    @brief   Validate attribute data prior to a write operation

    @param   connHandle - connection message was received on
    @param   pAttr - pointer to attribute
    @param   pValue - pointer to data to be written
    @param   len - length of data
    @param   offset - offset of the first octet to be written

    @return  Success or Failure
*/
// TODO: test this function
static bStatus_t ppsp_serv_set_attr( uint16 connHandle, gattAttribute_t* pAttr,
                                     uint8* pValue, uint16 len, uint16 offset )
{
    bStatus_t status = SUCCESS;
    uint8 upda_para = 0xFF;

    // LOG("[PANDA][ENT] %s: len:%d, offs:%d \n\r", __func__, len, offset);
    // printf("[PANDA][ENT] %s: len:%d, offs:%d \n\r", __func__, len, offset);
    // LOG("[PANDA][INF] mtu:%d", ATT_GetCurrentMTUSize());
    // LOG("\r\n[DUMP] >> "); for ( int itr0 = 0; itr0 < 4; itr0 += 1 ) LOG("%02x,", pValue[itr0]); LOG("[DUMP] << \r\n");

    // If attribute permissions require authorization to write, return error
    if ( gattPermitAuthorWrite( pAttr->permissions ) )
    {
        // Insufficient authorization
        return ( ATT_ERR_INSUFFICIENT_AUTHOR );
    }

    // Make sure it's not a blob operation (no attributes in the profile are long)
    // if ( 0 <  offset ) {
    //   return ( ATT_ERR_ATTR_NOT_LONG );
    // }

    if ( pAttr->type.len == ATT_BT_UUID_SIZE )
    {
        // 16-bit UUID
        uint16 uuid = BUILD_UINT16( pAttr->type.uuid[0], pAttr->type.uuid[1]);

        switch ( uuid )
        {
        case PPSP_SERV_CFGS_CHAR_FED4_UUID:
        {
            if ( PPSP_SERV_CFGS_CHAR_FED4_DLEN <= offset )
            {
                status = ATT_ERR_INVALID_VALUE_SIZE;
            }
            else
            {
                // Write the value
                VOID osal_memcpy( pAttr->pValue + offset, pValue, len );
                upda_para = PPSP_SERV_CFGS_CHAR_FFD4_INDX;
            }
        }
        break;

        case PPSP_SERV_CFGS_CHAR_FED5_UUID:
        {
            if ( PPSP_SERV_CFGS_CHAR_FED5_DLEN <= offset )
            {
                status = ATT_ERR_INVALID_VALUE_SIZE;
            }
            else
            {
                // Write the value
                VOID osal_memcpy( pAttr->pValue + offset, pValue, PPSP_SERV_CFGS_CHAR_FED5_DLEN );
                upda_para = PPSP_SERV_CFGS_CHAR_FFD5_INDX;
            }
        }
        break;

        case PPSP_SERV_CFGS_CHAR_FED6_UUID:
        {
            if ( PPSP_SERV_CFGS_CHAR_FED6_DLEN <= offset )
            {
                status = ATT_ERR_INVALID_VALUE_SIZE;
            }
            else
            {
                // Write the value
                VOID osal_memcpy( pAttr->pValue + offset, pValue, PPSP_SERV_CFGS_CHAR_FED6_DLEN );
                upda_para = PPSP_SERV_CFGS_CHAR_FFD6_INDX;
            }
        }
        break;

        case PPSP_SERV_CFGS_CHAR_FED7_UUID:
        {
            if ( PPSP_SERV_CFGS_CHAR_FED7_DLEN <= offset )
            {
                status = ATT_ERR_INVALID_VALUE_SIZE;
            }
            else
            {
                // Write the value
                VOID osal_memcpy( pAttr->pValue + offset, pValue, PPSP_SERV_CFGS_CHAR_FED7_DLEN );
                upda_para = PPSP_SERV_CFGS_CHAR_FFD7_INDX;
            }
        }
        break;

        case PPSP_SERV_CFGS_CHAR_FED8_UUID:
        {
            if ( PPSP_SERV_CFGS_CHAR_FED8_DLEN <= offset )
            {
                status = ATT_ERR_INVALID_VALUE_SIZE;
            }
            else
            {
                // Write the value
                VOID osal_memcpy( pAttr->pValue + offset, pValue, PPSP_SERV_CFGS_CHAR_FED8_DLEN );
                upda_para = PPSP_SERV_CFGS_CHAR_FFD8_INDX;
            }
        }
        break;

        case GATT_CLIENT_CHAR_CFG_UUID:
        {
            // Validate/Write Temperature measurement setting
            if ( pAttr->handle == simpleProfileAttrTbl[7].handle )
            {
                // LOG("[PANDA][INF] CHAR FED6 CFGS: %d \n\r", *pValue);
                uint16 valu;

                if ( 0 != *pValue ) valu = 0x02;
                else                valu = 0x00;

                status = GATTServApp_ProcessCCCWriteReq(connHandle, pAttr, (uint8*)&valu, sizeof(valu),
                                                        offset, GATT_CLIENT_CFG_INDICATE );
            }
            else if ( pAttr->handle == simpleProfileAttrTbl[12].handle )
            {
                // LOG("[PANDA][INF] CHAR FED8 CFGS: %d \n\r", *pValue);
                status = GATTServApp_ProcessCCCWriteReq(connHandle, pAttr, pValue, 2,
                                                        offset, GATT_CLIENT_CFG_NOTIFY );
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
    if ( upda_para != 0xFF &&
            NULL != simpleProfile_AppCBs &&
            NULL != simpleProfile_AppCBs->char_upda )
    {
        simpleProfile_AppCBs->char_upda( upda_para, len );
    }

    return ( status );
}

/*********************************************************************
    @fn          simpleProfile_HandleConnStatusCB

    @brief       Simple Profile link status change handler function.

    @param       conn_hdle - connection handle
    @param       changeType - type of change

    @return      none
*/
static void ppsp_serv_hdlr_conn_stat( uint16 conn_hdle, uint8 chng_type )
{
    // Make sure this is not loopback connection
    if ( conn_hdle != LOOPBACK_CONNHANDLE )
    {
        // Reset Client Char Config if connection has dropped
        if ( ( chng_type == LINKDB_STATUS_UPDATE_REMOVED )      ||
                ( ( chng_type == LINKDB_STATUS_UPDATE_STATEFLAGS ) &&
                  ( !linkDB_Up( conn_hdle ) ) ) )
        {
            // __ppsp_impl_update_cnt = 0;
            GATTServApp_InitCharCfg( conn_hdle, __ppsp_serv_char_ffd6_cfgs );
            GATTServApp_InitCharCfg( conn_hdle, __ppsp_serv_char_ffd8_cfgs );
            ppsp_impl_ack_conn(0);  // ack upper impl loss of connection
        }
    }
}

/*********************************************************************
    PROFILE CALLBACKS
*/
// Simple Profile Service Callbacks
CONST gattServiceCBs_t ppspCBs =
{
    ppsp_serv_get_attr,   // Read callback function pointer
    ppsp_serv_set_attr,   // Write callback function pointer
    NULL,                 // Authorization callback function pointer
};

/*********************************************************************
    PUBLIC FUNCTIONS
*/

/*********************************************************************
    @fn      SimpleProfile_AddService

    @brief   Initializes the Simple Profile service by registering
            GATT attributes with the GATT server.

    @param   services - services to add. This is a bit map and can
                       contain more than one service.

    @return  Success or Failure
*/
bStatus_t ppsp_serv_add_serv(uint32 serv)
{
    uint8 status = SUCCESS;
    // Initialize Client Characteristic Configuration attributes
    GATTServApp_InitCharCfg( INVALID_CONNHANDLE, __ppsp_serv_char_ffd6_cfgs );
    GATTServApp_InitCharCfg( INVALID_CONNHANDLE, __ppsp_serv_char_ffd8_cfgs );
    // Register with Link DB to receive link status change callback
    VOID linkDB_Register( ppsp_serv_hdlr_conn_stat );

    if ( serv & PPSP_SERV_CFGS_SERV_FEB3_MASK )
    {
        // Register GATT attribute list and CBs with GATT Server App
        status = GATTServApp_RegisterService( simpleProfileAttrTbl,
                                              GATT_NUM_ATTRS( simpleProfileAttrTbl ),
                                              &ppspCBs );
    }

    // for (int i = 0; i < PPSP_SERV_CFGS_CHAR_FED4_DLEN; i++)
    // {
    //   __ppsp_serv_char_FFD4_para[i] =i;
    // }
    return ( status );
}


/*********************************************************************
    @fn      SimpleProfile_RegisterAppCBs

    @brief   Registers the application callback function. Only call
            this function once.

    @param   callbacks - pointer to application callbacks.

    @return  SUCCESS or bleAlreadyInRequestedMode
*/
bStatus_t ppsp_serv_reg_appl( ppsp_serv_appl_CBs_t* appCallbacks )
{
    if ( appCallbacks )
    {
        simpleProfile_AppCBs = appCallbacks;
        return ( SUCCESS );
    }
    else
    {
        return ( bleAlreadyInRequestedMode );
    }
}


/*********************************************************************
    @fn      SimpleProfile_SetParameter

    @brief   Set a Simple Profile parameter.

    @param   param - Profile parameter ID
    @param   len - length of data to right
    @param   value - pointer to data to write.  This is dependent on
            the parameter ID and WILL be cast to the appropriate
            data type (example: data type of uint16 will be cast to
            uint16 pointer).

    @return  bStatus_t
*/
bStatus_t ppsp_serv_set_para( uint8 para, uint16 leng, void* valu )
{
    // LOG("[PANDA][ENT] %s: para:%d, leng:%d, valu:%x \n\r", __func__, para, leng, valu);
    // LOG("\r\n[DUMP] >> "); for ( int itr0 = 0; itr0 < leng; itr0 += 1 ) LOG("%02x,", ((uint8*)valu)[itr0]); LOG("[DUMP] << \r\n");
    bStatus_t ret = SUCCESS;

    switch ( para )
    {
    case PPSP_SERV_CFGS_CHAR_FFD4_INDX:
    {
        if ( leng <= PPSP_SERV_CFGS_CHAR_FED4_DLEN )
        {
            osal_memcpy(__ppsp_serv_char_ffd4_para, valu, __ppsp_serv_char_ffd4_para_size = leng);
        }
        else
        {
            ret = bleInvalidRange;
        }
    }
    break;

    // case PPSP_SERV_CFGS_CHAR_2_INDX:
    //   if ( len <= PPSP_SERV_CFGS_CHAR_2_DLEN )
    //   {
    //     osal_memcpy(__ppsp_serv_char_2_para, value, len);
    //   }
    //   else
    //   {
    //     ret = bleInvalidRange;
    //   }
    //   break;

    // case PPSP_SERV_CFGS_CHAR_3_INDX:
    //   if ( len == sizeof ( uint16 ) )
    //   {
    //     // simpleProfileChar3 = (*(uint8 *)value) << 8 | *((uint8 *)value + 1);
    //   }
    //   else
    //   {
    //     ret = bleInvalidRange;
    //   }
    //   break;

    case PPSP_SERV_CFGS_CHAR_FFD6_INDX:
    {
        uint16 cfgs = GATTServApp_ReadCharCfg( 0, __ppsp_serv_char_ffd6_cfgs );

        if ( GATT_CLIENT_CFG_INDICATE == cfgs )
        {
            if ( leng <= PPSP_SERV_CFGS_CHAR_FED6_DLEN )
            {
                osal_memcpy( __ppsp_serv_char_ffd6_para, valu, __ppsp_serv_char_ffd6_para_size = leng );
                // send Noti/Indi if enabled
                GATTServApp_ProcessCharCfg( __ppsp_serv_char_ffd6_cfgs,
                                            __ppsp_serv_char_ffd6_para,
                                            FALSE,
                                            simpleProfileAttrTbl,
                                            GATT_NUM_ATTRS( simpleProfileAttrTbl ),
                                            INVALID_TASK_ID );
                /*  send_notity_indication( __ppsp_serv_char_ffd6_cfgs,
                                            __ppsp_serv_char_ffd6_para,
                                            FALSE,
                                            simpleProfileAttrTbl,
                                            GATT_NUM_ATTRS( simpleProfileAttrTbl ),
                                            INVALID_TASK_ID,__ppsp_serv_char_ffd6_para_size ); */
            }
            else
            {
                ret = bleInvalidRange;
            }
        }
    }
    break;

    // case PPSP_SERV_CFGS_CHAR_5_INDX:
    // if ( leng == sizeof ( uint8 ) )
    // {
    //   // simpleProfileChar5 = *((uint8*)value);
    // }
    // else
    // {
    //   ret = bleInvalidRange;
    // }
    // break;

    case PPSP_SERV_CFGS_CHAR_FFD8_INDX:
    {
        uint16 cfgs = GATTServApp_ReadCharCfg( 0, __ppsp_serv_char_ffd8_cfgs );

        if ( GATT_CLIENT_CFG_NOTIFY == cfgs )
        {
            if ( leng <= PPSP_SERV_CFGS_CHAR_FED8_DLEN )
            {
                osal_memcpy( __ppsp_serv_char_ffd8_para, valu, __ppsp_serv_char_ffd8_para_size = leng );
                // send Noti/Indi if enabled
                GATTServApp_ProcessCharCfg( __ppsp_serv_char_ffd8_cfgs,
                                            __ppsp_serv_char_ffd8_para,
                                            FALSE,
                                            simpleProfileAttrTbl,
                                            GATT_NUM_ATTRS( simpleProfileAttrTbl ),
                                            INVALID_TASK_ID );
                /*  send_notity_indication( __ppsp_serv_char_ffd8_cfgs,
                                    __ppsp_serv_char_ffd8_para,
                                    FALSE,
                                    simpleProfileAttrTbl,
                                    GATT_NUM_ATTRS( simpleProfileAttrTbl ),
                                    INVALID_TASK_ID,__ppsp_serv_char_ffd8_para_size ); */
            }
            else
            {
                ret = bleInvalidRange;
            }
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
    @fn      SimpleProfile_GetParameter

    @brief   Get a Simple Profile parameter.

    @param   param - Profile parameter ID
    @param   value - pointer to data to put.  This is dependent on
            the parameter ID and WILL be cast to the appropriate
            data type (example: data type of uint16 will be cast to
            uint16 pointer).

    @return  bStatus_t
*/
bStatus_t ppsp_serv_get_para( uint8 para, void* valu, uint16 leng )
{
    bStatus_t ret = SUCCESS;

    switch ( para )
    {
    case PPSP_SERV_CFGS_CHAR_FFD4_INDX:
        VOID osal_memcpy( valu, __ppsp_serv_char_ffd4_para, leng );
        break;

    case PPSP_SERV_CFGS_CHAR_FFD5_INDX:
        VOID osal_memcpy( valu, __ppsp_serv_char_ffd5_para, leng );
        break;

    case PPSP_SERV_CFGS_CHAR_FFD6_INDX:
        // *((uint16*)value) = simpleProfileChar3;
        break;

    case PPSP_SERV_CFGS_CHAR_FFD7_INDX:
        VOID osal_memcpy( valu, __ppsp_serv_char_ffd7_para, leng );
        break;

    case PPSP_SERV_CFGS_CHAR_FFD8_INDX:
        // *((uint8*)value) = simpleProfileChar5;
        break;

    default:
        ret = INVALIDPARAMETER;
        break;
    }

    return ( ret );
}


/*  static bStatus_t send_notity_indication( gattCharCfg_t *charCfgTbl, uint8 *pValue,
                                      uint8 authenticated, gattAttribute_t *attrTbl,
                                      uint16 numAttrs, uint8 taskId,uint8 dlen)
    {
    bStatus_t status = SUCCESS;

    for ( uint8 i = 0; i < GATT_MAX_NUM_CONN; i++ )
    {
    gattCharCfg_t *pItem = &(charCfgTbl[i]);

    if ( ( pItem->connHandle != INVALID_CONNHANDLE ) &&
         ( pItem->value != GATT_CFG_NO_OPERATION ) )
    {
      gattAttribute_t *pAttr;

      // Find the characteristic value attribute
      pAttr = GATTServApp_FindAttr( attrTbl, numAttrs, pValue );
      if ( pAttr != NULL )
      {
        attHandleValueNoti_t noti;

        // If the attribute value is longer than (ATT_MTU - 3) octets, then
        // only the first (ATT_MTU - 3) octets of this attributes value can
        // be sent in a notification.
        if ( GATTServApp_ReadAttr( pItem->connHandle, pAttr,
                                   GATT_SERVICE_HANDLE( attrTbl ), noti.value,
                                   &noti.len, 0, (g_ATT_MTU_SIZE-3) ) == SUCCESS )
        {
          noti.handle = pAttr->handle;

          if(dlen)
            noti.len=dlen;

          if ( pItem->value & GATT_CLIENT_CFG_NOTIFY )
          {
            status |= GATT_Notification( pItem->connHandle, &noti, authenticated );
          }

          if ( pItem->value & GATT_CLIENT_CFG_INDICATE )
          {
            status |= GATT_Indication( pItem->connHandle, (attHandleValueInd_t *)&noti,
                                       authenticated, taskId );
          }
        }
      }
    }
    } // for

    return ( status );
    } */

/*********************************************************************
*********************************************************************/
