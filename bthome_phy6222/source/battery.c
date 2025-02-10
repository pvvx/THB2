/*
	battery.c
	Author: pvvx
*/
#include "types.h"
#include "config.h"
#include "adc.h"
#include "battery.h"
#include "thb2_main.h"
#include "pwrmgr.h"
#include "jump_function.h"
#include "sensors.h"
#if (DEV_SERVICES & SERVICE_SCREEN)
#include "lcd.h"
#endif
/*
#ifndef ADC_PIN
#define ADC_PIN GPIO_P11
#endif
#ifndef ADC_VBAT_CHL
#define ADC_VBAT_CHL ADC_CH1N_P11
#endif
*/
#define MIN_ADC_CH 2

#define BAT_AVERAGE_COUNT	32

struct {
	uint32_t summ;
	uint16_t count;
	uint16_t battery_mv;
} bat_average;

static void init_adc_batt(void);

__ATTR_SECTION_SRAM__
void __attribute__((used)) hal_ADC_IRQHandler(void) {
	uint32_t adc_sum = 0, i;
	//int status = AP_ADCC->intr_status;
	AP_ADCC->intr_mask = 0; // MASK coresponding channel
	for (i = 0; i < (MAX_ADC_SAMPLE_SIZE - 2); i++) {
		adc_sum += (uint16_t) (read_reg(ADC_CH_BASE
				+ (
#if (ADC_VBAT_CHL & 1)
					(ADC_VBAT_CHL-1)
#else
					(ADC_VBAT_CHL+1)
#endif
						* 0x80) + ((i+2) * 4))	& 0xfff);
	}
	AP_ADCC->intr_clear = 0x1FF;
	// stop_adc_batt
	AP_AON->PMCTL2_1 = 0;
	NVIC_DisableIRQ((IRQn_Type) ADCC_IRQn);
//  JUMP_FUNCTION(ADCC_IRQ_HANDLER) = 0;    
//	AP_ADCC->intr_clear = 0x1FF;	
	AP_PCRM->ANA_CTL &= ~(BIT(3) | BIT(0)); // ADC disable, Power down analog LDO
#if defined(CLK_16M_ONLY) &&  CLK_16M_ONLY != 0
	AP_PCRM->CLKHF_CTL1 &= ~BIT(13);
#else
	if (g_system_clk != SYS_CLK_DBL_32M) {
		AP_PCRM->CLKHF_CTL1 &= ~BIT(13);
	}
#endif
	AP_IOMUX->Analog_IO_en = 0; /// &= ~BIT(ADC_PIN - P11); // hal_gpio_cfg_analog_io(ADC_PIN, Bit_DISABLE);
	hal_clk_reset(MOD_ADCC);
	hal_clk_gate_disable(MOD_ADCC);
	AP_IOMUX->pad_ps0 &= ~BIT(ADC_PIN); // hal_gpio_ds_control(ADC_PIN, Bit_ENABLE);
	LOG("ADC_measure = %d\n", adc_sum);
#if ADC_VBAT_CHL == VBAT_ADC_P15
	bat_average.battery_mv = (adc_sum * 1710) >> 16; // 3200*65536/(30*4096)=1706.666
#else
	bat_average.battery_mv = (adc_sum * 1904) >> 16;
#endif
	adv_wrk.new_battery = 1; // new battery
#if ((DEV_SERVICES & SERVICE_THS) == 0)
//	measured_data.count++;
#endif
	hal_pwrmgr_unlock(MOD_ADCC);

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
#if ADC_VBAT_CHL == VBAT_ADC_P11
	AP_PCRM->ADC_CTL1 |= BIT(20);
#elif ADC_VBAT_CHL == VBAT_ADC_P23
	AP_PCRM->ADC_CTL1 |= BIT(4);
#elif ADC_VBAT_CHL == VBAT_ADC_P24
	AP_PCRM->ADC_CTL2 |= BIT(20);
#elif ADC_VBAT_CHL == VBAT_ADC_P14
	AP_PCRM->ADC_CTL2 |= BIT(4);
#elif ADC_VBAT_CHL == VBAT_ADC_P15
	AP_PCRM->ADC_CTL3 |= BIT(20);
#elif ADC_VBAT_CHL == VBAT_ADC_P20
	AP_PCRM->ADC_CTL3 |= BIT(4);
#endif
	AP_PCRM->ANA_CTL |= BIT(3) | BIT(0); // ADC enable, Power on analog LDO

	NVIC_SetPriority((IRQn_Type) ADCC_IRQn, IRQ_PRIO_HAL);
	NVIC_EnableIRQ((IRQn_Type) ADCC_IRQn); //ADC_IRQ_ENABLE;
#if (ADC_VBAT_CHL & 1)
	AP_ADCC->intr_mask = BIT(ADC_VBAT_CHL - 1); // ENABLE_ADC_INT;
#else
	AP_ADCC->intr_mask = BIT(ADC_VBAT_CHL + 1); // ENABLE_ADC_INT;
#endif
}

