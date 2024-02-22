/************
	pwrmgr.c
	SDK_LICENSE
**************/
#include "types.h"
#include "rom_sym_def.h"
#include "ll_sleep.h"
#include "bus_dev.h"
#include "string.h"

#include "pwrmgr.h"
#include "error.h"
#include "gpio.h"
#include "log.h"
#include "clock.h"
#include "jump_function.h"
#include "flash.h"
#include "rf_phy_driver.h"

#if(CFG_SLEEP_MODE == PWR_MODE_NO_SLEEP)
	static uint8_t mPwrMode = PWR_MODE_NO_SLEEP;
#elif(CFG_SLEEP_MODE == PWR_MODE_SLEEP)
	static uint8_t mPwrMode = PWR_MODE_SLEEP;
#elif(CFG_SLEEP_MODE == PWR_MODE_PWROFF_NO_SLEEP)
	static uint8_t mPwrMode = PWR_MODE_PWROFF_NO_SLEEP;
#else
	#error "CFG_SLEEP_MODE define incorrect"
#endif

//#define CFG_FLASH_ENABLE_DEEP_SLEEP
#ifdef CFG_FLASH_ENABLE_DEEP_SLEEP
	#warning "CONFIG FLASH ENABLE DEEP SLEEP !!!"
#endif

#define CFG_SRAM_RETENTION_LOW_CURRENT_LDO_ENABLE
#ifdef CFG_SRAM_RETENTION_LOW_CURRENT_LDO_ENABLE
//	#warning "ENABLE LOW CURRENT LDO FOR SRAM RETENTION !!!"
#endif

//#define CFG_HCLK_DYNAMIC_CHANGE
#if(CFG_HCLK_DYNAMIC_CHANGE)
	#warning "ENABLE CFG_HCLK_DYNAMIC_CHANGE !!!"
#endif
typedef struct _pwrmgr_Context_t
{
	MODULE_e	 moudle_id;
	bool		 lock;
	pwrmgr_Hdl_t sleep_handler;
	pwrmgr_Hdl_t wakeup_handler;
} pwrmgr_Ctx_t;

static pwrmgr_Ctx_t mCtx[HAL_PWRMGR_TASK_MAX_NUM];
static PWRMGR_CFG_BIT s_pwrmgr_cfg;
static uint32_t s_config_swClk0 = DEF_CLKG_CONFIG_0;

uint32_t s_config_swClk1 = DEF_CLKG_CONFIG_1;

#if(CFG_SLEEP_MODE == PWR_MODE_SLEEP)
	uint32_t s_gpio_wakeup_src_group1,s_gpio_wakeup_src_group2;
	extern void config_RTC1(uint32 time);
#endif

// /*
//	   osal_idle_task will be call
// */
// extern void osal_pwrmgr_powerconserve1( void );
// __ATTR_SECTION_SRAM__ void osal_idle_task (void)
// {
//	   AP_WDT_FEED;
//	   osal_pwrmgr_powerconserve1();
// }
int hal_pwrmgr_init(void)
{
	memset(&mCtx, 0, sizeof(mCtx));

	switch(mPwrMode)
	{
	case PWR_MODE_NO_SLEEP:
	case PWR_MODE_PWROFF_NO_SLEEP:
		disableSleep();
		break;

	case PWR_MODE_SLEEP:
		enableSleep();
		break;
	}

	// /*
	//	   if wdt enable, set osal idle task to feed wdt before powerconserve
	// */
	// if(AP_WDT_ENABLE_STATE)
	//	   JUMP_FUNCTION(OSAL_POWER_CONSERVE)=(uint32_t)&osal_idle_task;
	return PPlus_SUCCESS;
}

int hal_pwrmgr_clk_gate_config(MODULE_e module)
{
	if (module < MOD_CP_CPU)
	{
		s_config_swClk0 |= BIT(module);
	}
	else if (module < MOD_PCLK_CACHE)
	{
		s_config_swClk1 |= BIT(module - MOD_CP_CPU);
	}

	return PPlus_SUCCESS;
}

