/**************************************************************************************************
    Filename:       bsp_button_task.c
    Revised:        $Date $
    Revision:       $Revision $
    SDK_LICENSE
**************************************************************************************************/

/*********************************************************************
    INCLUDES
*/
#include <string.h>
#include "OSAL.h"
#include "OSAL_Timers.h"
#include "pwrmgr.h"
#include "bsp_button_task.h"

uint8 Bsp_Btn_TaskID;

bool bsp_btn_timer_flag = FALSE;
bool bsp_btn_gpio_flag = FALSE;
bool bsp_btn_kscan_flag = FALSE;
bsp_btn_callback_t bsp_btn_cb = NULL;

//uint32_t bsp_btn_counter = 0;
//void wake_test(void)
//{
//  bsp_btn_counter++;
//}

#if ((BSP_BTN_HARDWARE_CONFIG == BSP_BTN_JUST_KSCAN) ||  (BSP_BTN_HARDWARE_CONFIG ==BSP_BTN_GPIO_AND_KSCAN))

uint8 KscanMK_row[KSCAN_ALL_ROW_NUM];
uint8 KscanMK_col[KSCAN_ALL_COL_NUM];

static void kscan_evt_handler(kscan_Evt_t* evt)
{
    bool ret;

    for(uint8_t i=0; i<evt->num; i++)
    {
        if(evt->keys[i].type == KEY_PRESSED)
        {
            ret = bsp_set_key_value_by_row_col(NUM_KEY_COLS,KscanMK_row[evt->keys[i].row],KscanMK_col[evt->keys[i].col],TRUE);
            {
                if(ret == TRUE)
                {
                    bsp_btn_timer_flag = TRUE;
                    osal_start_reload_timer(Bsp_Btn_TaskID,BSP_BTN_EVT_SYSTICK,BTN_SYS_TICK);
                }
            }
        }
        else
        {
            bsp_set_key_value_by_row_col(NUM_KEY_COLS,KscanMK_row[evt->keys[i].row],KscanMK_col[evt->keys[i].col],FALSE);
        }
    }
}

void kscan_button_init(uint8 task_id)
{
    kscan_Cfg_t cfg;
    cfg.ghost_key_state = NOT_IGNORE_GHOST_KEY;
    cfg.key_rows = rows;
    cfg.key_cols = cols;
    cfg.interval = 5;
    cfg.evt_handler = kscan_evt_handler;
    memset(KscanMK_row,0xff,KSCAN_ALL_ROW_NUM);

    for(int i=0; i<NUM_KEY_ROWS; i++)
    {
        KscanMK_row[rows[i]] = i;
    }

    memset(KscanMK_col,0xff,KSCAN_ALL_COL_NUM);

    for(int i=0; i<NUM_KEY_COLS; i++)
    {
        KscanMK_col[cols[i]] = i;
    }

    hal_kscan_init(cfg, task_id, KSCAN_WAKEUP_TIMEOUT_EVT);
}
#endif

#if ((BSP_BTN_HARDWARE_CONFIG == BSP_BTN_JUST_GPIO) || (BSP_BTN_HARDWARE_CONFIG == BSP_BTN_GPIO_AND_KSCAN))
uint8_t Bsp_Btn_Get_Index(gpio_pin_e pin)
{
    uint8_t i;

    if(hal_gpio_btn_get_index(pin,&i) == PPlus_SUCCESS)
    {
        #if  (BSP_BTN_HARDWARE_CONFIG == BSP_BTN_GPIO_AND_KSCAN)
        return (BSP_KSCAN_SINGLE_BTN_NUM + i);
        #else
        return (i);
        #endif
    }

    return 0xFF;
}
#endif

static void Bsp_Btn_Check(uint8_t ucKeyCode)
{
    #if (BSP_BTN_HARDWARE_CONFIG == BSP_BTN_JUST_GPIO)
    hal_gpio_btn_cb(ucKeyCode);
    #else
    bsp_btn_cb(ucKeyCode);
    #endif

    if(bsp_btn_timer_flag == TRUE)
    {
        if(bsp_KeyEmpty() == TRUE)
        {
            osal_stop_timerEx(Bsp_Btn_TaskID,BSP_BTN_EVT_SYSTICK);
            bsp_btn_timer_flag = FALSE;
        }
    }
}

