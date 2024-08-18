/*
 * lcd_th05.c
 *
 *  Created on: 23 янв. 2024 г.
 *      Author: pvvx, Shestoperd
 */
#include <string.h>
#include "types.h"
#include "config.h"
#if (DEV_SERVICES & SERVICE_SCREEN)  && (DEVICE == DEVICE_TH05D)
#include "OSAL.h"
#include "gpio.h"
#include "rom_sym_def.h"
#include "dev_i2c.h"
#include "sensors.h"
#include "lcd.h"
#include "thb2_peripheral.h"

#define LCD_I2C_ADDR	0x3E
#define I2C_WAIT_ms		1

dev_i2c_t i2c_dev1 = {
		//.pi2cdev = AP_I2C1,
		.scl = I2C_LCD_SCL,
		.sda = I2C_LCD_SDA,
		.speed = I2C_100KHZ,
		.i2c_num = 0
};

/* 0,1,2,3,4,5,6,7,8,9,A,b,C,d,E,F*/
const uint8_t display_numbers67[] = {
		// 76543210
		0b011111010, // 0
		0b001100000, // 1
		0b011010110, // 2
		0b011110100, // 3
		0b001101100, // 4
		0b010111100, // 5
		0b010111110, // 6
		0b011100000, // 7
		0b011111110, // 8
		0b011111100, // 9
		0b011101110, // A
		0b000111110, // b
		0b010011010, // C
		0b001110110, // d
		0b010011110, // E
		0b010001110  // F
};
#define LCD_SYM67_b  0b000111110 // "b"
#define LCD_SYM67_H  0b001101110 // "H"
#define LCD_SYM67_h  0b001001110 // "h"
#define LCD_SYM67_i  0b000000010 // "i"
#define LCD_SYM67_L  0b000011010 // "L"
#define LCD_SYM67_o  0b000110110 // "o"
#define LCD_SYM67_t  0b000011110 // "t"
#define LCD_SYM67_0  0b011111010 // "0"
#define LCD_SYM67_A  0b011101110 // "A"
#define LCD_SYM67_a  0b010110110 // "a"
#define LCD_SYM67_P  0b011001110 // "P"

const uint8_t display_numbers45[] = {
		// 76543210
		0b010101111, // 0
		0b000000110, // 1
		0b001101101, // 2
		0b001001111, // 3
		0b011000110, // 4
		0b011001011, // 5
		0b011101011, // 6
		0b000001110, // 7
		0b011101111, // 8
		0b011001111, // 9
		0b011101110, // A
		0b011000011, // b
		0b010101001, // C
		0b001100110, // d
		0b011101001, // E
		0b011101000  // F
};

#define LCD_SYM45_b  0b011100011 // "b"
#define LCD_SYM45_H  0b011100110 // "H"
#define LCD_SYM45_h  0b011100010 // "h"
#define LCD_SYM45_i  0b000100000 // "i"
#define LCD_SYM45_L  0b010100001 // "L"
#define LCD_SYM45_o  0b001100011 // "o"
#define LCD_SYM45_t  0b011100001 // "t"
#define LCD_SYM45_0  0b010101111 // "0"
#define LCD_SYM45_A  0b011101110 // "A"
#define LCD_SYM45_a  0b001101111 // "a"
#define LCD_SYM45_P  0b011101100 // "P"

const uint8_t display_numbers12[] = {
		// 76543210
		0b001011111, // 0
		0b000000110, // 1
		0b001101011, // 2
		0b000101111, // 3
		0b000110110, // 4
		0b000111101, // 5
		0b001111101, // 6
		0b000000111, // 7
		0b001111111, // 8
		0b000111111, // 9
		0b001110111, // A
		0b001111100, // b
		0b001011001, // C
		0b001101110, // d
		0b001111001, // E
		0b001110001  // F
};

