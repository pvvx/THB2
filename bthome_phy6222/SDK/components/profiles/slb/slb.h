/*************
 slb.h
 SDK_LICENSE
***************/
	
#ifndef __SLB_H
#define __SLB_H


#ifndef SLB_EXCH_AREA_BASE
    #define SLB_EXCH_AREA_BASE 0x11055000
#endif
#ifndef SLB_EXCH_AREA_SIZE
    #define SLB_EXCH_AREA_SIZE (172*1024) //172K
#endif

#define SLB_OOB_PIECE_SIZE_MAX  256

enum
{
    SLB_FLASH_INTERNAL_512 = 1,
    SLB_FLASH_INTERNAL_1M,
    SLB_FLASH_INTERNAL_4M,
    SLB_FLASH_EXTERNAL_SPI = 0xf0
};

#define SLB_DATA_OUT_OF_ORDER 1     //case data out-of-order, each data record should have certain run address

#if(SLB_DATA_OUT_OF_ORDER == 1)
    #define SLB_PART_INFO_SAVED OTAF_PARTITION_NUM_MAX
#else
    #define SLB_PART_INFO_SAVED 1
#endif

#define SLB_FLASH SLB_FLASH_INTERNAL_512


#define SLB_FLASH_FW_BASE           (SLB_EXCH_AREA_BASE)
#define SLB_FLASH_FW_PART_NUM       (SLB_EXCH_AREA_BASE+4)

#define SLB_FLASH_FW_PART_FADDR(n)  (SLB_EXCH_AREA_BASE + 0x10 + 0x10*(n))
#define SLB_FLASH_FW_PART_RADDR(n)  (SLB_EXCH_AREA_BASE + 0x14 + 0x10*(n))
#define SLB_FLASH_FW_PART_SIZE(n)   (SLB_EXCH_AREA_BASE + 0x18 + 0x10*(n))
#define SLB_FLASH_FW_PART_CHKSUM(n) (SLB_EXCH_AREA_BASE + 0x1c + 0x10*(n))

#define SLB_FLASH_PART_DATA_BASE    (SLB_EXCH_AREA_BASE + 0x1000)

#define SLB_FLASH_CACHE_ENTER_BYPASS_SECTION()  do{ \
        AP_CACHE->CTRL0 = 0x02; \
        AP_PCR->CACHE_RST = 0x02;\
        AP_PCR->CACHE_BYPASS = 1;    \
    }while(0);


#define SLB_FLASH_CACHE_EXIT_BYPASS_SECTION()  do{ \
        AP_CACHE->CTRL0 = 0x00;\
        AP_PCR->CACHE_RST = 0x03;\
        AP_PCR->CACHE_BYPASS = 0;\
    }while(0);

typedef enum
{
    SLB_SST_SUCCESS = 0,
    SLB_SST_MOREDATA = 1,
    SLB_SST_ERROR = 0xff
} stream_st_t;

typedef struct
{
    uint32_t  boot_addr;
    uint8_t   partition_num;
    uint32_t  dev_type;
    uint32_t  manufactory_id;
    uint8_t   fwver_major;
    uint8_t   fwver_minor;
    uint8_t   fwver_revision;
    uint8_t   fwver_test;
    uint32_t  hwver;
} slb_fwinfo_t;

typedef int (*slb_chksum_cb_t)(uint8_t* data, uint32_t len);


#ifdef ON_SLB_BOOTLOADER

    int slb_boot_load_exch_zone(void);
    int slb_boot_recover_exch_zone(void);
#else
    int slb_upgrade_start(slb_fwinfo_t* pfwinfo,  char*  build_datetime);
    int slb_upgrade_partition_info(uint32_t runaddr, uint32_t len, uint16_t crc, bool* last_partition);
    stream_st_t slb_upgrade_partition_data(uint8_t* data, uint32_t len);
    int slb_upgrade_apply_new_fw(void);
#endif


#endif