bool hal_pwrmgr_is_lock(MODULE_e mod)
{
	int i;
	int ret = FALSE;

	if(mPwrMode == PWR_MODE_NO_SLEEP || mPwrMode == PWR_MODE_PWROFF_NO_SLEEP )
	{
		return TRUE;
	}

	HAL_ENTER_CRITICAL_SECTION();

	for(i = 0; i< HAL_PWRMGR_TASK_MAX_NUM; i++)
	{
		if(mCtx[i].moudle_id == MOD_NONE)
			break;

		if(mCtx[i].moudle_id == mod)
		{
			if(mCtx[i].lock == TRUE)
				ret = TRUE;

			break;
		}
	}

	HAL_EXIT_CRITICAL_SECTION();
	return ret;
}


int hal_pwrmgr_lock(MODULE_e mod)
{
	int i;
	int ret = PPlus_ERR_NOT_REGISTED;

	if(mPwrMode == PWR_MODE_NO_SLEEP || mPwrMode == PWR_MODE_PWROFF_NO_SLEEP )
	{
		disableSleep();
		return PPlus_SUCCESS;
	}

	HAL_ENTER_CRITICAL_SECTION();

	for(i = 0; i< HAL_PWRMGR_TASK_MAX_NUM; i++)
	{
		if(mCtx[i].moudle_id == MOD_NONE)
			break;

		if(mCtx[i].moudle_id == mod)
		{
			mCtx[i].lock = TRUE;
			disableSleep();
			//LOG("LOCK\n");
			ret = PPlus_SUCCESS;
			break;
		}
	}

	HAL_EXIT_CRITICAL_SECTION();
	return ret;
}

int hal_pwrmgr_unlock(MODULE_e mod)
{
	int i, cnt = 0;

	if(mPwrMode == PWR_MODE_NO_SLEEP || mPwrMode == PWR_MODE_PWROFF_NO_SLEEP )
	{
		disableSleep();
		return PPlus_SUCCESS;
	}

	HAL_ENTER_CRITICAL_SECTION();

	for(i = 0; i< HAL_PWRMGR_TASK_MAX_NUM; i++)
	{
		if(mCtx[i].moudle_id == MOD_NONE)
			break;

		if(mCtx[i].moudle_id == mod)
		{
			mCtx[i].lock = FALSE;
		}

		if(mCtx[i].lock)
			cnt ++;
	}

	if(cnt == 0)
		enableSleep();
	else
		disableSleep();

	HAL_EXIT_CRITICAL_SECTION();
	//LOG("sleep mode:%d\n", isSleepAllow());
	return PPlus_SUCCESS;
}

int hal_pwrmgr_register(MODULE_e mod, pwrmgr_Hdl_t sleepHandle, pwrmgr_Hdl_t wakeupHandle)
{
	int i;
	pwrmgr_Ctx_t* pctx = NULL;

	for(i = 0; i< HAL_PWRMGR_TASK_MAX_NUM; i++)
	{
		if(mCtx[i].moudle_id == mod)
			return PPlus_ERR_INVALID_STATE;

		if(mCtx[i].moudle_id == MOD_NONE)
		{
			pctx = &mCtx[i];
			s_pwrmgr_cfg.moudle_num++;
			break;
		}
	}

	if(pctx == NULL)
		return PPlus_ERR_NO_MEM;

	pctx->lock = FALSE;
	pctx->moudle_id = mod;
	pctx->sleep_handler = sleepHandle;
	pctx->wakeup_handler = wakeupHandle;
	return PPlus_SUCCESS;
}

int hal_pwrmgr_unregister(MODULE_e mod)
{
	int i;
	pwrmgr_Ctx_t* pctx = NULL;

	for(i = 0; i< HAL_PWRMGR_TASK_MAX_NUM; i++)
	{
		if(mCtx[i].moudle_id == mod)
		{
			s_pwrmgr_cfg.moudle_num--;
			pctx = &mCtx[i];
			break;
		}

		if(mCtx[i].moudle_id == MOD_NONE)
		{
			return PPlus_ERR_NOT_REGISTED;
		}
	}

	if(pctx == NULL)
		return PPlus_ERR_NOT_REGISTED;

	HAL_ENTER_CRITICAL_SECTION();
	memcpy(pctx, pctx+1, sizeof(pwrmgr_Ctx_t)*(HAL_PWRMGR_TASK_MAX_NUM-i-1));
	HAL_EXIT_CRITICAL_SECTION();
	return PPlus_SUCCESS;
}

void clk_change_mod_restore(void)
{
	int i;

	for(i = 0; i< HAL_PWRMGR_TASK_MAX_NUM; i++)
	{
		if(mCtx[i].moudle_id == MOD_NONE)
		{
			return ;
		}

		if(mCtx[i].wakeup_handler)
			mCtx[i].wakeup_handler();
	}

	return;
}

