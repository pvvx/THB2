/**************************************************************************************************
    Filename:       gatt_uuid.c
    Revised:
    Revision:

    Description:    This file contains Generic Attribute Profile (GATT)
                  UUID types.

	SDK_LICENSE

**************************************************************************************************/


/*********************************************************************
    INCLUDES
*/
#include "comdef.h"
#include "OSAL.h"

#include "gatt.h"
#include "gatt_uuid.h"

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

/**
    GATT Services
*/
// Generic Access Profile Service UUID 0x1800
CONST uint8 gapServiceUUID[ATT_BT_UUID_SIZE] =
{
    LO_UINT16( GAP_SERVICE_UUID ), HI_UINT16( GAP_SERVICE_UUID )
};

// Generic Attribute Profile Service UUID 0x1801
CONST uint8 gattServiceUUID[ATT_BT_UUID_SIZE] =
{
    LO_UINT16( GATT_SERVICE_UUID ), HI_UINT16( GATT_SERVICE_UUID )
};

/**
    GATT Declarations
*/
// Primary Service UUID 0x2801
CONST uint8 primaryServiceUUID[ATT_BT_UUID_SIZE] =
{
    LO_UINT16( GATT_PRIMARY_SERVICE_UUID ), HI_UINT16( GATT_PRIMARY_SERVICE_UUID )
};

// Secondary Service UUID 0x2801
CONST uint8 secondaryServiceUUID[ATT_BT_UUID_SIZE] =
{
    LO_UINT16( GATT_SECONDARY_SERVICE_UUID ), HI_UINT16( GATT_SECONDARY_SERVICE_UUID )
};

// Include UUID 0x2802
CONST uint8 includeUUID[ATT_BT_UUID_SIZE] =
{
    LO_UINT16( GATT_INCLUDE_UUID ), HI_UINT16( GATT_INCLUDE_UUID )
};

// Characteristic UUID 0x2803
CONST uint8 characterUUID[ATT_BT_UUID_SIZE] =
{
    LO_UINT16( GATT_CHARACTER_UUID ), HI_UINT16( GATT_CHARACTER_UUID )
};

/**
    GATT Descriptors
*/
// Characteristic Extended Properties UUID 0x2900
CONST uint8 charExtPropsUUID[ATT_BT_UUID_SIZE] =
{
    LO_UINT16( GATT_CHAR_EXT_PROPS_UUID ), HI_UINT16( GATT_CHAR_EXT_PROPS_UUID )
};

// Characteristic User Description UUID 0x2901
CONST uint8 charUserDescUUID[ATT_BT_UUID_SIZE] =
{
    LO_UINT16( GATT_CHAR_USER_DESC_UUID ), HI_UINT16( GATT_CHAR_USER_DESC_UUID )
};

// Client Characteristic Configuration UUID 0x2902
CONST uint8 clientCharCfgUUID[ATT_BT_UUID_SIZE] =
{
    LO_UINT16( GATT_CLIENT_CHAR_CFG_UUID ), HI_UINT16( GATT_CLIENT_CHAR_CFG_UUID )
};

// Server Characteristic Configuration UUID 0x2903
CONST uint8 servCharCfgUUID[ATT_BT_UUID_SIZE] =
{
    LO_UINT16( GATT_SERV_CHAR_CFG_UUID ), HI_UINT16( GATT_SERV_CHAR_CFG_UUID )
};

// Characteristic Presentation Format UUID 0x2904
CONST uint8 charFormatUUID[ATT_BT_UUID_SIZE] =
{
    LO_UINT16( GATT_CHAR_FORMAT_UUID ), HI_UINT16( GATT_CHAR_FORMAT_UUID )
};

// Characteristic Aggregate Format UUID 0x2905
CONST uint8 charAggFormatUUID[ATT_BT_UUID_SIZE] =
{
    LO_UINT16( GATT_CHAR_AGG_FORMAT_UUID ), HI_UINT16( GATT_CHAR_AGG_FORMAT_UUID )
};

/**
    GATT Characteristics
*/
// Device Name UUID
CONST uint8 deviceNameUUID[ATT_BT_UUID_SIZE] =
{
    LO_UINT16( DEVICE_NAME_UUID ), HI_UINT16( DEVICE_NAME_UUID )
};

// Appearance UUID
CONST uint8 appearanceUUID[ATT_BT_UUID_SIZE] =
{
    LO_UINT16( APPEARANCE_UUID ), HI_UINT16( APPEARANCE_UUID )
};

// Peripheral Privacy Flag UUID
CONST uint8 periPrivacyFlagUUID[ATT_BT_UUID_SIZE] =
{
    LO_UINT16( PERI_PRIVACY_FLAG_UUID ), HI_UINT16( PERI_PRIVACY_FLAG_UUID )
};

