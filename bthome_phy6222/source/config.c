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
#include "thb2_peripheral.h"
#include "gapbondmgr.h"
#include "pwrmgr.h"
#include "gpio.h"
#include "thb2_main.h"
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
#include "sensors.h"
#include "battery.h"
#include "sbp_profile.h"
#include "logger.h"
#include "trigger.h"

extern gapPeriConnectParams_t periConnParameters;

clock_time_t clkt;
cfg_t cfg;
work_parm_t wrk;
adv_work_t adv_wrk;

const cfg_t def_cfg = {
		.flg = FLG_MEAS_NOTIFY | FLG_SHOW_SMILEY,
		.rf_tx_power = RF_PHY_TX_POWER_0DBM,
		.advertising_interval = 80, // 80 * 62.5 = 5000 ms
		.measure_interval = 2,  // 5 * 2 = 10 sec
		.batt_interval = 60, // 60 sec
		.connect_latency = DEFAULT_DESIRED_SLAVE_LATENCY,	// 30*(29+1) = 900 ms
		.averaging_measurements = 180 // 180*10 = 1800 sec, 30 min
};

#if 1
extern uint32_t g_counter_traking_avg;

void restore_utc_time_sec(void) {
	if(clkt.utc_set_time_sec == 0) {
		clkt.utc_time_add = AP_AON->SLEEP_R[2] + 0x10000; // + 65.536 ms
		clkt.utc_time_sec = AP_AON->SLEEP_R[3];
	}
	clkt.utc_time_tik = clock_time_rtc();
}

uint32_t get_utc_time_sec(void) {
	uint32_t new_time_tik, delta;
#if TEST_RTC_DELTA
	do {
		new_time_tik = AP_AON->RTCCNT;
	} while(new_time_tik != AP_AON->RTCCNT);
#else
	new_time_tik = AP_AON->RTCCNT;
#endif
	if(new_time_tik <= clkt.utc_time_tik)
		delta = new_time_tik - clkt.utc_time_tik;
	else
		delta = 0xffffffff - clkt.utc_time_tik + new_time_tik;
	clkt.utc_time_tik = new_time_tik;
	delta &= 0x1fffff; // 64 sec max
	// delta in us
	delta = ((((delta & 0xffff0000) >> 16) * g_counter_traking_avg) << 8)
	                   + (((delta & 0xffff) * g_counter_traking_avg) >> 8);
	clkt.utc_time_add += delta;
	while(clkt.utc_time_add > 1000000) {
			clkt.utc_time_add -= 1000000;
			clkt.utc_time_sec++;
	}
	AP_AON->SLEEP_R[2] = clkt.utc_time_add; // сохранить для восстановления часов после перезагрузки
	AP_AON->SLEEP_R[3] = clkt.utc_time_sec; // сохранить для восстановления часов после перезагрузки
	return clkt.utc_time_sec;
}

#else

void restore_utc_time_sec(void) {
	if(clkt.utc_set_time_sec == 0) {
		clkt.utc_time_add = (AP_AON->SLEEP_R[2] &((1<<15) - 1)) + 10;
		clkt.utc_time_sec = AP_AON->SLEEP_R[3];
		//if(clkt.utc_time_sec < 1704067200ul) clkt.utc_time_sec = 1704067200ul;
	}
	clkt.utc_time_tik = clock_time_rtc();
}

uint32_t get_utc_time_sec(void) {
	uint32_t new_time_tik;
	HAL_ENTER_CRITICAL_SECTION();
#if TEST_RTC_DELTA
	do {
		new_time_tik = AP_AON->RTCCNT;
	} while(new_time_tik != AP_AON->RTCCNT);
#else
	new_time_tik = AP_AON->RTCCNT;
#endif
	if(new_time_tik <= clkt.utc_time_tik)
		clkt.utc_time_add += new_time_tik - clkt.utc_time_tik;
	else
		clkt.utc_time_add += 0xffffffff - clkt.utc_time_tik + new_time_tik;
	clkt.utc_time_tik = new_time_tik;
	clkt.utc_time_sec += (clkt.utc_time_add >> 15) * counter_tracking; // div 32768
	clkt.utc_time_add &= (1<<15) - 1;
	AP_AON->SLEEP_R[2] = clkt.utc_time_add; // сохранить для восстановления часов после перезагрузки
	AP_AON->SLEEP_R[3] = clkt.utc_time_sec; // сохранить для восстановления часов после перезагрузки
	HAL_EXIT_CRITICAL_SECTION();
#if (DEV_SERVICES & SERVICE_TIME_ADJUST)
	// TODO
#endif
	return clkt.utc_time_sec;
}
#endif

