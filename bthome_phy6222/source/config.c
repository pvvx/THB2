/*
 * config.c
 *
 *  Created on: 16 янв. 2024 г.
 *      Author: pvvx
 */
/*********************************************************************
 * INCLUDES
 */
#include "bcomdef.h"
#include "config.h"
#include "rf_phy_driver.h"
#include "global_config.h"
#include "OSAL.h"
#include "OSAL_PwrMgr.h"
#include "gatt.h"
#include "hci.h"
#include "gapgattserver.h"
#include "gattservapp.h"
#include "devinfoservice.h"
#include "ota_app_service.h"
#include "thb2_peripheral.h"
#include "gapbondmgr.h"
#include "pwrmgr.h"
#include "gpio.h"
#include "bleperipheral.h"
#include "ll.h"
#include "ll_hw_drv.h"
#include "ll_def.h"
#include "hci_tl.h"
#include "flash.h"
//#include "fs.h"
#include "flash_eep.h"
#include "battservice.h"
#include "thservice.h"
#include "thb2_peripheral.h"
#include "bthome_beacon.h"
#include "sensor.h"
#include "battery.h"
#include "sbp_profile.h"
#include "logger.h"

extern gapPeriConnectParams_t periConnParameters;

clock_time_t clkt;

cfg_t cfg;

const cfg_t def_cfg = {
		.rf_tx_power = RF_PHY_TX_POWER_0DBM,
		.advertising_interval = 80, // 80 * 62.5 = 5000 ms
		.measure_interval = 2,  // 5 * 2 = 10 sec
		.batt_interval = 60, // 60 sec
		.connect_latency = 29,	// 30*30 = 900 ms
		.averaging_measurements = 2 // 180*10 = 1800 sec, 30 min
};

/*
uint32_t get_delta_time_rtc(uint32_t start_time_rtc) {
	uint32_t new_time_rtc = clock_time_rtc();
	if(new_time_rtc  < start_time_rtc)
		new_time_rtc += 0x1000000; // + 512 sec
	return  new_time_rtc - start_time_rtc;
}
*/

uint32_t get_utc_time_sec(void) {
	uint32_t new_time_tik = clock_time_rtc();
	if(new_time_tik  < clkt.utc_time_tik)
		new_time_tik += 0x1000000; // + 512 sec
	clkt.utc_time_add += new_time_tik - clkt.utc_time_tik;
	clkt.utc_time_tik = new_time_tik;
	clkt.utc_time_sec += clkt.utc_time_add >> 15; // div 32768
	clkt.utc_time_add &= 32767;
#if (DEV_SERVICES & SERVICE_TIME_ADJUST)
	// TODO
#endif
	return clkt.utc_time_sec;
}

void test_config(void) {
	if (cfg.rf_tx_power > RF_PHY_TX_POWER_EXTRA_MAX)
		cfg.rf_tx_power = RF_PHY_TX_POWER_EXTRA_MAX;
	g_rfPhyTxPower = cfg.rf_tx_power;

	gapRole_MinConnInterval = periConnParameters.intervalMin = DEFAULT_DESIRED_MIN_CONN_INTERVAL;
	gapRole_MaxConnInterval = periConnParameters.intervalMax = DEFAULT_DESIRED_MAX_CONN_INTERVAL;
	gapRole_SlaveLatency = periConnParameters.latency = cfg.connect_latency;

	periConnParameters.timeout = (cfg.connect_latency + 1) * 3 * 4;
	if(periConnParameters.timeout > 2048)
		periConnParameters.timeout = 2048; // 20.48 sec мax
	gapRole_TimeoutMultiplier = periConnParameters.timeout;

	if(cfg.advertising_interval == 0)
		cfg.advertising_interval = 1;
	if(cfg.measure_interval < 2)
		cfg.measure_interval = 2;
	adv_wrk.measure_interval_ms = cfg.advertising_interval * cfg.measure_interval * 625 / 10;
}

void load_eep_config(void) {
	if(!flash_supported_eep_ver(0, APP_VERSION)) {
		osal_memcpy(&cfg, &def_cfg, sizeof(cfg));
	} else {
		if (flash_read_cfg(&cfg, EEP_ID_CFG, sizeof(cfg)) != sizeof(cfg))
			osal_memcpy(&cfg, &def_cfg, sizeof(cfg));
#if (DEV_SERVICES & SERVICE_THS)
		if(flash_read_cfg(&thsensor_cfg.coef, EEP_ID_CFS, sizeof(thsensor_cfg.coef)) != sizeof(thsensor_cfg.coef)) {
			osal_memset(&thsensor_cfg.coef, 0, sizeof(thsensor_cfg.coef));
		}
#endif
	}
#if (DEV_SERVICES & SERVICE_TIME_ADJUST)
	if (flash_read_cfg(&clkt.delta_time, EEP_ID_TIM, sizeof(&clkt.delta_time)) != sizeof(&clkt.delta_time)) {
		clkt.delta_time = 0;
	}
#endif
#if (DEV_SERVICES & SERVICE_HISTORY)
	memo_init();
#endif
	test_config();
}


