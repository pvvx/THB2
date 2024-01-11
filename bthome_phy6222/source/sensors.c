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


#ifndef I2C_SDA
#define I2C_SDA GPIO_P18
#define I2C_SCL GPIO_P20
#endif

measured_data_t measured_data;
unsigned short th_sensor_id;

void init_i2c(void) {
	hal_gpio_fmux_set(I2C_SCL, FMUX_IIC0_SCL);
	hal_gpio_fmux_set(I2C_SDA, FMUX_IIC0_SDA);

	//hal_i2c_init(I2C_0, I2C_CLOCK_400K):

	int pclk = clk_get_pclk();

	AP_I2C_TypeDef* pi2cdev = AP_I2C0;
	hal_clk_gate_enable(MOD_I2C0);
	pi2cdev->IC_ENABLE = 0;
	pi2cdev->IC_CON = 0x61;
	pi2cdev->IC_CON = ((pi2cdev->IC_CON) & 0xfffffff9)|(0x02 << 1);
	if(pclk == 16000000)
	{
		pi2cdev->IC_FS_SCL_HCNT = 10;
		pi2cdev->IC_FS_SCL_LCNT = 17;
	}
	else if(pclk == 32000000)
	{
		pi2cdev->IC_FS_SCL_HCNT = 30;
		pi2cdev->IC_FS_SCL_LCNT = 35;
	}
	else if(pclk == 48000000)
	{
		pi2cdev->IC_FS_SCL_HCNT = 48;
		pi2cdev->IC_FS_SCL_LCNT = 54;
	}
	else if(pclk == 64000000)
	{
		pi2cdev->IC_FS_SCL_HCNT = 67;
		pi2cdev->IC_FS_SCL_LCNT = 75;
	}
	else if(pclk == 96000000)
	{
		pi2cdev->IC_FS_SCL_HCNT = 105;
		pi2cdev->IC_FS_SCL_LCNT = 113;
	}
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
		if(osal_sys_tick - to > 10)
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
		if(osal_sys_tick - to > 10)
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
			if(osal_sys_tick - to > 10)
				return 1;
	}
	return 0;
}

__ATTR_SECTION_XIP__ void init_sensor(void) {
	init_i2c();
	send_i2c_byte(0, 0x06); // Reset command using the general call address
	WaitMs(3);
	send_i2c_wreg(CHT8310_I2C_ADDR0, CHT8310_REG_CRT, 0x0300); // Set conversion ratio 5 sec
	WaitMs(1);
	read_i2c_bytes(CHT8310_I2C_ADDR0, CHT8310_REG_ID, (uint8 *)&th_sensor_id, 2);
	deinit_i2c();
}


int read_sensor(void) {
	int32 _r32;
	int16 _r16;
	uint8 reg_data[4];
	init_i2c();
	_r32 = read_i2c_bytes(CHT8310_I2C_ADDR0, CHT8310_REG_TMP, reg_data, 2);
	//? WaitUs(100);
	_r32 |=	read_i2c_bytes(CHT8310_I2C_ADDR0, CHT8310_REG_HMD, &reg_data[2], 2);
	deinit_i2c();
	if (!_r32) {
		_r16 = (reg_data[0] << 8) | reg_data[1];
		measured_data.temp = (int32)(_r16 * 25606 + 0x7fff) >> 16; // x 0.01 C
		_r32 = ((reg_data[2] << 8) | reg_data[3]) & 0x7fff;
		measured_data.humi = (uint32)(_r32 * 20000 + 0x7fff) >> 16; // x 0.01 %
		if (measured_data.humi > 9999)
			measured_data.humi = 9999;
		measured_data.count++;
		return 0;
	}
	init_sensor();
	return 1;
}
