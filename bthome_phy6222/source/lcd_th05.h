/*
 * lcd_th05.h
 *
 *  Created on: 23 янв. 2024 г.
 *      Author: pvvx
 */

#ifndef _LCD_TH05_H_
#define _LCD_TH05_H_

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

#define LCD_BUF_SIZE	6

extern uint8_t lcd_i2c_addr; // LCD controller I2C address
extern uint8_t display_buff[LCD_BUF_SIZE];


/* 0x0 = "  "
 * 0x1 = "°Г"
 * 0x2 = " _"
 * 0x3 = "°C"
 * 0x4 = " -"
 * 0x5 = "°F"
 * 0x6 = " ="
 * 0x7 = "°E" */
typedef enum {
	CLD_TSYMBOL_NONE, 	// "  "
	CLD_TSYMBOL_C = 3,	// "°C"
	CLD_TSYMBOL_F = 5,	// "°F"
	CLD_TSYMBOL_EQ = 6,	// " ="
	CLD_TSYMBOL_E = 7	// "°E"
} CLD_TEMP_SYMBOL;

void init_lcd(void);
void update_lcd(void);
void show_small_number(int16_t number, bool percent);
void show_big_number_x10(int16_t number);
void show_battery_symbol(bool state);
void show_ble_symbol(bool state);
void show_smiley(uint8_t state);
void show_temp_symbol(CLD_TEMP_SYMBOL symbol);
//void chow_clock(void);
//void chow_measure(void);

void chow_lcd(int flg);
void lcd_show_version(void);

#endif /* _LCD_TH05_H_ */
