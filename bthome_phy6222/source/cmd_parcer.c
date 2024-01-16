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
#include "flash_eep.h"
#include "bleperipheral.h"
#include "sbp_profile.h"
#include "sensor.h"
#include "cmd_parcer.h"

/*********************************************************************/
#define SEND_DATA_SIZE	18

int cmd_parser(uint8_t * obuf, uint8_t * ibuf, uint32_t len) {
	int olen = 0;
	if (len) {
		uint8_t cmd = ibuf[0];
		obuf[0] = cmd;
		obuf[1] = 0; // no err
		if (cmd == CMD_ID_DEVID) { // Get DEV_ID
			pdev_id_t p = (pdev_id_t) obuf;
			// p->pid = CMD_ID_DEV_ID;
			// p->revision = 0; // уже = 0
			p->hw_version = DEVICE;
			p->sw_version = APP_VERSION;
			p->dev_spec_data = 0;
			p->services = 0;
			olen = sizeof(dev_id_t);
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
			osal_memcpy(&obuf[1], &thsensor_cfg.coef, sizeof(thsensor_cfg.coef));
			olen = sizeof(thsensor_cfg.coef) + 1;
		} else if (cmd == CMD_ID_CFS_DEF) {	// Get/Set default sensor config
			osal_memcpy(&thsensor_cfg.coef, &def_thcoef, sizeof(thsensor_cfg.coef));
			flash_write_cfg(&thsensor_cfg.coef, EEP_ID_CFS, sizeof(thsensor_cfg.coef));
			osal_memcpy(&obuf[1], &thsensor_cfg.coef, sizeof(thsensor_cfg.coef));
			olen = sizeof(thsensor_cfg.coef) + 1;

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
			uint8_t *p = (uint8_t *)(ibuf[1] | (ibuf[2]<<8) | (ibuf[3]<<16) | (ibuf[4]<<24));
			if(len > 5) {
				len -= 5;
				osal_memcpy(p, &ibuf[5], len);
			} else
				len = SEND_DATA_SIZE;
			osal_memcpy(&obuf, &ibuf, 5);
			osal_memcpy(&obuf[5], p, len);
			olen = len + 1 + 4;
		} else if (cmd == CMD_ID_REG_RW && len > 4) { // Read/Write register
			volatile uint32_t *p = (volatile uint32_t *)(ibuf[1] | (ibuf[2]<<8) | (ibuf[3]<<16) | (ibuf[4]<<24));
			uint32_t tmp;
			if(len > 8) {
				tmp = ibuf[5] | (ibuf[6]<<8) | (ibuf[7]<<16) | (ibuf[8]<<24);
				*p = tmp;
			} else {
				obuf[1] = 0xfe; // Error size
				olen = 2;
			}
			tmp = *p;
			osal_memcpy(&obuf, &ibuf, 5);
			osal_memcpy(&obuf[5], &tmp, 4);
			olen = 1 + 4 + 4;
		} else {
			obuf[1] = 0xff; // Error cmd
			olen = 2;
		}
	}
	return olen;
}
