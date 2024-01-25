/*
 * dev_i2c.c
 *
 *  Created on: 25 янв. 2024 г.
 *      Author: pvvx
 */
#include "config.h"
#include "OSAL.h"
#include "gpio.h"
#include "rom_sym_def.h"
#include "i2c.h"
#include "dev_i2c.h"

#define I2C_WAIT_ms		1

void init_i2c(bool speed400khz) {
	hal_gpio_fmux_set(I2C_SCL, FMUX_IIC0_SCL);
	hal_gpio_fmux_set(I2C_SDA, FMUX_IIC0_SDA);

	//hal_i2c_init(I2C_0, I2C_CLOCK_400K):

	int pclk = clk_get_pclk();

	AP_I2C_TypeDef* pi2cdev = AP_I2C0;
	hal_clk_gate_enable(MOD_I2C0);
	pi2cdev->IC_ENABLE = 0;
	pi2cdev->IC_CON = 0x61;
	if(speed400khz) {
		// SET_I2C_SPEED 400 kHz
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
	} else {
		// SET_I2C_SPEED 100 kHz
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
		if(osal_sys_tick - to > I2C_WAIT_ms)
			return 1;
	}
	return 0;
}

int read_i2c_nabuf(uint8 addr, uint8 * data, uint8 size) {
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
			if(osal_sys_tick - to > I2C_WAIT_ms)
				return 1;
	}
	return 0;
}

int send_i2c_buf(uint8 addr, uint8 * pdata, int len) {
	AP_I2C_TypeDef * pi2cdev = AP_I2C0;
	pi2cdev->IC_ENABLE = 0;
	pi2cdev->IC_TAR = addr;
	HAL_ENTER_CRITICAL_SECTION();
	pi2cdev->IC_ENABLE = 1;
	while(len--) {
	  pi2cdev->IC_DATA_CMD = *pdata++;
	  while(!(pi2cdev->IC_RAW_INTR_STAT & 0x10));
	}
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
