/*******************************************************************************
    @file   adc.c
    @brief  Contains all functions support for adc driver
    @version  0.0
    @date   18. Oct. 2017
    @author qing.han

	SDK_LICENSE

*******************************************************************************/
#include <string.h>
#include "error.h"
#include "gpio.h"
#include "pwrmgr.h"
#include "clock.h"
#include "adc.h"
#include "log.h"
#include "jump_function.h"
#include "version.h"

typedef struct _adc_Contex_t
{
    bool        enable;
    uint8_t     all_channel;
	uint8_t     chs_en_shadow;
    bool        continue_mode;
	
	//sysclk_t    clk_src;
    uint16_t    adc_cal_postive;
    uint16_t    adc_cal_negtive;
	
    adc_Hdl_t   evt_handler;
} adc_Ctx_t;

static adc_Ctx_t mAdc_Ctx = 
{
	.enable = FALSE,
	.all_channel = 0x00,
	.chs_en_shadow = 0x00,
	.continue_mode = FALSE,
	.adc_cal_postive = 0xFFF,
	.adc_cal_negtive = 0xFFF,
	.evt_handler = NULL
};

gpio_pin_e s_pinmap[ADC_CH_NUM] =
{
    GPIO_DUMMY, //ADC_CH0 =0,
    GPIO_DUMMY, //ADC_CH1 =1,
    P11, //ADC_CH1N =2,
    P23, //ADC_CH1P =3,  ADC_CH1DIFF = 3,
    P24, //ADC_CH2N =4,
    P14, //ADC_CH2P =5,  ADC_CH2DIFF = 5,
    P15, //ADC_CH3N =6,
    P20, //ADC_CH3P =7,  ADC_CH3DIFF = 7,
    GPIO_DUMMY,  //ADC_CH_VOICE =8,
};

/**************************************************************************************
    @fn          hal_adc_value

    @brief       This function process for get adc value

    input parameters

    @param       ADC_CH_e adc_pin: adc pin select;ADC_CH0~ADC_CH7 and ADC_CH_VOICE

    output parameters

    @param       None.

    @return      ADC value
 **************************************************************************************/
static void hal_adc_load_calibration_value(void)
{
  uint32_t adc_cal = read_reg(SPIF_RSVD1_ADC_CALIBRATE);
  mAdc_Ctx.adc_cal_negtive = (uint16_t)(adc_cal & 0x0fff);
  mAdc_Ctx.adc_cal_postive = (uint16_t)((adc_cal >> 16) & 0x0fff);
  LOG("AD_CAL[%x %x]\n", mAdc_Ctx.adc_cal_negtive, mAdc_Ctx.adc_cal_postive);

  if((mAdc_Ctx.adc_cal_negtive < 0x733) || (mAdc_Ctx.adc_cal_negtive > 0x8cc) ||
            (mAdc_Ctx.adc_cal_postive < 0x733) || (mAdc_Ctx.adc_cal_postive > 0x8cc))
  {
    mAdc_Ctx.adc_cal_negtive = 0xfff;
    mAdc_Ctx.adc_cal_postive = 0xfff;
    LOG("->AD_CAL[%x %x]\n",mAdc_Ctx.adc_cal_negtive, mAdc_Ctx.adc_cal_postive);
  }
}

static void set_sampling_resolution(adc_CH_t channel, bool is_high_resolution,bool is_differential_mode)
{
    uint8_t aio = 0;
    uint8_t diff_aio = 0;

    switch(channel)
    {
    case ADC_CH1N_P11:
        aio = 0;
        diff_aio = 1;
        break;

    case ADC_CH1P_P23:
        aio = 1;
        diff_aio = 0;
        break;

    case ADC_CH2N_P24:
        aio = 2;
        diff_aio = 3;
        break;

    case ADC_CH2P_P14:
        aio = 3;
        diff_aio = 2;
        break;

    case ADC_CH3N_P15:
        aio = 4;
        diff_aio = 7;
        break;

    case ADC_CH3P_P20:
        aio = 7;
        diff_aio = 4;
        break;

    default:
        return;
    }

    if(is_high_resolution)
    {
        if(is_differential_mode)
        {
            subWriteReg(&(AP_AON->PMCTL2_1),(diff_aio+8),(diff_aio+8),0);
            subWriteReg(&(AP_AON->PMCTL2_1),diff_aio,diff_aio,1);
        }

        subWriteReg(&(AP_AON->PMCTL2_1),(aio+8),(aio+8),0);
        subWriteReg(&(AP_AON->PMCTL2_1),aio,aio,1);
    }
    else
    {
        if(is_differential_mode)
        {
            subWriteReg(&(AP_AON->PMCTL2_1),(diff_aio+8),(diff_aio+8),1);
            subWriteReg(&(AP_AON->PMCTL2_1),diff_aio,diff_aio,0);
        }

        subWriteReg(&(AP_AON->PMCTL2_1),(aio+8),(aio+8),1);
        subWriteReg(&(AP_AON->PMCTL2_1),aio,aio,0);
    }
}

