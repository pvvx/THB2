/*******************************************************************************
    @file     adc.h
    @brief    Contains all functions support for adc driver
    @version  0.0
    @date     18. Oct. 2017
    @author   qing.han

	SDK_LICENSE

*******************************************************************************/
#ifndef __ADC__H__
#define __ADC__H__

#ifdef __cplusplus
extern "C" {
#endif


#include "types.h"
#include "bus_dev.h"
#include "gpio.h"

#define    MAX_ADC_SAMPLE_SIZE     32
#define    ADC_CH_BASE             (0x40050400UL)



#define    CLEAR_ADC_INT(n)     AP_ADC->intr_clear |= BIT(n)

#define    IS_CLAER_ADC_INT_VOICE (AP_ADC->intr_clear & BIT(8))
#define    IS_CLAER_ADC_INT(n)    (AP_ADC->intr_clear & BIT(n))

#define    GET_IRQ_STATUS         (AP_ADCC->intr_status & 0x3ff)

#define    ENABLE_ADC             (AP_PCRM->ANA_CTL |= BIT(3))
#define    DISABLE_ADC            (AP_PCRM->ANA_CTL &= ~BIT(3))

#define    ADC_CLOCK_ENABLE       (AP_PCRM->CLKHF_CTL1 |= BIT(13))
#define    ADC_CLOCK_DISABLE       (AP_PCRM->CLKHF_CTL1 &= ~BIT(13))


#define SPIF_RSVD_AREA_1                 (0x1000)
#define pSPIF_RSVD1_ADC_CALIBRATE       ((volatile uint32_t*)(SPIF_BASE_ADDR + SPIF_RSVD_AREA_1))
#define SPIF_RSVD1_ADC_CALIBRATE        (SPIF_BASE_ADDR + SPIF_RSVD_AREA_1)



/**************************************************************************************
    @fn          hal_get_adc_int_source

    @brief       This function process for get adc interrupt source,such as adc channel NO

    input parameters

    @param       None.

    output parameters

    @param       None.

    @return      adc interrupt source bit loaction(uint8_t)
 **************************************************************************************/
/*
    ADC note:
    There are ten pins which can config as analogy,there are some differences between them.
    hardware analogy index:
    gpio<11>/aio<0>
    gpio<23>/aio<1>/micphone bias reference voltage
    gpio<24>/aio<2>
    gpio<14>/aio<3>
    gpio<15>/aio<4>/micphone bias
    gpio<16>/aio<5>/32K XTAL input
    gpio<17>/aio<6>/32K XTAL output
    gpio<18>/aio<7>/pga in+
    gpio<25>/aio<8>
    gpio<20>/aio<9>/pga in-

    There are six pins which can work in adc single mode.Such as:
    ADC_CH0 = 2,ADC_CH1N_P11 = 2,
    ADC_CH1 = 3,ADC_CH1P_P23 = 3,
    ADC_CH2 = 4,ADC_CH2N_P24 = 4,
    ADC_CH3 = 5,ADC_CH2P_P14 = 5,
    ADC_CH4 = 6,ADC_CH3N_P15 = 6,
    ADC_CH9 = 7,ADC_CH3P_P20 = 7,

    There are four pair pins which can work in adc diff mode.Such as:
    ADC_CH0DIFF = 1,p18(p) and P25(n)
    ADC_CH1DIFF = 3,P23(p) and P11(n)
    ADC_CH2DIFF = 5,P14(p) and P24(n)
    ADC_CH3DIFF = 7,P20(p) and P15(n)

    There are two pins which uses with 32.768K crystal oscillator.
    gpio<16>/aio<5>/32K XTAL input
    gpio<17>/aio<6>/32K XTAL output

    There are four pins which uses as pga,voice and so on.
    gpio<23>/aio<1>/micphone bias reference voltage,this pin is selected
    gpio<15>/aio<4>/micphone bias
    gpio<18>/aio<7>/pga in+
    gpio<20>/aio<9>/pga in-
*/
typedef enum
{
    ADC_CH0DIFF = 1,/*p18(positive),p25(negative),only works in diff*/
    ADC_CH0 = 2,ADC_CH1N_P11 = 2,MIN_ADC_CH = 2,
    ADC_CH1 = 3,ADC_CH1P_P23 = 3,ADC_CH1DIFF = 3,/*P23 and P11*/
    ADC_CH2 = 4,ADC_CH2N_P24 = 4,
    ADC_CH3 = 5,ADC_CH2P_P14 = 5,ADC_CH2DIFF = 5,/*P14 and P24*/
    ADC_CH4 = 6,ADC_CH3N_P15 = 6,
    ADC_CH9 = 7,ADC_CH3P_P20 = 7,MAX_ADC_CH = 7,ADC_CH3DIFF = 7,/*P20 and P15*/
    ADC_CH_VOICE = 8,
    ADC_CH_NUM =9,
} adc_CH_t;


#define ADC_BIT(ch) (1<<ch)

enum
{
    HAL_ADC_EVT_DATA = 1,
    HAL_ADC_EVT_FAIL = 0xff
};

typedef enum
{
    HAL_ADC_CLOCK_80K = 0,
    HAL_ADC_CLOCK_160K = 1,
    HAL_ADC_CLOCK_320K = 2,
} adc_CLOCK_SEL_t;

typedef struct _adc_Cfg_t
{
    uint8_t channel;
    bool  is_continue_mode;
    uint8_t  is_differential_mode;
    uint8_t  is_high_resolution;
} adc_Cfg_t;


typedef struct _adc_Evt_t
{
    int       type;
    adc_CH_t  ch;
    uint16_t* data;
    uint8_t   size; //word size
} adc_Evt_t;

typedef void (*adc_Hdl_t)(adc_Evt_t* pev);



extern gpio_pin_e s_pinmap[ADC_CH_NUM];
/**************************************************************************************
    @fn          hal_adc_init

    @brief       This function process for adc initial

    input parameters

    @param       ADC_CH_e adc_pin: adc pin select;ADC_CH0~ADC_CH7 and ADC_CH_VOICE
                ADC_SEMODE_e semode: single-end mode and diff mode select; 1:SINGLE_END(single-end mode) 0:DIFF(Diff mode)
                IO_CONTROL_e amplitude: input signal amplitude, 0:BELOW_1V,1:UP_1V

    output parameters

    @param       None.

    @return      None.
 **************************************************************************************/
void hal_adc_init(void);

int hal_adc_config_channel(adc_Cfg_t cfg, adc_Hdl_t evt_handler);

int hal_adc_clock_config(adc_CLOCK_SEL_t clk);

int hal_adc_start(void);

int hal_adc_stop(void);

void __attribute__((weak)) hal_ADC_IRQHandler(void);

float hal_adc_value_cal(adc_CH_t ch,uint16_t* buf, uint32_t size, uint8_t high_resol, uint8_t diff_mode);

#ifdef __cplusplus
}
#endif

#endif
