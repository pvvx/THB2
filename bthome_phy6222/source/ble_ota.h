/******************************************************************************
 * @file     ble_ota.h
 *
 ******************************************************************************/

#ifndef BLE_OTA_H_
#define BLE_OTA_H_

/* FLASH */

#define	FLASH_SIZE			0x80000 	// 512k (512*1024)
#define	FLASH_MAX_SIZE		0x200000 	// 2M (2048*1024)
#define	FLASH_SECTOR_SIZE	0x01000 	// 4k (4*1024)
#define	FADD_START_ADDR		0x11000000
#define	FADD_BOOT_ROM_INFO	(FADD_START_ADDR + 0x02000)		// 4k
#define	FADD_BOOT_OTA		(FADD_START_ADDR + 0x03000)		// 4k
#define	FADD_APP_INFO		(FADD_START_ADDR + 0x04000)		// 4k
#define	FADD_OTA_SEC		(FADD_START_ADDR + 0x05000)		// 44k
#define	FADD_APP_SEC		(FADD_START_ADDR + 0x10000)		// 236k (2x118k)
#define	FADD_DATA_SEC		(FADD_START_ADDR + 0x40000)		// 248k
#define	FADD_EEP_SEC		(FADD_START_ADDR + (FLASH_SIZE - 2*FLASH_SECTOR_SIZE))

#define	START_UP_FLAG		0x12345678


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

extern int otaWrite(void *p);
extern int otaRead(void *p);

#endif /* BLE_OTA_H_ */
