/*
 * cmd_parcer.c
 *
 *  Created on: 16 янв. 2024 г.
 *      Author: pvvx
 */

/*********************************************************************
	INCLUDES
*/
#include "bcomdef.h"
#include "config.h"
#include "OSAL.h"
#include "linkdb.h"
#include "att.h"
#include "gatt.h"
#include "gatt_uuid.h"
#include "gattservapp.h"
#include "gapbondmgr.h"
#include "flash.h"
#include "flash_eep.h"
#include "bleperipheral.h"
#include "sbp_profile.h"
#include "sensors.h"
#include "cmd_parser.h"
#include "devinfoservice.h"
#include "ble_ota.h"
#include "thb2_peripheral.h"
#include "lcd_th05.h"
/*********************************************************************/
#define SEND_DATA_SIZE	16

const dev_id_t dev_id = {
		.pid = CMD_ID_DEVID,
		.revision = 1,
		.hw_version = DEVICE,
		.sw_version = APP_VERSION,
		.dev_spec_data = 0,
		.services = DEV_SERVICES
};

int cmd_parser(uint8_t * obuf, uint8_t * ibuf, uint32_t len) {
	int olen = 0;
	if (len) {
		uint8_t cmd = ibuf[0];
		obuf[0] = cmd;
		obuf[1] = 0; // no err
		if (cmd == CMD_ID_DEVID) { // Get DEV_ID
			osal_memcpy(obuf, &dev_id, sizeof(dev_id));
			olen = sizeof(dev_id);
		} else if (cmd == CMD_ID_CFG) {		// Get/Set device config
			if (--len > sizeof(cfg))
				len = sizeof(cfg);
			if (len) {
				osal_memcpy(&cfg, &ibuf[1], len);
				test_config();
				flash_write_cfg(&cfg, EEP_ID_CFG, sizeof(cfg));
			}
			osal_memcpy(&obuf[1], &cfg, sizeof(cfg));
			olen = sizeof(cfg) + 1;
		} else if (cmd == CMD_ID_CFG_DEF) { // Set default device config
			osal_memcpy(&cfg, &def_cfg, sizeof(cfg));
			test_config();
			flash_write_cfg(&cfg, EEP_ID_CFG, sizeof(cfg));
			osal_memcpy(&obuf[1], &cfg, sizeof(cfg));
			olen = sizeof(cfg) + 1;
		} else if (cmd == CMD_ID_CFS) {	// Get/Set sensor config
			if (--len > sizeof(thsensor_cfg.coef))
				len = sizeof(thsensor_cfg.coef);
			if (len) {
				osal_memcpy(&thsensor_cfg.coef, &ibuf[1], len);
				flash_write_cfg(&thsensor_cfg.coef, EEP_ID_CFS, sizeof(thsensor_cfg.coef));
			}
			osal_memcpy(&obuf[1], &thsensor_cfg, thsensor_cfg_send_size);
			olen = thsensor_cfg_send_size + 1;
		} else if (cmd == CMD_ID_CFS_DEF) {	// Get/Set default sensor config
			osal_memset(&thsensor_cfg, 0, thsensor_cfg_send_size);
			init_sensor();
			osal_memcpy(&obuf[1], &thsensor_cfg, thsensor_cfg_send_size);
			olen = thsensor_cfg_send_size + 1;
		} else if (cmd == CMD_ID_SERIAL) {
			osal_memcpy(&obuf[1], devInfoSerialNumber, sizeof(devInfoSerialNumber)-1);
			olen = 1 + sizeof(devInfoSerialNumber)-1;
		} else if (cmd == CMD_ID_FLASH_ID) {
			osal_memcpy(&obuf[1], (uint8_t *)&phy_flash.IdentificationID, 8);
			olen = 1 + 8;
		} else if (cmd == CMD_ID_SEN_ID) {
			osal_memcpy(&obuf[1], (uint8_t *)&thsensor_cfg.mid, 5);
			olen = 1 + 5;
//		} else if (cmd == CMD_ID_DNAME) {
//		} else if (cmd == CMD_ID_DEV_MAC) {
		} else if (cmd == CMD_ID_MTU) {
			if (ibuf[1] <= MTU_SIZE)
				ATT_UpdateMtuSize(gapRole_ConnectionHandle, ibuf[1]);
			else
				obuf[1] = 0xff;
			olen = 2;
		} else if (cmd == CMD_ID_REBOOT) {
			GAPRole_TerminateConnection();
			if(len >= 2) {
				write_reg(BOOT_MODE_SELECT_REG, ibuf[1]);
			}
			hal_system_soft_reset();
#if (DEV_SERVICES & SERVICE_SCREEN)
		} else if (cmd == CMD_ID_LCD_DUMP) { // Get/set lcd buf
			if (--len > sizeof(display_buff))
				len = sizeof(display_buff);
			if (len) {
				osal_memcpy(display_buff, &ibuf[1], len);
				update_lcd();
			}
			osal_memcpy(&obuf[1], display_buff, sizeof(display_buff));
			olen = 1 + sizeof(display_buff);
#endif
	//---------- Debug commands (unsupported in different versions!):

		} else if (cmd == CMD_ID_EEP_RW && len > 2) {
			obuf[1] = ibuf[1];
			obuf[2] = ibuf[2];
			uint16_t id = ibuf[1] | (ibuf[2] << 8);
			if(len > 3) {
				flash_write_cfg(&ibuf[3], id, len - 3);
			}
			int16_t i = flash_read_cfg(&obuf[3], id, SEND_DATA_SIZE);
			if(i < 0) {
				obuf[1] = (uint8_t)(i & 0xff); // Error
				olen = 2;
			} else
				olen = i + 3;
		} else if (cmd == CMD_ID_MEM_RW && len > 4) { // Read/Write memory
			uint8_t *p = (uint8_t *)
				((uint32_t)(ibuf[1] | (ibuf[2]<<8) | (ibuf[3]<<16) | (ibuf[4]<<24)));
			if(len > 5) {
				len -= 5;
				osal_memcpy(p, &ibuf[5], len);
			} else
				len = SEND_DATA_SIZE;
			osal_memcpy(obuf, ibuf, 5);
			osal_memcpy(&obuf[5], p, len);
			olen = len + 1 + 4;
		} else if (cmd == CMD_ID_REG_RW && len > 4) { // Read/Write register
			volatile uint32_t *p = (volatile uint32_t *)
					((uint32_t)(ibuf[1] | (ibuf[2]<<8) | (ibuf[3]<<16) | (ibuf[4]<<24)));
			uint32_t tmp;
			if(len > 8) {
				tmp = ibuf[5] | (ibuf[6]<<8) | (ibuf[7]<<16) | (ibuf[8]<<24);
				*p = tmp;
			} else {
				obuf[1] = 0xfe; // Error size
				olen = 2;
			}
			tmp = *p;
			osal_memcpy(obuf, ibuf, 5);
			osal_memcpy(&obuf[5], &tmp, 4);
			olen = 1 + 4 + 4;
		} else {
			obuf[1] = 0xff; // Error cmd
			olen = 2;
		}
	}
	return olen;
}
