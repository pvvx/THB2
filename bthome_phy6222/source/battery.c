/*
	battery.c
	Author: pvvx
*/
#include "types.h"
#include "config.h"
#include "adc.h"
#include "battery.h"
#include "bleperipheral.h"
#include "pwrmgr.h"
#include "jump_function.h"
#include "sensors.h"

#ifndef ADC_PIN
#define ADC_PIN GPIO_P11
#endif
#ifndef ADC_CHL
#define ADC_CHL ADC_CH1N_P11
#endif
#define MIN_ADC_CH 2

static void init_adc_batt(void);

void __attribute__((used)) hal_ADC_IRQHandler(void) {
	uint32_t adc_sum = 0, i;
	//int status = AP_ADCC->intr_status;
	AP_ADCC->intr_mask = 0; // MASK coresponding channel
	for (i = 0; i < (MAX_ADC_SAMPLE_SIZE - 2); i++) {
		adc_sum +=
				(uint16_t) (read_reg(
						ADC_CH_BASE + ((ADC_CH1N_P11+1) * 0x80) + ((i+2) * 4))
						& 0xfff);
	}
	AP_ADCC->intr_clear = 0x1FF;
	// stop_adc_batt
	AP_AON->PMCTL2_1 = 0x00;
	NVIC_DisableIRQ((IRQn_Type) ADCC_IRQn);
//  JUMP_FUNCTION(ADCC_IRQ_HANDLER) = 0;    
//	AP_ADCC->intr_clear = 0x1FF;	
	AP_PCRM->ANA_CTL &= ~BIT(3);
	if (g_system_clk != SYS_CLK_DBL_32M) {
		AP_PCRM->CLKHF_CTL1 &= ~BIT(13);
	}
	AP_IOMUX->Analog_IO_en &= ~BIT(ADC_PIN - P11); // hal_gpio_cfg_analog_io(ADC_PIN, Bit_DISABLE);
#if !ADC_PIN_USE_OUT
//	hal_gpio_pin_init(ADC_PIN, GPIO_INPUT);    // ie=0, oen=1 set to imput
//	hal_gpio_pull_set(ADC_PIN, GPIO_FLOATING);
#endif
	AP_PCRM->ANA_CTL &= ~BIT(0); 						// Power down analog LDO
	hal_clk_reset(MOD_ADCC);
	hal_clk_gate_disable(MOD_ADCC);
	hal_pwrmgr_unlock(MOD_ADCC);

	// 3280/3764 = 0.8714
	// 30*3764 = 112920
	// 112920 * 1904 = 214999680
	// 214999680 >> 16 = 3280
	LOG("ADC_measure = %d\n", adc_sum);
	measured_data.battery_mv = (adc_sum * 1904) >> 16;
	if (measured_data.battery_mv < 3000)
		if (measured_data.battery_mv > 2000)
			measured_data.battery = (measured_data.battery_mv - 2000) / 10;
		else
			measured_data.battery = 0;
	else
		measured_data.battery = 100;
#if ((DEV_SERVICES & SERVICE_THS) == 0)
	measured_data.count++;
#endif
	extern uint8 gapRole_AdvEnabled;
	if (!gapRole_AdvEnabled)
		osal_set_event(simpleBLEPeripheral_TaskID, BATT_VALUE_EVT);
}

void hal_adc_init(void) {
	hal_pwrmgr_register(MOD_ADCC, NULL, NULL);
}

void batt_start_measure(void) {
	LOG("batt_meassured\n");
	init_adc_batt();
	// start_adc_bat
	hal_pwrmgr_lock(MOD_ADCC);
	JUMP_FUNCTION(ADCC_IRQ_HANDLER) = (uint32_t) &hal_ADC_IRQHandler;

	AP_PCRM->ADC_CTL1 |= BIT(20);
	AP_PCRM->ANA_CTL |= BIT(3); //ENABLE_ADC;
	AP_PCRM->ANA_CTL |= BIT(0); //new

	NVIC_SetPriority((IRQn_Type) ADCC_IRQn, IRQ_PRIO_HAL);
	NVIC_EnableIRQ((IRQn_Type) ADCC_IRQn); //ADC_IRQ_ENABLE;
	AP_ADCC->intr_mask = BIT(ADC_CHL + 1); //ENABLE_ADC_INT;
}

static void init_adc_batt(void) {
	AP_AON->PMCTL2_1 = 0x00;
	AP_PCRM->ANA_CTL &= ~BIT(0);
	AP_PCRM->ANA_CTL &= ~BIT(3);
	hal_clk_gate_disable(MOD_ADCC);
	hal_clk_reset(MOD_ADCC);
	hal_clk_gate_enable(MOD_ADCC);
	//CLK_1P28M_ENABLE;
	AP_PCRM->CLKSEL |= BIT(6);
	//ENABLE_XTAL_OUTPUT;         //enable xtal 16M output,generate the 32M dll clock
	AP_PCRM->CLKHF_CTL0 |= BIT(18);
	//ENABLE_DLL;                  //enable DLL
	AP_PCRM->CLKHF_CTL1 |= BIT(7);
	//ADC_DBLE_CLOCK_DISABLE;      //disable double 32M clock,we are now use 32M clock,should enable bit<13>, diable bit<21>
	AP_PCRM->CLKHF_CTL1 &= ~BIT(21);	//check
	//subWriteReg(0x4000F044,21,20,3);
	//ADC_CLOCK_ENABLE;            //adc clock enbale,always use clk_32M
	AP_PCRM->CLKHF_CTL1 |= BIT(13);
	//subWriteReg(0x4000f07c,4,4,1);    //set adc mode,1:mannual,0:auto mode
//  AP_PCRM->ADC_CTL4 |= BIT(4); // mannual mode
	AP_PCRM->ADC_CTL4 &= ~BIT(4); //enable auto mode
	AP_PCRM->ADC_CTL4 |= BIT(0);
	AP_AON->PMCTL2_1 = BIT((ADC_CHL - MIN_ADC_CH) + 8);
	AP_PCRM->ADC_CTL0 &= ~(BIT(20) | BIT(4));
	AP_PCRM->ADC_CTL1 &= ~(BIT(20) | BIT(4));
	AP_PCRM->ADC_CTL2 &= ~(BIT(20) | BIT(4));
	AP_PCRM->ADC_CTL3 &= ~(BIT(20) | BIT(4));
	AP_PCRM->ANA_CTL &= ~BIT(23); //disable micbias
#if ADC_PIN_USE_OUT
	hal_gpio_pin_init(ADC_PIN, GPIO_OUTPUT);
	hal_gpio_write(ADC_PIN, 1);
	AP_IOMUX->pad_ps0 |= BIT(ADC_PIN); // hal_gpio_ds_control(ADC_PIN, Bit_ENABLE);
#else
	AP_IOMUX->Analog_IO_en |= BIT(ADC_PIN - P11); //	hal_gpio_cfg_analog_io(ADC_PIN, Bit_ENABLE);
#endif
}