#if(CFG_SLEEP_MODE == PWR_MODE_SLEEP)

int __attribute__((used)) hal_pwrmgr_wakeup_process(void)
{
	int i;
#ifdef CFG_FLASH_ENABLE_DEEP_SLEEP
	extern void spif_release_deep_sleep(void);
	spif_release_deep_sleep();
	WaitRTCCount(8);
#endif
	AP_PCR->SW_CLK	= s_config_swClk0;
	AP_PCR->SW_CLK1 = s_config_swClk1|0x01;//force set M0 CPU
	s_gpio_wakeup_src_group1 = AP_AON->GPIO_WAKEUP_SRC[0];
	s_gpio_wakeup_src_group2 = AP_AON->GPIO_WAKEUP_SRC[1];
	//restore BB TIMER IRQ_PRIO
	NVIC_SetPriority((IRQn_Type)BB_IRQn,	IRQ_PRIO_REALTIME);
	NVIC_SetPriority((IRQn_Type)TIM1_IRQn,	IRQ_PRIO_HIGH);		//ll_EVT
	NVIC_SetPriority((IRQn_Type)TIM2_IRQn,	IRQ_PRIO_HIGH);		//OSAL_TICK
	NVIC_SetPriority((IRQn_Type)TIM4_IRQn,	IRQ_PRIO_HIGH);		//LL_EXA_ADV
	//peripheral_interrupt_restore_default();
	NVIC_SetPriority((IRQn_Type)KSCAN_IRQn, IRQ_PRIO_HAL);
	NVIC_SetPriority((IRQn_Type)WDT_IRQn,	IRQ_PRIO_HAL);
	NVIC_SetPriority((IRQn_Type)UART0_IRQn, IRQ_PRIO_HAL);
	NVIC_SetPriority((IRQn_Type)UART1_IRQn, IRQ_PRIO_HAL);
	NVIC_SetPriority((IRQn_Type)I2C0_IRQn,	IRQ_PRIO_HAL);
	NVIC_SetPriority((IRQn_Type)I2C1_IRQn,	IRQ_PRIO_HAL);
	NVIC_SetPriority((IRQn_Type)SPI0_IRQn,	IRQ_PRIO_HAL);
	NVIC_SetPriority((IRQn_Type)SPI1_IRQn,	IRQ_PRIO_HAL);
	NVIC_SetPriority((IRQn_Type)GPIO_IRQn,	IRQ_PRIO_APP);
	NVIC_SetPriority((IRQn_Type)DMAC_IRQn,	IRQ_PRIO_HAL);
	NVIC_SetPriority((IRQn_Type)TIM5_IRQn,	IRQ_PRIO_HAL);
	NVIC_SetPriority((IRQn_Type)TIM6_IRQn,	IRQ_PRIO_HAL);
	NVIC_SetPriority((IRQn_Type)ADCC_IRQn,	IRQ_PRIO_APP);


	for(i = 0; i< s_pwrmgr_cfg.moudle_num; i++)
	{
		if(mCtx[i].moudle_id == MOD_NONE)
		{
			return PPlus_ERR_NOT_REGISTED;
		}

		if(mCtx[i].wakeup_handler)
			mCtx[i].wakeup_handler();
	}

	return PPlus_SUCCESS;
}

int __attribute__((used)) hal_pwrmgr_sleep_process(void)
{
	int i;
	//20181013 ZQ :
	hal_pwrmgr_RAM_retention_set();

	//LOG("Sleep\n");
	for(i = s_pwrmgr_cfg.moudle_num-1; i >= 0; i--)
	{
		if(mCtx[i].moudle_id == MOD_NONE)
		{
			return PPlus_ERR_NOT_REGISTED;
		}

		if(mCtx[i].sleep_handler)
			mCtx[i].sleep_handler();
	}

#if(CFG_HCLK_DYNAMIC_CHANGE==1)
	/*
		hclk will change to SYS_CLK_XTAL_16M in next wakeup_process
	*/
	hal_system_clock_change_req(SYS_CLK_XTAL_16M);
#endif
#ifdef CFG_FLASH_ENABLE_DEEP_SLEEP
	extern void spif_set_deep_sleep(void);
	spif_set_deep_sleep();
#endif
	return PPlus_SUCCESS;
}
#else
int __attribute__((used)) hal_pwrmgr_wakeup_process(void)
{
	return PPlus_SUCCESS;
}