void test_config(void) {
	if (cfg.rf_tx_power > RF_PHY_TX_POWER_EXTRA_MAX)
		cfg.rf_tx_power = RF_PHY_TX_POWER_EXTRA_MAX;
	g_rfPhyTxPower = cfg.rf_tx_power;
	rf_phy_set_txPower(g_rfPhyTxPower);
#if  1 // FIX_CONN_INTERVAL
	gapRole_MinConnInterval = periConnParameters.intervalMin = DEFAULT_DESIRED_MIN_CONN_INTERVAL;
	gapRole_MaxConnInterval = periConnParameters.intervalMax = DEFAULT_DESIRED_MAX_CONN_INTERVAL;
	gapRole_SlaveLatency = periConnParameters.latency = cfg.connect_latency;

	gapRole_TimeoutMultiplier = (cfg.connect_latency + 1) * 3 * 4;
	if(gapRole_TimeoutMultiplier > 2048)
		gapRole_TimeoutMultiplier = 2048; // 20.48 sec мax
	periConnParameters.timeout = gapRole_TimeoutMultiplier;
#else
	gapRole_MinConnInterval = DEFAULT_DESIRED_MIN_CONN_INTERVAL;
	gapRole_MaxConnInterval = DEFAULT_DESIRED_MAX_CONN_INTERVAL;
	gapRole_SlaveLatency = cfg.connect_latency;
	gapRole_TimeoutMultiplier = (cfg.connect_latency + 1) * 3 * 4;
	if(gapRole_TimeoutMultiplier > 2048)
		gapRole_TimeoutMultiplier = 2048; // 20.48 sec мax
#endif
	if(cfg.advertising_interval == 0)
		cfg.advertising_interval = 1;
	else if(cfg.advertising_interval > 160)
		cfg.advertising_interval = 160;
	if(cfg.measure_interval < 2)
		cfg.measure_interval = 2;
	adv_wrk.measure_interval_ms = (cfg.advertising_interval * cfg.measure_interval * 625) / 10;
}

void load_eep_config(void) {
	if(!flash_supported_eep_ver(0, APP_VERSION)) {
		memcpy(&cfg, &def_cfg, sizeof(cfg));
#if (DEV_SERVICES & SERVICE_THS)
		memset(&thsensor_cfg.coef, 0, sizeof(thsensor_cfg.coef));
#endif
#if (OTA_TYPE == OTA_TYPE_APP) && ((DEV_SERVICES & SERVICE_TH_TRG) || (DEV_SERVICES & SERVICE_SCREEN))
		memcpy(&trg, &def_trg, sizeof(trg));
#endif
	} else {
		if (flash_read_cfg(&cfg, EEP_ID_CFG, sizeof(cfg)) != sizeof(cfg))
			memcpy(&cfg, &def_cfg, sizeof(cfg));
#if (DEV_SERVICES & SERVICE_THS)
		if(flash_read_cfg(&thsensor_cfg.coef, EEP_ID_CFS, sizeof(thsensor_cfg.coef)) != sizeof(thsensor_cfg.coef)) {
			memset(&thsensor_cfg.coef, 0, sizeof(thsensor_cfg.coef));
		}
#endif
#if (OTA_TYPE == OTA_TYPE_APP) && ((DEV_SERVICES & SERVICE_TH_TRG) || (DEV_SERVICES & SERVICE_SCREEN))
		if (flash_read_cfg(&trg, EEP_ID_TRG, trigger_send_size) != trigger_send_size) {
			memcpy(&trg, &def_trg, sizeof(trg));
		}
#endif
#if (DEV_SERVICES & SERVICE_TIME_ADJUST)
		if (flash_read_cfg(&clkt.delta_time, EEP_ID_TIM, sizeof(&clkt.delta_time)) != sizeof(&clkt.delta_time)) {
			clkt.delta_time = 0;
		}
#endif
	}
#if (DEV_SERVICES & SERVICE_HISTORY)
	memo_init();
#endif
	test_config();
}