// Reconnection Address UUID
CONST uint8 reconnectAddrUUID[ATT_BT_UUID_SIZE] =
{
    LO_UINT16( RECONNECT_ADDR_UUID ), HI_UINT16( RECONNECT_ADDR_UUID )
};

// Peripheral Preferred Connection Parameters UUID
CONST uint8 periConnParamUUID[ATT_BT_UUID_SIZE] =
{
    LO_UINT16( PERI_CONN_PARAM_UUID ), HI_UINT16( PERI_CONN_PARAM_UUID )
};

// Service Changed UUID
CONST uint8 serviceChangedUUID[ATT_BT_UUID_SIZE] =
{
    LO_UINT16( SERVICE_CHANGED_UUID ), HI_UINT16( SERVICE_CHANGED_UUID )
};

// Valid Range UUID
CONST uint8 validRangeUUID[ATT_BT_UUID_SIZE] =
{
    LO_UINT16( GATT_VALID_RANGE_UUID ), HI_UINT16( GATT_VALID_RANGE_UUID )
};

// External Report Reference Descriptor
CONST uint8 extReportRefUUID[ATT_BT_UUID_SIZE] =
{
    LO_UINT16( GATT_EXT_REPORT_REF_UUID ), HI_UINT16( GATT_EXT_REPORT_REF_UUID )
};

// Report Reference characteristic descriptor
CONST uint8 reportRefUUID[ATT_BT_UUID_SIZE] =
{
    LO_UINT16( GATT_REPORT_REF_UUID ), HI_UINT16( GATT_REPORT_REF_UUID )
};

/*********************************************************************
    GLOBAL FUNCTIONS
*/

/*********************************************************************
    LOCAL VARIABLES
*/

/*********************************************************************
    LOCAL FUNCTIONS
*/

/*********************************************************************
    API FUNCTIONS
*/

/*********************************************************************
    @fn      GATT_FindUUIDRec

    @brief   Find the UUID record for a given UUID.

    @param   pUUID - UUID to look for.
    @param   len - length of UUID.

    @return  Pointer to UUID record. NULL, otherwise.
*/
const uint8* GATT_FindUUIDRec( const uint8* pUUID, uint8 len )
{
    const uint8* pRec = NULL;

    if ( len == ATT_BT_UUID_SIZE )
    {
        // 16-bit UUID
        uint16 uuid = BUILD_UINT16( pUUID[0], pUUID[1] );

        switch ( uuid )
        {
        /*** GATT Services ***/
        case GAP_SERVICE_UUID:
            pRec = gapServiceUUID;
            break;

        case GATT_SERVICE_UUID:
            pRec = gattServiceUUID;
            break;

        /*** GATT Declarations ***/

        case GATT_PRIMARY_SERVICE_UUID:
            pRec = primaryServiceUUID;
            break;

        case GATT_SECONDARY_SERVICE_UUID:
            pRec = secondaryServiceUUID;
            break;

        case GATT_INCLUDE_UUID:
            pRec = includeUUID;
            break;

        case GATT_CHARACTER_UUID:
            pRec = characterUUID;
            break;

        /*** GATT Descriptors ***/

        case GATT_CHAR_EXT_PROPS_UUID:
            pRec = charExtPropsUUID;
            break;

        case GATT_CHAR_USER_DESC_UUID:
            pRec = charUserDescUUID;
            break;

        case GATT_CLIENT_CHAR_CFG_UUID:
            pRec = clientCharCfgUUID;
            break;

        case GATT_SERV_CHAR_CFG_UUID:
            pRec = servCharCfgUUID;
            break;

        case GATT_CHAR_FORMAT_UUID:
            pRec = charFormatUUID;
            break;

        case GATT_CHAR_AGG_FORMAT_UUID:
            pRec = charAggFormatUUID;
            break;

        case GATT_VALID_RANGE_UUID:
            pRec = validRangeUUID;
            break;

        case GATT_EXT_REPORT_REF_UUID:
            pRec = extReportRefUUID;
            break;

        case GATT_REPORT_REF_UUID:
            pRec = reportRefUUID;
            break;

        /*** GATT Characteristics ***/

        case DEVICE_NAME_UUID:
            pRec = deviceNameUUID;
            break;

        case APPEARANCE_UUID:
            pRec = appearanceUUID;
            break;

        case RECONNECT_ADDR_UUID:
            pRec = reconnectAddrUUID;
            break;

        case PERI_PRIVACY_FLAG_UUID:
            pRec = periPrivacyFlagUUID;
            break;

        case PERI_CONN_PARAM_UUID:
            pRec = periConnParamUUID;
            break;

        case SERVICE_CHANGED_UUID:
            pRec = serviceChangedUUID;
            break;

        /*** GATT Units ***/

        default:
            break;
        }
    }
    else if ( len == ATT_UUID_SIZE )
    {
        // 128-bit UUID
    }

    return ( pRec );
}

/****************************************************************************
****************************************************************************/
