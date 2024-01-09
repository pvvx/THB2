/**************************************************************
    Module Name: key
    File name:   key.c
    Brief description:
    key driver module
    Data:    2020-06-30
    Revision:V0.01

  SDK_LICENSE

****************************************************************/
#include "rom_sym_def.h"
#include "key.h"
#include "log.h"
#include "OSAL.h"
#include "pwrmgr.h"
#include "error.h"

key_contex_t key_state;

//extern uint32 getMcuPrecisionCount(void);

static int key_timer_start(uint32 intval_ms)
{
    osal_start_timerEx(key_state.task_id, HAL_KEY_EVENT, intval_ms);
    return 0;
}

static void key_idle_handler(uint8 i,IO_Wakeup_Pol_e type)
{
    if(((type == NEGEDGE) && (key_state.key[i].idle_level == HAL_HIGH_IDLE)) ||
            ((type == POSEDGE) && (key_state.key[i].idle_level == HAL_LOW_IDLE)))
    {
        hal_pwrmgr_lock(MOD_USR1);
        key_state.key[i].state = HAL_STATE_KEY_PRESS_DEBOUNCE;
        key_state.temp[i].in_enable = TRUE;
        key_timer_start(HAL_KEY_DEBOUNCD);
    }
}

static void key_press_debonce_handler(uint8 i,IO_Wakeup_Pol_e type)
{
    if(((type == NEGEDGE) && (key_state.key[i].idle_level == HAL_HIGH_IDLE)) ||
            ((type == POSEDGE) && (key_state.key[i].idle_level == HAL_LOW_IDLE)))
    {
        key_timer_start(HAL_KEY_DEBOUNCD);
    }
}

static void key_press_handler(uint8 i,IO_Wakeup_Pol_e type)
{
    if(((type == POSEDGE) && (key_state.key[i].idle_level == HAL_HIGH_IDLE)) ||
            ((type == NEGEDGE) && (key_state.key[i].idle_level == HAL_LOW_IDLE)))
    {
        hal_pwrmgr_lock(MOD_USR1);
        key_state.key[i].state = HAL_STATE_KEY_RELEASE_DEBOUNCE;
        key_timer_start(HAL_KEY_DEBOUNCD);
    }
}

static void key_release_debonce_handler(uint8 i,IO_Wakeup_Pol_e type)
{
    if(((type == POSEDGE) && (key_state.key[i].idle_level == HAL_HIGH_IDLE)) ||
            ((type == NEGEDGE) && (key_state.key[i].idle_level == HAL_LOW_IDLE)))
    {
        key_timer_start(HAL_KEY_DEBOUNCD);
    }
}

static void pin_event_handler(gpio_pin_e pin,IO_Wakeup_Pol_e type)
{
    uint8 i;

    for(i = 0; i < HAL_KEY_NUM; i++)
    {
        if(pin == key_state.key[i].pin)
            break;
    }

    if(i < HAL_KEY_NUM)
    {
        switch(key_state.key[i].state)
        {
        case HAL_STATE_KEY_IDLE:
            key_idle_handler(i,type);
            break;

        case HAL_STATE_KEY_PRESS_DEBOUNCE:
            key_press_debonce_handler(i,type);
            break;

        case HAL_STATE_KEY_PRESS:
            key_press_handler(i,type);
            break;

        case HAL_STATE_KEY_RELEASE_DEBOUNCE:
            key_release_debonce_handler(i,type);
            break;

        default:
            break;
        }
    }
}

void key_init(void)
{
    uint8 i;

    for(i = 0; i < HAL_KEY_NUM; ++i)
    {
        if(key_state.key[i].idle_level == HAL_LOW_IDLE)
        {
            hal_gpio_pull_set(key_state.key[i].pin,PULL_DOWN);
        }
        else
        {
            hal_gpio_pull_set(key_state.key[i].pin,WEAK_PULL_UP);
        }

        key_state.temp[i].timer_tick = 0;
        hal_gpioin_register(key_state.key[i].pin, pin_event_handler, pin_event_handler);
    }

    hal_pwrmgr_register(MOD_USR1, NULL, NULL);
}

static void key_press_debonce_timer_handler(uint8 i)
{
    if(((hal_gpio_read(key_state.key[i].pin) == FALSE) && (key_state.key[i].idle_level == HAL_HIGH_IDLE)) ||
            ((hal_gpio_read(key_state.key[i].pin) == TRUE) && (key_state.key[i].idle_level == HAL_LOW_IDLE)))
    {
        #ifdef HAL_KEY_SUPPORT_LONG_PRESS
        osal_start_timerEx(key_state.task_id,KEY_DEMO_LONG_PRESS_EVT,HAL_KEY_LONG_PRESS_TIME);
        #endif
        hal_pwrmgr_unlock(MOD_USR1);
        key_state.key[i].state = HAL_STATE_KEY_PRESS;
        key_state.temp[i].timer_tick = getMcuPrecisionCount();

        if(key_state.key_callbank != NULL)
        {
            key_state.key_callbank(i,HAL_KEY_EVT_PRESS);
        }
    }
    else
    {
        key_state.key[i].state = HAL_STATE_KEY_IDLE;
        key_state.temp[i].in_enable = FALSE;
    }
}

static void key_release_debonce_timer_handler(uint8 i)
{
    if(key_state.key[i].idle_level == hal_gpio_read(key_state.key[i].pin))
    {
        osal_stop_timerEx(key_state.task_id,HAL_KEY_EVT_LONG_PRESS);
        uint32_t hold_tick = (getMcuPrecisionCount() - key_state.temp[i].timer_tick)*625;
        hal_pwrmgr_unlock(MOD_USR1);

        if(key_state.key_callbank != NULL)
        {
            #ifdef HAL_KEY_SUPPORT_LONG_PRESS

            if(hold_tick >= (HAL_KEY_LONG_PRESS_TIME * 1000))//2s
            {
                key_state.key_callbank(i,HAL_KEY_EVT_LONG_RELEASE);
                key_state.key[i].state = HAL_STATE_KEY_IDLE;
            }
            else
            #endif
            {
                key_state.key_callbank(i,HAL_KEY_EVT_RELEASE);
                key_state.key[i].state = HAL_STATE_KEY_IDLE;
            }
        }
    }
    else
    {
        key_state.key[i].state = HAL_STATE_KEY_PRESS;
    }
}

void gpio_key_timer_handler(uint8 i)
{
    switch(key_state.key[i].state)
    {
    case HAL_STATE_KEY_PRESS_DEBOUNCE:
        key_press_debonce_timer_handler(i);
        break;

    case HAL_STATE_KEY_RELEASE_DEBOUNCE:
        key_release_debonce_timer_handler(i);
        break;

    default:
        break;
    }
}