void gpio_btn_pin_event_handler(gpio_pin_e pin,IO_Wakeup_Pol_e type)
{
    #if ((BSP_BTN_HARDWARE_CONFIG == BSP_BTN_JUST_GPIO) ||  (BSP_BTN_HARDWARE_CONFIG == BSP_BTN_GPIO_AND_KSCAN))

    if(((GPIO_SINGLE_BTN_IDLE_LEVEL == 0) && (POL_RISING == type)) || \
            ((GPIO_SINGLE_BTN_IDLE_LEVEL == 1) && (POL_RISING != type)))
    {
        bsp_btn_timer_flag = TRUE;
        osal_start_reload_timer(Bsp_Btn_TaskID,BSP_BTN_EVT_SYSTICK,BTN_SYS_TICK);
        bsp_set_key_value_by_index(Bsp_Btn_Get_Index(pin),1);
    }
    else
    {
        bsp_set_key_value_by_index(Bsp_Btn_Get_Index(pin),0);
    }

    #endif
}

void Bsp_Btn_Init( uint8 task_id )
{
    Bsp_Btn_TaskID = task_id;
    #if (BSP_BTN_HARDWARE_CONFIG == BSP_BTN_JUST_GPIO)

    if(bsp_btn_gpio_flag == TRUE)
    {
        ;
    }

    #elif  (BSP_BTN_HARDWARE_CONFIG == BSP_BTN_JUST_KSCAN)

    if(bsp_btn_kscan_flag == TRUE)
    {
        kscan_button_init(task_id);
    }

    #elif  (BSP_BTN_HARDWARE_CONFIG == BSP_BTN_GPIO_AND_KSCAN)

    if((bsp_btn_gpio_flag == TRUE) && (bsp_btn_kscan_flag == TRUE))
    {
        kscan_button_init(task_id);
    }

    #endif
    else
    {
        LOG("btn config error %d %d %d\n",__LINE__,bsp_btn_gpio_flag,bsp_btn_kscan_flag);
        return;
    }

    for(int i = 0; i < BSP_TOTAL_BTN_NUM; i++)
    {
        usr_sum_btn_array[i].KeyConfig = (BSP_BTN_PD_CFG | BSP_BTN_UP_CFG | BSP_BTN_LPS_CFG|BSP_BTN_LPK_CFG);
    }

    #if (BSP_COMBINE_BTN_NUM > 0)

    if(PPlus_SUCCESS != bsp_InitBtn(usr_sum_btn_array,BSP_TOTAL_BTN_NUM,BSP_SINGLE_BTN_NUM,usr_combine_btn_array))
    #else
    if(PPlus_SUCCESS != bsp_InitBtn(usr_sum_btn_array,BSP_TOTAL_BTN_NUM,0,NULL))
    #endif
    {
        LOG("bsp button init error\n");
    }

    //hal_pwrmgr_register(MOD_USR8, NULL, wake_test);
}

uint16 Bsp_Btn_ProcessEvent( uint8 task_id, uint16 events )
{
    uint8_t ucKeyCode;

    if(Bsp_Btn_TaskID != task_id)
    {
        return 0;
    }

    #if ((BSP_BTN_HARDWARE_CONFIG == BSP_BTN_JUST_KSCAN) || (BSP_BTN_HARDWARE_CONFIG == BSP_BTN_GPIO_AND_KSCAN))

    if ( events & KSCAN_WAKEUP_TIMEOUT_EVT )
    {
        hal_kscan_timeout_handler();
        return (events ^ KSCAN_WAKEUP_TIMEOUT_EVT);
    }

    #endif

    if ( events & BSP_BTN_EVT_SYSTICK )
    {
//      if((bsp_btn_counter%10)==0)
//          LOG(".:%d ",bsp_btn_counter);
        ucKeyCode = bsp_KeyPro();

        if(ucKeyCode != BTN_NONE)
        {
            Bsp_Btn_Check(ucKeyCode);
        }

        return (events ^ BSP_BTN_EVT_SYSTICK);
    }

    return 0;
}
