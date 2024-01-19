/******************************************************************************
 * @file     ble_ota.h
 *
 ******************************************************************************/

#ifndef BLE_OTA_H_
#define BLE_OTA_H_

#if OTA_TYPE
/* FLASH */

#ifndef FLASH_SIZE
#define	FLASH_SIZE			0x80000 	// 512k (512*1024)
#endif
#define	FLASH_MAX_SIZE		0x200000 	// 2M (2048*1024)
#ifndef FLASH_SECTOR_SIZE
#define	FLASH_SECTOR_SIZE	0x01000 	// 4k (4*1024)
#endif
#define	FADDR_START_ADDR	0x11000000
#define	FADDR_BOOT_ROM_INFO	(FADDR_START_ADDR + 0x02000)		// 4k
#define	FADDR_OTA_SEC		(FADDR_START_ADDR + 0x03000)		// 60k
#define	FADDR_APP_SEC		(FADDR_START_ADDR + 0x12000)		//
//#define	FADDR_DATA_SEC		(FADDR_START_ADDR + 0x40000)
#define	FADDR_EEP_SEC		(FADDR_START_ADDR + (FLASH_SIZE - 4*FLASH_SECTOR_SIZE))

#define	START_UP_FLAG		0x36594850	// "PHY6"

#define CMD_OTA_START		0xff00
#define CMD_OTA_SET			0xff01
#define CMD_OTA_END			0xff02

enum {
	OTA_SUCCESS = 0,		// success
	OTA_UNKNOWN_CMD,		// bad command
	OTA_NO_START,			// no start
	OTA_NO_PARAM,			// no parameters specified
	OTA_ERR_PARAM,			// invalid parameter(s)
	OTA_PKT_SIZE_ERR,		// packet size err
	OTA_PKT_CRC_ERR,		// packet CRC err
	OTA_PACKET_LOSS,		// lost one or more OTA PDU
	OTA_WRITE_FLASH_ERR,	// write OTA data to flash ERR
	OTA_DATA_UNCOMPLETE,	//lost last one or more OTA PDU
	OTA_TIMEOUT,			// timeout
	OTA_OVERFLOW,			// the ota adr overflow to 0x30000
	OTA_FW_CHECK_ERR,		//
	OTA_FW_CRC32_ERR,		//
	OTA_END = 0xff
};

typedef struct _ota_par_t {
	uint8_t  err_flag;
	uint8_t  version;
	uint8_t  start_flag;
	uint8_t  reboot_flag;
	uint32_t program_offset;
	uint16_t pkt_index;
	uint16_t pkt_total;
	uint32_t fw_value;
	uint32_t crc32;
	uint32_t erase_addr;
} ota_par_t;

extern ota_par_t ota;

int ota_parser(unsigned char *pout, unsigned char *pmsg, unsigned int msg_size);

#if OTA_TYPE == OTA_TYPE_BOOT
void startup_ota(void);
#endif

#endif // OTA_TYPE

#endif /* BLE_OTA_H_ */
