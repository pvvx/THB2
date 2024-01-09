/*************
 slb.c
 SDK_LICENSE
***************/

#include <stdlib.h>
#include <string.h>
#include "bus_dev.h"
//#include "ap_cp.h"
//#include "hal_mcu.h"
#include "flash.h"
//#include "common.h"
#include "crc16.h"
#include "slb.h"
#include "log.h"

#ifdef CFG_FLASH
    #undef CFG_FLASH
#endif
#ifdef USE_FCT
    #undef USE_FCT
#endif
#ifdef CFG_OTA_BANK_MODE
    #undef CFG_OTA_BANK_MODE
#endif
#define CFG_FLASH 512
#define USE_FCT 0
#define CFG_OTA_BANK_MODE 1

#include "ota_flash.h"
#include "error.h"

#ifdef ON_SLB_BOOTLOADER
//boot loader mode
#define SLB_ASSERT(r) {if(r) return r;}
uint8_t s_partition_buf[16*1024+16] __attribute__((section("ota_partition_buffer_area")));
#else
//application mode

#define SLB_ASSERT(r) {if(r){m_slb_ctx.state |= 0x80000000; return r;}}
enum
{
    SLB_PROG_ST_UNINIT = 0,
    SLB_PROG_ST_IDLE = 1,
    SLB_PROG_ST_PART_INFO,
    SLB_PROG_ST_PART_DATA,
    SLB_PROG_ST_COMPLETED,
    SLB_PROG_ST_OOO_PIECE   //for out of order pieces
};
typedef struct
{
    uint32_t        state;
    uint8_t         part_num;
    ota_fw_part_t   part;
    uint32_t        offset;
    uint32_t        flash_offset;
    uint32_t        ooo_total_numbers;
    uint32_t        ooo_recv_numbers;
    uint32_t        ooo_piece_len;
    slb_chksum_cb_t ccb;
} slb_ctx_t;

slb_ctx_t    m_slb_ctx =
{
    .state = 0,
    .part_num = 0,
    .offset = 0,
};
#endif
const char*  tag_ota_fw= "OTAF";
const char*  tag_ota_fw_used= "@TAF";
const char*  tag_selfload_ota_info= "SOIF";

int slb_spif_write(uint32_t faddr, uint8_t* pdata, uint32_t size)
{
    if(faddr & 3)
    {
        return PPlus_ERR_DATA_ALIGN;
    }

    //#if(SLB_FLASH == SLB_FLASH_INTERNAL_512)
    #if 0
    uint32_t data,datatmp,tmp,i,j;

    for(i = 0; i<(size+3)/4; i++)
    {
        data = 0;
        tmp = 4;

        if((i+1)*4 > size)
            tmp = size & 3;

        for(j = 0; j < tmp; j++)
        {
            datatmp = (*pdata)&0xff;
            datatmp = datatmp<<(j*8);
            data |= datatmp;
            pdata ++;
        }

        if(!flash_write_word(faddr, data))
            return PPlus_ERR_SPI_FLASH;

        faddr += 4;
    }

    return PPlus_SUCCESS;
    #endif
    return(hal_flash_write_by_dma(faddr, (uint8_t*) pdata, size));
}

int slb_spif_read(uint32_t faddr, uint8_t* pdata, uint32_t size)
{
    #if(SLB_FLASH == SLB_FLASH_INTERNAL_512)
    SLB_FLASH_CACHE_ENTER_BYPASS_SECTION();
    memcpy(pdata, (uint8_t*)faddr, size);
    SLB_FLASH_CACHE_EXIT_BYPASS_SECTION();
    return PPlus_SUCCESS;
    #endif
}
int slb_spif_erase(uint32_t faddr, uint32_t size)
{
    #if(SLB_FLASH == SLB_FLASH_INTERNAL_512)
    uint32_t endaddr = faddr + size;
    faddr = faddr & 0xfffff000;

    while(faddr < endaddr)
    {
        hal_flash_erase_sector(faddr);
        faddr += 0x1000;//unit is 4K
    }

    return PPlus_SUCCESS;
    #endif
}


int slb_check_currfw_exchfw(void)
{
    return PPlus_SUCCESS;
}

uint16_t slb_flash_calc_checksum(uint32_t addr, uint32_t size)
{
    return crc16(0, (const volatile void*) addr, size);
}

//mark the exchange zone as garbage data
#define SLB_DELETE_EXCHANGE_ZONE()    {uint32_t tmpdata = 0;slb_spif_write(SLB_FLASH_FW_BASE, (uint8_t*)(&tmpdata), 4);}

