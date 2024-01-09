/*************
 led_light.h
 SDK_LICENSE
***************/

#include "led_light.h"
#include "pwm.h"
#include "OSAL.h"
#include "gpio.h"
#include "error.h"

static uint16_t s_light[3];
static light_blink_cfg_t s_lightBlink;

static pwm_ch_t pwm_ch[3];
static gpio_pin_e* led_pin_ptr = NULL;
static uint16_t led_pin_num = 0;

static void light_start_timer(void)
{
    //osal_start_timerEx(AppWrist_TaskID, TIMER_LIGHT_EVT, 30*1000);
}
static void light_stop_timer(void)
{
    //osal_stop_timerEx(AppWrist_TaskID, TIMER_LIGHT_EVT);
}

void light_reflash(void)
{
    pwm_ch[0].cmpVal = s_light[0];
    pwm_ch[1].cmpVal = s_light[1];
    pwm_ch[2].cmpVal = s_light[2];
    hal_pwm_ch_start(pwm_ch[0]);
    hal_pwm_ch_start(pwm_ch[1]);
    hal_pwm_ch_start(pwm_ch[2]);

    if(s_light[LIGHT_RED] + s_light[LIGHT_GREEN] + s_light[LIGHT_BLUE])
    {
        light_stop_timer();
        light_start_timer();
    }
    else
    {
        light_stop_timer();
    }
}

void light_timeout_handle(void)
{
//    s_light[0] = 0;
//    s_light[1] = 0;
//    s_light[2] = 0;
//    hal_pwm_close_channel(PWM_CH0);
//    hal_pwm_destroy(PWM_CH0);
//    hal_pwm_close_channel(PWM_CH1);
//    hal_pwm_destroy(PWM_CH1);
//    hal_pwm_close_channel(PWM_CH2);
//    hal_pwm_destroy(PWM_CH2);
//    hal_pwm_stop();
//    hal_gpio_pin_init(GPIO_GREEN, IE);
//    hal_gpio_pin_init(GPIO_RED, IE);
//    hal_gpio_pin_init(GPIO_BLUE, IE);
//    hal_gpio_pull_set(GPIO_GREEN, WEAK_PULL_UP);
//    hal_gpio_pull_set(GPIO_RED, WEAK_PULL_UP);
//    hal_gpio_pull_set(GPIO_BLUE, WEAK_PULL_UP);
}

int light_config(uint8_t ch, uint16_t value)
{
    if(ch >2 || (value > LIGHT_TOP_VALUE))
    {
        return PPlus_ERR_INVALID_PARAM;
    }

    s_light[ch] = (uint16_t)value;
    return PPlus_SUCCESS;
}

int light_ctrl(uint8_t ch, uint16_t value)
{
    if(ch >2 || (value > LIGHT_TOP_VALUE))
    {
        return PPlus_ERR_INVALID_PARAM;
    }

    s_light[ch] = (uint16_t)value;
    light_reflash();
    return PPlus_SUCCESS;
}

static void led_init(gpio_pin_e* pin_ptr,uint16_t pin_num)
{
    gpio_pin_e pin;

    for(int i = 0; i < pin_num; i++)
    {
        pin = *(pin_ptr + i);
        hal_gpio_pin_init(pin,IE);
        hal_gpio_pull_set(pin,WEAK_PULL_UP);
    }
}

int light_init(gpio_pin_e* pin_ptr,uint16_t pin_num)
{
    if((pin_ptr == NULL) || (pin_num == 0))
    {
        return PPlus_ERR_INVALID_PARAM;
    }
    else
    {
        led_pin_ptr = pin_ptr;
        led_pin_num = pin_num;
    }

    led_init(led_pin_ptr,led_pin_num);
    s_light[LIGHT_GREEN] = 0;
    s_light[LIGHT_BLUE] = 0;
    s_light[LIGHT_RED] = 0;
    osal_memset(&s_lightBlink, 0, sizeof(s_lightBlink));
    s_lightBlink.val0 = LIGHT_TURN_OFF;
    s_lightBlink.val1 = LIGHT_TURN_ON;
    light_pwm_init();
    return PPlus_SUCCESS;
}