#define LCD_SYM12_b  0b001111100 // "b"
#define LCD_SYM12_H  0b001110110 // "H"
#define LCD_SYM12_h  0b001110100 // "h"
#define LCD_SYM12_i  0b001000000 // "i"
#define LCD_SYM12_L  0b001011000 // "L"
#define LCD_SYM12_o  0b001101100 // "o"
#define LCD_SYM12_t  0b001111000 // "t"
#define LCD_SYM12_0  0b001011111 // "0"
#define LCD_SYM12_A  0b001110111 // "A"
#define LCD_SYM12_a  0b001101101 // "a"
#define LCD_SYM12_P  0b001110011 // "P"

uint8_t lcd_i2c_addr; // = 0x3E

uint8_t display_buff[LCD_BUF_SIZE] = {
		0, LCD_SYM12_o, LCD_SYM12_o, 0
};
uint8_t display_out_buff[LCD_BUF_SIZE+1] = { 0 };

const uint8_t lcd_init_cmd[]	=	{
		// LCD controller initialize:
		0xea, // Set IC Operation(ICSET): Software Reset, Internal oscillator circuit
		0xd8, // Mode Set (MODE SET): Display enable, 1/3 Bias, power saving
		0xbc, // Display control (DISCTL): Power save mode 3, FRAME flip, Power save mode 1
		0x80, // load data pointer
		0xf0, // blink control off,  0xf2 - blink
		0xfc, // All pixel control (APCTL): Normal
		0x60,
		0x00,0x00,000,0x00,0x00,0x00,0x00,0x00,0x00
};

/* 0x0 = "  "
 * 0x1 = "°Г"
 * 0x2 = " _"
 * 0x3 = "°C"
 * 0x4 = " -"
 * 0x5 = "°F"
 * 0x6 = " ="
 * 0x7 = "°E" */
void show_temp_symbol(LCD_TEMP_SYMBOLS symbol) {
	display_buff[5] &= ~(BIT(1) | BIT(2) | BIT(3));
	if(symbol & 1)
		display_buff[5] |= BIT(3);
	if(symbol & 2)
		display_buff[5] |= BIT(1);
	if(symbol & 4)
		display_buff[5] |= BIT(2);
}

/* 0 = "     " off,
 * 1 = " ^_^ " happy
 * 2 = " -^- " sad
 * 3 = " ooo "
 * 4 = "(   )"
 * 5 = "(^_^)" happy
 * 6 = "(-^-)" sad
 * 7 = "(ooo)" */
void show_smiley(LCD_SMILEY_SYMBOLS symbol) {
	display_buff[0] &= ~(BIT(0) | BIT(1) | BIT(2) | BIT(3));
	if(symbol & 1)
		display_buff[0] |= BIT(1) | BIT(3);
	if(symbol & 2)
		display_buff[0] |= BIT(1) | BIT(2);
	if(symbol & 4)
		display_buff[0] |= BIT(0);
}

void show_ble_symbol(bool state) {
	if (state)
		display_buff[0] |= BIT(4);
	else
		display_buff[0] &= ~BIT(4);
}

void show_battery(uint8_t level) {
	display_buff[3] &= ~(BIT(1) | BIT(2) | BIT(5) | BIT(6));
	display_buff[4] &= ~(BIT(5)); // | BIT(6));
	display_buff[4] |= BIT(6);
	if(level > 84)
		display_buff[4] |=  BIT(5);
	if(level > 68)
		display_buff[3] |=  BIT(2);
	if(level > 52)
		display_buff[3] |=  BIT(1);
	if(level > 36)
		display_buff[3] |=  BIT(5);
	if(level > 20)
		display_buff[3] |=  BIT(6);
}

