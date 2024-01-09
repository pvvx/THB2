/*************
 ota_flash_mesh.h
 SDK_LICENSE
***************/

#ifndef __OTA_MESH_FLASH_
#define __OTA_MESH_FLASH_

#include "ota_flash.h"



#ifndef CFG_OTA_MESH
#error "unsupported OTA_mesh config, please check micro:CFG_OTA_MESH!"
#else


#if(CFG_FLASH >= 512)
    #define OTAFM_APP_RAMRUN_ADDR    0x11017000  //108K bytes
    #define OTAFM_APP_XIP_ADDR       0x11032000  //120K bytes
    #define OTAFM_FW_OTA_ADDR        0x11050000  //256K bytes
    #define OTAFM_FW_OTA_INFO_SZ     0x1000  //size of ota info, 4K
    #define OTAFM_FW_OTA_DATA_OFFSET 0x2000
    #define OTAFM_FW_OTA_DATA_ADDR   (OTAFM_FW_OTA_ADDR + OTAFM_FW_OTA_DATA_OFFSET)

#endif

#define OTAFM_DEV_FLG_USED      0
#define OTAFM_DEV_FLG_UNINIT    0xff
#define OTAFM_DEV_FLG_INVALID   0x80
#define OTAFM_DEV_FLG_READY     0xfe
#define OTAFM_DEV_FLG_OTAING    0xfc
#define OTAFM_DEV_FLG_COMPLETED 0xf8
#define OTAFM_DEV_FLG_FAILED    0xec


typedef struct
{
    uint16_t dev_type;
    uint16_t index;
    uint8_t  dev_addr[6];
} otafmesh_dev_t;

int otafm_write_partition(uint32 addr, uint32_t* p_sect, uint32_t size);
int otafm_write_boot_sector(uint32_t* p_sect, uint32_t size, uint32_t offset);
int otafm_dev_add(otafmesh_dev_t* pdev);
int otafm_dev_pull(otafmesh_dev_t* pdev);
int otafm_dev_clear(otafmesh_dev_t* pdev);
int otafm_fw_load(ota_fw_t* pfw);
int otafm_fw_execute(void);
int otafm_format(void);
#endif //CFG_FLASH
#endif //__OTA_MESH_FLASH_

