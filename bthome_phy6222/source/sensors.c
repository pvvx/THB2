/*
	sensor.c
	Author: pvvx
*/

#include "types.h"
#include "config.h"
#include "gpio.h"
#include "rom_sym_def.h"
#include "i2c.h"
#include "sensor.h"

#define SET_I2C_SPEED	400 // 100 or 400 kHz
#define I2C_WAIT_ms		1

measured_data_t measured_data;
thsensor_cfg_t thsensor_cfg;

void init_i2c(void) {
	hal_gpio_fmux_set(I2C_SCL, FMUX_IIC0_SCL);
	hal_gpio_fmux_set(I2C_SDA, FMUX_IIC0_SDA);

	//hal_i2c_init(I2C_0, I2C_CLOCK_400K):

	int pclk = clk_get_pclk();

	AP_I2C_TypeDef* pi2cdev = AP_I2C0;
	hal_clk_gate_enable(MOD_I2C0);
	pi2cdev->IC_ENABLE = 0;
	pi2cdev->IC_CON = 0x61;
#if SET_I2C_SPEED == 100
	pi2cdev->IC_CON = ((pi2cdev->IC_CON) & 0xfffffff9) | (0x01 << 1); // SPEED_STANDARD
	if (pclk == 16000000) {
		pi2cdev->IC_SS_SCL_HCNT = 70;  //16
		pi2cdev->IC_SS_SCL_LCNT = 76;  //32)
	} else if (pclk == 32000000) {
		pi2cdev->IC_SS_SCL_HCNT = 148;  //16
		pi2cdev->IC_SS_SCL_LCNT = 154;  //32)
	} else if (pclk == 48000000) {
		pi2cdev->IC_SS_SCL_HCNT = 230;  //16
		pi2cdev->IC_SS_SCL_LCNT = 236;  //32)
	} else if (pclk == 64000000) {
		pi2cdev->IC_SS_SCL_HCNT = 307;  //16
		pi2cdev->IC_SS_SCL_LCNT = 320;  //32)
	} else if (pclk == 96000000) {
		pi2cdev->IC_SS_SCL_HCNT = 460;  //16
		pi2cdev->IC_SS_SCL_LCNT = 470;  //32)
	}
#elif SET_I2C_SPEED == 400
	pi2cdev->IC_CON = ((pi2cdev->IC_CON) & 0xfffffff9) | (0x02 << 1); // SPEED_FAST
	if (pclk == 16000000) {
		pi2cdev->IC_FS_SCL_HCNT = 10;
		pi2cdev->IC_FS_SCL_LCNT = 17;
	} else if (pclk == 32000000) {
		pi2cdev->IC_FS_SCL_HCNT = 30;
		pi2cdev->IC_FS_SCL_LCNT = 35;
	} else if (pclk == 48000000) {
		pi2cdev->IC_FS_SCL_HCNT = 48;
		pi2cdev->IC_FS_SCL_LCNT = 54;
	} else if (pclk == 64000000) {
		pi2cdev->IC_FS_SCL_HCNT = 67;
		pi2cdev->IC_FS_SCL_LCNT = 75;
	} else if (pclk == 96000000) {
		pi2cdev->IC_FS_SCL_HCNT = 105;
		pi2cdev->IC_FS_SCL_LCNT = 113;
	}
#else
#error SET_I2C_SPEED = ?
#endif
//	pi2cdev->IC_TAR = I2C_MASTER_ADDR_DEF;
	pi2cdev->IC_INTR_MASK = 0;
	pi2cdev->IC_RX_TL = 0x0;
	pi2cdev->IC_TX_TL = 0x1;
//	pi2cdev->IC_ENABLE = 1;
}

void deinit_i2c(void) {
	AP_I2C_TypeDef * pi2cdev = AP_I2C0;
	pi2cdev->IC_ENABLE = 0;
	hal_clk_gate_disable(MOD_I2C0);
	hal_gpio_pin_init(I2C_SCL, IE);
	hal_gpio_pin_init(I2C_SDA, IE);
}

extern volatile uint32 osal_sys_tick;