void show_big_number_x10(int16_t number) {
	display_buff[5] &= BIT(1) | BIT(2) | BIT(3); // F/C "_"
	display_buff[4] &= BIT(5) | BIT(6) | BIT(7); // bat, %
	if (number > 19995) {
   		display_buff[7] = LCD_SYM67_H; // "H"
   		display_buff[6] = LCD_SYM67_i; // "i"
	} else if (number < -995) {
   		display_buff[7] = LCD_SYM67_L; // "L"
   		display_buff[6] = LCD_SYM67_o; // "o"
	} else {
		display_buff[6] = 0;
		display_buff[7] = 0;
		/* number: -995..19995 */
		if (number > 1995 || number < -1995) {
			//display_buff[5] &= ~BIT(0); // no point, show: -1999..1999
			if (number < 0) {
				number = -number;
				display_buff[6] = BIT(0); // "-"
			}
			number = (number + 5) / 10; // round(div 10)
		} else { // show: -199.9..199.9
			display_buff[5] |= BIT(0); // point,
			if (number < 0){
				number = -number;
				display_buff[6] = BIT(2); // "-"
			}
		}
		/* number: -1999..1999 */
		if (number > 999) display_buff[7] = BIT(0); // "1" 1000..1999
		if (number > 99) display_buff[7] |= display_numbers67[number / 100 % 10];
		if (number > 9) display_buff[6] |= display_numbers67[number / 10 % 10];
		else display_buff[6] |= LCD_SYM67_0; // "0"
	    display_buff[4] |= display_numbers45[number %10] & 0x0f;
	    display_buff[5] |= display_numbers45[number %10] & 0xe0;
	}
}

/* -9 .. 99 */
void show_small_number(int16_t number, bool percent) {

	if(percent)
		display_buff[4] |= BIT(7);
	else
		display_buff[4] &= ~BIT(7);

	if (number > 99) {
		display_buff[1] |= LCD_SYM12_H; // "H"
		display_buff[2] |= LCD_SYM12_i; // "i"
	} else if (number < -9) {
		display_buff[1] |= LCD_SYM12_L; // "L"
		display_buff[2] |= LCD_SYM12_o; // "o"
	} else {
		if (number < 0) {
			number = -number;
			display_buff[1] = BIT(5); // "-"
		}
		if (number > 9) display_buff[1] = display_numbers12[number / 10 % 10];
		display_buff[2] = display_numbers12[number %10];
	}
}

void lcd_show_version(void) {
	display_buff[0] &= BIT(4); // connect
	display_buff[3] &= BIT(1) | BIT(2) | BIT(5) | BIT(6); // bat
	display_buff[4] &= BIT(5) | BIT(6);
#if OTA_TYPE
	display_buff[7] = LCD_SYM67_b;
	display_buff[6] = LCD_SYM67_o;
	display_buff[5] = LCD_SYM45_t & 0xe0;
	display_buff[4] |= LCD_SYM45_t & 0x0f;
#else
	display_buff[7] = LCD_SYM67_A;
	display_buff[6] = LCD_SYM67_P;
	display_buff[5] = LCD_SYM45_P & 0xe0;
	display_buff[4] |= LCD_SYM45_P & 0x0f;
#endif
	display_buff[1] = display_numbers12[(APP_VERSION>>4) & 0x0f];
	display_buff[2] = display_numbers12[APP_VERSION & 0x0f];
	update_lcd();
}

void chow_clock(void) {
	uint32_t tmp = clkt.utc_time_sec / 60;
	uint32_t min = tmp % 60;
	uint32_t hrs = (tmp / 60) % 24;

	display_buff[0] &= BIT(4); // connect
	display_buff[3] &= BIT(1) | BIT(2) | BIT(5) | BIT(6); // bat
	display_buff[4] &= BIT(5) | BIT(6);

	display_buff[7] = display_numbers67[(hrs / 10) % 10];
	display_buff[6] = display_numbers67[hrs % 10];
	display_buff[5] = 0;

	display_buff[1] = display_numbers12[(min / 10) % 10];
	display_buff[2] = display_numbers12[min % 10];
	update_lcd();
}

