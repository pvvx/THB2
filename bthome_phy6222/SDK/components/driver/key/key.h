/**************************************************************
    Module Name: key
    File name:   key.h
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


#include "types.h"
#include "gpio.h"

#define HAL_KEY_NUM                     1      //config key's number
#define HAL_KEY_EVENT                   0x4000 //assign short key event in your app event process

#define HAL_KEY_SUPPORT_LONG_PRESS      //if use long key,please enable it
#ifdef  HAL_KEY_SUPPORT_LONG_PRESS
#define KEY_DEMO_LONG_PRESS_EVT         0x8000 //if use long key,assign long key event in your app process
#define HAL_KEY_LONG_PRESS_TIME     3000    //2s
#endif

#define HAL_KEY_DEBOUNCD                20      //20ms

typedef enum
{
    HAL_STATE_KEY_IDLE              = 0x00,
    HAL_STATE_KEY_PRESS_DEBOUNCE    = 0x01,
    HAL_STATE_KEY_PRESS             = 0x02,
    HAL_STATE_KEY_RELEASE_DEBOUNCE  = 0x03,
} key_state_e;

typedef enum
{
    HAL_KEY_EVT_IDLE            = 0x0000,
    HAL_KEY_EVT_PRESS           = 0x0002,
    HAL_KEY_EVT_RELEASE         = 0x0004,
    HAL_KEY_EVT_LONG_PRESS      = 0x0010,
    HAL_KEY_EVT_LONG_RELEASE    = 0x0020,
} key_evt_t;

typedef enum
{
    HAL_LOW_IDLE    = 0x00,
    HAL_HIGH_IDLE   = 0x01,
} idle_level_e;

typedef void (* key_callbank_hdl_t)(uint8_t,key_evt_t);

typedef struct gpio_key_t
{
    gpio_pin_e          pin;
    key_state_e         state;
    idle_level_e        idle_level;

} gpio_key;

typedef struct gpio_internal_t
{
    uint32_t        timer_tick;
    bool            in_enable;

} gpio_internal;

typedef struct key_state
{
    gpio_key                key[HAL_KEY_NUM];
    gpio_internal           temp[HAL_KEY_NUM];
    uint8_t                 task_id;
    key_callbank_hdl_t      key_callbank;

} key_contex_t;


void key_init(void);
void gpio_key_timer_handler(uint8 index);
extern key_contex_t key_state;

#ifdef __cplusplus
}
#endif


#endif

