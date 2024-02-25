/******************************************************************************
 * @file     ota_boot.h
 *
 ******************************************************************************/

#ifndef OTA_BOOT_H_
#define OTA_BOOT_H_

/* FLASH */
#ifndef FLASH_SIZE
#define	FLASH_SIZE			0x80000 	// 512k (512*1024)
#endif
#define	FLASH_MAX_SIZE		0x200000 	// 2M (2048*1024)
#ifndef FLASH_SECTOR_SIZE
#define	FLASH_SECTOR_SIZE	0x01000 	// 4k (4*1024)
#endif
#define	FADDR_START_ADDR	(0x11000000)
#define	FADDR_BOOT_ROM_INFO	(FADDR_START_ADDR + 0x02000)		// 4k
#define	FADDR_OTA_SEC		(FADDR_START_ADDR + 0x03000)		// 52k
#define	FADDR_APP_SEC		(FADDR_START_ADDR + 0x10000)		// 176k (for 256k Flash)

#define	START_UP_FLAG		0x36594850	// "PHY6"

#define OTA_MODE_SELECT_REG 0x4000f034
//#define OTA_MODE_SELECT_REG (AP_AON->RTCCC2) // [0x4000f034] == 0x55 -> OTA
#define BOOT_FLG_OTA	0x55 // перезагрузка в FW Boot для OTA (ожидание соединения 80 сек)
#define BOOT_FLG_FW0	0x33 // перезагрузка в FW Boot

typedef enum  _SYSCLK_SEL
{
    SYS_CLK_RC_32M      = 0,
    SYS_CLK_DBL_32M     = 1,
    SYS_CLK_XTAL_16M    = 2,
    SYS_CLK_DLL_48M     = 3,
    SYS_CLK_DLL_64M     = 4,
    SYS_CLK_DLL_96M     = 5,
    SYS_CLK_8M          = 6,
    SYS_CLK_4M          = 7,
    SYS_CLK_NUM         = 8,
} sysclk_t;

extern sysclk_t g_system_clk;

int _spif_wait_nobusy(uint8_t flg, uint32_t tout_ns);
int  spif_write(uint32_t addr, uint8_t* data, uint32_t size);
int  spif_write_dma(uint32_t addr, uint8_t* data, uint32_t size);
int  spif_read(uint32_t addr, uint8_t* data, uint32_t size);
int  spif_read_dma(uint32_t addr, uint8_t* data, uint32_t size);
int spif_erase_sector(unsigned int addr);
int spif_erase_block64(unsigned int addr);
int spif_erase_all(void);
uint8_t spif_flash_status_reg_0(void);
int spif_write_protect(bool en);
void spif_cmd(uint8_t op, uint8_t addrlen, uint8_t rdlen, uint8_t wrlen, uint8_t mbit, uint8_t dummy);
void spif_rddata(uint8_t* data, uint8_t len);
int spif_config(sysclk_t ref_clk, uint8_t div,  uint32_t rd_instr,  uint8_t mode_bit, uint8_t QE);


#endif /* OTA_BOOT_H_ */
