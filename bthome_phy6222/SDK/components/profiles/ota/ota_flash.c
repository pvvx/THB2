/*************
 ota_flash.c
 SDK_LICENSE
***************/
	
#include "flash.h"
#include "ota_flash.h"
#include "ota_app_service.h"
#include "ota_protocol.h"
#include "error.h"
#include "log.h"
#include "bus_dev.h"
#include "crc16.h"

bool is_crypto_app(void);
//void flash_load_parition(unsigned char* pflash, int size, unsigned char* run_addr);
int flash_load_parition(unsigned char* pflash, int size, unsigned char* micIn,unsigned char* run_addr);
int flash_check_parition(unsigned char* pflash, int size, unsigned char* run_addr,unsigned char* micOut);
//extern uint16_t $Supper$$crc16(uint16_t seed, const volatile void * p_data, uint32_t size);
#define OTA_FLASH_CACHE_ENTER_BYPASS_SECTION()  do{ \
        AP_CACHE->CTRL0 = 0x02; \
        AP_PCR->CACHE_RST = 0x02;\
        AP_PCR->CACHE_BYPASS = 1;    \
    }while(0);


#define OTA_FLASH_CACHE_EXIT_BYPASS_SECTION()  do{ \
        AP_CACHE->CTRL0 = 0x00;\
        AP_PCR->CACHE_RST = 0x03;\
        AP_PCR->CACHE_BYPASS = 0;\
    }while(0);

#if   defined ( __CC_ARM )
    #define OTA_APP_LOADAREA_T __attribute__((section("ota_app_loader_area")))
    #define OTA_APP_LOADAREA_D __attribute__((section("ota_app_loader_area")))
#elif defined ( __GNUC__ )
    #define OTA_APP_LOADAREA_T __attribute__((section("ota_app_loader_area")))
    #define OTA_APP_LOADAREA_D __attribute__((section("ota_app_loader_area_d")))
#endif

uint16_t OTA_APP_LOADAREA_D crc16_table[16] =
{
    0x0000, 0xCC01, 0xD801, 0x1400, 0xF001, 0x3C00, 0x2800, 0xE401,
    0xA001, 0x6C00, 0x7800, 0xB401, 0x5000, 0x9C01, 0x8801, 0x4400
};

uint32_t OTA_APP_LOADAREA_D  ota_load_flash_addr;
uint32_t OTA_APP_LOADAREA_D  ota_load_run_addr;
uint32_t OTA_APP_LOADAREA_D  ota_load_size;
uint32_t OTA_APP_LOADAREA_D  ota_load_checksum;
uint32_t OTA_APP_LOADAREA_D  ota_load_crc;
uint32_t OTA_APP_LOADAREA_D  ota_boot_bypass_crc;
uint32_t OTA_APP_LOADAREA_D  ota_slb_xip_addr = 0;
//uint32_t OTA_APP_LOADAREA_D  ota_load_mic;

static uint16_t OTA_APP_LOADAREA_T crc16_byte(uint16_t crc, uint8_t byte)
{
//    static const uint16_t crc16_table[16] =
//    {
//        0x0000, 0xCC01, 0xD801, 0x1400, 0xF001, 0x3C00, 0x2800, 0xE401,
//        0xA001, 0x6C00, 0x7800, 0xB401, 0x5000, 0x9C01, 0x8801, 0x4400
//    };
    uint16_t temp;
    // Compute checksum of lower four bits of a byte.
    temp         = crc16_table[crc & 0xF];
    crc  = (crc >> 4u) & 0x0FFFu;
    crc  = crc ^ temp ^ crc16_table[byte & 0xF];
    // Now compute checksum of upper four bits of a byte.
    temp         = crc16_table[crc & 0xF];
    crc  = (crc >> 4u) & 0x0FFFu;
    crc  = crc ^ temp ^ crc16_table[(byte >> 4u) & 0xF];
    return crc;
}

// uint16_t OTA_APP_LOADAREA_T OTA_crc16(uint16_t seed, const volatile void * p_data, uint32_t size)
//{
//    uint8_t * p_block = (uint8_t *)p_data;

//    while (size != 0)
//    {
//        seed = crc16_byte(seed, *p_block);
//        p_block++;
//        size--;
//    }

//   return seed;
//}
uint16_t OTA_APP_LOADAREA_T crc16(uint16_t seed, const volatile void* p_data, uint32_t size)
{
//  OTA_crc16(seed,p_data,size);
    uint8_t* p_block = (uint8_t*)p_data;

    while (size != 0)
    {
        seed = crc16_byte(seed, *p_block);
        p_block++;
        size--;
    }

    return seed;
}

