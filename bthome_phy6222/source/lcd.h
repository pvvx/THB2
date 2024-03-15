/*
 * lcd_th05.h
 *
 *  Created on: 23 янв. 2024 г.
 *      Author: pvvx, Shestoperd
 */

#ifndef _LCD_TH05_H_
#define _LCD_TH05_H_

#include "config.h"
#if (DEV_SERVICES & SERVICE_SCREEN)

/*
 *  TH-05 v1.3 LCD buffer:  byte.bit
               --7.7--         --6.7--            --4.3--             
        |    |         |     |         |        |         |           
        |   7.3       7.6   6.3       6.6      5.7       4.2
        |    |         |     |         |        |         |      o 5.3
 -6.0- 7.0     --7.2--         --6.2--            --5.6--          +--- 5.3
        |    |         |     |         |        |         |     5.3|
        |   7.1       7.5   6.1       6.5      5.5       4.1       ---- 5.2
        |    |         |     |         |        |         |     5.3|
               --7.4--         --6.4--     *      --4.0--          ---- 5.1
                                          5.0

            BLE                 --1.0--         --2.0--     |-------------4.6-------------|
            0.4               |         |     |         |    3.6 | 3.5 | 3.1 | 3.2 | 4.5 | ]
                             1.4       1.1   2.4       2.1  |-------------4.6-------------|
        0.1      0.1          |         |     |         |
  0.0(   O  0.2   O   )0.0      --1.5--         --2.5--
           -----              |         |     |         |
           \___/             1.6       1.2   2.6       2.2     %
            0.3               |         |     |         |     4.7
                                --1.3--         --2.3--
*/

/*
 *  TH-05 v1.4 LCD buffer:  byte.bit

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

/*
 *  THB05F LCD buffer:  byte.bit

         --0.4--         --1.4--            --2.4--          BAT
  |    |         |     |         |        |         |        3.5
  |   0.5       0.0   1.5       1.0      2.5       2.0
  |    |         |     |         |        |         |      o 3.6
 0.3     --0.1--         --1.1--            --2.1--          +--- 3.6
  |    |         |     |         |        |         |     3.6|
  |   0.6       0.2   1.6       1.2      2.6       2.2       ---- 3.7
  |    |         |     |         |        |         |     3.6|
         --0.7--         --1.7--     *      --2.7--          ---- 2.3
                                    1.3
                                        --4.4--         --5.4--
                                      |         |     |         |     ooo
          3.0      3.0               4.5       4.0   5.5       5.0    4.3
          / \      / \                |         |     |         |
    3.4(  \_/  3.1 \_/  )3.4            --4.1--         --5.1--
          3.1  / \ 3.1                |         |     |         |
               \_/                   4.6       4.2   5.6       5.2     %
               3.0                    |         |     |         |     5.3
                                        --4.7--         --5.7--

 None:
*/

/*
 *  THB1 LCD buffer:  byte.bit

         --0.0--         --1.0--            --2.0--          BAT
  |    |         |     |         |        |         |        3.0 | 3.4 | 3.5 | 3.6 | 3.7
  |   0.4       0.1   1.4       1.1      2.4       2.1
  |    |         |     |         |        |         |      o 3.2
 0.7     --0.5--         --1.5--            --2.5--          +--- 3.2
  |    |         |     |         |        |         |     3.2|
  |   0.6       0.2   1.6       1.2      2.6       2.2       ---- 3.1
  |    |         |     |         |        |         |     3.2|
         --0.3--         --1.3--     *      --2.3--          ---- 3.3
                                    2.7
                                        --4.0--         --5.0--
                                      |         |     |         |
          6.6      6.6               4.4       4.1   5.4       5.1
          / \      / \                |         |     |         |
    4.7(  \_/  6.7 \_/  )4.7            --4.5--         --5.5--
          6.7  / \ 6.7                |         |     |         |
               \_/                   4.6       4.2   5.6       5.2     %
               6.6                    |         |     |         |     5.7
                                        --4.3--         --5.3--
                           OO 1.7
*/


#if (DEVICE == DEVICE_THB1)
#define LCD_BUF_SIZE	7
#elif (DEVICE == DEVICE_THB3)
#define LCD_BUF_SIZE	7
#elif (DEVICE == DEVICE_TH05D)
#define LCD_BUF_SIZE	8
#elif (DEVICE == DEVICE_TH05)
#define LCD_BUF_SIZE	6
#elif (DEVICE == DEVICE_TH05F)
#define LCD_BUF_SIZE	6
#else
#error "DEVICE Not released!"
#endif

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
	LCD_TSYMBOL_NONE, 	// "  "
	LCD_TSYMBOL_C = 3,	// "°C"
	LCD_TSYMBOL_F = 5,	// "°F"
	LCD_TSYMBOL_EQ = 6,	// " ="
	LCD_TSYMBOL_E = 7	// "°E"
} LCD_TEMP_SYMBOLS;

/* 0 = "     " off,
 * 1 = " ^_^ " happy
 * 2 = " -^- " sad
 * 3 = " ooo "
 * 4 = "(   )"
 * 5 = "(^_^)" happy
 * 6 = "(-^-)" sad
 * 7 = "(ooo)" */
typedef enum {
	LD_SSYMBOL_OFF,		// 0 = "     " off,
	LD_SSYMBOL__HAPPY,	// 1 = " ^_^ " happy
	LD_SSYMBOL__SAD,	// 2 = " -^- " sad
	LD_SSYMBOL_OOO,		// 3 = " ooo "
	LD_SSYMBOL_CC,		// 4 = "(   )"
	LD_SSYMBOL_HAPPY,	// 5 = "(^_^)" happy
	LD_SSYMBOL_SAD,		// 6 = "(-^-)" sad
	LD_SSYMBOL_ALL,		// 7 = "(ooo)"
} LCD_SMILEY_SYMBOLS;

void init_lcd(void);
void update_lcd(void);
void show_small_number(int16_t number, bool percent);
void show_big_number_x10(int16_t number);
void show_battery_symbol(bool state);
void show_ble_symbol(bool state);
void show_smiley(LCD_SMILEY_SYMBOLS symbol);
void show_temp_symbol(LCD_TEMP_SYMBOLS symbol);
//void chow_clock(void);
//void chow_measure(void);

void chow_lcd(int flg);
void lcd_show_version(void);
extern void send_to_lcd(uint8_t *pbuf, int len);

#endif // (DEV_SERVICES & SERVICE_SCREEN)
#endif /* _LCD_TH05_H_ */
