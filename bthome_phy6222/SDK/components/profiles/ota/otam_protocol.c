/*************
 otam_protocol.c
 SDK_LICENSE
***************/

#include "bcomdef.h"
#include "OSAL.h"
#include "OSAL_PwrMgr.h"
#include "OSAL_bufmgr.h"
#include "gatt.h"
#include "ll.h"
#include "ll_common.h"
#include "hci.h"
#include "gapgattserver.h"
#include "gattservapp.h"
#include "central.h"
#include "gapbondmgr.h"
#include "simpleGATTprofile_ota.h"
#include "ota_mesh_master.h"
#include "timer.h"
#include "log.h"
#include "ll_def.h"
#include "global_config.h"
#include "flash.h"
#include "rflib.h"
#include "otam_cmd.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "otam_protocol.h"
#include "ota_mesh.h"
#include "ota_app_service.h"
#include "ota_protocol.h"
#include "ota_flash.h"
#include "ota_flash_mesh.h"
//#include "otam_1clk_ota.h"
#include "crc16.h"
#include "error.h"

#define OTAF_BASE_ADDR          0x11000000
#define OTAF_END_ADDR           0x1107ffff

enum
{
    OTAM_ST_DISCONNECT = 0,
    OTAM_ST_CONNECTED,  //connected, idle
    OTAM_ST_WAIT_STARTED,
    OTAM_ST_PARAM,
    OTAM_ST_WAIT_PARTITION_INFO,
    OTAM_ST_DATA,
    OTAM_ST_COMPLETE,
    OTAM_ST_ERROR,
    OTAM_ST_CANCELING,  //wait stop
};





typedef struct
{
    ota_fw_t         fw;

    //data cache for a block transmit
    uint32_t          cache_size;
    uint32_t          cache_offset;
    uint8_t           cache_retry;
    uint8_t*          cache;
} otam_fw_t;


typedef struct
{
    uint8_t             state;
    uint8_t             run_mode;
    bool                reset_mode;
    uint16_t            mtu_a;
    uint16_t            burst_size;
    uint8_t             opcode;
    otam_fw_t           fw;
    otam_proto_meth_t   method;
} otam_proto_ctx_t;


static otam_proto_ctx_t s_otap_ctx_t;

static void print_hex (const uint8* data, uint16 len)
{
    //return;
    #if(DEBUG_INFO > 1)
    uint16 i;
    char strdata[5];

    for (i = 0; i < len - 1; i++)
    {
        //if(i %16 ==  0 && i >0)
        //  LOG("\n");
        sprintf(strdata, "%.2x", data[i]);
        AT_LOG("%s ",strdata);
    }

    sprintf(strdata, "%.2x", data[i]);
    AT_LOG("%s\n",strdata);
    #endif
}




static int otam_proto_ctx_reset(uint8_t st)
{
    otam_proto_ctx_t* pctx = &s_otap_ctx_t;
    otam_proto_meth_t* method = &(pctx->method);
    pctx->state = st;
    pctx->opcode = 0;
    memset(&(pctx->fw), 0, sizeof(otam_fw_t));
    method->clear();
    return PPlus_SUCCESS;
}

static void otam_proto_disconnect(void* param)
{
    otam_proto_ctx_reset(OTAM_ST_DISCONNECT);
}
static void otam_proto_connect(void* param)
{
    otam_proto_ctx_t* pctx = &s_otap_ctx_t;
    otam_proto_conn_param_t* pconn =  (otam_proto_conn_param_t*)param;
    pctx->state = OTAM_ST_CONNECTED;
    pctx->mtu_a = pconn->mtu -3;
    pctx->opcode = 0xff;

    if(pconn->run_mode == OTAC_RUNMODE_APP)
    {
        pctx->run_mode = OTAC_RUNMODE_APP;
        pctx->reset_mode = 0;
    }
    else if(pconn->run_mode == OTAC_RUNMODE_OTA)
    {
        pctx->run_mode = OTAC_RUNMODE_OTA;

        if(pctx->reset_mode == OTAC_RUNMODE_OTARES)
            pctx->run_mode = OTAC_RUNMODE_OTARES;

        pctx->reset_mode = 0;
    }
    else
    {
        pctx->run_mode = 0;
        pctx->reset_mode = 0;
    }
}