int OTA_APP_LOADAREA_T ota_flash_read(uint32_t* dest, uint32_t addr, uint32_t size)
{
    int i;
    uint32_t cb = AP_PCR->CACHE_BYPASS;

    if((((uint32_t)dest)%4) || (addr %4)|| (size%4))
        return PPlus_ERR_DATA_ALIGN;

    if(addr < OTAF_START_ADDR || addr >= OTAF_END_ADDR)
        return PPlus_ERR_INVALID_ADDR;

    //read flash addr direct access
    //bypass cache

    if(cb == 0)
    {
        OTA_FLASH_CACHE_ENTER_BYPASS_SECTION();
    }

    for(i = 0; i < size ; i += 4)
        *dest++ = read_reg(addr + i);

    if(cb == 0)
    {
        OTA_FLASH_CACHE_EXIT_BYPASS_SECTION();
    }

    return PPlus_SUCCESS;
}


int OTA_APP_LOADAREA_T ota_flash_load_app(void)
{
    int i,ret;
    bool is_encrypt = FALSE;
    uint32_t partition_num = 0;
    uint32_t bank_info = 0;
    uint32_t bank_addr = 0;
    uint32_t xip_entry = 0;
    bool  is_not_xip = false;
    ota_flash_read(&partition_num, OTAF_2nd_BOOTINFO_ADDR, 4);

    if(partition_num == 0xffffffff)
    {
        return PPlus_ERR_OTA_NO_APP;
    }

    if(partition_num > OTAF_PARTITION_NUM_MAX || partition_num == 0)
    {
        // LOG("PPlus_ERR_OTA_BAD_DATA - part numb > OTAF_PARTITION_NUM_MAX\r\n");
        return PPlus_ERR_OTA_BAD_DATA;
    }

    ota_flash_read(&bank_info, OTAF_2nd_BOOTINFO_ADDR + 4, 4);

    if(bank_info == OTAF_DUAL_BANK_1)
    {
        // LOG("OTAF_APP_BANK_1_ADDR\r\n");
        bank_addr = OTAF_APP_BANK_1_ADDR;
    }
    else if(bank_info == OTAF_DUAL_BANK_0 || bank_info == OTAF_SINGLE_BANK)
    {
        // LOG("OTAF_APP_BANK_0_ADDR\r\n");
        bank_addr = OTAF_APP_BANK_0_ADDR;
    }
    else if(bank_info == OTAF_DUAL_SLBXIP)
    {
        // LOG("OTAF_DUAL_SLBXIP\r\n");
        bank_addr = 0;
    }
    else
    {
        // LOG("PPlus_ERR_OTA_BAD_DATA - bank numb > 2\r\n");
        return PPlus_ERR_OTA_BAD_DATA;
    }

    is_encrypt = is_crypto_app();
    volatile uint32_t t = 100;

    while(t--) {};

    ota_flash_read(&ota_boot_bypass_crc, OTAF_2nd_BOOT_FAST_BOOT, 4);

    for(i = 1; i< partition_num+1; i++)
    {
//    uint32_t flash_addr;
//    uint32_t run_addr;
//    uint32_t size;
//    uint32_t checksum;
//    uint16_t crc;
        ota_flash_read(&ota_load_flash_addr, OTAF_2nd_BOOTINFO_ADDR + i*4*4, 4);
        ota_flash_read(&ota_load_run_addr,   OTAF_2nd_BOOTINFO_ADDR + i*4*4 + 4,  4);
        ota_flash_read(&ota_load_size,       OTAF_2nd_BOOTINFO_ADDR + i*4*4 + 8,  4);
        ota_flash_read(&ota_load_checksum,   OTAF_2nd_BOOTINFO_ADDR + i*4*4 + 12, 4);

        if(is_not_xip==false && i == 1)
            xip_entry = ota_load_run_addr;

        if((ota_load_flash_addr==0xffffffff) || (ota_load_run_addr == 0xffffffff )||(ota_load_size == 0xffffffff )||(ota_load_checksum == 0xffffffff ))
        {
            return PPlus_ERR_OTA_NO_APP;
        }

        //ota_sec_flag = ota_load_size&0x80000000;

        //case XIP mode, shoud be in single bank and no fct
        if((ota_load_run_addr == ota_load_flash_addr ) &&
                ((ota_load_flash_addr & 0xff000000) == OTAF_BASE_ADDR))
        {
            // LOG("XIP MODE >>> ");
            if(USE_FCT==0 && CFG_OTA_BANK_MODE==OTA_SINGLE_BANK)
            {
                // LOG("NEXT PART! \r\n");
                // continue;
                if(is_encrypt)
                {
                    if(ota_boot_bypass_crc!=OTA_FAST_BOOT_MAGIC)
                    {
                        flash_check_parition((uint8_t*)ota_load_flash_addr,(int)ota_load_size,NULL,(uint8_t*)&ota_load_crc);

                        if ( ota_load_crc!=ota_load_checksum )
                        {
                            write_reg(OTA_MODE_SELECT_REG, OTA_MODE_OTA);
                            hal_system_soft_reset();
                        }
                    }
                }
                else
                {
                    if(ota_boot_bypass_crc!=OTA_FAST_BOOT_MAGIC)
                    {
                        // ota_flash_read((uint32_t*)ota_load_run_addr, ota_load_flash_addr + bank_addr, ota_load_size);
                        ota_load_crc = crc16(0, (const volatile void* )ota_load_flash_addr, ota_load_size);

                        if(ota_load_crc != (uint16)ota_load_checksum)
                        {
                            //if crc incorrect, reboot to OTA mode
                            write_reg(OTA_MODE_SELECT_REG, OTA_MODE_OTA);
                            hal_system_soft_reset();
                        }
                    }
                }

                continue;
            }
            else
            {
                // LOG("PPlus_ERR_INVALID_DATA\r\n");
                return PPlus_ERR_INVALID_DATA;
            }
        }

        is_not_xip = true;

        if((ota_load_run_addr&0xffff0000) == 0xffff0000)
        {
            continue;
        }

        //load binary
        if(is_encrypt)
        {
//              if(ota_sec_flag){
            ret = flash_load_parition((uint8_t*)(ota_load_flash_addr + bank_addr), (int)ota_load_size, (uint8_t*)&ota_load_checksum,(uint8_t*)ota_load_run_addr);

            //LOG("ret=%d\n",ret);
            if(ret!=0)
            {
                //if crc incorrect, reboot to OTA mode
                write_reg(OTA_MODE_SELECT_REG, OTA_MODE_OTA);
                hal_system_soft_reset();
            }

//              }
//              else{
//                  flash_check_parition((uint8_t*)(ota_load_flash_addr + bank_addr),(int)ota_load_size&0xffffff,NULL,(uint8_t*)&ota_load_crc);
//                  if(ota_load_crc!=ota_load_checksum){
//                          write_reg(OTA_MODE_SELECT_REG, OTA_MODE_OTA);
//                          hal_system_soft_reset();
//                  }
//                  ota_flash_read((uint32_t*)ota_load_run_addr, ota_load_flash_addr + bank_addr, ota_load_size);
//              }
        }
        else
        {
            ota_flash_read((uint32_t*)ota_load_run_addr, ota_load_flash_addr + bank_addr, ota_load_size);

            if(ota_boot_bypass_crc!=OTA_FAST_BOOT_MAGIC)
            {
                ota_load_crc = crc16(0, (const volatile void* )ota_load_run_addr, ota_load_size);

                if(ota_load_crc != (uint16)ota_load_checksum)
                {
                    //if crc incorrect, reboot to OTA mode
                    write_reg(OTA_MODE_SELECT_REG, OTA_MODE_OTA);
                    hal_system_soft_reset();
                }
            }
        }
    }

    ota_slb_xip_addr = xip_entry;
    // LOG("PPlus_SUCCESS\r\n");
    return PPlus_SUCCESS;
}

