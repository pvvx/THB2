/*
 * trigger.c
 *
 *  Created on: 5 февр. 2024 г.
 *      Author: pvvx
 */
#include "config.h"
#if (OTA_TYPE == OTA_TYPE_APP) && (DEV_SERVICES & SERVICE_THS)
#include "flash.h"
#include "flash_eep.h"
#include "logger.h"
#include "sensors.h"
#include "cmd_parser.h"
#include "trigger.h"

trigger_t trg;

const trigger_t def_trg = {
#if (DEV_SERVICES & SERVICE_SCREEN)
	.temp_min = 2000,
	.temp_max = 2500,
	.humi_min = 3000,
	.humi_max = 6000,
#endif
#if (DEV_SERVICES & SERVICE_TH_TRG)
	.temp_threshold = 2250,
	.humi_threshold = 5000
#endif
};

void set_trigger_out(void) {
#if (DEV_SERVICES & SERVICE_SCREEN)
	if (measured_data.temp >= trg.temp_min && measured_data.temp <= trg.temp_max
			&& measured_data.humi >= trg.humi_min && measured_data.humi <= trg.humi_max)
		measured_data.flg.comfort = true;
	else
		measured_data.flg.comfort = false;
#endif
#if (DEV_SERVICES & SERVICE_TH_TRG)
	if (trg.temp_hysteresis) {
		if (measured_data.flg.temp_trg_on) { // temp_out on
			if (trg.temp_hysteresis < 0) {
				if (measured_data.temp > trg.temp_threshold - trg.temp_hysteresis) {
					measured_data.flg.temp_trg_on = false;
				}
			} else {
				if (measured_data.temp < trg.temp_threshold - trg.temp_hysteresis) {
					measured_data.flg.temp_trg_on = false;
				}
			}
		} else { // temp_out off
			if (trg.temp_hysteresis < 0) {
				if (measured_data.temp < trg.temp_threshold + trg.temp_hysteresis) {
					measured_data.flg.temp_trg_on = true;
				}
			} else {
				if (measured_data.temp > trg.temp_threshold + trg.temp_hysteresis) {
					measured_data.flg.temp_trg_on = true;
				}
			}
		}
	} else measured_data.flg.temp_trg_on = false;

	if (trg.humi_hysteresis) {
		if (measured_data.flg.humi_trg_on) { // humi_out on
			if (trg.humi_hysteresis < 0) {
				if (measured_data.humi > trg.humi_threshold - trg.humi_hysteresis) {
					// humi > threshold
					measured_data.flg.humi_trg_on = false;
				}
			} else { // hysteresis > 0
				if (measured_data.humi < trg.humi_threshold - trg.humi_hysteresis) {
					// humi < threshold
					measured_data.flg.humi_trg_on = false;
				}
			}
		} else { // humi_out off
			if (trg.humi_hysteresis < 0) {
				if (measured_data.humi < trg.humi_threshold + trg.humi_hysteresis) {
					// humi < threshold
					measured_data.flg.humi_trg_on = true;
				}
			} else { // hysteresis > 0
				if (measured_data.humi > trg.humi_threshold + trg.humi_hysteresis) {
					// humi > threshold
					measured_data.flg.humi_trg_on = true;
				}
			}
		}
	} else measured_data.flg.humi_trg_on = false;

	measured_data.flg.trg_on = measured_data.flg.temp_trg_on || measured_data.flg.humi_trg_on;
	measured_data.flg.trg_output = (trg.cfg & TRG_CFG_OUT_INV)? !measured_data.flg.trg_on : measured_data.flg.trg_on;
#ifdef GPIO_TRG
	hal_gpio_fast_write(GPIO_TRG, measured_data.flg.trg_output);
#endif
#endif
#ifdef GPIO_INP
	measured_data.flg.pin_input = hal_gpio_read(GPIO_INP);
#endif
}


#endif // SERVICE_THS
