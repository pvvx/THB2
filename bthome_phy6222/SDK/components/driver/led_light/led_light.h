/*************
 led_light.c
 SDK_LICENSE
***************/

#ifndef _LED_LIGHT_H
#define _LED_LIGHT_H

#include "types.h"
#include "gpio.h"

#define LIGHT_TOP_VALUE                 256
#define LIGHT_TURN_ON                  (LIGHT_TOP_VALUE-1)
#define LIGHT_TURN_OFF                  0


#define LIGHT_BLINK_EXTRA_FAST          1
#define LIGHT_BLINK_FAST                3
#define LIGHT_BLINK_SLOW                10
#define LIGHT_BLINK_EXTRA_SLOW          30


#define LIGHT_GREEN          0
#define LIGHT_BLUE           1
#define LIGHT_RED            2

typedef struct
{
    uint8_t light;
    uint8_t curCnt;
    uint8_t tagCnt;
    uint8_t intv;
    uint16_t val0;
    uint16_t val1;
    uint8_t status;
    uint8_t task_id;
    uint16_t event_id;
} light_blink_cfg_t;

typedef enum
{
    LIGHT_COLOR_OFF = 0,
    LIGHT_COLOR_RED=1,
    LIGHT_COLOR_GREEN,
    LIGHT_COLOR_BLUE,
    LIGHT_COLOR_CYAN,
    LIGHT_COLOR_YELLOW,
    LIGHT_COLOR_MEGENTA,
    LIGHT_COLOR_WHITE,
    LIGHT_COLOR_NUM
} light_color_t;
static light_blink_cfg_t s_lightBlink;

void light_timeout_handle(void);
int light_ctrl(uint8_t ch, uint16_t value);

int light_config(uint8_t ch, uint16_t value);
int light_init(gpio_pin_e* pin_ptr,uint16_t pin_num);
void light_pwm_init(void);
void light_pwm_deinit(void);
void light_reflash(void);
int light_blink_evt_cfg(uint8_t task_id,uint16_t event_id);
int light_blink_set(uint8_t light,uint8 blinkIntv,uint8 blinkCnt);
void light_blink_porcess_evt(void);
void light_color_quickSet(light_color_t color);


#define LIGHT_ONLY_RED_ON       \
    { \
        light_config(LIGHT_RED    ,LIGHT_TURN_ON);\
        light_config(LIGHT_GREEN  ,LIGHT_TURN_OFF);\
        light_config(LIGHT_BLUE   ,LIGHT_TURN_OFF);\
        light_reflash();\
        \
    }

#define LIGHT_ONLY_GREEN_ON     \
    { \
        light_config(LIGHT_RED    ,LIGHT_TURN_OFF);\
        light_config(LIGHT_GREEN  ,LIGHT_TURN_ON);\
        light_config(LIGHT_BLUE   ,LIGHT_TURN_OFF);\
        light_reflash();\
        \
    }

#define LIGHT_ONLY_BLUE_ON      \
    { \
        light_config(LIGHT_RED    ,LIGHT_TURN_OFF);\
        light_config(LIGHT_GREEN  ,LIGHT_TURN_OFF);\
        light_config(LIGHT_BLUE   ,LIGHT_TURN_ON);\
        light_reflash();\
        \
    }

#define LIGHT_ON_OFF(r,g,b)      \
    { \
        light_config(LIGHT_RED    ,r);\
        light_config(LIGHT_GREEN  ,g);\
        light_config(LIGHT_BLUE   ,b);\
        light_reflash();\
        \
    }

#define LIGHT_ON_CYAN      \
    { \
        light_config(LIGHT_RED    ,0);\
        light_config(LIGHT_GREEN  ,255);\
        light_config(LIGHT_BLUE   ,255);\
        light_reflash();\
        \
    }

#define LIGHT_ON_YELLOW      \
    { \
        light_config(LIGHT_RED    ,255);\
        light_config(LIGHT_GREEN  ,255);\
        light_config(LIGHT_BLUE   ,0);\
        light_reflash();\
        \
    }

#define LIGHT_ON_MEGENTA      \
    { \
        light_config(LIGHT_RED    ,255);\
        light_config(LIGHT_GREEN  ,0);\
        light_config(LIGHT_BLUE   ,255);\
        light_reflash();\
        \
    }


#define LIGHT_ON_WHITE      \
    { \
        light_config(LIGHT_RED    ,255);\
        light_config(LIGHT_GREEN  ,255);\
        light_config(LIGHT_BLUE   ,255);\
        light_reflash();\
        \
    }
#endif

