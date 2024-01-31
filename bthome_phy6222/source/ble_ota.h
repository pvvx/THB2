/******************************************************************************
 * @file     ble_ota.h
 *
 ******************************************************************************/

#ifndef BLE_OTA_H_
#define BLE_OTA_H_

/* FLASH */
#ifndef FLASH_SIZE
#define	FLASH_SIZE			0x80000 	// 512k (512*1024)
#endif
#define	FLASH_MAX_SIZE		0x200000 	// 2M (2048*1024)
#ifndef FLASH_SECTOR_SIZE
#define	FLASH_SECTOR_SIZE	0x01000 	// 4k (4*1024)
#endif
#define	FADDR_START_ADDR	FLASH_BASE_ADDR
#define	FADDR_BOOT_ROM_INFO	(FADDR_START_ADDR + 0x02000)		// 4k
#define	FADDR_OTA_SEC		(FADDR_START_ADDR + 0x03000)		// 52k
#define	FADDR_APP_SEC		(FADDR_START_ADDR + 0x10000)		// 176k (for 256k Flash)

#if OTA_TYPE

#define	START_UP_FLAG		0x36594850	// "PHY6"

#define CMD_OTA_START		0xff00
#define CMD_OTA_SET			0xff01
#define CMD_OTA_END			0xff02

enum {
	OTA_SUCCESS = 0,		//0 success
	OTA_UNKNOWN_CMD,		//1 bad command
	OTA_NO_START,			//2 no start
	OTA_NO_PARAM,			//3 no parameters specified
	OTA_ERR_PARAM,			//4 invalid parameter(s)
	OTA_PKT_SIZE_ERR,		//5 packet size err
	OTA_PKT_CRC_ERR,		//6 packet CRC err
	OTA_PACKET_LOSS,		//7 lost one or more OTA PDU
	OTA_WRITE_FLASH_ERR,	//8 write OTA data to flash ERR
	OTA_OVERFLOW,			//9 the ota addr overflow
	OTA_FW_CHECK_ERR,		//10
	OTA_FW_CRC32_ERR,		//11
	OTA_END = 0xff
};

typedef struct _ota_par_t {
	uint8_t  err_flag;
	uint8_t  version;
	uint8_t  start_flag;
	uint8_t  debug_flag; // debug flag
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
void startup_app(void);
#endif

#endif // OTA_TYPE

#endif /* BLE_OTA_H_ */
