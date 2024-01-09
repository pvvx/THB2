/*************
 bsp_button.h
 SDK_LICENSE
***************/

#ifndef __BSP_BUTTON_H__
#define __BSP_BUTTON_H__

#include "types.h"

//#define BSP_BTN_LONG_PRESS_ENABLE

#define BTN_SYS_TICK                            10  //unit:ms
#define BTN_FILTER_TICK_COUNT                   5   //(BTN_SYS_TICK*BTN_FILTER_TICK_COUNT)            ms
#define BTN_LONG_PRESS_START_TICK_COUNT         10  //(BTN_SYS_TICK*BTN_LONG_PRESS_START_TICK_COUNT)  ms
#define BTN_LONG_PRESS_KEEP_TICK_COUNT          100 //(BTN_SYS_TICK*BTN_LONG_PRESS_KEEP_TICK_COUNT)   ms

#define BTN_NUMBER                              32  //valid:[0,0x2F].reserved[0x30,0x3F]
#define BTN_NONE                                0xFF

#if (BTN_NUMBER > 48)
    #error "error bsp button config,please check"
#endif

/*
    bit0:press down
    bit1:press up
    bit2:long press start
    bit3:long press keep
    bit4:combine or not
*/
#define BSP_BTN_PD_CFG                         0x01
#define BSP_BTN_UP_CFG                         0x02
#define BSP_BTN_LPS_CFG                        0x04
#define BSP_BTN_LPK_CFG                        0x08
#define BSP_BTN_CM_CFG                         0x10

#define BSP_BTN_PD_TYPE                        (0U<<6)
#define BSP_BTN_UP_TYPE                        (1U<<6)
#define BSP_BTN_LPS_TYPE                       (2U<<6)
#define BSP_BTN_LPK_TYPE                       (3U<<6)

#define BSP_BTN_PD_BASE                       BSP_BTN_PD_TYPE
#define BSP_BTN_UP_BASE                       BSP_BTN_UP_TYPE
#define BSP_BTN_LPS_BASE                      BSP_BTN_LPS_TYPE
#define BSP_BTN_LPK_BASE                      BSP_BTN_LPK_TYPE

#define BSP_BTN_TYPE(key)                    (key & 0xC0)
#define BSP_BTN_INDEX(key)                   (key & 0x3F)

typedef uint32_t BTN_COMBINE_T;

typedef struct
{
    uint8_t KeyConfig;
    uint8_t State;
    uint8_t Count;
    uint8_t FilterTime;

    #ifdef BSP_BTN_LONG_PRESS_ENABLE
    uint8_t LongCount;
    uint8_t LongTime;
    uint8_t RepeatSpeed;
    uint8_t RepeatCount;
    #endif
} BTN_T;


#define KEY_FIFO_SIZE   20
typedef struct
{
    uint8_t Buf[KEY_FIFO_SIZE];
    uint8_t Read;
    uint8_t Write;
} KEY_FIFO_T;

bool bsp_InitBtn(BTN_T* sum_btn_array,uint8_t sum_btn_num,uint8_t combine_btn_start,BTN_COMBINE_T* combine_btn_array);

uint8_t bsp_KeyPro(void);
uint8_t bsp_GetKey(void);
bool bsp_set_key_value_by_row_col(uint8_t cols_num,uint8_t row,uint8_t col,bool value);
void bsp_set_key_value_by_index(uint8_t index,bool value);
bool bsp_KeyEmpty(void);

#endif