static int send_patition_info(void)
{
    otam_proto_ctx_t* pctx = &s_otap_ctx_t;
    otam_fw_t* pfw = &(pctx->fw);
    otam_proto_meth_t* method = &(pctx->method);
    uint8_t data[20];

    if(pctx->state != OTAM_ST_WAIT_STARTED && pctx->state != OTAM_ST_DATA)
    {
        return PPlus_ERR_INVALID_STATE;
    }

    //partition cmd: 02 ID FA FA FA FA RA RA RA RA SZ SZ SZ SZ CS CS
    //            ID: index
    //            FA: flash address
    //            RA: run address
    //            SZ: partition size
    //            CS: checksum
    uint32_t val;
    uint16_t offset = 0;
    uint32_t flash_addr = 0;
    uint32_t run_addr = pfw->fw.part[pfw->fw.part_current].run_addr;

    if(run_addr > OTAF_BASE_ADDR && run_addr < OTAF_END_ADDR )
    {
        flash_addr = run_addr;
    }
    else
    {
        //calculate store address in flash
        for(int i = 0; i<pfw->fw.part_current; i++ )
        {
            if(pfw->fw.part[i].run_addr > OTAF_BASE_ADDR && pfw->fw.part[i].run_addr < OTAF_END_ADDR)
                continue;

            val = pfw->fw.part[i].size +3;
            val = val - (val%4);
            flash_addr += val;
        }
    }

    data[offset ++] = OTA_CMD_PARTITION_INFO;
    data[offset ++] = pfw->fw.part_current;
    val = flash_addr;
    data[offset ++] = (uint8_t)(val&0xff);
    data[offset ++] = (uint8_t)((val>>8)&0xff);
    data[offset ++] = (uint8_t)((val>>16)&0xff);
    data[offset ++] = (uint8_t)((val>>24)&0xff);
    val = run_addr;
    data[offset ++] = (uint8_t)(val&0xff);
    data[offset ++] = (uint8_t)((val>>8)&0xff);
    data[offset ++] = (uint8_t)((val>>16)&0xff);
    data[offset ++] = (uint8_t)((val>>24)&0xff);
    val = pfw->fw.part[pfw->fw.part_current].size;
    data[offset ++] = (uint8_t)(val&0xff);
    data[offset ++] = (uint8_t)((val>>8)&0xff);
    data[offset ++] = (uint8_t)((val>>16)&0xff);
    data[offset ++] = (uint8_t)((val>>24)&0xff);
    val = (uint32_t)(pfw->fw.part[pfw->fw.part_current].checksum);
    data[offset ++] = (uint8_t)(val&0xff);
    data[offset ++] = (uint8_t)((val>>8)&0xff);
    pctx->state = OTAM_ST_WAIT_PARTITION_INFO;
    pfw->cache_offset = 0;
    pfw->cache_size = 0;
    pfw->cache_retry = 0;
    pfw->fw.offset = 0;
    return method->write_cmd(data, offset, 1000);
}


static int load_data_cache(void)
{
    otam_proto_ctx_t* pctx = &s_otap_ctx_t;
    otam_fw_t* pfw = &(pctx->fw);
    ota_fw_part_t* ppart = &(pfw->fw.part[pfw->fw.part_current]);
    uint16_t mtu_a = pctx->mtu_a;
    uint32_t size = mtu_a * pctx->burst_size;

    if(pfw->fw.offset + size > ppart->size)
    {
        size = ppart->size - pfw->fw.offset;
    }

    //memset(pfw->cache, 0, (ATT_MTU_SIZE-3));
    pfw->cache = (uint8_t*)(ppart->flash_addr + pfw->fw.offset+OTAFM_FW_OTA_DATA_ADDR);
    pfw->cache_size = size;
    pfw->cache_offset = 0;
    pfw->cache_retry = 0;
    pfw->fw.offset += size;
    return PPlus_SUCCESS;
}

static int send_data(void)
{
    otam_proto_ctx_t* pctx = &s_otap_ctx_t;
    otam_fw_t* pfw = &(pctx->fw);
    otam_proto_meth_t* method = &(pctx->method);

    if(!(method->write_data))
        return PPlus_ERR_NOT_REGISTED;

    if(pctx->state != OTAM_ST_DATA)
        return PPlus_ERR_INVALID_STATE;

    int ret = PPlus_SUCCESS;
    uint16_t size = 0;
    uint16_t mtu_a = pctx->mtu_a;

    while(pfw->cache_size - pfw->cache_offset)
    {
        size = mtu_a;

        if((pfw->cache_size - pfw->cache_offset) < mtu_a)
            size = pfw->cache_size - pfw->cache_offset;

        ret = method->write_data(pfw->cache + pfw->cache_offset, size);

        if(ret != PPlus_SUCCESS)
        {
            method->write_data_delay(2);
            return PPlus_SUCCESS;
        }

        pfw->cache_offset += size;
    }

    return PPlus_SUCCESS;
}

void print_version(uint8_t* vinfo)
{
}