int __attribute__((used)) hal_pwrmgr_sleep_process(void)
{
	return PPlus_SUCCESS;
}


#endif


/**************************************************************************************
	@fn			 hal_pwrmgr_RAM_retention

	@brief		 This function process for enable retention sram

	input parameters

	@param		 uint32_t sram: sram bit map

	output parameters

	@param		 None.

	@return		 refer error.h.
 **************************************************************************************/
int hal_pwrmgr_RAM_retention(uint32_t sram)
{
	if(sram & 0xffffffe0)
	{
		s_pwrmgr_cfg.sramRet_config = 0x00;
		return PPlus_ERR_INVALID_PARAM;
	}

	s_pwrmgr_cfg.sramRet_config = sram;
	return PPlus_SUCCESS;
}

int hal_pwrmgr_RAM_retention_clr(void)
{
	subWriteReg(0x4000f01c,21,17,0);
	return PPlus_SUCCESS;
}

int hal_pwrmgr_RAM_retention_set(void)
{
	subWriteReg(0x4000f01c,21,17,s_pwrmgr_cfg.sramRet_config);
	return PPlus_SUCCESS;
}

int hal_pwrmgr_LowCurrentLdo_enable(void)
{
#ifdef CFG_SRAM_RETENTION_LOW_CURRENT_LDO_ENABLE
	uint32_t retention_flag;
	hal_flash_read(CHIP_RETENTION_FLG_FLASH_ADDRESS,(uint8_t*)&retention_flag,4);

	if(retention_flag == 0xffffffff)
	{
		subWriteReg(0x4000f014,26,26, 1);
	}
	subWriteReg(0x4000f014,26,26, 1);
	return PPlus_SUCCESS;
#else
	subWriteReg(0x4000f014,26,26, 0);
	return PPlus_ERR_FORBIDDEN;
#endif
}

int hal_pwrmgr_LowCurrentLdo_disable(void)
{
	subWriteReg(0x4000f014,26,26, 0);
	return PPlus_SUCCESS;
}
extern void gpio_wakeup_set(gpio_pin_e pin, gpio_polarity_e type);
extern void gpio_pull_set(gpio_pin_e pin, gpio_pupd_e type);

void hal_pwrmgr_poweroff(pwroff_cfg_t* pcfg, uint8_t wakeup_pin_num)
{
	HAL_ENTER_CRITICAL_SECTION();
	subWriteReg(0x4000f01c,6,6,0x00);	//disable software control
	//(void)(wakeup_pin_num);

	for(uint8_t i = 0; i < wakeup_pin_num; i++ )
	{
		if(pcfg[i].type==POL_FALLING)
			gpio_pull_set(pcfg[i].pin,GPIO_PULL_UP_S);
		else
			gpio_pull_set(pcfg[i].pin,GPIO_PULL_DOWN);

		gpio_wakeup_set(pcfg[i].pin, pcfg[i].type);
	}

	/**
		config reset casue as RSTC_OFF_MODE
		reset path walkaround dwc
	*/
	AON_CLEAR_XTAL_TRACKING_AND_CALIB;
	AP_AON->SLEEP_R[0] = 2;
	enter_sleep_off_mode(SYSTEM_OFF_MODE);

	while(1);
}

__ATTR_SECTION_SRAM__ void hal_pwrmgr_enter_sleep_rtc_reset(uint32_t sleepRtcTick)
{
	HAL_ENTER_CRITICAL_SECTION();
	subWriteReg(0x4000f01c,6,6,0x00);	//disable software control
	config_RTC1(sleepRtcTick);
	// clear sram retention
	hal_pwrmgr_RAM_retention_clr();
	/**
		config reset casue as RSTC_WARM_NDWC
		reset path walkaround dwc
	*/
	AON_CLEAR_XTAL_TRACKING_AND_CALIB;
	AP_AON->SLEEP_R[0] = 4;
	enter_sleep_off_mode(SYSTEM_SLEEP_MODE);

	while(1) {};
}


