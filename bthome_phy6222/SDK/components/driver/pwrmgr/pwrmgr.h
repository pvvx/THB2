/************
	pwrmgr.c
	SDK_LICENSE
**************/

#ifndef _HAL_PWRMGR_HD
#define _HAL_PWRMGR_HD

#ifdef __cplusplus
extern "C" {
#endif


#include "bus_dev.h"
#include "gpio.h"
#include "clock.h"

#define PWR_MODE_NO_SLEEP			1
#define PWR_MODE_SLEEP				2
#define PWR_MODE_PWROFF_NO_SLEEP	4

//WAKEUP FROM STANDBY MODE
#define WAKEUP_PIN_MAX	 3

#define HAL_PWRMGR_TASK_MAX_NUM		10

#define	  RET_SRAM0			BIT(0)	/*32K, 0x1fff0000~0x1fff7fff*/
#define	  RET_SRAM1			BIT(1)	/*16K, 0x1fff8000~0x1fffbfff*/
#define	  RET_SRAM2			BIT(2)	/*16K, 0x1fffc000~0x1fffffff*/

#define	  DEF_CLKG_CONFIG_0		  (_CLK_IOMUX|_CLK_UART0|_CLK_GPIO|_CLK_SPIF|_CLK_DMA|_CLK_TIMER5)

#define	  DEF_CLKG_CONFIG_1		  (_CLK_M0_CPU | _CLK_BB |_CLK_TIMER |_CLK_BBREG \
								   |_CLK_TIMER1|_CLK_TIMER2|_CLK_TIMER3|_CLK_TIMER4|_CLK_COM)

typedef struct
{
	gpio_pin_e pin;
	gpio_polarity_e type;
	uint16_t on_time;
} pwroff_cfg_t;

typedef struct
{
	uint8_t sramRet_config:3;
	uint8_t moudle_num:5;
} PWRMGR_CFG_BIT;

#define CHIP_RETENTION_FLG_FLASH_ADDRESS 0x1100181c

extern uint32_t g_system_reset_cause;
extern sysclk_t g_system_clk_change;

typedef void (*pwrmgr_Hdl_t)(void);

int hal_pwrmgr_init(void);
bool hal_pwrmgr_is_lock(MODULE_e mod);
int hal_pwrmgr_lock(MODULE_e mod);
int hal_pwrmgr_unlock(MODULE_e mod);
int hal_pwrmgr_register(MODULE_e mod, pwrmgr_Hdl_t sleepHandle, pwrmgr_Hdl_t wakeupHandle);
int hal_pwrmgr_unregister(MODULE_e mod);
int hal_pwrmgr_wakeup_process(void) __attribute__((weak));
int hal_pwrmgr_sleep_process(void) __attribute__((weak));
int hal_pwrmgr_RAM_retention(uint32_t sram);
int hal_pwrmgr_clk_gate_config(MODULE_e module);
int hal_pwrmgr_RAM_retention_clr(void);
int hal_pwrmgr_RAM_retention_set(void);
int hal_pwrmgr_LowCurrentLdo_enable(void);
int hal_pwrmgr_LowCurrentLdo_disable(void);
int hal_pwrmgr_get_module_lock_status(void);

void hal_pwrmgr_poweroff(pwroff_cfg_t* pcfg, uint8_t wakeup_pin_num);
__ATTR_SECTION_SRAM__ void hal_pwrmgr_enter_sleep_rtc_reset(uint32_t sleepRtcTick);
void hal_pwrmgr_enter_standby(pwroff_cfg_t* pcfg,uint8_t wakeup_pin_num) ;

void clk_change_mod_restore(void);
extern uint8_t hal_system_clock_change_req(sysclk_t clk);
extern uint8_t hal_system_clock_change_active(sysclk_t clk,pwrmgr_Hdl_t restoreHandle);

#ifdef __cplusplus
}
#endif


#endif


