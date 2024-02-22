/*************
 watchdog.c
 SDK_LICENSE
***************/
#include "watchdog.h"
#include "error.h"
#include "clock.h"
#include "jump_function.h"

extern volatile uint8 g_clk32K_config;
extern uint32_t s_config_swClk1;
uint8_t g_wdt_cycle = 0xFF;//valid value:0~7.0xFF:watchdog disable.

void hal_WATCHDOG_IRQHandler(void)
{
//    volatile uint32_t a;
//    a = AP_WDT->EOI;
	AP_WDT->EOI;
    AP_WDT->CRR = 0x76;
    //LOG("WDT IRQ[%08x]\n",rtc_get_counter());
}

__ATTR_SECTION_SRAM__ void hal_watchdog_init(void)
{
    //volatile uint32_t a;
	volatile uint8_t delay;

    if(g_wdt_cycle > 7)
        return ;

    if(g_clk32K_config == CLK_32K_XTAL)//rtc use 32K XOSC,watchdog use the same
    {
        AP_PCRM->CLKSEL |= (1UL<<16);
    }
    else
    {
        AP_PCRM->CLKSEL &= ~(1UL<<16); //rtc use 32K RCOSC,watchdog use the same
    }

    hal_clk_gate_enable(MOD_WDT);
    s_config_swClk1|=_CLK_WDT; //add watchdog clk in pwrmg wakeup restore clk;

    if((AP_PCR->SW_RESET0 & 0x04)==0)
    {
        AP_PCR->SW_RESET0 |= 0x04;
        delay = 20;

        while(delay > 0)
        	delay--;
    }

    if((AP_PCR->SW_RESET2 & 0x04)==0)
    {
        AP_PCR->SW_RESET2 |= 0x04;
        delay=20;

        while(delay-->0);
    }

    AP_PCR->SW_RESET2 &= ~0x20;
    delay=20;

    while(delay > 0)
    	delay--;

    AP_PCR->SW_RESET2 |= 0x20;
    delay=20;

    while(delay > 0)
    	delay--;

//    a = AP_WDT->EOI;
    AP_WDT->EOI;
    AP_WDT->TORR = g_wdt_cycle;
#if (HAL_WDG_CFG_MODE==WDG_USE_INT_MODE)
    NVIC_SetPriority((IRQn_Type)WDT_IRQn, IRQ_PRIO_HAL);
    NVIC_EnableIRQ((IRQn_Type)WDT_IRQn);
    JUMP_FUNCTION(WDT_IRQ_HANDLER) = (uint32_t)&hal_WATCHDOG_IRQHandler;
    AP_WDT->CR = 0x1F;//use int
#else
    AP_WDT->CR = 0x1D;//not use int
#endif
    AP_WDT_FEED;
}

void hal_watchdog_feed(void)
{
    AP_WDT_FEED;
}

int watchdog_config(uint8 cycle)
{
    if(cycle > 7)
        return PPlus_ERR_INVALID_PARAM;
    else
        g_wdt_cycle = cycle;

    hal_watchdog_init();
    JUMP_FUNCTION(HAL_WATCHDOG_INIT) = (uint32_t)&hal_watchdog_init;
    return PPlus_SUCCESS;
}
