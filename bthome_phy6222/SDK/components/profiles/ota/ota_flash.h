/*************
 ota_flash.h
 SDK_LICENSE
***************/

#ifndef _OTA_FLASH_H
#define _OTA_FLASH_H

#include "types.h"

#define OTAF_PARTITION_NUM_MAX  32

enum
{
    OTAF_SINGLE_BANK  = 0,
    OTAF_DUAL_BANK_0  = 1,
    OTAF_DUAL_BANK_1  = 2,
    OTAF_DUAL_SLBXIP  = 3
};

#define DBG_BOOT_IO_TOGGLE do{\
        *(volatile unsigned int  *)(0x40008004) |= 0x1;\
        *(volatile unsigned int  *)(0x40008000) |= 0x1;\
        *(volatile unsigned int  *)(0x40008000) &= ~0x1;}while(0)

#define OTA_MAGIC_CODE    0x32af08cb  //random data
#define OTA_FAST_BOOT_MAGIC   0xa5a55a5a  //random data

#define OTAF_SECTOR_SIZE  (1024*4)  //4k size

#define OTA_DUMMY_BANK  0 //undefined mode, will report error
#define OTA_SINGLE_BANK 1
#define OTA_DUAL_BANK   2

#ifndef CFG_OTA_BANK_MODE
    #define CFG_OTA_BANK_MODE OTA_DUMMY_BANK
#endif

#ifndef USE_FCT
    #define USE_FCT 0
#endif

#ifndef CFG_FLASH
    #define CFG_FLASH 0
#endif

#if(CFG_FLASH == 256 && USE_FCT==0 && CFG_OTA_BANK_MODE==OTA_SINGLE_BANK)

    #define OTAF_BASE_ADDR          0x11000000

    #define OTAF_1st_BOOTINFO_ADDR  0x11002000  //4k bytes
    #define OTAF_1st_BOOTINFO_SIZE  0x1000  //4k bytes

    #define OTAF_2nd_BOOTINFO_ADDR  0x11003000  //4k bytes
    #define OTAF_APP_BANK_0_ADDR    0x11011000  //60K bytes
    #define OTAF_APP_BANK_1_ADDR    OTA_MAGIC_CODE  //Dummy

    #define OTAF_APP_BANK_SIZE      (1024*60)

    #define OTAF_NVM_ADDR           0x11020000  //8K bytes


    #define OTAF_APPLICATION_RUN    0x1fff1838  //??? need confirm

    #elif(CFG_FLASH >= 512 && USE_FCT==1 && CFG_OTA_BANK_MODE==OTA_DUAL_BANK)

    #define OTAF_BASE_ADDR          0x11000000

    #define OTAF_1st_BOOTINFO_ADDR  0x11002000  //4k bytes
    #define OTAF_1st_BOOTINFO_SIZE  0x1000  //4k bytes

    #define OTAF_2nd_BOOTINFO_ADDR  0x11009000  //4k bytes
    #define OTAF_APP_FCT_ADDR       0x11012000  //120K bytes, first 4k is FCT boot info
    #define OTAF_APP_BANK_0_ADDR    0x11030000  //128K bytes
    #define OTAF_APP_BANK_1_ADDR    0x11050000  //128K bytes

    #define OTAF_APP_BANK_SIZE      (1024*128)

    #define OTAF_NVM_ADDR           0x11070000  //64K bytes


    #define OTAF_APPLICATION_RUN    0x1fff1838  //??? need confirm

    #elif(CFG_FLASH >= 512 && USE_FCT==0 && CFG_OTA_BANK_MODE==OTA_DUAL_BANK)

    #define OTAF_BASE_ADDR          0x11000000

    #define OTAF_1st_BOOTINFO_ADDR  0x11002000  //4k bytes
    #define OTAF_1st_BOOTINFO_SIZE  0x1000  //4k bytes

    #define OTAF_2nd_BOOTINFO_ADDR  0x11009000  //4k bytes
    #define OTAF_APP_BANK_0_ADDR    0x11012000  //128K bytes
    #define OTAF_APP_BANK_1_ADDR    0x11032000  //128K bytes

    #define OTAF_APP_BANK_SIZE      (1024*128)

    #define OTAF_NVM_ADDR           0x11052000  //64K bytes


    #define OTAF_APPLICATION_RUN    0x1fff1838  //??? need confirm

    #elif(CFG_FLASH >= 512 && USE_FCT==1 && CFG_OTA_BANK_MODE==OTA_SINGLE_BANK)

    #define OTAF_BASE_ADDR          0x11000000

    #define OTAF_1st_BOOTINFO_ADDR  0x11002000  //4k bytes
    #define OTAF_1st_BOOTINFO_SIZE  0x1000  //4k bytes

    #define OTAF_2nd_BOOTINFO_ADDR  0x11009000  //4k bytes
    #define OTAF_APP_FCT_ADDR       0x11012000  //120K bytes, first 4k is FCT boot info
    #define OTAF_APP_BANK_0_ADDR    0x11030000  //128K bytes
    #define OTAF_APP_BANK_1_ADDR    OTA_MAGIC_CODE  //Dummy

    #define OTAF_APP_BANK_SIZE      (1024*128)

    #define OTAF_NVM_ADDR           0x11050000  //64K bytes


    #define OTAF_APPLICATION_RUN    0x1fff1838  //??? need confirm

    #elif(CFG_FLASH >= 512 && USE_FCT==0 && CFG_OTA_BANK_MODE==OTA_SINGLE_BANK)

    #define OTAF_BASE_ADDR          0x11000000

    #define OTAF_1st_BOOTINFO_ADDR  0x11002000  //4k bytes
    #define OTAF_1st_BOOTINFO_SIZE  0x1000  //4k bytes

    #define OTAF_2nd_BOOTINFO_ADDR  0x11003000  //4k bytes
    #define OTAF_APP_BANK_0_ADDR    0x11011000  //128K bytes
    #define OTAF_APP_BANK_1_ADDR    OTA_MAGIC_CODE  //Dummy

    #define OTAF_APP_BANK_SIZE      (1024*60)

    #define OTAF_NVM_ADDR           0x11020000  //64K bytes


    #define OTAF_APPLICATION_RUN    0x1fff1838  //??? need confirm

    #define OTAF_2nd_BOOT_FAST_BOOT 0x11003ff0  //bypass 2nd boot crc check for fast boot

