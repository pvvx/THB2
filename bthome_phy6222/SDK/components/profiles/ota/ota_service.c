/*************
 ota_service.c
 SDK_LICENSE
***************/

#include "bcomdef.h"
#include "OSAL.h"
#include "linkdb.h"
#include "att.h"
#include "gatt.h"
#include "gatt_uuid.h"
#include "gatt_profile_uuid.h"
#include "peripheral.h"
#include "gattservapp.h"

#include "ota_service.h"
#include "log.h"

#ifndef CFG_OTA_MESH
static const uint8 ota_ServiceUUID[ATT_UUID_SIZE] =
{0x23, 0xf1, 0x6e, 0x53, 0xa4, 0x22, 0x42, 0x61, 0x91, 0x51, 0x8b, 0x9b, 0x01, 0xff, 0x33, 0x58};

//command characteristic
static const uint8 ota_CommandUUID[ATT_UUID_SIZE] =
{0x23, 0xf1, 0x6e, 0x53, 0xa4, 0x22, 0x42, 0x61, 0x91, 0x51, 0x8b, 0x9b, 0x02, 0xff, 0x33, 0x58};

// Sensor location characteristic
static const uint8 ota_ResponseUUID[ATT_UUID_SIZE] =
{0x23, 0xf1, 0x6e, 0x53, 0xa4, 0x22, 0x42, 0x61, 0x91, 0x51, 0x8b, 0x9b, 0x03, 0xff, 0x33, 0x58};

// Command characteristic
static const uint8 ota_DataUUID[ATT_UUID_SIZE] =
{0x23, 0xf1, 0x6e, 0x53, 0xa4, 0x22, 0x42, 0x61, 0x91, 0x51, 0x8b, 0x9b, 0x04, 0xff, 0x33, 0x58};

#else
static const uint8 ota_ServiceUUID[ATT_UUID_SIZE] =
{0xde, 0x61, 0x49, 0x67, 0xea, 0x50, 0x57, 0x2c, 0xbb, 0xba, 0x52, 0x9b, 0x01, 0xff, 0x33, 0x58};

//command characteristic
static const uint8 ota_CommandUUID[ATT_UUID_SIZE] =
{0xde, 0x61, 0x49, 0x67, 0xea, 0x50, 0x57, 0x2c, 0xbb, 0xba, 0x52, 0x9b, 0x02, 0xff, 0x33, 0x58};

// Sensor location characteristic
static const uint8 ota_ResponseUUID[ATT_UUID_SIZE] =
{0xde, 0x61, 0x49, 0x67, 0xea, 0x50, 0x57, 0x2c, 0xbb, 0xba, 0x52, 0x9b, 0x03, 0xff, 0x33, 0x58};

// Command characteristic
static const uint8 ota_DataUUID[ATT_UUID_SIZE] =
{0xde, 0x61, 0x49, 0x67, 0xea, 0x50, 0x57, 0x2c, 0xbb, 0xba, 0x52, 0x9b, 0x04, 0xff, 0x33, 0x58};
#endif

static const gattAttrType_t ota_Service = {ATT_UUID_SIZE, ota_ServiceUUID};

static uint8 ota_CommandProps = GATT_PROP_WRITE;
static uint8 ota_CommandValue = 0;

// OTA response Characteristic
static uint8 ota_ResponseProps = GATT_PROP_NOTIFY;
static uint8 ota_ResponseValue = 0;
static gattCharCfg_t ota_ResponseCCCD[GATT_MAX_NUM_CONN];

// OTA Data Characteristic
static uint8 ota_DataProps = GATT_PROP_WRITE_NO_RSP;
static uint8 ota_DataValue = 0;

#define OTA_COMMAND_HANDLE 2
#define OTA_RSP_HANDLE 4
#define OTA_DATA_HANDLE 7
static gattAttribute_t ota_AttrTbl[] =
{
    //OTA Service
    {
        {ATT_BT_UUID_SIZE, primaryServiceUUID}, /* type */
        GATT_PERMIT_READ,                       /* permissions */
        0,                                      /* handle */
        (uint8*)& ota_Service                   /* pValue */
    },

    //OTA Command Declaration
    {
        {ATT_BT_UUID_SIZE, characterUUID},
        GATT_PERMIT_READ,
        0,
        &ota_CommandProps
    },

    //OTA Command Value
    {
        {ATT_UUID_SIZE, ota_CommandUUID},
        GATT_PERMIT_WRITE,
        0,
        &ota_CommandValue
    },

    // OTA response Declaration
    {
        {ATT_BT_UUID_SIZE, characterUUID},
        GATT_PERMIT_READ,
        0,
        &ota_ResponseProps
    },

    //response Value
    {
        {ATT_UUID_SIZE, ota_ResponseUUID},
        GATT_PERMIT_READ,
        0,
        &ota_ResponseValue
    },

    // OTA response Client Characteristic Configuration
    {
        {ATT_BT_UUID_SIZE, clientCharCfgUUID},
        GATT_PERMIT_READ | GATT_PERMIT_WRITE,
        0,
        (uint8*)ota_ResponseCCCD
    },

    //Data Declaration
    {
        {ATT_BT_UUID_SIZE, characterUUID},
        GATT_PERMIT_READ,
        0,
        &ota_DataProps
    },

    // Command Value
    {
        {ATT_UUID_SIZE, ota_DataUUID},
        GATT_PERMIT_WRITE,
        0,
        &ota_DataValue
    }
};

static ota_ProfileChangeCB_t ota_AppCBs = NULL;

static void handleConnStatusCB(uint16 connHandle, uint8 changeType);

static uint8 ota_ReadAttrCB(uint16 connHandle, gattAttribute_t* pAttr,
                            uint8* pValue, uint16* pLen, uint16 offset, uint8 maxLen);