#define STANDBY_WAIT_MS(a)	WaitRTCCount((a)<<5) // 32us * 32  around 1ms
pwroff_cfg_t s_pwroff_cfg[WAKEUP_PIN_MAX];
__attribute__((used)) uint8 pwroff_register_number=0;
__attribute__((section("_section_standby_code_"))) void wakeupProcess_standby(void)
{
	subWriteReg(0x4000f014,29,27,0x07);
	STANDBY_WAIT_MS(5);
#ifdef CFG_FLASH_ENABLE_DEEP_SLEEP
	extern void spif_release_deep_sleep(void);
	spif_release_deep_sleep();
	STANDBY_WAIT_MS(15);
#endif
	uint32_t volatile cnt=0;
	uint8_t volatile find_flag=0;
	uint8 pin_n=0;
	extern bool gpio_read(gpio_pin_e pin);

	for(pin_n=0; pin_n<pwroff_register_number; pin_n++)
	{
		if((s_pwroff_cfg[pin_n].pin == P2) || (s_pwroff_cfg[pin_n].pin == P3))
		{
			hal_gpio_pin2pin3_control(s_pwroff_cfg[pin_n].pin,1);
		}
	}

	for(pin_n=0; pin_n<pwroff_register_number; pin_n++)
	{
		if(gpio_read(s_pwroff_cfg[pin_n].pin)==s_pwroff_cfg[pin_n].type)
		{
			find_flag=1;
			break;
		}
	}

	while(1)
	{
		if(gpio_read(s_pwroff_cfg[pin_n].pin)==s_pwroff_cfg[pin_n].type&&find_flag==1)
		{
			cnt++;
			STANDBY_WAIT_MS(32);

			if(cnt>(s_pwroff_cfg[pin_n].on_time>>5))
			{
				write_reg(0x4000f030, 0x01);
				break;
			}
		}
		else
			hal_pwrmgr_enter_standby(&s_pwroff_cfg[0],pwroff_register_number);
	}

	set_sleep_flag(0);
	AP_AON->SLEEP_R[0] = 4;
	AON_CLEAR_XTAL_TRACKING_AND_CALIB;
	HAL_ENTER_CRITICAL_SECTION();
	AP_PCR->SW_RESET1 = 0;

	while(1);
}
extern void gpio_wakeup_set(gpio_pin_e pin, gpio_polarity_e type);
extern void gpio_pull_set(gpio_pin_e pin, gpio_pupd_e type);
__attribute__((section("_section_standby_code_"))) void hal_pwrmgr_enter_standby(pwroff_cfg_t* pcfg,uint8_t wakeup_pin_num)
{
	HAL_ENTER_CRITICAL_SECTION();
	subWriteReg(0x4000f01c,6,6,0x00);	//disable software control
	uint8_t i = 0;

	if(wakeup_pin_num>WAKEUP_PIN_MAX)
	{
		wakeup_pin_num=WAKEUP_PIN_MAX;
	}

	pwroff_register_number=wakeup_pin_num;

	for(i = 0; i < wakeup_pin_num; i++)
	{
		if(pcfg[i].type==POL_FALLING)
			gpio_pull_set(pcfg[i].pin,GPIO_PULL_UP_S);
		else
			gpio_pull_set(pcfg[i].pin,GPIO_PULL_DOWN);

		gpio_wakeup_set(pcfg[i].pin, pcfg[i].type);
		osal_memcpy(&s_pwroff_cfg[i],&(pcfg[i]),sizeof(pwroff_cfg_t));
	}

	JUMP_FUNCTION(WAKEUP_PROCESS)=	 (uint32_t)&wakeupProcess_standby;
#ifdef CFG_FLASH_ENABLE_DEEP_SLEEP
	extern void spif_set_deep_sleep(void);
	spif_set_deep_sleep();
	WaitRTCCount(50);
#endif
	subWriteReg(0x4000f014,29,27,0);
	set_sleep_flag(1);
	AP_AON->SLEEP_R[0] = 2;
	subWriteReg(0x4000f01c,21,17,RET_SRAM0);
	enter_sleep_off_mode(SYSTEM_SLEEP_MODE);

	while(1);
}

__ATTR_SECTION_XIP__ int hal_pwrmgr_get_module_lock_status(void)
{
	if (mPwrMode == PWR_MODE_NO_SLEEP || mPwrMode == PWR_MODE_PWROFF_NO_SLEEP)
	{
		disableSleep();
		return FALSE;
	}

	for (int i = 0; i < HAL_PWRMGR_TASK_MAX_NUM; i++)
	{
		if (mCtx[i].lock == TRUE)
		{
			return FALSE;
		}
	}

	return TRUE;
}
