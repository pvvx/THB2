/*************
 ota_flash_mesh.c
 SDK_LICENSE
***************/

#include "flash.h"
#include "ota_flash.h"
#include "ota_flash_mesh.h"
#include "ota_app_service.h"
#include "ota_protocol.h"
#include "string.h"
#include "error.h"
#include "crc16.h"
#include "log.h"
#include "hal_mcu.h"


typedef struct
{
    uint8_t flg;
    uint8_t rsv;
    uint16_t dev_type;
    uint8_t mac[6];
} otafm_meshinfo_t;



int otafm_write_partition(uint32 addr, uint32_t* p_sect, uint32_t size)
{
    uint32_t i;
    int ret = 0;
    uint32_t offset = OTAFM_FW_OTA_DATA_ADDR;

    if(addr % 4)
        return PPlus_ERR_DATA_ALIGN;

    size = (size + 3) & 0xfffffffc;

    for(i = 0; i < size /4; i++)
    {
        ret = flash_write_word(offset + addr + i*4, p_sect[i]);

        if(ret == PPlus_SUCCESS)
            continue;

        ret = flash_write_word(offset + addr + i*4, p_sect[i]);

        if(ret == PPlus_SUCCESS)
            continue;

        ret = flash_write_word(offset + addr + i*4, p_sect[i]);

        if(ret == PPlus_SUCCESS)
            continue;

        ret = flash_write_word(offset + addr + i*4, p_sect[i]);

        if(ret != PPlus_SUCCESS)
            return PPlus_ERR_SPI_FLASH;
    }

    return PPlus_SUCCESS;
}

int otafm_write_boot_sector(uint32_t* p_sect, uint32_t size, uint32_t offset)
{
    uint32_t i;
    int ret = 0;

    if(size % 4)
        return PPlus_ERR_DATA_ALIGN;

    for(i = 0; i < size /4; i++)
    {
        ret = flash_write_word(OTAFM_FW_OTA_ADDR + OTAFM_FW_OTA_INFO_SZ + i*4 + offset, p_sect[i]);

        if(ret == PPlus_SUCCESS)
            continue;

        ret = flash_write_word(OTAFM_FW_OTA_ADDR + OTAFM_FW_OTA_INFO_SZ + i*4 + offset, p_sect[i]);

        if(ret == PPlus_SUCCESS)
            continue;

        ret = flash_write_word(OTAFM_FW_OTA_ADDR + OTAFM_FW_OTA_INFO_SZ + i*4 + offset, p_sect[i]);

        if(ret == PPlus_SUCCESS)
            continue;

        ret = flash_write_word(OTAFM_FW_OTA_ADDR + OTAFM_FW_OTA_INFO_SZ + i*4 + offset, p_sect[i]);

        if(ret != PPlus_SUCCESS)
            return PPlus_ERR_SPI_FLASH;
    }

    return PPlus_SUCCESS;
}



int otafm_dev_add(otafmesh_dev_t* pdev)
{
    return PPlus_SUCCESS;
}



int otafm_dev_pull(otafmesh_dev_t* pdev)
{
    uint32_t i = 0;
    uint32_t faddr = OTAFM_FW_OTA_ADDR;
    uint8_t* pdata = (uint8_t*) (faddr);
    uint32_t data_u32 = NULL;
    otafm_meshinfo_t* pinfo;
    uint8_t flg = 0;

    if(!((char)(pdata[0]) == 'M' && (char)(pdata[1]) == 'O' &&(char)(pdata[2]) == 'T'&&(char)(pdata[3]) == 'A'))
    {
        return PPlus_ERR_INVALID_DATA;
    }

    pdata += 0x100;

    for(i = 0; i<240; i++)
    {
        if(pdata[0] == OTAFM_DEV_FLG_UNINIT)
            return PPlus_ERR_NOT_FOUND;

        if(pdata[0] == OTAFM_DEV_FLG_READY||
                pdata[0] == OTAFM_DEV_FLG_FAILED||
                pdata[0] == OTAFM_DEV_FLG_OTAING)
        {
            pinfo = (otafm_meshinfo_t*) pdata;
            pdev->dev_type = pinfo->dev_type;
            pdev->dev_addr[0] = pinfo->mac[5];
            pdev->dev_addr[1] = pinfo->mac[4];
            pdev->dev_addr[2] = pinfo->mac[3];
            pdev->dev_addr[3] = pinfo->mac[2];
            pdev->dev_addr[4] = pinfo->mac[1];
            pdev->dev_addr[5] = pinfo->mac[0];
            pdev->index = (uint16_t)i;
            flg = pinfo->flg & OTAFM_DEV_FLG_OTAING;
            data_u32 = *((uint32_t*)pdata);
            data_u32 &= 0xffffff00;
            data_u32 |= flg;
            flash_write_word((uint32_t)pdata, data_u32);
            return PPlus_SUCCESS;
        }

        pdata += 16;
    }

    return PPlus_ERR_NOT_FOUND;
}