static bStatus_t ota_WriteAttrCB(uint16 connHandle, gattAttribute_t* pAttr,
                                 uint8* pValue, uint16 len, uint16 offset);

CONST gattServiceCBs_t ota_ProfileCBs =
{
    ota_ReadAttrCB,  // Read callback function pointer
    ota_WriteAttrCB, // Write callback function pointer
    NULL             // Authorization callback function pointer
};

static uint8 ota_ReadAttrCB(uint16 connHandle, gattAttribute_t* pAttr,
                            uint8* pValue, uint16* pLen, uint16 offset, uint8 maxLen)
{
    bStatus_t status = ATT_ERR_READ_NOT_PERMITTED;
    LOG("ReadAttrCB\n");

    // If attribute permissions require authorization to read, return error
    if (gattPermitAuthorRead(pAttr->permissions))
    {
        // Insufficient authorization
        return (ATT_ERR_INSUFFICIENT_AUTHOR);
    }

    return (status);
}

static bStatus_t ota_WriteAttrCB(uint16 connHandle, gattAttribute_t* pAttr,
                                 uint8* pValue, uint16 len, uint16 offset)
{
    bStatus_t status = SUCCESS;

    //uint8 notifyApp = 0xFF;
    // If attribute permissions require authorization to write, return error
    if (gattPermitAuthorWrite(pAttr->permissions))
    {
        // Insufficient authorization
        return (ATT_ERR_INSUFFICIENT_AUTHOR);
    }

    if (pAttr->type.len == ATT_BT_UUID_SIZE)
    {
        // 16-bit UUID
        uint16 uuid = BUILD_UINT16(pAttr->type.uuid[0], pAttr->type.uuid[1]);

        if (uuid == GATT_CLIENT_CHAR_CFG_UUID)
        {
            status = GATTServApp_ProcessCCCWriteReq(connHandle, pAttr, pValue, len,
                                                    offset, GATT_CLIENT_CFG_NOTIFY);

            if (status == SUCCESS && ota_AppCBs)
            {
                uint16 charCfg = BUILD_UINT16(pValue[0], pValue[1]);
                LOG("CCCD set: [%d]\n", charCfg);

                if (ota_AppCBs)
                {
                    ota_Evt_t evt;
                    evt.ev = charCfg == 1 ? OTA_EVT_NOTIF_ENABLE : OTA_EVT_NOTIF_DISABLE;
                    evt.size = 0;
                    evt.data = NULL;
                    ota_AppCBs(&evt);
                }
            }
        }
    }
    else
    {
        //LOG("WR:%d\n", pAttr->handle);
        // 128-bit UUID Command
        if (pAttr->handle == ota_AttrTbl[OTA_COMMAND_HANDLE].handle)
        {
            if (ota_AppCBs)
            {
                ota_Evt_t evt;
                evt.ev = OTA_EVT_CONTROL;
                evt.size = len;
                evt.data = pValue;
                ota_AppCBs(&evt);
            }
        }

        // 128-bit UUID Data
        if (pAttr->handle == ota_AttrTbl[OTA_DATA_HANDLE].handle)
        {
            if (ota_AppCBs)
            {
                ota_Evt_t evt;
                evt.ev = OTA_EVT_DATA;
                evt.size = len;
                evt.data = pValue;
                ota_AppCBs(&evt);
            }
        }
    }

    return (status);
}

bStatus_t ota_AddService(ota_ProfileChangeCB_t cb)
{
    uint8 status = SUCCESS;
    // Register with Link DB to receive link status change callback
    VOID linkDB_Register(handleConnStatusCB);
    GATTServApp_InitCharCfg(INVALID_CONNHANDLE, ota_ResponseCCCD);
    // Register GATT attribute list and CBs with GATT Server App
    status = GATTServApp_RegisterService(ota_AttrTbl,
                                         GATT_NUM_ATTRS(ota_AttrTbl),
                                         &ota_ProfileCBs);

    if (status != SUCCESS)
        LOG("Add OTA service failed!\n");

    ota_AppCBs = cb;
    return (status);
}

static void handleConnStatusCB(uint16 connHandle, uint8 changeType)
{
    // Make sure this is not loopback connection
    if (connHandle != LOOPBACK_CONNHANDLE)
    {
        // Reset Client Char Config if connection has dropped
        if ((changeType == LINKDB_STATUS_UPDATE_REMOVED) ||
                ((changeType == LINKDB_STATUS_UPDATE_STATEFLAGS) &&
                 (!linkDB_Up(connHandle))))
        {
            GATTServApp_InitCharCfg(connHandle, ota_ResponseCCCD);

            if (ota_AppCBs)
            {
                ota_Evt_t evt;
                evt.ev = OTA_EVT_DISCONNECTED;
                evt.size = 0;
                evt.data = NULL;
                ota_AppCBs(&evt);
            }
        }
    }
}

bStatus_t ota_Notify(attHandleValueNoti_t* pNoti)
{
    uint16 connHandle;
    uint16 value;
    GAPRole_GetParameter(GAPROLE_CONNHANDLE, &connHandle);
    value = GATTServApp_ReadCharCfg(connHandle, ota_ResponseCCCD);

    if (connHandle == INVALID_CONNHANDLE)
        return bleIncorrectMode;

    // If notifications enabled
    if (value & GATT_CLIENT_CFG_NOTIFY)
    {
        AT_LOG("Notif %x,%x\n", pNoti->value[0], pNoti->value[1]);
        // Set the handle
        pNoti->handle = ota_AttrTbl[OTA_RSP_HANDLE].handle;
        // Send the Indication
        return GATT_Notification(connHandle, pNoti, FALSE);
    }

    return bleIncorrectMode;
}