static void chow_measure(void) {
#if (DEV_SERVICES & SERVICE_THS)
	if(cfg.flg & FLG_SHOW_TF) {
		show_big_number_x10(((int32_t)((int32_t)measured_data.temp * 9)/ 50) + 320); // convert C to F
		show_temp_symbol(LCD_TSYMBOL_F); // "°F"
	} else {
		show_big_number_x10((measured_data.temp + 5)/10);
		show_temp_symbol(LCD_TSYMBOL_C);
	}
	int16_t h = (measured_data.humi + 50)/100;
	if(h > 99)
		h = 99;
	show_small_number(h, true);
	show_battery(measured_data.battery);
#if (OTA_TYPE == OTA_TYPE_APP)
	if(cfg.flg & FLG_SHOW_SMILEY) {
#if (DEV_SERVICES & SERVICE_TH_TRG)
		if(cfg.flg & FLG_SHOW_TRG) {
			if(measured_data.flg.comfort) {
				if(measured_data.flg.trg_on)
					show_smiley(LD_SSYMBOL_HAPPY);
				else
					show_smiley(LD_SSYMBOL__HAPPY);
			} else {
				if(measured_data.flg.trg_on)
					show_smiley(LD_SSYMBOL_SAD);
				else
					show_smiley(LD_SSYMBOL__SAD);
			}
		} else
#endif // SERVICE_TH_TRG
		{
			if(measured_data.flg.comfort)
				show_smiley(LD_SSYMBOL_HAPPY);
			else
				show_smiley(LD_SSYMBOL_SAD);
		}
#if (DEV_SERVICES & SERVICE_TH_TRG)
	} else if(cfg.flg & FLG_SHOW_TRG) {
		if(measured_data.flg.trg_on)
			show_smiley(LD_SSYMBOL_ALL);
		else
			show_smiley(LD_SSYMBOL_OFF);
	} else
#endif // SERVICE_TH_TRG
#endif // OTA_TYPE
		show_smiley(LD_SSYMBOL_OFF);
#else
	show_big_number_x10(measured_data.battery_mv/100);
	show_small_number((measured_data.battery > 99)? 99 : measured_data.battery, true);
	show_battery_symbol(1);
	show_smiley(LD_SSYMBOL_OFF);
#endif // SERVICE_THS
	show_ble_symbol(gapRole_state == GAPROLE_CONNECTED);
	update_lcd();
}

/* flg != 0 -> chow_measure */
void chow_lcd(int flg) {
#if OTA_TYPE == OTA_TYPE_BOOT
	if(flg)
		chow_measure();
#else
	if(cfg.flg & FLG_SHOW_TIME) {
		if(wrk.lcd_count++ & 1)
			chow_clock();
		else
			chow_measure();
	} else if(flg) {
		chow_measure();
	}
#endif
}

void send_to_lcd(uint8_t *pbuf, int len) {
	if (lcd_i2c_addr) {
		init_i2c(&i2c_dev1);
		send_i2c_buf(&i2c_dev1, lcd_i2c_addr, pbuf, len);
		deinit_i2c(&i2c_dev1);
	}
}


void update_lcd(void) {
#if (OTA_TYPE == OTA_TYPE_APP)
	if(lcd_i2c_addr == 0 || (cfg.flg & FLG_DISPLAY_OFF) != 0)
		return;
#endif
	if(memcmp(&display_out_buff[1], display_buff, sizeof(display_buff))) {
		memcpy(&display_out_buff[1], display_buff, sizeof(display_buff));
		send_to_lcd(display_out_buff, sizeof(display_out_buff));
	}
}

void init_lcd(void) {
	i2c_dev1.speed = I2C_100KHZ;
	init_i2c(&i2c_dev1);
	if(!send_i2c_buf(&i2c_dev1, LCD_I2C_ADDR, (uint8_t *) lcd_init_cmd, sizeof(lcd_init_cmd))) {
#if (OTA_TYPE == OTA_TYPE_APP)
		if(cfg.flg & FLG_DISPLAY_OFF)
			send_i2c_byte(&i2c_dev1, LCD_I2C_ADDR, 0xd0); // Mode Set (MODE SET): Display disable, 1/3 Bias, power saving
#endif
		lcd_i2c_addr = LCD_I2C_ADDR;
	} else
		lcd_i2c_addr = 0;
	deinit_i2c(&i2c_dev1);
	i2c_dev1.speed = I2C_400KHZ;
}

/****************************************************/
#endif // (DEV_SERVICES & SERVICE_SCREEN)