#ifdef ON_SLB_BOOTLOADER

static int slb_erase_fw(void)
{
    return ota_flash_erase(OTAF_APP_BANK_0_ADDR);
}
static int slb_apply_exch_zone_to_fw(uint32_t part_num)
{
    int i;
    uint32_t part_info[4];
    uint32_t flash_offset = 0;
    uint32_t ram_part_idx = 0;

    //validate partition
    for(i = 1; i< part_num; i++)
    {
        uint32_t crc = 0;
        slb_spif_read(SLB_FLASH_FW_PART_FADDR(i), (uint8_t*) part_info, 0x10);
        slb_spif_read(SLB_FLASH_PART_DATA_BASE + part_info[0], (uint8_t*)s_partition_buf, part_info[2]);
        crc = (uint32_t)crc16(0, (const volatile void*)s_partition_buf, part_info[2]);

        if(crc != part_info[3])
        {
            SLB_DELETE_EXCHANGE_ZONE();
            return PPlus_ERR_INVALID_DATA;
        }
    }

    //make exchange zone effect(load data to boot zone)
    slb_erase_fw();

    for(i = 0; i< part_num; i++)
    {
        slb_spif_read(SLB_FLASH_FW_PART_FADDR(i), (uint8_t*) part_info, 0x10);
        slb_spif_read(SLB_FLASH_PART_DATA_BASE + part_info[0], (uint8_t*)s_partition_buf, part_info[2]);
        //LOG("part_num:%d,fladdr:%x,size:%x,crc:%x \n",i,part_info[1],part_info[2],part_info[3]);

        if((part_info[1] & 0xff000000) == OTAF_BASE_ADDR)//case runaddr is in XIP
        {
            //erase
            slb_spif_erase(part_info[1], part_info[2]);
            slb_spif_write(part_info[1], (uint8_t*)s_partition_buf, part_info[2]);
        }
        else
        {
            slb_spif_write(OTAF_APP_BANK_0_ADDR + flash_offset, (uint8_t*)s_partition_buf, part_info[2]);

            if(i > 0) //prepare boot sector
            {
                part_info[0] = flash_offset;
                slb_spif_write(OTAF_2nd_BOOTINFO_ADDR + 0x10 + 0x10*ram_part_idx, (uint8_t*) part_info, 16);
                ram_part_idx ++;
            }

            flash_offset += part_info[2] +4;
            flash_offset &= 0xfffffffc;//word align
        }
    }

    //prepare boot
    part_info[0] = ram_part_idx;
    part_info[1] = 0;//single bank
    part_info[2] = 0;
    part_info[3] = 0xffffffff;
    slb_spif_write(OTAF_2nd_BOOTINFO_ADDR, (uint8_t*) part_info, 16);
    SLB_DELETE_EXCHANGE_ZONE();
    return PPlus_SUCCESS;
}


int slb_boot_load_exch_zone(void)
{
    int ret = PPlus_SUCCESS;
    uint32_t part_num = 0;
    uint8_t read_tmp_data[32];
    uint32_t part_info[4];
    //check tag "OTAF"
    read_tmp_data[4] = 0;
    slb_spif_read(SLB_FLASH_FW_BASE, read_tmp_data, 4);

    if(strcmp((const char*)read_tmp_data, tag_ota_fw) != 0)
    {
        return PPlus_ERR_NOT_FOUND;
    }

    slb_spif_read(SLB_FLASH_FW_PART_NUM, (uint8_t*)(&part_num), 4);

    if(part_num < 2 || part_num> OTAF_PARTITION_NUM_MAX)
    {
        SLB_DELETE_EXCHANGE_ZONE();
        return PPlus_ERR_INVALID_DATA;
    }

    //check partition 0, if run address is 0xffffxxxx, it is special partition, need not load
    slb_spif_read(SLB_FLASH_FW_PART_FADDR(0), (uint8_t*) part_info, 0x10);

    if((part_info[1] & 0xffff0000) != 0xffff0000)
    {
        SLB_DELETE_EXCHANGE_ZONE();
        return PPlus_ERR_INVALID_DATA;
    }

    //check partition 0,
    //if the fw information of exhcange zone(version, device type,hw version, boot address) match to current fw
    ret = slb_check_currfw_exchfw();

    if(ret != PPlus_SUCCESS)
    {
        SLB_DELETE_EXCHANGE_ZONE();
        return PPlus_ERR_INVALID_DATA;
    }

    slb_apply_exch_zone_to_fw(part_num);
    return PPlus_SUCCESS;
}



