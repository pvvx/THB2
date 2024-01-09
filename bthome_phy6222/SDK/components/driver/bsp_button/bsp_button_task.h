/**************************************************************************************************
    Filename:       bsp_button_task.h
    Revised:        $Date $
    Revision:       $Revision $

	SDK_LICENSE

**************************************************************************************************/
#ifndef __BSP_BUTTON_TASK_H__
#define __BSP_BUTTON_TASK_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "bus_dev.h"
#include "bsp_gpio.h"
#include "kscan.h"
#include "bsp_button.h"
#include "log.h"


/*********************************************************************
    INCLUDES
*/

/*********************************************************************
    CONSTANTS
*/
#define BSP_BTN_JUST_GPIO                                   (0x01)
#define BSP_BTN_JUST_KSCAN                                  (0x02)
#define BSP_BTN_GPIO_AND_KSCAN                              (0x03)
#define BSP_BTN_HARDWARE_CONFIG                             BSP_BTN_JUST_GPIO

#if (BSP_BTN_HARDWARE_CONFIG == BSP_BTN_JUST_GPIO)

#define BSP_SINGLE_BTN_NUM                                  (GPIO_SINGLE_BTN_NUM)
#define BSP_COMBINE_BTN_NUM                                 (2)
#define BSP_TOTAL_BTN_NUM                                   (BSP_SINGLE_BTN_NUM + BSP_COMBINE_BTN_NUM)

#elif  (BSP_BTN_HARDWARE_CONFIG == BSP_BTN_JUST_KSCAN)

#define BSP_SINGLE_BTN_NUM                                  (NUM_KEY_ROWS * NUM_KEY_COLS)
#define BSP_COMBINE_BTN_NUM                                 (2)
#define BSP_TOTAL_BTN_NUM                                   (BSP_SINGLE_BTN_NUM + BSP_COMBINE_BTN_NUM)

#elif  (BSP_BTN_HARDWARE_CONFIG == BSP_BTN_GPIO_AND_KSCAN)

#define BSP_KSCAN_SINGLE_BTN_NUM                            (NUM_KEY_ROWS * NUM_KEY_COLS)
#define BSP_GPIO_SINGLE_BTN_NUM                             (GPIO_SINGLE_BTN_NUM)

#define BSP_KSCAN_COMBINE_BTN_NUM                           (1)
#define BSP_GPIO_COMBINE_BTN_NUM                            (1)

#define BSP_SINGLE_BTN_NUM                                  (BSP_KSCAN_SINGLE_BTN_NUM + BSP_GPIO_SINGLE_BTN_NUM)
#define BSP_COMBINE_BTN_NUM                                 (BSP_KSCAN_COMBINE_BTN_NUM + BSP_GPIO_COMBINE_BTN_NUM)
#define BSP_TOTAL_BTN_NUM                                   (BSP_SINGLE_BTN_NUM + BSP_COMBINE_BTN_NUM)

#else

#error "error bsp button config,please check"

#endif


extern BTN_T usr_sum_btn_array[BSP_TOTAL_BTN_NUM];
#if (BSP_COMBINE_BTN_NUM > 0)
extern uint32_t usr_combine_btn_array[BSP_COMBINE_BTN_NUM];
#endif

extern bool bsp_btn_gpio_flag;
extern bool bsp_btn_kscan_flag;

typedef void (*bsp_btn_callback_t)(uint8_t evt);
extern bsp_btn_callback_t bsp_btn_cb;

#define BSP_BTN_EVT_SYSTICK                                 (0x0010)
#define KSCAN_WAKEUP_TIMEOUT_EVT                            (0x0020)
#define BSP_BTN_EVT_DBG                                     (0x0040)

uint16 Bsp_Btn_ProcessEvent( uint8 task_id, uint16 events);
void   Bsp_Btn_Init( uint8 task_id );

#if ((BSP_BTN_HARDWARE_CONFIG == BSP_BTN_JUST_KSCAN) ||  (BSP_BTN_HARDWARE_CONFIG ==BSP_BTN_GPIO_AND_KSCAN))
extern KSCAN_ROWS_e rows[NUM_KEY_ROWS];
extern KSCAN_COLS_e cols[NUM_KEY_COLS];
#endif


/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* BSP_BUTTON_TASK_H */
