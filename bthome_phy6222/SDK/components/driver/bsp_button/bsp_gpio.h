/**************************************************************
    Module Name: bsp_gpio
    File name:   bsp_gpio.h
    Brief description:
    key driver module
    Data:    2020-06-30
    Revision:V0.01

    SDK_LICENSE

****************************************************************/

#ifndef __KEY_H__
#define __KEY_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "error.h"
#include "gpio.h"

#define GPIO_SINGLE_BTN_NUM           3
#define GPIO_SINGLE_BTN_IDLE_LEVEL    1

typedef void (*gpio_btn_callback_t)(uint8_t evt);

typedef struct _Gpio_Btn_Info
{
    gpio_pin_e          s_key[GPIO_SINGLE_BTN_NUM];
    gpio_btn_callback_t cb;
} Gpio_Btn_Info;

int hal_gpio_btn_init(Gpio_Btn_Info* gpio_btn_ptr);

int hal_gpio_btn_get_index(gpio_pin_e pin,uint8_t* index);

void hal_gpio_btn_cb(uint8_t ucKeyCode);

#ifdef __cplusplus
}
#endif

#endif