int slb_boot_recover_exch_zone(void)
{
    uint32_t part_num;
    uint8_t read_tmp_data[32];
    uint32_t part_info[4];
    //check tag "@TAF"
    read_tmp_data[4] = 0;
    slb_spif_read(SLB_EXCH_AREA_BASE, read_tmp_data, 4);
    read_tmp_data[0] &= 0xf0;

    if(strcmp((const char*)read_tmp_data, tag_ota_fw_used) != 0)
    {
        return PPlus_ERR_NOT_FOUND;
    }

    slb_spif_read(SLB_FLASH_FW_PART_NUM, (uint8_t*)(&part_num), 4);

    if(part_num < 2 || part_num)
    {
        SLB_DELETE_EXCHANGE_ZONE();
        return PPlus_ERR_INVALID_DATA;
    }

    //check partition 0, if run address is 0xffffxxxx, it is special partition, need not load
    slb_spif_read(SLB_FLASH_FW_PART_FADDR(0), (uint8_t*) part_info, 0x10);

    if((part_info[1] & 0xffff0000) != 0xffff0000)
    {
        SLB_DELETE_EXCHANGE_ZONE();
        return PPlus_ERR_INVALID_DATA;
    }

    slb_apply_exch_zone_to_fw(part_num);
    return PPlus_SUCCESS;
}
#else

//buf[0] is piece data length
static uint32_t slb_ooo_buffer[SLB_OOB_PIECE_SIZE_MAX/4+1];

static int slb_ooo_pieces_upgrade_start(void)
{
    //erase exchange zone
    slb_spif_erase(SLB_EXCH_AREA_BASE, SLB_EXCH_AREA_SIZE);
    m_slb_ctx.state = SLB_PROG_ST_OOO_PIECE;
    return PPlus_SUCCESS;
}
static int slb_oob_piece_write(uint32_t index)
{
    int ret;
    uint32_t unit_size = m_slb_ctx.ooo_piece_len;
    ret = slb_spif_write(SLB_EXCH_AREA_BASE + unit_size * index, (uint8_t*)(slb_ooo_buffer+1), slb_ooo_buffer[0]);
    slb_ooo_buffer[0] = 0;
    return ret;
}

//build_datetime; ascii string: YYMMDDHHMMSS\n,this field can be NULL
int slb_upgrade_start(slb_fwinfo_t* pfwinfo,  char*  build_datetime)
{
    int ret;
    uint32_t value = 0;
    //reset data structure of contex
    memset(&m_slb_ctx, 0, sizeof(m_slb_ctx));
    m_slb_ctx.state = SLB_PROG_ST_IDLE;

    if(pfwinfo == NULL && build_datetime == NULL)
    {
        return slb_ooo_pieces_upgrade_start();
    }

    //check parameter
    if(pfwinfo->boot_addr != 0xffffffff)
    {
        if(pfwinfo->boot_addr < 0x1fff1838 || pfwinfo->boot_addr >SRAM2_BASE_ADDRESS)
            SLB_ASSERT( PPlus_ERR_INVALID_ADDR);
    }

    if(pfwinfo->partition_num > OTAF_PARTITION_NUM_MAX)
        SLB_ASSERT( PPlus_ERR_INVALID_PARAM);

    //erase exchange zone
    slb_spif_erase(SLB_EXCH_AREA_BASE, SLB_EXCH_AREA_SIZE);
    //save fw information
    value = (uint32_t)(pfwinfo->partition_num + 1);
    ret = slb_spif_write(SLB_FLASH_FW_PART_NUM, (uint8_t*) (&value), 4);
    SLB_ASSERT(ret);
    //write special partition
    ret = slb_spif_write(SLB_FLASH_PART_DATA_BASE, (uint8_t*)tag_selfload_ota_info, 4);
    SLB_ASSERT(ret);
    value = pfwinfo->manufactory_id &0xffff;
    value = (value << 16) | (pfwinfo->dev_type&0xffff);
    ret = slb_spif_write(SLB_FLASH_PART_DATA_BASE+4, (uint8_t*) (&value), 4);
    SLB_ASSERT(ret);
    value = (uint32_t)pfwinfo->fwver_major;
    value = (value<<8)|(uint32_t)pfwinfo->fwver_minor;
    value = (value<<8)|(uint32_t)pfwinfo->fwver_revision;
    value = (value<<8)|(uint32_t)pfwinfo->fwver_test;
    ret = slb_spif_write(SLB_FLASH_PART_DATA_BASE+8, (uint8_t*) (&value), 4);
    SLB_ASSERT(ret);
    value = pfwinfo->hwver;
    ret = slb_spif_write(SLB_FLASH_PART_DATA_BASE+0xc, (uint8_t*) (&value), 4);
    SLB_ASSERT(ret);

    if(build_datetime && strlen(build_datetime) < 0x10)
    {
        ret = slb_spif_write(SLB_FLASH_PART_DATA_BASE+0x10, (uint8_t*) (&build_datetime), strlen(build_datetime)+1);
        SLB_ASSERT(ret);
    }

    value = pfwinfo->boot_addr;
    ret = slb_spif_write(SLB_FLASH_PART_DATA_BASE+0x20, (uint8_t*) (&value), 4);
    SLB_ASSERT(ret);
    //write partition info
    value = 0;
    ret = slb_spif_write(SLB_FLASH_FW_PART_FADDR(0), (uint8_t*) (&value), 4);
    SLB_ASSERT(ret);
    value = 0xffff00f0;
    ret = slb_spif_write(SLB_FLASH_FW_PART_RADDR(0), (uint8_t*) (&value), 4);
    SLB_ASSERT(ret);
    value = 256;
    ret = slb_spif_write(SLB_FLASH_FW_PART_SIZE(0), (uint8_t*) (&value), 4);
    SLB_ASSERT(ret);
    value = slb_flash_calc_checksum(SLB_FLASH_PART_DATA_BASE, 256);
    ret = slb_spif_write(SLB_FLASH_FW_PART_CHKSUM(0), (uint8_t*) (&value), 4);
    SLB_ASSERT(ret);
    m_slb_ctx.state = SLB_PROG_ST_PART_INFO;
    m_slb_ctx.part_num = pfwinfo->partition_num + 1;
    m_slb_ctx.flash_offset = 256;//first 256 bytes is for special partition
    return PPlus_SUCCESS;
}

