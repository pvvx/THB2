/*
	sensor.c
	Author: pvvx
*/

#include "config.h"
#include "sensor.h"

measured_data_t measured_data;

#if (DEV_SERVICES & SERVICE_THS)
#include "clock.h"
#include "OSAL.h"
#include "gpio.h"
#include "rom_sym_def.h"
#include "dev_i2c.h"

#define I2C_SPEED 1

thsensor_cfg_t thsensor_cfg;

const thsensor_coef_t def_thcoef_cht8215 = {
		.temp_k = 25606,
		.humi_k = 20000,
		.temp_z = 0,
		.humi_z = 0
};

const thsensor_coef_t def_thcoef_cht8305 = {
		.temp_k = 16500,
		.humi_k = 10000,
		.temp_z = -4000,
		.humi_z = 0
};

const thsensor_coef_t def_thcoef_aht30 = {
		.temp_k = 1250,
		.humi_k = 625,
		.temp_z = -5000,
		.humi_z = 0
};


int read_sensor_cht8xxx(void) {
	uint8_t reg_data[4];
	int32_t _r32;
	int16_t _r16;
	init_i2c(I2C_SPEED);
	if(thsensor_cfg.vid == CHT8305_VID) {
		_r32 = read_i2c_nabuf(thsensor_cfg.i2c_addr, reg_data, 4);
	} else  {
		_r32 = read_i2c_bytes(thsensor_cfg.i2c_addr, CHT83xx_REG_TMP, reg_data, 2);
		_r32 |= read_i2c_bytes(thsensor_cfg.i2c_addr, CHT83xx_REG_HMD, &reg_data[2], 2);
	}
	deinit_i2c();
	if (!_r32) {
		_r16 = (reg_data[0] << 8) | reg_data[1];
		measured_data.temp = ((int32)(_r16 * thsensor_cfg.coef.temp_k) >> 16)  + thsensor_cfg.coef.temp_z;; // x 0.01 C
		_r32 = ((reg_data[2] << 8) | reg_data[3]); // & 0x7fff ;
		measured_data.humi = ((uint32)(_r32 * thsensor_cfg.coef.humi_k) >> 16) + thsensor_cfg.coef.humi_z; // x 0.01 %
		if (measured_data.humi < 0)
			measured_data.humi = 0;
		else if (measured_data.humi > 9999)
			measured_data.humi = 9999;
		measured_data.count++;
		return 0;
	}
	return 1;
}

int read_sensor_ahtxx(void) {
	uint32_t _temp;
	uint8_t reg_data[8];
	init_i2c(I2C_SPEED);
	if(!read_i2c_nabuf(thsensor_cfg.i2c_addr, reg_data, 7)
			&& (reg_data[0] & 0x80) == 0) { // busy
		deinit_i2c();
		_temp = ((reg_data[3] & 0x0F) << 16) | (reg_data[4] << 8) | reg_data[5];
		measured_data.temp = ((uint32_t)(_temp * thsensor_cfg.coef.temp_k) >> 16) + thsensor_cfg.coef.temp_z; // x 0.01 C
		_temp = (reg_data[1] << 12) | (reg_data[2] << 4) | (reg_data[3] >> 4);
		measured_data.humi = ((uint32_t)(_temp * thsensor_cfg.coef.humi_k) >> 16) + thsensor_cfg.coef.humi_z; // x 0.01 %
		if (measured_data.humi < 0)
			measured_data.humi = 0;
		else if (measured_data.humi > 9999)
			measured_data.humi = 9999;
		return 0;
	}
	deinit_i2c();
	return 1;
}


int read_sensor(void) {
	int ret = 1;
	if(thsensor_cfg.i2c_addr && thsensor_cfg.read_sensor != NULL)
		ret = thsensor_cfg.read_sensor();
	if(ret)
		init_sensor();
	return ret;
}


void start_measure(void) {
	if(thsensor_cfg.i2c_addr) {
		if(thsensor_cfg.i2c_addr == AHT2x_I2C_ADDR) {
			init_i2c(I2C_SPEED);
			send_i2c_wreg(thsensor_cfg.i2c_addr, AHT2x_CMD_TMS, AHT2x_DATA_TMS);
			deinit_i2c();
		}
		else if(thsensor_cfg.vid == CHT8305_VID) {
			init_i2c(I2C_SPEED);
			send_i2c_byte(thsensor_cfg.i2c_addr, CHT83xx_REG_TMP); // start measure T/H
			deinit_i2c();
		}
	}
}

__ATTR_SECTION_XIP__ void init_sensor(void) {
	uint8_t *ptabinit = NULL;
	thsensor_cfg.read_sensor = NULL;
	init_i2c(I2C_SPEED);
	//send_i2c_byte(0,6);
	thsensor_cfg.i2c_addr = CHT83xx_I2C_ADDR;
	if(!read_i2c_bytes(thsensor_cfg.i2c_addr, CHT83xx_REG_MID, (uint8 *)&thsensor_cfg.mid, 2) // 0x5959
	&& !read_i2c_bytes(thsensor_cfg.i2c_addr, CHT83xx_REG_VID, (uint8 *)&thsensor_cfg.vid, 2)) { // 0x8215
		if(thsensor_cfg.mid == CHT83xx_MID) {
			if(thsensor_cfg.vid == CHT8305_VID) {
#if 0 // USE_DEFAULT_SETS_SENSOR
				// Soft reset command
				send_i2c_wreg(thsensor_cfg.i2c_addr, CHT8305_REG_CFG,
				CHT8305_CFG_SOFT_RESET | CHT8305_CFG_MODE);
				WaitMs(SENSOR_RESET_TIMEOUT_ms);
				// Configure
				send_i2c_wreg(thsensor_cfg.i2c_addr, CHT8305_REG_CFG, CHT8305_CFG_MODE );
#endif
				ptabinit = (uint8_t *)&def_thcoef_cht8305;
				thsensor_cfg.read_sensor = read_sensor_cht8xxx;
			} else if(thsensor_cfg.vid == CHT8215_VID) { // 0x8210/0x8215 ?
				if(adv_wrk.measure_interval_ms >= 5000) // > 5 sec
					send_i2c_wreg(thsensor_cfg.i2c_addr, CHT8215_REG_CRT, 0x0300); // Set conversion ratio 5 sec
				// else 1 sec
				ptabinit = (uint8_t *)&def_thcoef_cht8215;
				thsensor_cfg.read_sensor = read_sensor_cht8xxx;
			}
		} else
			thsensor_cfg.i2c_addr = 0;
	} else {
		thsensor_cfg.i2c_addr = AHT2x_I2C_ADDR;
		if(!send_i2c_wreg(thsensor_cfg.i2c_addr, AHT2x_CMD_TMS, AHT2x_DATA_TMS)) {
			ptabinit = (uint8_t *)&def_thcoef_aht30;
			thsensor_cfg.read_sensor = read_sensor_ahtxx;
			thsensor_cfg.vid = 0xAAAA;
		} else
			thsensor_cfg.i2c_addr = 0;
	}
	if(thsensor_cfg.coef.temp_k == 0 && ptabinit) {
		osal_memcpy(&thsensor_cfg.coef, ptabinit, sizeof(thsensor_cfg.coef));
	}
	deinit_i2c();
}

#endif // (DEV_SERVICES & SERVICE_THS)