static void set_sampling_resolution_auto(uint8_t channel, uint8_t is_high_resolution,uint8_t is_differential_mode)
{
    uint8_t i_channel;
    adc_CH_t a_channel;
    AP_AON->PMCTL2_1 = 0x00;

    for(i_channel =MIN_ADC_CH; i_channel<=MAX_ADC_CH; i_channel++)
    {
        if(channel & BIT(i_channel))
        {
            a_channel = (adc_CH_t)i_channel;
            set_sampling_resolution(a_channel,
                                    (is_high_resolution & BIT(i_channel)),
                                    (is_differential_mode & BIT(i_channel)));
        }
    }
}

static void set_differential_mode(void)
{
    subWriteReg(&( AP_PCRM->ANA_CTL),8,8,0);
    subWriteReg(&( AP_PCRM->ANA_CTL),11,11,0);
}

static void disable_analog_pin(adc_CH_t channel)
{
    int index = (int)channel;
    gpio_pin_e pin = s_pinmap[index];

    if(pin == GPIO_DUMMY)
        return;

    hal_gpio_cfg_analog_io(pin,Bit_DISABLE);
    hal_gpio_pin_init(pin,GPIO_INPUT);       //ie=0,oen=1 set to imput
    hal_gpio_pull_set(pin,GPIO_FLOATING);    //
}

static void clear_adcc_cfg(void)
{
	mAdc_Ctx.all_channel = 0x00;
	mAdc_Ctx.chs_en_shadow = 0x00;
	mAdc_Ctx.continue_mode = FALSE;
	mAdc_Ctx.evt_handler = NULL;
}

#if 0
static void disable_channel(adc_CH_t ch)
{
	switch (ch)
	{
		case ADC_CH1N_P11:
			AP_PCRM->ADC_CTL1 &= ~BIT(20);
			break;

		case ADC_CH1P_P23:
			AP_PCRM->ADC_CTL1 &= ~BIT(4);
			break;

		case ADC_CH2N_P24:
			AP_PCRM->ADC_CTL2 &= ~BIT(20);
			break;

		case ADC_CH2P_P14:
			AP_PCRM->ADC_CTL2 &= ~BIT(4);
			break;

		case ADC_CH3N_P15:
			AP_PCRM->ADC_CTL3 &= ~BIT(20);
			break;

		case ADC_CH3P_P20:
			AP_PCRM->ADC_CTL3 &= ~BIT(4);
			break;
	}
}
#endif
/////////////// adc ////////////////////////////
/**************************************************************************************
    @fn          hal_ADC_IRQHandler

    @brief       This function process for adc interrupt

    input parameters

    @param       None.

    output parameters

    @param       None.

    @return      None.
 **************************************************************************************/
void __attribute__((used)) hal_ADC_IRQHandler(void)
{
	int ch,ch2,status =0,n;
	uint16_t adc_data[MAX_ADC_SAMPLE_SIZE-2];
	
	status = GET_IRQ_STATUS;
	for (ch = MIN_ADC_CH; ch <= MAX_ADC_CH; ch++)
	{
		if ((mAdc_Ctx.all_channel & BIT(ch)) &&(status & BIT(ch)))
		{
			ch2=(ch%2)?(ch-1):(ch+1);
			if(mAdc_Ctx.continue_mode == FALSE)
			{
				AP_ADCC->intr_mask &= ~ BIT(ch); //MASK coresponding channel
				mAdc_Ctx.all_channel &= ~BIT(ch);//disable channel				
			}

			for (n = 0; n < (MAX_ADC_SAMPLE_SIZE-3); n++)
			{
				adc_data[n] = (uint16_t)(read_reg(ADC_CH_BASE + (ch * 0x80) + ((n+2) * 4))&0xfff);
				adc_data[n+1] = (uint16_t)((read_reg(ADC_CH_BASE + (ch * 0x80) + ((n+2) * 4))>>16)&0xfff);
			}

			AP_ADCC->intr_clear = BIT(ch);

			if(mAdc_Ctx.enable == FALSE)
				continue;

			if (mAdc_Ctx.evt_handler)
			{
				adc_Evt_t evt;
				evt.type = HAL_ADC_EVT_DATA;
				evt.ch = (adc_CH_t)ch2;
				evt.data = adc_data;
				evt.size = MAX_ADC_SAMPLE_SIZE-3;
				mAdc_Ctx.evt_handler(&evt);
			}
		}
	}

	//LOG("> %x\n",mAdc_Ctx.all_channel);
	if((mAdc_Ctx.all_channel == 0) && (mAdc_Ctx.continue_mode == FALSE))//
	{
		hal_adc_stop();
	}
}

