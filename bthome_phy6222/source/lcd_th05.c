/*
 * lcd_th05.c
 *
 *  Created on: 23 янв. 2024 г.
 *      Author: pvvx
 */
#include <string.h>
#include "types.h"
#include "config.h"
#if (DEV_SERVICES & SERVICE_SCREEN)
#include "OSAL.h"
#include "gpio.h"
#include "rom_sym_def.h"
#include "dev_i2c.h"
#include "sensor.h"
#include "lcd_th05.h"

#define LCD_I2C_SPEED	100 // 100 or 400 kHz
#define LCD_I2C_ADDR	0x3E
#define I2C_WAIT_ms		1

/* 0,1,2,3,4,5,6,7,8,9,A,b,C,d,E,F*/
const uint8_t display_numbers[] = {
		// 76543210
		0b011110011, // 0
		0b000000011, // 1
		0b010110101, // 2
		0b010010111, // 3
		0b001000111, // 4
		0b011010110, // 5
		0b011110110, // 6
		0b000010011, // 7
		0b011110111, // 8
		0b011010111, // 9
		0b001110111, // A
		0b011100110, // b
		0b011110000, // C
		0b010100111, // d
		0b011110100, // E
		0b001110100  // F
};
#define LCD_SYM_H  0b001100111 // "H"
#define LCD_SYM_h  0b001100110 // "h"
#define LCD_SYM_i  0b000100000 // "i"
#define LCD_SYM_L  0b011100000 // "L"
#define LCD_SYM_o  0b010100110 // "o"
#define LCD_SYM_t  0b011100100 // "t"
#define LCD_SYM_0  0b011110011 // "0"
#define LCD_SYM_a  0b011110110 // 'a'

uint8_t lcd_i2c_addr; // = 0x3E

uint8_t display_buff[LCD_BUF_SIZE] = {
		LCD_SYM_o, LCD_SYM_o, LCD_SYM_o,
};
uint8_t display_out_buff[LCD_BUF_SIZE+1];
/* blink off: display_out_buff[0] = 0xf0, on: display_out_buff[0] = 0xf2 */

const uint8_t lcd_init_cmd[]	=	{
		// LCD controller initialize:
		//0xC8, // Mode Set (MODE SET): Display enable, 1/3 Bias
		0xD8, // Mode Set (MODE SET): Display enable, 1/3 Bias, power saving
		0x80, // load data pointer
		0xF0, // blink control 0xf2
		0x60,
		0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff
};


/*
 *  TH-05 LCD buffer:  byte.bit

         --0.4--         --1.4--            --2.4--          BAT
  |    |         |     |         |        |         |        3.6
  |   0.6       0.0   1.6       1.0      2.6       2.0
  |    |         |     |         |        |         |      o 3.5
 0.3     --0.2--         --1.2--            --2.2--          +--- 3.5
  |    |         |     |         |        |         |     3.5|
  |   0.5       0.1   1.5       1.1      2.5       2.1       ---- 3.7
  |    |         |     |         |        |         |     3.5|
         --0.7--         --1.7--     *      --2.7--          ---- 2.3
                                    1.3
                                        --4.4--         --5.4--
                                      |         |     |         |
          3.0      3.0               4.6       4.0   5.6       5.0
          / \      / \                |         |     |         |
    3.4(  \_/  3.2 \_/  )3.4            --4.2--         --5.2--
          3.2  / \ 3.2                |         |     |         |
               \_/                   4.5       4.1   5.5       5.1     %
               3.0                    |         |     |         |     5.3
                                        --4.7--         --5.7--
                           OO 4.3
 None: 3.1, 3.3
*/

/* 0x0 = "  "
 * 0x1 = "°Г"
 * 0x2 = " _"
 * 0x3 = "°C"
 * 0x4 = " -"
 * 0x5 = "°F"
 * 0x6 = " ="
 * 0x7 = "°E" */
void show_temp_symbol(uint8_t symbol) {
	display_buff[2] &= ~BIT(3);
	display_buff[3] &= ~(BIT(7)|BIT(5)) ;
	display_buff[2] |= (symbol << 2) & BIT(3);
	display_buff[3] |= (symbol << 5) & (BIT(7)|BIT(5));
}

/* 0 = "     " off,
 * 1 = " ^_^ " happy
 * 2 = " -^- " sad
 * 3 = " ooo "
 * 4 = "(   )"
 * 5 = "(^_^)" happy
 * 6 = "(-^-)" sad
 * 7 = "(ooo)" */