static void handle_app_notify_event(void* param, uint8_t len)
{
    otam_proto_ctx_t* pctx = &s_otap_ctx_t;

    switch(pctx->opcode)
    {
    case OTAAPP_CMD_START_OTA:
        break;

    case OTAAPP_CMD_VER:
        print_version(param);
        break;

    default:
        break;
    }
}

static void handle_ota_notify_event(void* param, uint8_t len)
{
    otam_proto_ctx_t* pctx = &s_otap_ctx_t;
    otam_fw_t* pfw = &(pctx->fw);
    uint8_t* pnotify = (uint8_t*)param;
    otam_proto_meth_t* method = &(pctx->method);
    int retval = pnotify[0];

    if(len == 1)  //fatal error
    {
        otam_proto_ctx_reset(OTAM_ST_CONNECTED);
        return;
    }

    LOG("OTA Notif %x, %x\n",pnotify[0],pnotify[1]);
    print_hex(pnotify, len);

    switch(pnotify[1]) //response type
    {
    case OTA_RSP_START_OTA:
    {
        if(retval == PPlus_SUCCESS && pctx->state == OTAM_ST_WAIT_STARTED)
        {
            send_patition_info();
        }
        else
        {
            pctx->state = OTAM_ST_ERROR;
        }

        break;
    }

    case OTA_RSP_PARAM:
    case OTA_RSP_BLOCK_INFO:
    case OTA_RSP_BLOCK_COMPLETE:
    {
        pctx->state = OTAM_ST_ERROR;
        break;
    }

    case OTA_RSP_OTA_COMPLETE:
    {
        uint8_t data[20];

        if(method->write_cmd)
        {
            data[0] = OTA_CMD_REBOOT;
            data[1] = 1;
            pctx->state = OTAM_ST_COMPLETE;
            LOG("OTA completed!!\n");
            method->write_cmd(data, 2, 1000);
        }

        break;
    }

    case OTA_RSP_PARTITION_INFO:
    {
        pctx->state = OTAM_ST_DATA;

        if(pfw->fw.part_current)
        {
            LOG(" ");
        }

        load_data_cache();
        send_data();
        break;
    }

    case OTA_RSP_PARTITION_COMPLETE:
    {
        pfw->fw.total_offset += pfw->cache_size;
        pfw->fw.part_current ++;
        send_patition_info();
        break;
    }

    case OTA_RSP_BLOCK_BURST:
    {
        if(pctx->state != OTAM_ST_DATA)
        {
            pctx->state = OTAM_ST_ERROR;
            break;
        }

        if(retval == PPlus_SUCCESS)
        {
            load_data_cache();
            send_data();
        }
        else if(retval == PPlus_ERR_OTA_BAD_DATA)
        {
            //case block data is not completed, retry block data
            pfw->cache_retry++;
            pfw->cache_offset = 0;

            if(pfw->cache_retry > 3)
            {
                pctx->state = OTAM_ST_ERROR;
                break;
            }

            send_data();
        }
        else
        {
            pctx->state = OTAM_ST_ERROR;
        }

        break;
    }

    case OTA_RSP_REBOOT:
    {
        LOG("[OTA_RSP_REBOOT]GAPCentralRole_TerminateLink\n");
        GAPCentralRole_TerminateLink(0);
    }

    case OTA_RSP_ERASE:
    case OTA_RSP_ERROR:
    default:
    {
        pctx->state = OTAM_ST_ERROR;
        break;
    }
    }
}


void otamProtocol_event(otap_evt_t* pev)
{
    otam_proto_ctx_t* pctx = &s_otap_ctx_t;

    switch(pev->ev)
    {
    case OTAP_EVT_DISCONNECTED:
        otam_proto_disconnect(pev->data);
        break;

    case OTAP_EVT_CONNECTED:
        otam_proto_connect(pev->data);
        break;

    case OTAP_EVT_NOTIFY:
    {
        if(pctx->run_mode == OTAC_RUNMODE_APP)
            handle_app_notify_event(pev->data, pev->len);
        else
            handle_ota_notify_event(pev->data, pev->len);

        break;
    }

    case OTAP_EVT_DATA_WR_DELAY:
    {
        send_data();
        break;
    }

    case OTAP_EVT_BLE_TIMEOUT:
    default:
    {
        //pctx->state = OTAM_ST_ERROR;
        break;
    }
    }
}
#define OTAM_FW_DATA_ADDR   0x11040000
#define OTAM_FW_DATA_ADDR1  0x11060000

/*
    word      | desc:
    0         | flag: "OTAF"
    1         | partition number
    i*2 + 2   | run address
    i*2 + 3   | size
    N*2 +2    | data area

*/