static void adc_wakeup_hdl(void)
{
    NVIC_SetPriority((IRQn_Type)ADCC_IRQn, IRQ_PRIO_HAL);
}

/**************************************************************************************
    @fn          hal_adc_init

    @brief       This function process for adc initial

    input parameters

    @param       ADC_MODE_e mode: adc sample mode select;1:SAM_MANNUAL(mannual mode),0:SAM_AUTO(auto mode)
                ADC_CH_e adc_pin: adc pin select;ADC_CH0~ADC_CH7 and ADC_CH_VOICE
                ADC_SEMODE_e semode: signle-ended mode negative side enable; 1:SINGLE_END(single-ended mode) 0:DIFF(Differentail mode)
                IO_CONTROL_e amplitude: input signal amplitude, 0:BELOW_1V,1:UP_1V

    output parameters

    @param       None.

    @return      None.
 **************************************************************************************/
void hal_adc_init(void)
{
  hal_pwrmgr_register(MOD_ADCC,NULL,adc_wakeup_hdl);
	clear_adcc_cfg();
	hal_adc_load_calibration_value();   
	mAdc_Ctx.enable = TRUE;
}

int hal_adc_clock_config(adc_CLOCK_SEL_t clk)
{
	if(mAdc_Ctx.enable == FALSE)
	{
		return PPlus_ERR_NOT_REGISTED;
	}
    subWriteReg(&(AP_PCRM->ADC_CTL4),2,1,clk);
    return PPlus_SUCCESS;
}

int hal_adc_start(void)
{
	uint8_t     all_channel2 = (((mAdc_Ctx.chs_en_shadow&0x80)>>1)|\
								((mAdc_Ctx.chs_en_shadow&0x40)<<1)|\
								((mAdc_Ctx.chs_en_shadow&0x20)>>1)|\
								((mAdc_Ctx.chs_en_shadow&0x10)<<1)|\
								((mAdc_Ctx.chs_en_shadow&0x08)>>1)|\
								((mAdc_Ctx.chs_en_shadow&0x04)<<1));
	
	if(mAdc_Ctx.enable == FALSE)
	{
		return PPlus_ERR_NOT_REGISTED;
	}
	
	//LOG("all_channel2:0x%x\n",all_channel2);
    hal_pwrmgr_lock(MOD_ADCC);
    JUMP_FUNCTION(ADCC_IRQ_HANDLER)                  =   (uint32_t)&hal_ADC_IRQHandler;

	for(int i=MIN_ADC_CH; i<=MAX_ADC_CH; i++)
	{
		if(all_channel2 & (BIT(i)))
		{
			switch (i)
			{
			case ADC_CH1N_P11:
				AP_PCRM->ADC_CTL1 |= BIT(20);
				break;

			case ADC_CH1P_P23:
				AP_PCRM->ADC_CTL1 |= BIT(4);
				break;

			case ADC_CH2N_P24:
				AP_PCRM->ADC_CTL2 |= BIT(20);
				break;

			case ADC_CH2P_P14:
				AP_PCRM->ADC_CTL2 |= BIT(4);
				break;

			case ADC_CH3N_P15:
				AP_PCRM->ADC_CTL3 |= BIT(20);
				break;

			case ADC_CH3P_P20:
				AP_PCRM->ADC_CTL3 |= BIT(4);
				break;
			}
		}		
	}
	    
    AP_PCRM->ANA_CTL |= BIT(3);//ENABLE_ADC;
    AP_PCRM->ANA_CTL |= BIT(0);//new
   
    NVIC_EnableIRQ((IRQn_Type)ADCC_IRQn); //ADC_IRQ_ENABLE;   
	AP_ADCC->intr_mask = mAdc_Ctx.all_channel; //ENABLE_ADC_INT;
	
    //disableSleep();
    return PPlus_SUCCESS;
}