void show_smiley(uint8_t state) {
	display_buff[3] &= ~0x15;
	state = (state & 1) | ((state << 1) & 4) | ((state << 2) & 0x10);
	display_buff[3] |= state;
}

void show_ble_symbol(bool state) {
	if (state)
		display_buff[4] |= BIT(3);
	else
		display_buff[4] &= ~BIT(3);
}

void show_battery_symbol(bool state) {
	if (state)
		display_buff[3] |= BIT(6);
	else
		display_buff[3] &= ~BIT(6);
}

void show_big_number_x10(int16_t number) {
	display_buff[2] &= BIT(3); // F/C
	if (number > 19995) {
   		display_buff[0] = LCD_SYM_H; // "H"
   		display_buff[1] = LCD_SYM_i; // "i"
	} else if (number < -995) {
   		display_buff[0] = LCD_SYM_L; // "L"
   		display_buff[1] = LCD_SYM_o; // "o"
	} else {
		display_buff[0] = 0;
		display_buff[1] = 0;
		/* number: -995..19995 */
		if (number > 1995 || number < -95) {
			display_buff[1] = 0; // no point, show: -99..1999
			if (number < 0){
				number = -number;
				display_buff[0] = BIT(2); // "-"
			}
			number = (number + 5) / 10; // round(div 10)
		} else { // show: -9.9..199.9
			display_buff[1] = BIT(3); // point,
			if (number < 0){
				number = -number;
				display_buff[0] = BIT(2); // "-"
			}
		}
		/* number: -99..1999 */
		if (number > 999) display_buff[0] |= BIT(3); // "1" 1000..1999
		if (number > 99) display_buff[0] |= display_numbers[number / 100 % 10];
		if (number > 9) display_buff[1] |= display_numbers[number / 10 % 10];
		else display_buff[1] |= LCD_SYM_0; // "0"
	    display_buff[2] = display_numbers[number %10];
	}
}

/* -9 .. 99 */
void show_small_number(int16_t number, bool percent) {

	display_buff[4] &= BIT(3); // connect
	display_buff[5] = percent? BIT(3) : 0;
	if (number > 99) {
		display_buff[4] |= LCD_SYM_i; // "i"
		display_buff[5] |= LCD_SYM_H; // "H"
	} else if (number < -9) {
		display_buff[4] |= LCD_SYM_o; // "o"
		display_buff[5] |= LCD_SYM_L; // "L"
	} else {
		if (number < 0) {
			number = -number;
			display_buff[4] = BIT(2); // "-"
		}
		if (number > 9) display_buff[4] |= display_numbers[number / 10 % 10];
		display_buff[5] |= display_numbers[number %10];
	}
}

#if	USE_CLOCK
void show_clock(void) {
	uint32_t tmp = utc_time_sec / 60;
	uint32_t min = tmp % 60;
	uint32_t hrs = tmp / 60 % 24;
	display_buff[0] = display_numbers[min % 10];
	display_buff[1] = display_numbers[min / 10 % 10];
	display_buff[2] = 0;
	display_buff[3] &= BIT(6); // bat
	display_buff[4] &= BIT(3); // connect
	display_buff[4] |= display_numbers[hrs % 10];
	display_buff[5] = display_numbers[hrs / 10 % 10];
}
#endif // USE_CLOCK

extern volatile uint32 osal_sys_tick;



static void send_to_lcd(uint8_t *pbuf, int len){
	init_i2c(0);
	if (send_i2c_buf(0x3E, pbuf, len))
	deinit_i2c();
}

void update_lcd(void) {
	if (lcd_i2c_addr && memcmp(&display_out_buff[1], display_buff, sizeof(display_buff))) {
		memcpy(&display_out_buff[1], display_buff, sizeof(display_buff));
		send_to_lcd(display_out_buff, sizeof(display_out_buff));
	}
}

void init_lcd(void) {
	init_i2c(0);
	if(!send_i2c_buf(LCD_I2C_ADDR, (uint8_t *) lcd_init_cmd, sizeof(lcd_init_cmd)))  // sleep: 15.5 uA
		lcd_i2c_addr = LCD_I2C_ADDR;
	else
		lcd_i2c_addr = 0;
	deinit_i2c();
}

/****************************************************/
#endif // (DEV_SERVICES & SERVICE_SCREEN)