/* size max = 7 ! */
int read_i2c_bytes(uint8 addr, uint8 reg, uint8 * data, uint8 size) {
	int i = size;
	AP_I2C_TypeDef * pi2cdev = AP_I2C0;
	pi2cdev->IC_ENABLE = 0;
	pi2cdev->IC_TAR = addr;

	HAL_ENTER_CRITICAL_SECTION();

	pi2cdev->IC_ENABLE = 1;
	pi2cdev->IC_DATA_CMD = reg;
	//while(!(pi2cdev->IC_RAW_INTR_STAT & 0x10));
	while(i--)
		pi2cdev->IC_DATA_CMD = 0x100;

	HAL_EXIT_CRITICAL_SECTION();

	uint32 to = osal_sys_tick;
	i = size;
	while(i) {
		if(pi2cdev->IC_STATUS & 0x08) { // fifo not empty
			*data = pi2cdev->IC_DATA_CMD & 0xff;
			data++;
			i--;
		}
		if(osal_sys_tick - to > I2C_WAIT_ms)
			return 1;
	}
	return 0;
}

int read_noreg_i2c_bytes(uint8 addr, uint8 * data, uint8 size) {
	int i = size;
	AP_I2C_TypeDef * pi2cdev = AP_I2C0;
	pi2cdev->IC_ENABLE = 0;
	pi2cdev->IC_TAR = addr;

	HAL_ENTER_CRITICAL_SECTION();
	pi2cdev->IC_ENABLE = 1;
	while(i--)
		pi2cdev->IC_DATA_CMD = 0x100;

	HAL_EXIT_CRITICAL_SECTION();

	uint32 to = osal_sys_tick;
	i = size;
	while(i) {
		if(pi2cdev->IC_STATUS & 0x08) { // fifo not empty
			*data = pi2cdev->IC_DATA_CMD & 0xff;
			data++;
			i--;
		}
		if(osal_sys_tick - to > I2C_WAIT_ms)
			return 1;
	}
	return 0;
}

int send_i2c_byte(uint8 addr, uint8 data) {
	AP_I2C_TypeDef * pi2cdev = AP_I2C0;
	pi2cdev->IC_ENABLE = 0;
	pi2cdev->IC_TAR = addr;
	HAL_ENTER_CRITICAL_SECTION();
	pi2cdev->IC_ENABLE = 1;
	pi2cdev->IC_DATA_CMD = data;
	// while(!(pi2cdev->IC_RAW_INTR_STAT & 0x10));
	HAL_EXIT_CRITICAL_SECTION();
	uint32 to = osal_sys_tick;
	while(1) {
		if(pi2cdev->IC_RAW_INTR_STAT & 0x200)// check tx empty
			break;
		if(osal_sys_tick - to > I2C_WAIT_ms)
			return 1;
	}
	return 0;
}

int send_i2c_wreg(uint8 addr, uint8 reg, uint16 data) {
	AP_I2C_TypeDef * pi2cdev = AP_I2C0;
	pi2cdev->IC_ENABLE = 0;
	pi2cdev->IC_TAR = addr;
	HAL_ENTER_CRITICAL_SECTION();
	pi2cdev->IC_ENABLE = 1;
	  pi2cdev->IC_DATA_CMD = reg;
		while(!(pi2cdev->IC_RAW_INTR_STAT & 0x10));
	  pi2cdev->IC_DATA_CMD = (data >> 8) & 0xff;
		while(!(pi2cdev->IC_RAW_INTR_STAT & 0x10));
	  pi2cdev->IC_DATA_CMD = data & 0xff;
		HAL_EXIT_CRITICAL_SECTION();
		uint32 to = osal_sys_tick;
		while(1) {
			if(pi2cdev->IC_RAW_INTR_STAT & 0x200)// check tx empty
				break;
			if(osal_sys_tick - to > 2)
				return 1;
	}
	return 0;
}