int otafm_dev_clear(otafmesh_dev_t* pdev)
{
    uint32_t faddr = OTAFM_FW_OTA_ADDR;
    uint8_t* pdata = (uint8_t*) (faddr);
    otafm_meshinfo_t* pinfo;
    uint32_t data_u32;

    if(!((char)(pdata[0]) == 'M' && (char)(pdata[1]) == 'O' &&(char)(pdata[2]) == 'T'&&(char)(pdata[3]) == 'A'))
    {
        return PPlus_ERR_INVALID_DATA;
    }

    pdata += 0x100 + pdev->index * 16;
    pinfo = (otafm_meshinfo_t*)pdata;

    if(memcmp(pdev->dev_addr, pinfo->mac, 6) != 0)
    {
        return PPlus_ERR_INVALID_ADDR;
    }

    if(pinfo->flg != OTAFM_DEV_FLG_READY &&
            pinfo->flg != OTAFM_DEV_FLG_FAILED &&
            pinfo->flg != OTAFM_DEV_FLG_COMPLETED &&
            pinfo->flg != OTAFM_DEV_FLG_OTAING
      )
    {
        return PPlus_ERR_INVALID_FLAGS;
    }

    data_u32 = *(uint32_t*)pdata;
    data_u32 &= 0xffffff00;
    data_u32 |= OTAFM_DEV_FLG_USED;
    flash_write_word((uint32_t)pdata, data_u32);
    return PPlus_SUCCESS;
}


int otafm_fw_load(ota_fw_t* pfw)
{
    uint32_t faddr = OTAFM_FW_OTA_ADDR + OTAFM_FW_OTA_INFO_SZ;
    uint8_t* pdata = (uint8_t*) (faddr);
    uint32_t* pdata32 = (uint32_t*) pdata;
    LOG("load fw %x\n", faddr);
    memset((void*)pfw, 0,sizeof(ota_fw_t));

    if(!((char)(pdata[0]) == 'O' && (char)(pdata[1]) == 'T' &&(char)(pdata[2]) == 'A'&&(char)(pdata[3]) == 'F'))
    {
        return PPlus_ERR_INVALID_DATA;
    }

    pfw->part_num = (uint8_t)(pdata32[1]);
    pfw->total_size = 0;

    for (uint8_t i = 0; i < pfw->part_num; i++)
    {
        uint16_t checksum = 0;
        pfw->part[i].flash_addr = pdata32[i*4+4];
        pfw->part[i].run_addr =   pdata32[i*4+5];
        pfw->part[i].size =       pdata32[i*4+6];
        pfw->part[i].checksum =   (uint16_t)(pdata32[i*4+7]);
        checksum = crc16(0, (const volatile void* )(pfw->part[i].flash_addr + OTAFM_FW_OTA_DATA_ADDR), pfw->part[i].size);

        if(checksum != pfw->part[i].checksum)
            return PPlus_ERR_INVALID_DATA;

        pfw->total_size += pfw->part[i].size;
    }

    return PPlus_SUCCESS;
}

//load data to
int otafm_fw_execute()
{
    return PPlus_SUCCESS;
}


int otafm_format(void)
{
    uint32_t i;

    for(i = 0; i< 4; i++)
    {
        flash_block64_erase(OTAFM_FW_OTA_ADDR + i * OTAF_SECTOR_SIZE);
    }

    return PPlus_SUCCESS;
}




