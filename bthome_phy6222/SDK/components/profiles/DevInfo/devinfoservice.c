/**************************************************************************************************
    Filename:       devinfoservice.c
    Revised:        $Date $
    Revision:       $Revision $

    Description:    This file contains the Device Information service.

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
#include "gatt_profile_uuid.h"
#include "gattservapp.h"

#include "devinfoservice.h"

/*********************************************************************
    MACROS
*/
#define		SYSTEM_ID_ENABLE				1
#define		MODEL_NUMBER_STR_ENABLE		1
#define		SERIAL_NUMBER_STR_ENABLE		1
#define		FIRMWARE_REVISION_ENABLE		1
#define		HARDWARE_REVISION_ENABLE		1
#define		SOFTWARE_REVISION_ENABLE		1
#define		MANUFACTURE_NAME_STR_ENABLE	1
#define		IEEE_DATA_ENABLE				1
#define		PNP_ID_ENABLE					1

/*********************************************************************
    CONSTANTS
*/

/*********************************************************************
    TYPEDEFS
*/

/*********************************************************************
    GLOBAL VARIABLES
*/
// Device information service
CONST uint8 devInfoServUUID[ATT_BT_UUID_SIZE] =
{
    LO_UINT16(DEVINFO_SERV_UUID), HI_UINT16(DEVINFO_SERV_UUID)
};

// System ID
CONST uint8 devInfoSystemIdUUID[ATT_BT_UUID_SIZE] =
{
    LO_UINT16(SYSTEM_ID_UUID), HI_UINT16(SYSTEM_ID_UUID)
};

// Model Number String
CONST uint8 devInfoModelNumberUUID[ATT_BT_UUID_SIZE] =
{
    LO_UINT16(MODEL_NUMBER_UUID), HI_UINT16(MODEL_NUMBER_UUID)
};

// Serial Number String
CONST uint8 devInfoSerialNumberUUID[ATT_BT_UUID_SIZE] =
{
    LO_UINT16(SERIAL_NUMBER_UUID), HI_UINT16(SERIAL_NUMBER_UUID)
};

// Firmware Revision String
CONST uint8 devInfoFirmwareRevUUID[ATT_BT_UUID_SIZE] =
{
    LO_UINT16(FIRMWARE_REV_UUID), HI_UINT16(FIRMWARE_REV_UUID)
};

// Hardware Revision String
CONST uint8 devInfoHardwareRevUUID[ATT_BT_UUID_SIZE] =
{
    LO_UINT16(HARDWARE_REV_UUID), HI_UINT16(HARDWARE_REV_UUID)
};

// Software Revision String
CONST uint8 devInfoSoftwareRevUUID[ATT_BT_UUID_SIZE] =
{
    LO_UINT16(SOFTWARE_REV_UUID), HI_UINT16(SOFTWARE_REV_UUID)
};

// Manufacturer Name String
CONST uint8 devInfoMfrNameUUID[ATT_BT_UUID_SIZE] =
{
    LO_UINT16(MANUFACTURER_NAME_UUID), HI_UINT16(MANUFACTURER_NAME_UUID)
};

// IEEE 11073-20601 Regulatory Certification Data List
CONST uint8 devInfo11073CertUUID[ATT_BT_UUID_SIZE] =
{
    LO_UINT16(IEEE_11073_CERT_DATA_UUID), HI_UINT16(IEEE_11073_CERT_DATA_UUID)
};

