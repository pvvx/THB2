/*
	sensor.c
	Author: pvvx
*/

#include "config.h"

#if (DEV_SERVICES & SERVICE_THS)
#include "clock.h"
#include "OSAL.h"
#include "gpio.h"
#include "rom_sym_def.h"
#include "dev_i2c.h"
#include "trigger.h"
#include "sensors.h"

measured_data_t measured_data;

dev_i2c_t i2c_dev0 = {
	//.pi2cdev = AP_I2C0,
	.scl = I2C_SCL,
	.sda = I2C_SDA,
	.speed = I2C_100KHZ,
	.i2c_num = 0
};


thsensor_cfg_t thsensor_cfg;

const thsensor_def_cfg_t def_thcoef_cht8215 = {
		.coef.temp_k = 25606,
		.coef.humi_k = 20000,
		.coef.temp_z = 0,
		.coef.humi_z = 0,
		.sensor_type = TH_SENSOR_CHT8215
};

const thsensor_def_cfg_t def_thcoef_cht8305 = {
		.coef.temp_k = 16500,
		.coef.humi_k = 10000,
		.coef.temp_z = -4000,
		.coef.humi_z = 0,
		.sensor_type = TH_SENSOR_CHT8305
};

const thsensor_def_cfg_t def_thcoef_aht30 = {
		.coef.temp_k = 1250,
		.coef.humi_k = 625,
		.coef.temp_z = -5000,
		.coef.humi_z = 0,
		.sensor_type = TH_SENSOR_AHT2x
};


int read_sensor_cht8305(pdev_i2c_t pi2c_dev) {
	uint32_t _temp;
	uint8_t reg_data[4];
	if (!read_i2c_nabuf(pi2c_dev, thsensor_cfg.i2c_addr, reg_data, 4)) {
		_temp = (reg_data[0] << 8) | reg_data[1];
		measured_data.temp = ((uint32_t)(_temp * thsensor_cfg.coef.temp_k) >> 16)  + thsensor_cfg.coef.temp_z;; // x 0.01 C
		_temp = (reg_data[2] << 8) | reg_data[3];
		measured_data.humi = ((uint32_t)(_temp * thsensor_cfg.coef.humi_k) >> 16) + thsensor_cfg.coef.humi_z; // x 0.01 %
		return 0;
	}
	return 1;
}

int read_sensor_cht821x(pdev_i2c_t pi2c_dev) {
	uint8_t reg_data[4];
	uint32_t _r32;
	int16_t _r16;
	if (read_i2c_bytes(pi2c_dev, thsensor_cfg.i2c_addr, CHT83xx_REG_TMP, reg_data, 2) == 0
		&& read_i2c_bytes(pi2c_dev, thsensor_cfg.i2c_addr, CHT83xx_REG_HMD, &reg_data[2], 2) == 0) {
		_r16 = (reg_data[0] << 8) | reg_data[1];
		measured_data.temp = ((int32_t)(_r16 * thsensor_cfg.coef.temp_k) >> 16)  + thsensor_cfg.coef.temp_z;; // x 0.01 C
		_r32 = ((reg_data[2] << 8) | reg_data[3]); // & 0x7fff;
		measured_data.humi = ((uint32_t)(_r32 * thsensor_cfg.coef.humi_k) >> 16) + thsensor_cfg.coef.humi_z; // x 0.01 %
		return 0;
	}
	return 1;
}

int read_sensor_ahtxx(pdev_i2c_t pi2c_dev) {
	uint32_t _temp;
	uint8_t reg_data[8];
	if(read_i2c_nabuf(pi2c_dev, thsensor_cfg.i2c_addr, reg_data, 7) == 0
		&& (reg_data[0] & 0x80) == 0) { // busy
		_temp = ((reg_data[3] & 0x0F) << 16) | (reg_data[4] << 8) | reg_data[5];
		measured_data.temp = ((uint32_t)(_temp * thsensor_cfg.coef.temp_k) >> 16) + thsensor_cfg.coef.temp_z; // x 0.01 C
		_temp = (reg_data[1] << 12) | (reg_data[2] << 4) | (reg_data[3] >> 4);
		measured_data.humi = ((uint32_t)(_temp * thsensor_cfg.coef.humi_k) >> 16) + thsensor_cfg.coef.humi_z; // x 0.01 %
		return 0;
	}
	//send_i2c_wreg(&i2c_dev0, thsensor_cfg.i2c_addr, AHT2x_CMD_INI, AHT2x_DATA_LPWR);
	return 1;
}