#if(CFG_FLASH >= 512)
int ota_flash_load_fct(void)
{
    return PPlus_SUCCESS;
}
#endif




int ota_flash_read_bootsector(uint32_t* bank_addr)
{
    uint32_t partition_num = 0;
    uint32_t bank_info = 0;
    *bank_addr = OTAF_APP_BANK_0_ADDR;
    ota_flash_read(&partition_num, OTAF_2nd_BOOTINFO_ADDR, 4);

    if(partition_num == 0xffffffff)
        return PPlus_ERR_OTA_NO_APP;

    if(partition_num > OTAF_PARTITION_NUM_MAX)
        return PPlus_ERR_OTA_BAD_DATA;

    ota_flash_read(&bank_info, OTAF_2nd_BOOTINFO_ADDR + 4, 4);

    if(bank_info == OTAF_DUAL_BANK_0)
    {
        *bank_addr = OTAF_APP_BANK_1_ADDR;
    }
    else if(bank_info == OTAF_DUAL_BANK_1 || bank_info == OTAF_SINGLE_BANK)
    {
        *bank_addr = OTAF_APP_BANK_0_ADDR;
    }
    else
    {
        return PPlus_ERR_OTA_BAD_DATA;
    }

    return PPlus_SUCCESS;
}

int ota_flash_write_partition(uint32 addr, uint32_t* p_sect, uint32_t size)
{
    if(addr % 4)
        return PPlus_ERR_DATA_ALIGN;

    size = (size + 3) & 0xfffffffc;
    #if 0
    int ret = 0;
    uint32_t i;

    for(i = 0; i < size /4; i++)
    {
        ret = flash_write_word(addr + i*4, p_sect[i]);

        if(ret == PPlus_SUCCESS)
            continue;

        ret = flash_write_word(addr + i*4, p_sect[i]);

        if(ret == PPlus_SUCCESS)
            continue;

        ret = flash_write_word(addr + i*4, p_sect[i]);

        if(ret == PPlus_SUCCESS)
            continue;

        ret = flash_write_word(addr + i*4, p_sect[i]);

        if(ret != PPlus_SUCCESS)
            return PPlus_ERR_SPI_FLASH;
    }

    return PPlus_SUCCESS;
    #endif
    return(hal_flash_write_by_dma(addr, (uint8_t*) p_sect, size));
}

