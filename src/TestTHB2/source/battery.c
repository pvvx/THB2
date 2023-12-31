/**************************************************************************************************
 
  Phyplus Microelectronics Limited confidential and proprietary. 
  All rights reserved.

  IMPORTANT: All rights of this software belong to Phyplus Microelectronics 
  Limited ("Phyplus"). Your use of this Software is limited to those 
  specific rights granted under  the terms of the business contract, the 
  confidential agreement, the non-disclosure agreement and any other forms 
  of agreements as a customer or a partner of Phyplus. You may not use this 
  Software unless you agree to abide by the terms of these agreements. 
  You acknowledge that the Software may not be modified, copied, 
  distributed or disclosed unless embedded on a Phyplus Bluetooth Low Energy 
  (BLE) integrated circuit, either as a product or is integrated into your 
  products.  Other than for the aforementioned purposes, you may not use, 
  reproduce, copy, prepare derivative works of, modify, distribute, perform, 
  display or sell this Software and/or its documentation for any purposes.

  YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
  PROVIDED AS IS WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
  INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE,
  NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
  PHYPLUS OR ITS SUBSIDIARIES BE LIABLE OR OBLIGATED UNDER CONTRACT,
  NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
  LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
  INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
  OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
  OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
  (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
  
**************************************************************************************************/


/************************************************************** 
 * COPYRIGHT(c)2018, Phy+.
 * All rights reserved. 
 *
 *
 * Module Name:	battery
 * File name:	battery.c 
 * Brief description:
 *    battery module
 * Author:	Eagle.Lao
 * Data:	2018-02-27
 * Revision:V0.01

****************************************************************/
#include "types.h"
#include "adc.h"
#include "battery.h"
#include "simpleBLEperipheral.h"
#include "OSAL.h"
#include "OSAL_Timers.h"
#include "string.h"
#include "gpio.h"
#include "gpio.h"
#include "error.h"
#include "log.h"
#include "pwrmgr.h"
#include "clock.h"
#include "adc.h"
#include "jump_function.h"
#include "version.h"
#include "sensor.h"
#include "rf_phy_driver.h"

#define ADC_PIN GPIO_P11
#define ADC_CHL ADC_CH1N_P11


void __attribute__((used)) hal_ADC_IRQHandler(void)
{
	uint32_t adc_sum = 0, i;
	int status = GET_IRQ_STATUS;
	AP_ADCC->intr_mask = 0; // MASK coresponding channel
	for (i = 0; i < (MAX_ADC_SAMPLE_SIZE-2); i++)
	{
		adc_sum += (uint16_t)(read_reg(ADC_CH_BASE + ((ADC_CH1N_P11+1) * 0x80) + ((i+2) * 4)) & 0xfff);
	}
	AP_ADCC->intr_clear = 0x1FF;	
	// stop_adc_batt
	AP_AON->PMCTL2_1 = 0x00;	
  NVIC_DisableIRQ((IRQn_Type)ADCC_IRQn);
//  JUMP_FUNCTION(ADCC_IRQ_HANDLER) = 0;    
//	AP_ADCC->intr_clear = 0x1FF;	
  AP_PCRM->ANA_CTL &= ~BIT(3);
  if(g_system_clk != SYS_CLK_DBL_32M)
	{
		AP_PCRM->CLKHF_CTL1 &= ~BIT(13);
	}
  hal_gpio_cfg_analog_io(ADC_PIN,Bit_DISABLE);
  hal_gpio_pin_init(ADC_PIN,GPIO_INPUT);    // ie=0, oen=1 set to imput
  hal_gpio_pull_set(ADC_PIN,GPIO_FLOATING); 
  AP_PCRM->ANA_CTL &= ~BIT(0); 							// Power down analog LDO
	hal_clk_reset(MOD_ADCC);
  hal_clk_gate_disable(MOD_ADCC);
  hal_pwrmgr_unlock(MOD_ADCC);
	
	// 3280/3764 = 0.8714
	// 30*3764 = 112920
	// 112920 * 1904 = 214999680
	// 214999680 >> 16 = 3280
	LOG("ADC_measure = %d\n", adc_sum);
	measured_data.battery_mv = (adc_sum * 1904)>> 16;
  if(measured_data.battery_mv < 3000)
		if (measured_data.battery_mv > 2000)		
			measured_data.battery = (measured_data.battery_mv - 2000)/10;
		else
	  	measured_data.battery = 0;
	else
		 measured_data.battery = 100;	
	extern uint8  gapRole_AdvEnabled;
	if(!gapRole_AdvEnabled)
		osal_set_event(simpleBLEPeripheral_TaskID, BATT_VALUE_EVT);
}

