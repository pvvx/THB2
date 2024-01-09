/*************
 ota_service.h
 SDK_LICENSE
***************/

#ifndef _OTA_SERVICE_H
#define _OTA_SERVICE_H
#include "att.h"

enum
{
    OTA_EVT_CONTROL = 1,
    OTA_EVT_DATA,
    OTA_EVT_CONNECTED,
    OTA_EVT_DISCONNECTED,
    OTA_EVT_NOTIF_ENABLE,
    OTA_EVT_NOTIF_DISABLE,
};


typedef struct
{
    uint8   ev;
    uint8_t size;
    uint8_t* data;
} ota_Evt_t;

typedef void (*ota_ProfileChangeCB_t)(ota_Evt_t* pev);


bStatus_t ota_AddService( ota_ProfileChangeCB_t cb);
bStatus_t ota_Notify(attHandleValueNoti_t* pNoti);



#endif