int read_sensors(void) {
	int ret = 1;
	if(thsensor_cfg.i2c_addr && thsensor_cfg.read_sensor != NULL) {
		init_i2c(&i2c_dev0);
		ret = thsensor_cfg.read_sensor(&i2c_dev0);
		deinit_i2c(&i2c_dev0);
		if(!ret) {
			if (measured_data.humi < 0)
				measured_data.humi = 0;
			else if (measured_data.humi > 9999)
				measured_data.humi = 9999;
			measured_data.count++;
		}
	}
#if (OTA_TYPE == OTA_TYPE_APP) && ((DEV_SERVICES & SERVICE_TH_TRG) || (DEV_SERVICES & SERVICE_SCREEN))
	set_trigger_out();
#endif
	if(ret)
		init_sensor();
	return ret;
}


void start_measure(void) {
	if(thsensor_cfg.i2c_addr) {
		if(thsensor_cfg.i2c_addr == AHT2x_I2C_ADDR) {
			init_i2c(&i2c_dev0);
			send_i2c_wreg(&i2c_dev0, thsensor_cfg.i2c_addr, AHT2x_CMD_TMS, AHT2x_DATA_TMS);
			deinit_i2c(&i2c_dev0);
		}
		else if(thsensor_cfg.vid == CHT8305_VID) {
			init_i2c(&i2c_dev0);
			send_i2c_byte(&i2c_dev0, thsensor_cfg.i2c_addr, CHT83xx_REG_TMP); // start measure T/H
			deinit_i2c(&i2c_dev0);
		}
	}
}

__ATTR_SECTION_XIP__
void init_sensor(void) {
	thsensor_def_cfg_t *ptabinit = NULL;
	thsensor_cfg.read_sensor = NULL;
	thsensor_cfg.sensor_type = TH_SENSOR_NONE;
	init_i2c(&i2c_dev0);
	//send_i2c_byte(0,6);
	thsensor_cfg.i2c_addr = CHT83xx_I2C_ADDR;
	if(!read_i2c_bytes(&i2c_dev0, thsensor_cfg.i2c_addr, CHT83xx_REG_MID, (uint8 *)&thsensor_cfg.mid, 2) // 0x5959
	&& !read_i2c_bytes(&i2c_dev0, thsensor_cfg.i2c_addr, CHT83xx_REG_VID, (uint8 *)&thsensor_cfg.vid, 2)) { // 0x8215
		if(thsensor_cfg.mid == CHT83xx_MID) {
			if(thsensor_cfg.vid == CHT8305_VID) {
#if 0 // USE_DEFAULT_SETS_SENSOR
				// Soft reset command
				send_i2c_wreg(thsensor_cfg.i2c_addr, CHT8305_REG_CFG,
				CHT8305_CFG_SOFT_RESET | CHT8305_CFG_MODE);
				WaitMs(SENSOR_RESET_TIMEOUT_ms);
				// Configure
				send_i2c_wreg(&i2c_dev0, thsensor_cfg.i2c_addr, CHT8305_REG_CFG, CHT8305_CFG_MODE );
#endif
				ptabinit = (thsensor_def_cfg_t *)&def_thcoef_cht8305;
				thsensor_cfg.read_sensor = read_sensor_cht8305;
			} else if(thsensor_cfg.vid == CHT8215_VID) { // 0x8210/0x8215 ?
				if(adv_wrk.measure_interval_ms >= 5000) // > 5 sec
					send_i2c_wreg(&i2c_dev0, thsensor_cfg.i2c_addr, CHT8215_REG_CRT, 0x0300); // Set conversion ratio 5 sec
				// else 1 sec
				ptabinit = (thsensor_def_cfg_t *)&def_thcoef_cht8215;
				thsensor_cfg.read_sensor = read_sensor_cht821x;
			}
		} else
			thsensor_cfg.i2c_addr = 0;
	} else {
		thsensor_cfg.i2c_addr = AHT2x_I2C_ADDR;
		if(!send_i2c_wreg(&i2c_dev0, thsensor_cfg.i2c_addr, AHT2x_CMD_TMS, AHT2x_DATA_TMS)) {
			ptabinit = (thsensor_def_cfg_t *)&def_thcoef_aht30;
			thsensor_cfg.read_sensor = read_sensor_ahtxx;
			thsensor_cfg.vid = 0xAAAA;
		} else
			thsensor_cfg.i2c_addr = 0;
	}
	if(thsensor_cfg.coef.temp_k == 0 && ptabinit) {
		memcpy(&thsensor_cfg.coef, ptabinit, sizeof(thsensor_cfg.coef));
		thsensor_cfg.sensor_type = ptabinit->sensor_type;
	}
	deinit_i2c(&i2c_dev0);
}

#endif // (DEV_SERVICES & SERVICE_THS)