void light_pwm_init(void)
{
    hal_pwm_module_init();

    for(unsigned int i = 0; i < sizeof(pwm_ch)/sizeof(pwm_ch[0]); i++)
    {
        pwm_ch[i].pwmN = (PWMN_e)(PWM_CH0 + (PWMN_e)i);
        pwm_ch[i].pwmPin = GPIO_DUMMY;
        pwm_ch[i].pwmDiv = PWM_CLK_NO_DIV;
        pwm_ch[i].pwmMode = PWM_CNT_UP;
        pwm_ch[i].pwmPolarity = PWM_POLARITY_RISING;
        pwm_ch[i].cmpVal = 0;
        pwm_ch[i].cntTopVal = LIGHT_TOP_VALUE;
    }

    pwm_ch[0].pwmPin = *(led_pin_ptr + 0);
    pwm_ch[1].pwmPin = *(led_pin_ptr + 1);
    pwm_ch[2].pwmPin = *(led_pin_ptr + 2);
}

void light_pwm_deinit(void)
{
    hal_pwm_ch_stop(pwm_ch[0]);
    hal_pwm_ch_stop(pwm_ch[1]);
    hal_pwm_ch_stop(pwm_ch[2]);
    hal_pwm_module_deinit();
}

int light_blink_evt_cfg(uint8_t task_id,uint16_t event_id)
{
    if(s_lightBlink.status == 0)
    {
        s_lightBlink.task_id = task_id;
        s_lightBlink.event_id = event_id;
        return PPlus_SUCCESS;
    }
    else
    {
        return PPlus_ERR_BUSY;
    }
}
int light_blink_set(uint8_t light,uint8 blinkIntv,uint8 blinkCnt)
{
    if(s_lightBlink.status == 0)
    {
        s_lightBlink.light = light;
        s_lightBlink.tagCnt = blinkCnt;
        s_lightBlink.intv = blinkIntv;
        s_lightBlink.status = 1;

        if(s_lightBlink.task_id > 0 && s_lightBlink.event_id > 0)
        {
            light_ctrl(LIGHT_RED,0);
            light_ctrl(LIGHT_GREEN,0);
            light_ctrl(LIGHT_BLUE,0);
            s_lightBlink.curCnt = 0;
            osal_set_event(s_lightBlink.task_id, s_lightBlink.event_id);
        }
        else
        {
            return PPlus_ERR_NOT_FOUND;
        }

        return PPlus_SUCCESS;
    }
    else
    {
        return PPlus_ERR_BUSY;
    }
}
void light_blink_porcess_evt(void)
{
    if(s_lightBlink.curCnt == (s_lightBlink.tagCnt*2) )
    {
        light_ctrl(LIGHT_RED,0);
        light_ctrl(LIGHT_GREEN,0);
        light_ctrl(LIGHT_BLUE,0);
        osal_stop_timerEx( s_lightBlink.task_id, s_lightBlink.event_id);
        s_lightBlink.status = 0;
    }
    else
    {
        if(s_lightBlink.curCnt&0x01)
        {
            light_ctrl(s_lightBlink.light,s_lightBlink.val1);
        }
        else
        {
            light_ctrl(s_lightBlink.light,s_lightBlink.val0);
        }

        s_lightBlink.curCnt++;
        osal_start_timerEx(s_lightBlink.task_id, s_lightBlink.event_id,s_lightBlink.intv*100);
    }
}
void light_color_quickSet(light_color_t color)
{
    switch ( color )
    {
    case LIGHT_COLOR_OFF:
        LIGHT_ON_OFF(0,0,0);
        break;

    case LIGHT_COLOR_RED:
        LIGHT_ONLY_RED_ON;
        break;

    case LIGHT_COLOR_GREEN:
        LIGHT_ONLY_GREEN_ON;
        break;

    case LIGHT_COLOR_BLUE:
        LIGHT_ONLY_BLUE_ON;
        break;

    case LIGHT_COLOR_CYAN:
        LIGHT_ON_CYAN;
        break;

    case LIGHT_COLOR_YELLOW:
        LIGHT_ON_YELLOW;
        break;

    case LIGHT_COLOR_MEGENTA:
        LIGHT_ON_MEGENTA;
        break;

    case LIGHT_COLOR_WHITE:
        LIGHT_ON_WHITE;
        break;

    default:
        break;
    }
}