int slb_upgrade_partition_info(uint32_t runaddr, uint32_t len, uint16_t crc, bool* last_partition)
{
    int ret;
    uint32_t value = 0;
    ota_fw_part_t* ppart = &(m_slb_ctx.part);
    *last_partition = FALSE;

    if( m_slb_ctx.state != SLB_PROG_ST_PART_INFO)
        SLB_ASSERT(PPlus_ERR_INVALID_STATE);

    //save part info
    ppart->flash_addr = m_slb_ctx.flash_offset;
    ppart->run_addr = runaddr;
    ppart->size = len;
    ppart->checksum = crc;

    if(m_slb_ctx.part_num == 2 && last_partition)
        *last_partition = TRUE;

    m_slb_ctx.offset = 0;
    //write part info
    value = ppart->flash_addr;
    ret = slb_spif_write(SLB_FLASH_FW_PART_FADDR(m_slb_ctx.part_num-1), (uint8_t*) (&value), 4);
    SLB_ASSERT(ret);
    value = ppart->run_addr;
    ret = slb_spif_write(SLB_FLASH_FW_PART_RADDR(m_slb_ctx.part_num-1), (uint8_t*) (&value), 4);
    SLB_ASSERT(ret);
    value = ppart->size;
    ret = slb_spif_write(SLB_FLASH_FW_PART_SIZE(m_slb_ctx.part_num-1), (uint8_t*) (&value), 4);
    SLB_ASSERT(ret);
    value = (uint32_t)ppart->checksum;
    ret = slb_spif_write(SLB_FLASH_FW_PART_CHKSUM(m_slb_ctx.part_num-1), (uint8_t*) (&value), 4);
    SLB_ASSERT(ret);
    m_slb_ctx.state = SLB_PROG_ST_PART_DATA;
    return PPlus_SUCCESS;
}

// partition data storing should be as stream, auto seek
stream_st_t slb_upgrade_partition_data(uint8_t* data, uint32_t len)
{
    int ret;
    ota_fw_part_t* ppart = &(m_slb_ctx.part);

    if( m_slb_ctx.state != SLB_PROG_ST_PART_DATA)
        SLB_ASSERT(SLB_SST_ERROR);

    if(m_slb_ctx.offset + len > ppart->size)
        SLB_ASSERT(SLB_SST_ERROR);

    ret = slb_spif_write(SLB_FLASH_PART_DATA_BASE + m_slb_ctx.flash_offset + m_slb_ctx.offset, data, len);

    if(ret)
        SLB_ASSERT(SLB_SST_ERROR);

    m_slb_ctx.offset += len;

    if(m_slb_ctx.offset == ppart->size)
    {
        uint16_t crc = slb_flash_calc_checksum(SLB_FLASH_PART_DATA_BASE + m_slb_ctx.flash_offset, ppart->size);

        if(crc != ppart->checksum)
        {
            SLB_ASSERT(SLB_SST_ERROR);
        }

        m_slb_ctx.flash_offset += (ppart->size + 4) & 0xfffffffc;
        m_slb_ctx.part_num --;
        m_slb_ctx.state = SLB_PROG_ST_PART_INFO;

        if(m_slb_ctx.part_num == 1)
        {
            m_slb_ctx.state = SLB_PROG_ST_COMPLETED;
        }

        return SLB_SST_SUCCESS;
    }

    return SLB_SST_MOREDATA;
}