int hal_adc_config_channel(adc_Cfg_t cfg, adc_Hdl_t evt_handler)
{
    uint8_t i;
    uint8_t chn_sel;
    gpio_pin_e pin,pin_neg;
	
    if(mAdc_Ctx.enable == FALSE)
	  {
		    return PPlus_ERR_NOT_REGISTED;
	  }

    if(evt_handler == NULL)
    {
        return PPlus_ERR_INVALID_PARAM;
    }

    if((cfg.channel & BIT(0)) || (cfg.channel & BIT(1)))
    {
        return PPlus_ERR_NOT_SUPPORTED;
    }

    if((!cfg.channel & BIT(1))&&(cfg.is_differential_mode && (cfg.channel & BIT(1))))
    {
        return PPlus_ERR_INVALID_PARAM;
    }

    if(cfg.is_differential_mode != 0)
    {
        if((cfg.is_differential_mode != 0x80) && (cfg.is_differential_mode != 0x20) && (cfg.is_differential_mode != 0x08))
        {
            return PPlus_ERR_INVALID_PARAM;
        }
    }

	clear_adcc_cfg();
	
	AP_AON->PMCTL2_1 = 0x00;
	AP_PCRM->ANA_CTL &= ~BIT(0);
	AP_PCRM->ANA_CTL &= ~BIT(3);
	hal_clk_gate_disable(MOD_ADCC);
	hal_clk_reset(MOD_ADCC);
	
    mAdc_Ctx.continue_mode = cfg.is_continue_mode;
    mAdc_Ctx.all_channel = cfg.channel & 0x03;
	
    for(i=2; i<8; i++)
    {
        if(cfg.channel & BIT(i))
        {
            if(i%2)
            {
                mAdc_Ctx.all_channel |= BIT(i-1);
            }
            else
            {
                mAdc_Ctx.all_channel |= BIT(i+1);
            }
        }
    }
	mAdc_Ctx.chs_en_shadow = mAdc_Ctx.all_channel;
	//LOG("cfg.channel:0x%x\n",cfg.channel);
	LOG("mAdc_Ctx.all_channel:0x%x\n",mAdc_Ctx.all_channel);
	
    if((AP_PCR->SW_CLK & BIT(MOD_ADCC)) == 0)
    {
        hal_clk_gate_enable(MOD_ADCC);
    }

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
    AP_PCRM->ADC_CTL4 |= BIT(4);
    AP_PCRM->ADC_CTL4 |= BIT(0);
    set_sampling_resolution_auto(cfg.channel, cfg.is_high_resolution,cfg.is_differential_mode);
    AP_PCRM->ADC_CTL0 &= ~BIT(20);
    AP_PCRM->ADC_CTL0 &= ~BIT(4);
    AP_PCRM->ADC_CTL1 &= ~BIT(20);
    AP_PCRM->ADC_CTL1 &= ~BIT(4);
    AP_PCRM->ADC_CTL2 &= ~BIT(20);
    AP_PCRM->ADC_CTL2 &= ~BIT(4);
    AP_PCRM->ADC_CTL3 &= ~BIT(20);
    AP_PCRM->ADC_CTL3 &= ~BIT(4);
    AP_PCRM->ANA_CTL &= ~BIT(23);//disable micbias

    if(cfg.is_differential_mode == 0)
    {
        AP_PCRM->ADC_CTL4 &= ~BIT(4); //enable auto mode
        mAdc_Ctx.evt_handler = evt_handler;
        for(i=MIN_ADC_CH; i<=MAX_ADC_CH; i++)
        {
            if(cfg.channel & BIT(i))
            {
				gpio_pin_e pin = s_pinmap[i];
				hal_gpio_pull_set(pin,GPIO_FLOATING);
				hal_gpio_ds_control(pin, Bit_ENABLE);
				hal_gpio_cfg_analog_io(pin, Bit_ENABLE);							
            }
        }
    }
    else
    {
        switch(cfg.is_differential_mode)
        {
        case 0x80:
            pin = P20;
            pin_neg = P15;
            chn_sel = 0x04;
            break;

        case 0x20:
            pin = P14;
            pin_neg = P24;
            chn_sel = 0x03;
            break;

        case 0x08:
            pin = P23;
            pin_neg = P11;
            chn_sel = 0x02;
            break;

        case 0x02:
            pin = P18;
            pin_neg = P25;
            chn_sel = 0x01;
            *(volatile int*)(0x4000F020) = 0x0060;
            break;

        default:
            break;
        }

        hal_gpio_ds_control(pin, Bit_ENABLE);
        subWriteReg(&(AP_PCRM->ANA_CTL),7,5,chn_sel);		
        set_differential_mode();
        //LOG("%d %d %x\n",pin,pin_neg,*(volatile int*)0x40003800);
        hal_gpio_pull_set(pin,GPIO_FLOATING);
        hal_gpio_pull_set(pin_neg,GPIO_FLOATING);
        hal_gpio_cfg_analog_io(pin,Bit_ENABLE);
        hal_gpio_cfg_analog_io(pin_neg,Bit_ENABLE);
        //LOG("%d %d %x\n",pin,pin_neg,*(volatile int*)0x40003800);
        mAdc_Ctx.all_channel = (cfg.is_differential_mode >> 1);
        mAdc_Ctx.evt_handler = evt_handler;
    }

    return PPlus_SUCCESS;
}

