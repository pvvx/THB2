/*
  main.c
*/
#include <string.h>
#include "global_config.h"
#include "rom_sym_def.h"
#include "ota_boot.h"
#include "bus_dev.h"


/*******************************************************************************/
extern int m_in_critical_region;
extern const uint32_t _sbss;
extern const uint32_t _ebss;

/****************************************************************************
    Name: c_start

    Description:
     This is the reset entry point.

 ****************************************************************************/

#define	WR_BLK_SIZE  		256

/* Заголовок OTA */
typedef struct _app_info_t {
	uint32_t flag;			// id = START_UP_FLAG
	uint32_t seg_count; 	// кол-во сегментов
	uint32_t start_addr;	// стартовый/run  адрес (if = -1 -> берестя из первого значения != -1 у сегмента)
	uint32_t app_size;		// размер OTA без 4-х байт CRC32
} app_info_t;

app_info_t info_app;

/* Описание сегментов OTA */
typedef struct _app_info_seg_t {
	uint32_t faddr;	// адрес записи в Flash
	uint32_t size;	// размер сегмента
	uint32_t waddr;	// рабочий адрес
	uint32_t chk;	// не используется
} app_info_seg_t;

app_info_seg_t seg_info;

uint8_t sector_buf[WR_BLK_SIZE];

__attribute__ ((naked))
void copy_app_code(void) {
	uint32_t blksize = WR_BLK_SIZE;
	uint32_t rfaddr = FADDR_APP_SEC + 0xfc;
	uint32_t dfaddr = 0;
	uint32_t wfaddr = FADDR_BOOT_ROM_INFO;
	uint32_t count;
    __disable_irq();
	spif_read(rfaddr, (uint8_t*)&rfaddr, 4);
	spif_read(rfaddr, (uint8_t*)&info_app, sizeof(info_app));
	if(info_app.flag == START_UP_FLAG
		&& info_app.seg_count
		&& info_app.seg_count < 16
		&& info_app.app_size < FADDR_APP_SEC - FADDR_OTA_SEC
		){
		dfaddr = rfaddr + 0x100;
		count = info_app.seg_count;
		spif_erase_sector(wfaddr);
		spif_write(wfaddr, (uint8_t*)&info_app.seg_count, 4);
		spif_write(wfaddr + 8, (uint8_t*)&info_app.start_addr, 4);
		wfaddr += 0x100;
		while(count--) {
			rfaddr += 16;
			spif_read(rfaddr, (uint8_t*)&seg_info, 12);
			spif_write(wfaddr, (uint8_t*)&seg_info, 12);
			wfaddr += 16;
		}
		wfaddr = FADDR_OTA_SEC;
		count = info_app.app_size;
		while(count) {
			if(count < WR_BLK_SIZE)
				blksize = count;
			if((wfaddr & (FLASH_SECTOR_SIZE - 1)) == 0)
				spif_erase_sector(wfaddr);
			spif_read(dfaddr, sector_buf, blksize);
			spif_write(wfaddr, sector_buf, blksize);
			dfaddr += blksize;
			wfaddr += blksize;
			count -= blksize;
		}
		spif_erase_sector(FADDR_APP_SEC);
	}
    //__disable_irq();
    m_in_critical_region++;
    /**
        config reset casue as RSTC_WARM_NDWC
        reset path walkaround dwc
    */
    AP_AON->RTCCC2 = BOOT_FLG_OTA;	// [0x4000f034] == 0x55 -> OTA
    AP_AON->SLEEP_R[0] = 4;
    AP_AON->SLEEP_R[1] = 0;
    AP_PCR->SW_RESET1 = 0;
    while(1);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
//xip flash read instrcution
#define XFRD_FCMD_READ          0x0000003
#define XFRD_FCMD_READ_DUAL     0x801003B
#define XFRD_FCMD_READ_QUAD     0x801006B

int main(void) {
    /*  Clear .bss.  We'll do this inline (vs. calling memset) just to be
        certain that there are no issues with the state of global variables.
    */
    uint8_t* dest = (uint8_t*)&_sbss;
    uint8_t* edest = (uint8_t*)&_ebss;

    memset(dest, 0, edest - dest);

	g_system_clk = SYS_CLK_XTAL_16M; // SYS_CLK_XTAL_16M, SYS_CLK_DBL_32M, SYS_CLK_DLL_64M
   	spif_config(SYS_CLK_DLL_64M, 1, XFRD_FCMD_READ_DUAL, 0, 0);
   	copy_app_code();
	return 0;
}