int load_fw(uint8_t fw_id)
{
    otam_proto_ctx_t* pctx = &s_otap_ctx_t;
    otam_fw_t* pfw = &(pctx->fw);
    uint32_t faddr = fw_id == 0 ? OTAM_FW_DATA_ADDR : OTAM_FW_DATA_ADDR1;
    uint8_t* pdata = (uint8_t*) (faddr);
    uint32_t* pdata32 = (uint32_t*) pdata;
    uint32_t offset = 0;
    LOG("load fw %x\n", faddr);
    memset((void*)pfw, 0,sizeof(otam_fw_t));

    if(!((char)(pdata[0]) == 'O' && (char)(pdata[1]) == 'T' &&(char)(pdata[2]) == 'A'&&(char)(pdata[3]) == 'F'))
    {
        return PPlus_ERR_INVALID_DATA;
    }

    pfw->fw.part_num = (uint8_t)(pdata32[1]);
    offset = 2 * 4 + 2 * 4 * pfw->fw.part_num;
    pfw->fw.total_size = 0;

    for (uint8_t i = 0; i < pfw->fw.part_num; i++)
    {
        pfw->fw.part[i].run_addr = pdata32[i*2+2];
        pfw->fw.part[i].size = pdata32[i*2+3];
        pfw->fw.part[i].flash_addr = faddr + offset;
        pfw->fw.part[i].checksum = crc16(0, (const volatile void* )(pfw->fw.part[i].flash_addr), pfw->fw.part[i].size);
        offset += pfw->fw.part[i].size;
        pfw->fw.total_size += pfw->fw.part[i].size;
    }

    pfw->fw.total_size += 0;
    return PPlus_SUCCESS;
}

int load_res(void)
{
    return PPlus_ERR_NOT_IMPLEMENTED;
}


int otamProtocol_start_ota(ota_fw_t* pffw)
{
    otam_proto_ctx_t* pctx = &s_otap_ctx_t;
    otam_fw_t* pfw = &(pctx->fw);
    otam_proto_meth_t* method = &(pctx->method);
    uint8_t data[20];
    memcpy(&(pfw->fw), pffw, sizeof(ota_fw_t));
    pfw->fw.offset = 0;
    pfw->fw.part_current = 0;
    pfw->fw.total_offset = 0;

    if(pctx->state < OTAM_ST_CONNECTED)
        return PPlus_ERR_BLE_NOT_READY;

    if(pctx->run_mode != OTAC_RUNMODE_OTA)
    {
        return PPlus_ERR_INVALID_STATE;
    }

    if(method->write_cmd)
    {
        pctx->state = OTAM_ST_WAIT_STARTED;
        pfw->fw.part_current = 0;

        if(pctx->mtu_a == 20)
        {
            pctx->burst_size = OTA_BURST_SIZE_DEFAULT;
            data[0] = OTA_CMD_START_OTA;
            data[1] = pfw->fw.part_num;
            data[2] = 0;
        }
        else
        {
            pctx->burst_size = 0xffff;
            data[0] = OTA_CMD_START_OTA;
            data[1] = pfw->fw.part_num;
            data[2] = OTA_BURST_SIZE_HISPEED;
        }

        return method->write_cmd(data, 3, 1000);
    }

    return PPlus_ERR_NOT_REGISTED;
}


int otamProtocol_stop_ota(void)
{
    otam_proto_ctx_t* pctx = &s_otap_ctx_t;
    otam_proto_meth_t* method = &(pctx->method);
    uint8_t data[20];

    if(method->write_cmd)
    {
        data[0] = OTA_CMD_START_OTA;
        data[1] = 0xff;
        data[2] = 0;
        pctx->state = OTAM_ST_CANCELING;
        return method->write_cmd(data, 3, 1000);
    }

    return PPlus_ERR_NOT_REGISTED;
}

int otamProtocol_app_start_ota(uint8_t mode)
{
    otam_proto_ctx_t* pctx = &s_otap_ctx_t;
    otam_proto_meth_t* method = &(pctx->method);
    uint8_t data[20];

    if(pctx->run_mode != OTAC_RUNMODE_APP)
        return PPlus_ERR_INVALID_STATE;

    if(mode > OTA_MODE_RESOURCE)
        return PPlus_ERR_INVALID_PARAM;

    if(method->write_cmd)
    {
        data[0] = OTAAPP_CMD_START_OTA;
        data[1] = mode;
        data[2] = 1;
        return method->write_cmd(data, 3, 0);
    }

    return PPlus_ERR_NOT_REGISTED;
}

int otamProtocol_init(otam_proto_meth_t* method)
{
    memset(&s_otap_ctx_t, 0, sizeof(s_otap_ctx_t));
    s_otap_ctx_t.method = *method;
    return PPlus_SUCCESS;
}