__ATTR_SECTION_XIP__ void init_sensor(void) {
	init_i2c();
#if DEVICE == DEVICE_THB2

	send_i2c_byte(0, 0x06); // Reset command using the general call address
	WaitMs(SENSOR_RESET_TIMEOUT_ms);
	thsensor_cfg.i2c_addr = CHT8310_I2C_ADDR0;
	read_i2c_bytes(thsensor_cfg.i2c_addr, CHT8310_REG_MID, (uint8 *)&thsensor_cfg._id[0], 2); // 0x5959
	read_i2c_bytes(thsensor_cfg.i2c_addr, CHT8310_REG_VID, (uint8 *)&thsensor_cfg._id[1], 2);
	//WaitMs(1);
	send_i2c_wreg(CHT8310_I2C_ADDR0, CHT8310_REG_CRT, 0x0300); // Set conversion ratio 5 sec

#elif DEVICE == DEVICE_BTH01
#define USE_DEFAULT_SETS_SENSOR	0 // for CHT8305
#if USE_DEFAULT_SETS_SENSOR
	hal_gpio_write(GPIO_SPWR, 0);
	WaitMs(SENSOR_POWER_TIMEOUT_ms);
	hal_gpio_write(GPIO_SPWR, 1);
#endif
	thsensor_cfg.i2c_addr = CHT8305_I2C_ADDR0;
	if(!read_i2c_bytes(thsensor_cfg.i2c_addr, CHT8310_REG_MID, (uint8 *)&thsensor_cfg._id[0], 2) // 0x5959
		&& !read_i2c_bytes(thsensor_cfg.i2c_addr, CHT8310_REG_VID, (uint8 *)&thsensor_cfg._id[1], 2)) { // 0x8305
#if !USE_DEFAULT_SETS_SENSOR
		// Soft reset command
		send_i2c_wreg(thsensor_cfg.i2c_addr, CHT8305_REG_CFG,
				CHT8305_CFG_SOFT_RESET
				| CHT8305_CFG_MODE
				);
		WaitMs(SENSOR_RESET_TIMEOUT_ms);
		// Configure
		send_i2c_wreg(thsensor_cfg.i2c_addr, CHT8305_REG_CFG,
				CHT8305_CFG_MODE
				);
#endif
		//		WaitMs(SENSOR_MEASURING_TIMEOUT_ms);
		send_i2c_byte(thsensor_cfg.i2c_addr, CHT8305_REG_TMP); // start measure T/H
	}
#endif // DEVICE == DEVICE_BTH01
	deinit_i2c();
}


int read_sensor(void) {
	uint8 reg_data[4];
	init_i2c();
#if DEVICE == DEVICE_THB2
	int32 _r32;
	int16 _r16;
	_r32 = read_i2c_bytes(thsensor_cfg.i2c_addr, CHT8310_REG_TMP, reg_data, 2);
	//? WaitUs(100);
	_r32 |= read_i2c_bytes(thsensor_cfg.i2c_addr, CHT8310_REG_HMD, &reg_data[2], 2);
	deinit_i2c();
	if (!_r32) {
		_r16 = (reg_data[0] << 8) | reg_data[1];
		measured_data.temp = ((int32)(_r16 * 25606 + 0x7fff) >> 16)  + thsensor_cfg.temp_offset;; // x 0.01 C
		_r32 = ((reg_data[2] << 8) | reg_data[3]) & 0x7fff;
		measured_data.humi = ((uint32)(_r32 * 20000 + 0x7fff) >> 16) +  + thsensor_cfg.humi_offset; // x 0.01 %
		if (measured_data.humi < 0)
			measured_data.humi = 0;
		else if (measured_data.humi > 9999)
			measured_data.humi = 9999;
		measured_data.count++;
		return 0;
	}
#elif DEVICE == DEVICE_BTH01
	uint16_t _temp;
	if (!read_noreg_i2c_bytes(thsensor_cfg.i2c_addr, reg_data, 4)) {
		_temp = (reg_data[0] << 8) | reg_data[1];
		measured_data.temp = ((uint32_t)(_temp * 16500) >> 16) - 4000 + thsensor_cfg.temp_offset; // x 0.01 C
		_temp = (reg_data[2] << 8) | reg_data[3];
		measured_data.humi = ((uint32_t)(_temp * 10000) >> 16) + thsensor_cfg.humi_offset; // x 0.01 %
		if (measured_data.humi < 0)
			measured_data.humi = 0;
		else if (measured_data.humi > 9999)
			measured_data.humi = 9999;
		send_i2c_byte(thsensor_cfg.i2c_addr, CHT8305_REG_TMP); // start measure T/H
		deinit_i2c();
		return 0;
	}
//	deinit_i2c();
#else
#error "DEVICE Not released!"
#endif
	init_sensor();
	return 1;
}
