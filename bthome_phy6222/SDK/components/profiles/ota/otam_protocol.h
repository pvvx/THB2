/*************
 otam_protocol.h
 SDK_LICENSE
***************/

#ifndef __OTA_MAST_PROTO_
#define __OTA_MAST_PROTO_

#include "ota_flash.h"


#define OTA_BURST_SIZE_DEFAULT    16
#define OTA_BURST_SIZE_HISPEED    0xff


enum
{
    OTAP_EVT_DISCONNECTED =1,
    OTAP_EVT_CONNECTED,
    OTAP_EVT_NOTIFY,
    OTAP_EVT_DATA_WR_DELAY,
    OTAP_EVT_BLE_TIMEOUT,
};


typedef struct
{
    uint8_t   run_mode;
    uint16_t  mtu;
} otam_proto_conn_param_t;

typedef struct
{
    uint8_t   ev;
    uint16_t  len;
    void*     data;
} otap_evt_t;

typedef int (*otam_clear_t)(void);
typedef int (*otam_wcmd_op_t)(uint8_t* data, uint16_t len, uint32_t timeout);
typedef int (*otam_wdata_op_t)(uint8_t* data, uint16_t len);
typedef int (*otam_wdata_delay_t)(uint32_t msec_delay);

typedef struct
{
    otam_clear_t        clear;
    otam_wcmd_op_t      write_cmd;
    otam_wdata_op_t     write_data;
    otam_wdata_delay_t   write_data_delay;
} otam_proto_meth_t;



void otamProtocol_event(otap_evt_t* pev);
int otamProtocol_start_ota(ota_fw_t* pffw);
int otamProtocol_stop_ota(void);
int otamProtocol_app_start_ota(uint8_t mode);
int otamProtocol_init(otam_proto_meth_t* method);

#endif