static void init_adc_batt(void) {
	AP_AON->PMCTL2_1 = 0;
	AP_PCRM->ANA_CTL &= ~(BIT(0) | BIT(3)); // ADC disable, Power down analog LDO
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
#if	ADC_VBAT_CHL == VBAT_ADC_P20
	AP_AON->PMCTL2_1 = BIT(7 + 8);
	//AP_AON->PMCTL2_1 |= BIT(7); // -> 0.8V, else 3.2V
#else
	AP_AON->PMCTL2_1 = BIT((ADC_VBAT_CHL - MIN_ADC_CH) + 8);
	// AP_AON->PMCTL2_1 |= BIT(ADC_VBAT_CHL - MIN_ADC_CH); // -> 0.8V, else 3.2V
#endif
	AP_PCRM->ADC_CTL0 &= ~(BIT(20) | BIT(4));
	AP_PCRM->ADC_CTL1 &= ~(BIT(20) | BIT(4));
	AP_PCRM->ADC_CTL2 &= ~(BIT(20) | BIT(4));
	AP_PCRM->ADC_CTL3 &= ~(BIT(20) | BIT(4));
	AP_PCRM->ANA_CTL &= ~BIT(23); //disable micbias
#if ADC_PIN_USE_OUT
	hal_gpio_pull_set(ADC_PIN, GPIO_PULL_UP);
	hal_gpio_write(ADC_PIN, 1);
	AP_IOMUX->pad_ps0 |= BIT(ADC_PIN); // hal_gpio_ds_control(ADC_PIN, Bit_ENABLE);
#else
	AP_IOMUX->Analog_IO_en |= BIT(ADC_PIN - P11); //	hal_gpio_cfg_analog_io(ADC_PIN, Bit_ENABLE);
#endif
}

void low_vbat(void) {
#if (DEV_SERVICES & SERVICE_SCREEN)
    power_off_lcd();
#endif
#if (DEV_SERVICES & SERVICE_THS)
    power_off_sensor();
#endif
#ifdef GPIO_SPWR  // питание сенсора CHT8305_VDD
	hal_gpio_pull_set(GPIO_SPWR, GPIO_FLOATING);
	hal_gpio_pull_set(I2C_SDA, GPIO_FLOATING);
	hal_gpio_pull_set(I2C_SCL, GPIO_FLOATING);
	hal_gpio_write(GPIO_SPWR, 0);
#endif
#ifdef GPIO_LPWR // питание LCD драйвера
	hal_gpio_pull_set(GPIO_LPWR, GPIO_FLOATING);
	hal_gpio_write(GPIO_LPWR, 0);
#endif
#ifdef GPIO_LED
	hal_gpio_write(GPIO_LED, LED_OFF);
#endif
    // 1.67 uA at 3.0V
    hal_pwrmgr_enter_sleep_rtc_reset((60*60)<<15); // 60 minutes
}


void check_battery(void) {
	if(bat_average.battery_mv == 0)
		return;
#if defined(OTA_TYPE) && OTA_TYPE == OTA_TYPE_BOOT
	if (bat_average.battery_mv < 2000) // It is not recommended to write Flash below 2V
		low_vbat();
#else
	if (bat_average.battery_mv < 1900)
		low_vbat();
#endif

	bat_average.summ += bat_average.battery_mv;
	bat_average.count++;

	measured_data.battery_mv = bat_average.summ / bat_average.count;

	if(bat_average.count >= BAT_AVERAGE_COUNT) {
		bat_average.summ >>= 1;
		bat_average.count >>= 1;
	}

	if (measured_data.battery_mv < 3000)
		if (measured_data.battery_mv > 2000)
			measured_data.battery = (measured_data.battery_mv - 2000) / 10;
		else
			measured_data.battery = 0;
	else
		measured_data.battery = 100;
	bat_average.battery_mv = 0;
}