//static void adc_batt_wakeup(void)
//{
//    NVIC_SetPriority((IRQn_Type)ADCC_IRQn, IRQ_PRIO_HAL);
//}

void hal_adc_init(void)
{
	hal_pwrmgr_register(MOD_ADCC, NULL, NULL);
}

void init_adc_batt(void) {
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
  AP_PCRM->CLKHF_CTL1 &= ~BIT(21);//check
  //subWriteReg(0x4000F044,21,20,3);
  //ADC_CLOCK_ENABLE;            //adc clock enbale,always use clk_32M
  AP_PCRM->CLKHF_CTL1 |= BIT(13);
  //subWriteReg(0x4000f07c,4,4,1);    //set adc mode,1:mannual,0:auto mode
//  AP_PCRM->ADC_CTL4 |= BIT(4); // mannual mode
  AP_PCRM->ADC_CTL4 &= ~BIT(4); //enable auto mode
  AP_PCRM->ADC_CTL4 |= BIT(0);
  AP_AON->PMCTL2_1 =  BIT((ADC_CHL - MIN_ADC_CH)+8);
  AP_PCRM->ADC_CTL0 &= ~BIT(20);
  AP_PCRM->ADC_CTL0 &= ~BIT(4);
  AP_PCRM->ADC_CTL1 &= ~BIT(20);
  AP_PCRM->ADC_CTL1 &= ~BIT(4);
  AP_PCRM->ADC_CTL2 &= ~BIT(20);
  AP_PCRM->ADC_CTL2 &= ~BIT(4);
  AP_PCRM->ADC_CTL3 &= ~BIT(20);
  AP_PCRM->ADC_CTL3 &= ~BIT(4);
  AP_PCRM->ANA_CTL &= ~BIT(23);//disable micbias
	hal_gpio_pull_set(ADC_PIN,GPIO_FLOATING);
	hal_gpio_ds_control(ADC_PIN, Bit_ENABLE);
	hal_gpio_cfg_analog_io(ADC_PIN, Bit_ENABLE);							
}

void batt_start_measure(void)
{
	LOG("batt_meassured\n");
  //Event handler is called immediately after conversion is finished.
  // init_adc_batt
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
  AP_PCRM->CLKHF_CTL1 &= ~BIT(21);//check
  //subWriteReg(0x4000F044,21,20,3);
  //ADC_CLOCK_ENABLE;            //adc clock enbale,always use clk_32M
  AP_PCRM->CLKHF_CTL1 |= BIT(13);
  //subWriteReg(0x4000f07c,4,4,1);    //set adc mode,1:mannual,0:auto mode
//  AP_PCRM->ADC_CTL4 |= BIT(4); // mannual mode
  AP_PCRM->ADC_CTL4 &= ~BIT(4); //enable auto mode
  AP_PCRM->ADC_CTL4 |= BIT(0);
  AP_AON->PMCTL2_1 =  BIT((ADC_CHL - MIN_ADC_CH)+8);
  AP_PCRM->ADC_CTL0 &= ~BIT(20);
  AP_PCRM->ADC_CTL0 &= ~BIT(4);
  AP_PCRM->ADC_CTL1 &= ~BIT(20);
  AP_PCRM->ADC_CTL1 &= ~BIT(4);
  AP_PCRM->ADC_CTL2 &= ~BIT(20);
  AP_PCRM->ADC_CTL2 &= ~BIT(4);
  AP_PCRM->ADC_CTL3 &= ~BIT(20);
  AP_PCRM->ADC_CTL3 &= ~BIT(4);
  AP_PCRM->ANA_CTL &= ~BIT(23);//disable micbias
	hal_gpio_pull_set(ADC_PIN,GPIO_FLOATING);
	hal_gpio_ds_control(ADC_PIN, Bit_ENABLE);
	hal_gpio_cfg_analog_io(ADC_PIN, Bit_ENABLE);			
  // start_adc_bat
  hal_pwrmgr_lock(MOD_ADCC);
  JUMP_FUNCTION(ADCC_IRQ_HANDLER) = (uint32_t)&hal_ADC_IRQHandler;

	AP_PCRM->ADC_CTL1 |= BIT(20);
	AP_PCRM->ANA_CTL |= BIT(3);//ENABLE_ADC;
  AP_PCRM->ANA_CTL |= BIT(0);//new

  NVIC_SetPriority((IRQn_Type)ADCC_IRQn, IRQ_PRIO_HAL);   
  NVIC_EnableIRQ((IRQn_Type)ADCC_IRQn); //ADC_IRQ_ENABLE;   
	AP_ADCC->intr_mask = BIT(ADC_CHL + 1); //ENABLE_ADC_INT;
}