// PnP ID
CONST uint8 devInfoPnpIdUUID[ATT_BT_UUID_SIZE] =
{
    LO_UINT16(PNP_ID_UUID), HI_UINT16(PNP_ID_UUID)
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

// Device Information Service attribute
static CONST gattAttrType_t devInfoService = { ATT_BT_UUID_SIZE, devInfoServUUID };

#if SYSTEM_ID_ENABLE
// System ID characteristic
static uint8 devInfoSystemIdProps 			= 	GATT_PROP_READ;
static uint8 devInfoSystemId[DEVINFO_SYSTEM_ID_LEN] 	= 	{0, 0, 0, 0, 0, 0, 0, 0};
#endif

#if	MODEL_NUMBER_STR_ENABLE
// Model Number String characteristic
static uint8 devInfoModelNumberProps 		= 	GATT_PROP_READ;
static const uint8 devInfoModelNumber[] 	= 	"Model Number";
#endif

#if	SERIAL_NUMBER_STR_ENABLE
// Serial Number String characteristic
static uint8 devInfoSerialNumberProps 	= 	GATT_PROP_READ;
static const uint8 devInfoSerialNumber[] 	= 	"Serial Number";
#endif

#if FIRMWARE_REVISION_ENABLE
// Firmware Revision String characteristic
static uint8 devInfoFirmwareRevProps 		= 	GATT_PROP_READ;
static const uint8 devInfoFirmwareRev[] 	= 	"Firmware Revision";
#endif

#if	HARDWARE_REVISION_ENABLE
// Hardware Revision String characteristic
static uint8 devInfoHardwareRevProps 		= 	GATT_PROP_READ;
static const uint8 devInfoHardwareRev[] 	= 	"Hardware Revision";
#endif

#if	SOFTWARE_REVISION_ENABLE
// Software Revision String characteristic
static uint8 devInfoSoftwareRevProps 		= 	GATT_PROP_READ;
static const uint8 devInfoSoftwareRev[] 	= 	"Software Revision";
#endif

#if	MANUFACTURE_NAME_STR_ENABLE
// Manufacturer Name String characteristic
static uint8 devInfoMfrNameProps 			= 	GATT_PROP_READ;
static const uint8 devInfoMfrName[] 		= 	"Manufacturer Name";
#endif

#if	IEEE_DATA_ENABLE
// IEEE 11073-20601 Regulatory Certification Data List characteristic
static uint8 devInfo11073CertProps 		= 	GATT_PROP_READ;
static const uint8 devInfo11073Cert[] =
{
	DEVINFO_11073_BODY_EXP,      // authoritative body type
	0x00,                       // authoritative body structure type
	                          // authoritative body data follows below:
	'e', 'x', 'p', 'e', 'r', 'i', 'm', 'e', 'n', 't', 'a', 'l'
};
#endif

#if	PNP_ID_ENABLE
// System ID characteristic
static uint8 devInfoPnpIdProps 			= 	GATT_PROP_READ;
static uint8 devInfoPnpId[DEVINFO_PNP_ID_LEN] =
{
	1,                                      // Vendor ID source (1=Bluetooth SIG)
	LO_UINT16(0x0f0f), HI_UINT16(0x0f0f),   // Vendor ID  
	LO_UINT16(0x0000), HI_UINT16(0x0000),   // Product ID (vendor-specific)
	LO_UINT16(0x0110), HI_UINT16(0x0110)    // Product version (JJ.M.N)
};
#endif

/*********************************************************************
    Profile Attributes - Table
*/

static gattAttribute_t devInfoAttrTbl[] =
{
	/* type */								/* permissions */			/* handle */	/* pValue */
	// Device Information Service
	{{ ATT_BT_UUID_SIZE, primaryServiceUUID },			GATT_PERMIT_READ,	0,	(uint8 *)&devInfoService		},

#if SYSTEM_ID_ENABLE
	// System ID Declaration
	{{ ATT_BT_UUID_SIZE, characterUUID },				GATT_PERMIT_READ,	0,	&devInfoSystemIdProps			},
	// System ID Value
	{{ ATT_BT_UUID_SIZE, devInfoSystemIdUUID },		GATT_PERMIT_READ,	0,	(uint8 *) devInfoSystemId		},
#endif

#if	MODEL_NUMBER_STR_ENABLE
	// Model Number String Declaration
	{{ ATT_BT_UUID_SIZE, characterUUID },				GATT_PERMIT_READ,	0,	&devInfoModelNumberProps		},
	// Model Number Value
	{{ ATT_BT_UUID_SIZE, devInfoModelNumberUUID },	GATT_PERMIT_READ,	0,	(uint8 *) devInfoModelNumber	},
#endif

#if	SERIAL_NUMBER_STR_ENABLE
	// Serial Number String Declaration
	{{ ATT_BT_UUID_SIZE, characterUUID },				GATT_PERMIT_READ,	0,	&devInfoSerialNumberProps		},
	// Serial Number Value
	{{ ATT_BT_UUID_SIZE, devInfoSerialNumberUUID },	GATT_PERMIT_READ,	0,	(uint8 *) devInfoSerialNumber	},
#endif

#if	FIRMWARE_REVISION_ENABLE
	// Firmware Revision String Declaration
	{{ ATT_BT_UUID_SIZE, characterUUID },				GATT_PERMIT_READ,	0,	&devInfoFirmwareRevProps		},
	// Firmware Revision Value
	{{ ATT_BT_UUID_SIZE, devInfoFirmwareRevUUID },	GATT_PERMIT_READ,	0,	(uint8 *) devInfoFirmwareRev	},
#endif

#if	HARDWARE_REVISION_ENABLE
	// Hardware Revision String Declaration
	{{ ATT_BT_UUID_SIZE, characterUUID },				GATT_PERMIT_READ,	0,	&devInfoHardwareRevProps		},
	// Hardware Revision Value
	{{ ATT_BT_UUID_SIZE, devInfoHardwareRevUUID },	GATT_PERMIT_READ,	0,	(uint8 *) devInfoHardwareRev	},
#endif

#if	SOFTWARE_REVISION_ENABLE
	// Software Revision String Declaration
	{{ ATT_BT_UUID_SIZE, characterUUID },				GATT_PERMIT_READ,	0,	&devInfoSoftwareRevProps		},
	// Software Revision Value
	{{ ATT_BT_UUID_SIZE, devInfoSoftwareRevUUID },	GATT_PERMIT_READ,	0,	(uint8 *) devInfoSoftwareRev	},
#endif

#if	MANUFACTURE_NAME_STR_ENABLE
	// Manufacturer Name String Declaration
	{{ ATT_BT_UUID_SIZE, characterUUID },				GATT_PERMIT_READ,	0,	&devInfoMfrNameProps			},
	// Manufacturer Name Value
	{{ ATT_BT_UUID_SIZE, devInfoMfrNameUUID },			GATT_PERMIT_READ,	0,	(uint8 *) devInfoMfrName		},
#endif

#if	IEEE_DATA_ENABLE
	// IEEE 11073-20601 Regulatory Certification Data List Declaration
	{{ ATT_BT_UUID_SIZE, characterUUID },				GATT_PERMIT_READ,	0,	&devInfo11073CertProps		},
	// IEEE 11073-20601 Regulatory Certification Data List Value
	{{ ATT_BT_UUID_SIZE, devInfo11073CertUUID },		GATT_PERMIT_READ,	0,	(uint8 *) devInfo11073Cert		},
#endif

#if	PNP_ID_ENABLE
	// PnP ID Declaration
	{{ ATT_BT_UUID_SIZE, characterUUID },				GATT_PERMIT_READ,	0,	&devInfoPnpIdProps				},
	// PnP ID Value
	{{ ATT_BT_UUID_SIZE, devInfoPnpIdUUID },			GATT_PERMIT_READ,	0,	(uint8 *) devInfoPnpId			},
#endif
};


/*********************************************************************
    LOCAL FUNCTIONS
*/
static uint8 devInfo_ReadAttrCB( uint16 connHandle, gattAttribute_t* pAttr,
                                 uint8* pValue, uint16* pLen, uint16 offset, uint8 maxLen );

/*********************************************************************
    PROFILE CALLBACKS
*/
// Device Info Service Callbacks
CONST gattServiceCBs_t devInfoCBs =
{
    devInfo_ReadAttrCB, // Read callback function pointer
    NULL,               // Write callback function pointer
    NULL                // Authorization callback function pointer
};

/*********************************************************************
    NETWORK LAYER CALLBACKS
*/

/*********************************************************************
    PUBLIC FUNCTIONS
*/

/*********************************************************************
    @fn      DevInfo_AddService

    @brief   Initializes the Device Information service by registering
            GATT attributes with the GATT server.

    @return  Success or Failure
*/
bStatus_t DevInfo_AddService( void )
{
  // Register GATT attribute list and CBs with GATT Server App
  return GATTServApp_RegisterService( devInfoAttrTbl,
                                      GATT_NUM_ATTRS( devInfoAttrTbl ),
                                      &devInfoCBs );
}

/*********************************************************************
    @fn      DevInfo_SetParameter

    @brief   Set a Device Information parameter.

    @param   param - Profile parameter ID
    @param   len - length of data to write
    @param   value - pointer to data to write.  This is dependent on
            the parameter ID and WILL be cast to the appropriate
            data type (example: data type of uint16 will be cast to
            uint16 pointer).

    @return  bStatus_t
*/
bStatus_t DevInfo_SetParameter( uint8 param, uint8 len, void* value )
{
	bStatus_t ret = SUCCESS;

	switch ( param )
	{
	#if	SYSTEM_ID_ENABLE
		case DEVINFO_SYSTEM_ID:
		osal_memcpy(devInfoSystemId, value, len);
		break;
	#endif

		default:
		ret = INVALIDPARAMETER;
		break;
	}

	return ( ret );
}

/*********************************************************************
    @fn      DevInfo_GetParameter

    @brief   Get a Device Information parameter.

    @param   param - Profile parameter ID
    @param   value - pointer to data to get.  This is dependent on
            the parameter ID and WILL be cast to the appropriate
            data type (example: data type of uint16 will be cast to
            uint16 pointer).

    @return  bStatus_t
*/
bStatus_t DevInfo_GetParameter( uint8 param, void* value )
{
	bStatus_t ret = SUCCESS;

	switch ( param )
	{
	#if SYSTEM_ID_ENABLE
		case DEVINFO_SYSTEM_ID:
			osal_memcpy(value, devInfoSystemId, sizeof(devInfoSystemId));
		break;
	#endif
	
	#if	MODEL_NUMBER_STR_ENABLE
		case DEVINFO_MODEL_NUMBER:
			osal_memcpy(value, devInfoModelNumber, sizeof(devInfoModelNumber));
		break;
	#endif
	
	#if	SERIAL_NUMBER_STR_ENABLE
		case DEVINFO_SERIAL_NUMBER:
			osal_memcpy(value, devInfoSerialNumber, sizeof(devInfoSerialNumber));
		break;
	#endif
	
	#if	FIRMWARE_REVISION_ENABLE
		case DEVINFO_FIRMWARE_REV:
			osal_memcpy(value, devInfoFirmwareRev, sizeof(devInfoFirmwareRev));
		break;
	#endif
	
	#if	HARDWARE_REVISION_ENABLE
		case DEVINFO_HARDWARE_REV:
			osal_memcpy(value, devInfoHardwareRev, sizeof(devInfoHardwareRev));
		break;
	#endif
	
	#if	SOFTWARE_REVISION_ENABLE
		case DEVINFO_SOFTWARE_REV:
			osal_memcpy(value, devInfoSoftwareRev, sizeof(devInfoSoftwareRev));
		break;
	#endif
	
	#if	MANUFACTURE_NAME_STR_ENABLE
		case DEVINFO_MANUFACTURER_NAME:
			osal_memcpy(value, devInfoMfrName, sizeof(devInfoMfrName));
		break;
	#endif
	
	#if	IEEE_DATA_ENABLE
		case DEVINFO_11073_CERT_DATA:
			osal_memcpy(value, devInfo11073Cert, sizeof(devInfo11073Cert));
		break;
	#endif
	
	#if	PNP_ID_ENABLE
		case DEVINFO_PNP_ID:
			osal_memcpy(value, devInfoPnpId, sizeof(devInfoPnpId));
		break;
	#endif
	
		default:
			ret = INVALIDPARAMETER;
		break;
	}

	return ( ret );
}

/*********************************************************************
    @fn          devInfo_ReadAttrCB

    @brief       Read an attribute.

    @param       connHandle - connection message was received on
    @param       pAttr - pointer to attribute
    @param       pValue - pointer to data to be read
    @param       pLen - length of data to be read
    @param       offset - offset of the first octet to be read
    @param       maxLen - maximum length of data to be read

    @return      Success or Failure
*/
static uint8 devInfo_ReadAttrCB( uint16 connHandle, gattAttribute_t* pAttr,
                                 uint8* pValue, uint16* pLen, uint16 offset, uint8 maxLen )
{
	bStatus_t status = SUCCESS;
	uint16 uuid = BUILD_UINT16( pAttr->type.uuid[0], pAttr->type.uuid[1]);

	switch (uuid)
	{
	#if	SYSTEM_ID_ENABLE
		case SYSTEM_ID_UUID:
		// verify offset
		if (offset >= sizeof(devInfoSystemId)){
			status = ATT_ERR_INVALID_OFFSET;
		}else{
			// determine read length
			*pLen = MIN(maxLen, (sizeof(devInfoSystemId) - offset));
			// copy data
			osal_memcpy(pValue, &devInfoSystemId[offset], *pLen);
		}
		break;
	#endif

	#if	MODEL_NUMBER_STR_ENABLE
		case MODEL_NUMBER_UUID:
		// verify offset
		if (offset >= (sizeof(devInfoModelNumber) - 1)){
			status = ATT_ERR_INVALID_OFFSET;
		}else{
			// determine read length (exclude null terminating character)
			*pLen = MIN(maxLen, ((sizeof(devInfoModelNumber) - 1) - offset));
			// copy data
			osal_memcpy(pValue, &devInfoModelNumber[offset], *pLen);
		}
		break;
	#endif

	#if	SERIAL_NUMBER_STR_ENABLE
		case SERIAL_NUMBER_UUID:
		// verify offset
		if (offset >= (sizeof(devInfoSerialNumber) - 1)){
			status = ATT_ERR_INVALID_OFFSET;
		}else{
			// determine read length (exclude null terminating character)
			*pLen = MIN(maxLen, ((sizeof(devInfoSerialNumber) - 1) - offset));
			// copy data
			osal_memcpy(pValue, &devInfoSerialNumber[offset], *pLen);
		}
		break;
	#endif

	#if	FIRMWARE_REVISION_ENABLE
		case FIRMWARE_REV_UUID:
		// verify offset
		if (offset >= (sizeof(devInfoFirmwareRev) - 1)){
			status = ATT_ERR_INVALID_OFFSET;
		}else{
			// determine read length (exclude null terminating character)
			*pLen = MIN(maxLen, ((sizeof(devInfoFirmwareRev) - 1) - offset));
			// copy data
			osal_memcpy(pValue, &devInfoFirmwareRev[offset], *pLen);
		}
		break;
	#endif

	#if	HARDWARE_REVISION_ENABLE
		case HARDWARE_REV_UUID:
		// verify offset
		if (offset >= (sizeof(devInfoHardwareRev) - 1)){
			status = ATT_ERR_INVALID_OFFSET;
		}else{
			// determine read length (exclude null terminating character)
			*pLen = MIN(maxLen, ((sizeof(devInfoHardwareRev) - 1) - offset));
			// copy data
			osal_memcpy(pValue, &devInfoHardwareRev[offset], *pLen);
		}
		break;
	#endif

	#if	SOFTWARE_REVISION_ENABLE
		case SOFTWARE_REV_UUID:
		// verify offset
		if (offset >= (sizeof(devInfoSoftwareRev) - 1)){
			status = ATT_ERR_INVALID_OFFSET;
		}else{
			// determine read length (exclude null terminating character)
			*pLen = MIN(maxLen, ((sizeof(devInfoSoftwareRev) - 1) - offset));
			// copy data
			osal_memcpy(pValue, &devInfoSoftwareRev[offset], *pLen);
		}
		break;
	#endif

	#if	MANUFACTURE_NAME_STR_ENABLE
		case MANUFACTURER_NAME_UUID:
		// verify offset
		if (offset >= (sizeof(devInfoMfrName) - 1)){
			status = ATT_ERR_INVALID_OFFSET;
		}else{
			// determine read length (exclude null terminating character)
			*pLen = MIN(maxLen, ((sizeof(devInfoMfrName) - 1) - offset));
			// copy data
			osal_memcpy(pValue, &devInfoMfrName[offset], *pLen);
		}
		break;
	#endif

	#if	IEEE_DATA_ENABLE
		case IEEE_11073_CERT_DATA_UUID:
		// verify offset
		if (offset >= sizeof(devInfo11073Cert)){
			status = ATT_ERR_INVALID_OFFSET;
		}else{
			// determine read length
			*pLen = MIN(maxLen, (sizeof(devInfo11073Cert) - offset));
			// copy data
			osal_memcpy(pValue, &devInfo11073Cert[offset], *pLen);
		}
		break;
	#endif

	#if	PNP_ID_ENABLE
		case PNP_ID_UUID:
		// verify offset
		if (offset >= sizeof(devInfoPnpId)){
			status = ATT_ERR_INVALID_OFFSET;
		}else{
			// determine read length
			*pLen = MIN(maxLen, (sizeof(devInfoPnpId) - offset));
			// copy data
			osal_memcpy(pValue, &devInfoPnpId[offset], *pLen);
		}
		break;
	#endif
	
		default:
			*pLen = 0;
			status = ATT_ERR_ATTR_NOT_FOUND;
		break;
	}

	return ( status );
}


/*********************************************************************
*********************************************************************/