int hal_adc_stop(void)
{
	int i;
	uint8_t     all_channel2 = (((mAdc_Ctx.chs_en_shadow&0x80)>>1)|\
								((mAdc_Ctx.chs_en_shadow&0x40)<<1)|\
								((mAdc_Ctx.chs_en_shadow&0x20)>>1)|\
								((mAdc_Ctx.chs_en_shadow&0x10)<<1)|\
								((mAdc_Ctx.chs_en_shadow&0x08)>>1)|\
								((mAdc_Ctx.chs_en_shadow&0x04)<<1));
	
	if(mAdc_Ctx.enable == FALSE)
	{
		return PPlus_ERR_NOT_REGISTED;
	}
	
	AP_AON->PMCTL2_1 = 0x00;	
    NVIC_DisableIRQ((IRQn_Type)ADCC_IRQn);
    JUMP_FUNCTION(ADCC_IRQ_HANDLER)                  =   0;    
	AP_ADCC->intr_clear = 0x1FF;	

    AP_PCRM->ANA_CTL &= ~BIT(3);
	
#include "rf_phy_driver.h"

    if(g_system_clk != SYS_CLK_DBL_32M)
	{
		AP_PCRM->CLKHF_CTL1 &= ~BIT(13);
	}
	
    for(i = MIN_ADC_CH; i<= MAX_ADC_CH; i++)
    {
		if(all_channel2 & BIT(i))
        {
            disable_analog_pin((adc_CH_t)i);
        }
    }

    AP_PCRM->ANA_CTL &= ~BIT(0);//Power down analog LDO
	hal_clk_reset(MOD_ADCC);
    hal_clk_gate_disable(MOD_ADCC);
    clear_adcc_cfg();
    //enableSleep();
    hal_pwrmgr_unlock(MOD_ADCC);
    return PPlus_SUCCESS;
}


#if(SDK_VER_CHIP==__DEF_CHIP_QFN32__)
const unsigned int adc_Lambda[MAX_ADC_CH - MIN_ADC_CH + 1] =
{
    4519602,//P11
    4308639,//P23
    4263287,//P24
    4482718,//P14
    4180401,//P15
    4072069,//P20
};

#elif(SDK_VER_CHIP == __DEF_CHIP_TSOP16__)
const unsigned int adc_Lambda[MAX_ADC_CH - MIN_ADC_CH + 1] =
{
    4488156,//P11
    4308639,//P23,
    4263287,//P24,
    4467981,//P14
    4142931,//P15
    4054721,//P20
};
#endif

float hal_adc_value_cal(adc_CH_t ch,uint16_t* buf, uint32_t size, uint8_t high_resol, uint8_t diff_mode)
{
    uint32_t i;
    int adc_sum = 0;
    volatile float result = 0.0;
	uint16_t adc_cal_postive = mAdc_Ctx.adc_cal_postive;
    uint16_t adc_cal_negtive = mAdc_Ctx.adc_cal_negtive;
	
    for (i = 0; i < size; i++)
    {
        adc_sum += (buf[i]&0xfff);
    }
    result = ((float)adc_sum)/size;
	if((adc_cal_postive != 0xfff) && (adc_cal_negtive != 0xfff))
    {
        float delta = ((int)(adc_cal_postive-adc_cal_negtive))/2.0;

        if(ch&0x01)
        {
            result = (diff_mode) ? ((result-2048-delta)*2/(adc_cal_postive+adc_cal_negtive))
                     : ((result-delta) /(adc_cal_postive+adc_cal_negtive));
        }
        else
        {
            result = (diff_mode) ? ((result-2048-delta)*2/(adc_cal_postive+adc_cal_negtive))
                     : ((result+delta) /(adc_cal_postive+adc_cal_negtive));
        }
    }
    else
    {
        result = (diff_mode) ? (float)(result / 2048 -1) : (float)(result /4096);
    }

    if(high_resol == TRUE)
    {
        result *= 800.0;
    }
    else
    {
        result = (float)result *(float)adc_Lambda[ch-2]*0.8/1000;
    }
    return result;
}