int ota_flash_write_boot_sector(uint32_t* p_sect, uint32_t size, uint32_t offset)
{
    if(size % 4)
        return PPlus_ERR_DATA_ALIGN;

    #if 0
    uint32_t i;
    int ret = 0;

    for(i = 0; i < size /4; i++)
    {
        ret = flash_write_word(OTAF_2nd_BOOTINFO_ADDR + i*4 + offset, p_sect[i]);

        if(ret == PPlus_SUCCESS)
            continue;

        ret = flash_write_word(OTAF_2nd_BOOTINFO_ADDR + i*4 + offset, p_sect[i]);

        if(ret == PPlus_SUCCESS)
            continue;

        ret = flash_write_word(OTAF_2nd_BOOTINFO_ADDR + i*4 + offset, p_sect[i]);

        if(ret == PPlus_SUCCESS)
            continue;

        ret = flash_write_word(OTAF_2nd_BOOTINFO_ADDR + i*4 + offset, p_sect[i]);

        if(ret != PPlus_SUCCESS)
            return PPlus_ERR_SPI_FLASH;
    }

    return PPlus_SUCCESS;
    #endif
    return(hal_flash_write_by_dma(OTAF_2nd_BOOTINFO_ADDR+offset, (uint8_t*) p_sect, size));
}

int ota_flash_erase(uint32_t bank_addr)
{
    int i;

    if(CFG_OTA_BANK_MODE == OTA_SINGLE_BANK)
    {
        if(bank_addr != OTAF_APP_BANK_0_ADDR)
            return PPlus_ERR_INVALID_PARAM;

        //erase boot sector
        hal_flash_erase_sector(OTAF_2nd_BOOTINFO_ADDR);

        for(i = 0; i< OTAF_APP_BANK_SIZE; i+= OTAF_SECTOR_SIZE)
            hal_flash_erase_sector(OTAF_APP_BANK_0_ADDR + i);

        return PPlus_SUCCESS;
    }
    else
    {
        if(bank_addr == OTAF_APP_BANK_0_ADDR || bank_addr == OTAF_APP_BANK_1_ADDR)
        {
            //erase application bank
            for(i = 0; i< OTAF_APP_BANK_SIZE; i+= OTAF_SECTOR_SIZE)
                hal_flash_erase_sector(bank_addr + i);

            return PPlus_SUCCESS;
        }
    }

    return PPlus_ERR_INVALID_PARAM;
}


int ota_flash_erase_area(uint32_t flash_addr, uint32_t size)
{
    int ret = PPlus_ERR_INVALID_ADDR;
    int offset;
    flash_addr = flash_addr | 0x11000000;

    if(flash_addr >=OTAF_1st_BOOTINFO_ADDR + OTAF_1st_BOOTINFO_SIZE && (flash_addr + size) <= OTAF_2nd_BOOTINFO_ADDR )
    {
        ret = PPlus_SUCCESS;
    }

    if(flash_addr >=OTAF_2nd_BOOTINFO_ADDR + 4*1024 && (flash_addr + size) <= OTAF_APP_BANK_0_ADDR )
    {
        ret = PPlus_SUCCESS;
    }

    if((flash_addr + size) <= OTAF_END_ADDR +1)
    {
        ret = PPlus_SUCCESS;
    }

    if(flash_addr & 0xfff || size & 0xfff)
    {
        ret = PPlus_ERR_DATA_ALIGN;
    }

    if(ret)
        return ret;

    if((flash_addr & 0xffff  == 0 ) && (size & 0xffff == 0))  //case 64K align, erase block
    {
        for(offset= 0; offset < size; offset += 64*1024)
            hal_flash_erase_block64(flash_addr + offset);
    }
    else  //erase sector
    {
        for(offset = 0; offset< size; offset+= OTAF_SECTOR_SIZE)
            hal_flash_erase_sector(flash_addr + offset);
    }

    return PPlus_SUCCESS;
}