#else
    #error "unsupported OTA config, please check these micro:CFG_FLASH, USE_FCT,CFG_OTA_BANK_MODE!"
#endif


#if(CFG_FLASH == 256)
    #define OTAF_START_ADDR         0x11003000
    #define OTAF_END_ADDR           0x1103ffff
    #elif(CFG_FLASH == 512)
    #define OTAF_START_ADDR         0x11003000
    #define OTAF_END_ADDR           0x1107ffff
    #elif(CFG_FLASH == 1024)
    #define OTAF_START_ADDR         0x11003000
    #define OTAF_END_ADDR           0x11100000
    #elif(CFG_FLASH == 4096)
    #define OTAF_START_ADDR         0x11003000
    #define OTAF_END_ADDR           0x111fffff
#else
    #error "unsupported OTA config, please check these micro:CFG_FLASH!"
#endif

#define MAX_SECT_SUPPORT  32//16


typedef struct
{
    uint32_t  flash_addr;
    uint32_t  run_addr;
    uint32_t  size;
    uint16_t  checksum;
} ota_fw_part_t;

typedef struct
{
    uint8_t           part_num;
    uint8_t           part_current;
    uint32_t          total_size;
    uint32_t          total_offset;
    uint32_t          offset;
    ota_fw_part_t     part[MAX_SECT_SUPPORT];
} ota_fw_t;


#if(CFG_FLASH >= 512)
    int ota_flash_load_fct(void);
#endif
int ota_flash_load_app(void);
int ota_flash_write_partition(uint32 addr, uint32_t* p_sect, uint32_t size);
int ota_flash_write_boot_sector(uint32_t* p_sect, uint32_t size, uint32_t offset);
int ota_flash_erase(uint32_t addr);
int ota_flash_erase_area(uint32_t flash_addr, uint32_t size);
int ota_flash_read_bootsector(uint32_t* bank_addr);

#endif

