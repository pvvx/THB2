/**************************************************************
    Module Name: bsp_gpio
    File name:   bsp_gpio.c
    Brief description:
    key driver module
    Data:    2020-06-30
    Revision:V0.01

    SDK_LICENSE

****************************************************************/
#include "bsp_gpio.h"

extern void gpio_btn_pin_event_handler(gpio_pin_e pin,IO_Wakeup_Pol_e type);

static Gpio_Btn_Info* s_gpio_btn_ptr = NULL;
int hal_gpio_btn_init(Gpio_Btn_Info* gpio_btn_ptr)
{
    if((gpio_btn_ptr == NULL)||(GPIO_SINGLE_BTN_NUM == 0))
    {
        return PPlus_ERR_INVALID_PARAM;
    }

    if((gpio_btn_ptr->s_key == NULL) || (gpio_btn_ptr->cb == NULL))
    {
        return PPlus_ERR_INVALID_PARAM;
    }

    if(GPIO_SINGLE_BTN_NUM != sizeof(gpio_btn_ptr->s_key)/sizeof(gpio_btn_ptr->s_key[0]))
    {
        return PPlus_ERR_INVALID_PARAM;
    }

    for(int i = 0; i < GPIO_SINGLE_BTN_NUM; i++)
    {
        if(GPIO_SINGLE_BTN_IDLE_LEVEL == 0)
        {
            hal_gpio_pull_set(*(gpio_btn_ptr->s_key+i),PULL_DOWN);
        }
        else
        {
            hal_gpio_pull_set(*(gpio_btn_ptr->s_key+i),WEAK_PULL_UP);
        }

        hal_gpioin_register(*(gpio_btn_ptr->s_key+i),gpio_btn_pin_event_handler, gpio_btn_pin_event_handler);
    }

    s_gpio_btn_ptr = gpio_btn_ptr;
    return PPlus_SUCCESS;
}

int hal_gpio_btn_get_index(gpio_pin_e pin,uint8_t* index)
{
    if(s_gpio_btn_ptr == NULL)
    {
        return PPlus_ERR_NOT_SUPPORTED;
    }

    for(int i = 0; i < GPIO_SINGLE_BTN_NUM; i++)
    {
        if(pin == s_gpio_btn_ptr->s_key[i])
        {
            *index = i;
            return PPlus_SUCCESS;
        }
    }

    return PPlus_ERR_NOT_FOUND;
}

void hal_gpio_btn_cb(uint8_t ucKeyCode)
{
    if(s_gpio_btn_ptr != NULL)
    {
        s_gpio_btn_ptr->cb(ucKeyCode);
    }
}