//upgrade out of order pieces
int slb_upgrade_ooo_pieces(uint32_t numbers, uint32_t index, uint8_t* data, uint32_t len)
{
    int ret;

    if( m_slb_ctx.state != SLB_PROG_ST_OOO_PIECE)
        SLB_ASSERT(PPlus_ERR_INVALID_STATE);

    if(m_slb_ctx.ooo_total_numbers == 0)
    {
        m_slb_ctx.ooo_total_numbers = numbers;
        m_slb_ctx.ooo_recv_numbers = 0;
    }
    else
    {
        if(m_slb_ctx.ooo_total_numbers != numbers)
            return PPlus_ERR_INVALID_PARAM;
    }

    if(m_slb_ctx.ooo_piece_len == 0)
    {
        if(numbers != (index+1))//not last packet
        {
            if(len & 3) return PPlus_ERR_INVALID_LENGTH;

            m_slb_ctx.ooo_piece_len = len;

            if(slb_ooo_buffer[0]) //case last piece data stored temporarily
            {
                ret = slb_oob_piece_write(index);
                SLB_ASSERT(ret);
            }

            slb_ooo_buffer[0] = len;
            memcpy((void*)(slb_ooo_buffer+1), data, len);
            ret = slb_oob_piece_write(index);
            SLB_ASSERT(ret);
        }
        else
        {
            slb_ooo_buffer[0] = len;
            memcpy((void*)(slb_ooo_buffer+1), data, len);
        }
    }
    else
    {
        if(numbers != (index+1) && m_slb_ctx.ooo_piece_len != len)
            return PPlus_ERR_INVALID_LENGTH;

        slb_ooo_buffer[0] = len;
        memcpy((void*)(slb_ooo_buffer+1), data, len);
        ret = slb_oob_piece_write(index);
        SLB_ASSERT(ret);
    }

    return PPlus_SUCCESS;
}


int slb_upgrade_apply_new_fw(void)
{
    int ret,i;

    if( m_slb_ctx.state == SLB_PROG_ST_COMPLETED)
    {
        //write tag
        ret = slb_spif_write(SLB_FLASH_FW_BASE, (uint8_t*)tag_ota_fw, 4);
        SLB_ASSERT(ret);
        WaitMs(100);
        NVIC_SystemReset();
    }
    else if(m_slb_ctx.state == SLB_PROG_ST_OOO_PIECE)
    {
        //data integrity check
        uint32_t part_num;
        uint32_t part_info[4];
        slb_spif_read(SLB_FLASH_FW_PART_NUM, (uint8_t*)(&part_num), 4);

        if(part_num < 2 || part_num> OTAF_PARTITION_NUM_MAX)
        {
            SLB_DELETE_EXCHANGE_ZONE();
            return PPlus_ERR_INVALID_DATA;
        }

        //check partition 0, if run address is 0xffffxxxx, it is special partition, need not load
        slb_spif_read(SLB_FLASH_FW_PART_FADDR(0), (uint8_t*) part_info, 0x10);

        if((part_info[1] & 0xffff0000) != 0xffff0000)
        {
            SLB_DELETE_EXCHANGE_ZONE();
            return PPlus_ERR_INVALID_DATA;
        }

        //validate partition
        for(i = 1; i< part_num; i++)
        {
            uint32_t crc = 0;
            slb_spif_read(SLB_FLASH_FW_PART_FADDR(i), (uint8_t*) part_info, 0x10);
            crc = slb_flash_calc_checksum(SLB_FLASH_PART_DATA_BASE + part_info[0], part_info[2]);

            if(crc != part_info[3])
            {
                SLB_DELETE_EXCHANGE_ZONE();
                return PPlus_ERR_INVALID_DATA;
            }
        }

        ret = slb_spif_write(SLB_FLASH_FW_BASE, (uint8_t*)tag_ota_fw, 4);
        SLB_ASSERT(ret);
        WaitMs(100);
        NVIC_SystemReset();
    }

    return PPlus_ERR_INVALID_STATE;
}




#endif

