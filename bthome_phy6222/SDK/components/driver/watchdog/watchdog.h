/*************
 watchdog.h
 SDK_LICENSE
***************/
#ifndef __WATCHDOG_H__
#define __WATCHDOG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"
#include "bus_dev.h"

#define WDG_2S   0
#define WDG_4S   1
#define WDG_8S   2
#define WDG_16S  3
#define WDG_32S  4
#define WDG_64S  5
#define WDG_128S 6
#define WDG_256S 7

#define WDG_USE_POLLING_MODE 0//this mode is recommended
#define WDG_USE_INT_MODE     1

#define HAL_WDG_CFG_MODE        WDG_USE_POLLING_MODE
/*
    hal watchdog init function.it will be regist in wakeupinit .
    watchdog will be restored in wakeup process
*/
__ATTR_SECTION_SRAM__ void hal_watchdog_init(void);

/*
    watchdog interrupt function.
    in this function,feed watchdog and clear int flag.
*/
void hal_WATCHDOG_IRQHandler(void);


/*
    watchdog feed function.
    we also can feed watchdog in our code.
    for example,if disable all int for a long time,but we want to avoid the watchdog reset.
    in most case,it is not needed.
*/
void hal_watchdog_feed(void);

/*
    watchdog init function.it runs in polling mode.
    if use watchdog,please init it in main before system run,valid parameter 0~7.
    if not,do not init in main.
*/
int watchdog_config(uint8 cycle);

#ifdef __cplusplus
}
#endif

#endif
