/*******************************************************************************
    @file     pwm.h
    @brief    Contains all functions support for pwm driver
    @version  0.0
    @date     30. Oct. 2017
    @author   Ding

 SDK_LICENSE

*******************************************************************************/
#ifndef __PWM__H__
#define __PWM__H__

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"
#include "gpio.h"


#define    PWM_ENABLE_ALL               do{\
        AP_PWM->pwmen |= BIT(0);\
        AP_PWM->pwmen |= BIT(4);\
    }while(0)
#define    PWM_DISABLE_ALL              do{\
        AP_PWM->pwmen &= ~BIT(0);\
        AP_PWM->pwmen &= ~BIT(4);\
    }while(0)
#define    PWM_ENABLE_CH_012            do{\
        AP_PWM->pwmen |= BIT(8);\
        AP_PWM->pwmen |= BIT(9);\
    }while(0)
#define    PWM_DISABLE_CH_012           do{\
        AP_PWM->pwmen &= ~BIT(8);\
        AP_PWM->pwmen &= ~BIT(9);\
    }while(0)
#define    PWM_ENABLE_CH_345            do{\
        AP_PWM->pwmen |= BIT(10);\
        AP_PWM->pwmen |= BIT(11);\
    }while(0)
#define    PWM_DISABLE_CH_345           do{\
        AP_PWM->pwmen &= ~BIT(10);\
        AP_PWM->pwmen &= ~BIT(11);\
    }while(0)
#define    PWM_ENABLE_CH_01             do{\
        AP_PWM->pwmen |= BIT(12);\
        AP_PWM->pwmen |= BIT(13);\
    }while(0)
#define    PWM_DISABLE_CH_01            do{\
        AP_PWM->pwmen &= ~BIT(12);\
        AP_PWM->pwmen &= ~BIT(13);\
    }while(0)
#define    PWM_ENABLE_CH_23             do{\
        AP_PWM->pwmen |= BIT(14);\
        AP_PWM->pwmen |= BIT(15);\
    }while(0)
#define    PWM_DISABLE_CH_23            do{\
        AP_PWM->pwmen &= ~BIT(14);\
        AP_PWM->pwmen &= ~BIT(15);\
    }while(0)
#define    PWM_ENABLE_CH_45             do{\
        AP_PWM->pwmen |= BIT(16);\
        AP_PWM->pwmen |= BIT(17);\
    }while(0)
#define    PWM_DISABLE_CH_45            do{\
        AP_PWM->pwmen &= ~BIT(16);\
        AP_PWM->pwmen &= ~BIT(17);\
    }while(0)

#define    PWM_INSTANT_LOAD_CH(n)       subWriteReg(&(AP_PWM_CTRL(n)->ctrl0),31,31,1)
#define    PWM_NO_INSTANT_LOAD_CH(n)    subWriteReg(&(AP_PWM_CTRL(n)->ctrl0),31,31,0)
#define    PWM_LOAD_CH(n)               subWriteReg(&(AP_PWM_CTRL(n)->ctrl0),16,16,1)
#define    PWM_NO_LOAD_CH(n)            subWriteReg(&(AP_PWM_CTRL(n)->ctrl0),16,16,0)
#define    PWM_SET_DIV(n,v)             subWriteReg(&(AP_PWM_CTRL(n)->ctrl0),14,12,v)
#define    PWM_SET_MODE(n,v)            subWriteReg(&(AP_PWM_CTRL(n)->ctrl0),8,8,v)
#define    PWM_SET_POL(n,v)             subWriteReg(&(AP_PWM_CTRL(n)->ctrl0),4,4,v)
#define    PWM_ENABLE_CH(n)             subWriteReg(&(AP_PWM_CTRL(n)->ctrl0),0,0,1)
#define    PWM_DISABLE_CH(n)            subWriteReg(&(AP_PWM_CTRL(n)->ctrl0),0,0,0)

#define    PWM_SET_CMP_VAL(n,v)         subWriteReg(&(AP_PWM_CTRL(n)->ctrl1),31,16,v)
#define    PWM_SET_TOP_VAL(n,v)         subWriteReg(&(AP_PWM_CTRL(n)->ctrl1),15,0,v)
#define    PWM_GET_CMP_VAL(n)           ((AP_PWM_CTRL(n)->ctrl1 & 0xFFFF0000) >> 8)
#define    PWM_GET_TOP_VAL(n)           AP_PWM_CTRL(n)->ctrl1 & 0x0000FFFF


/*************************************************************
    @brief      enum variable, the number of PWM channels supported

*/
typedef enum
{
    PWM_CH0 = 0,
    PWM_CH1 = 1,
    PWM_CH2 = 2,
    PWM_CH3 = 3,
    PWM_CH4 = 4,
    PWM_CH5 = 5
} PWMN_e;

/*************************************************************
    @brief      enum variable used for PWM clock prescaler

*/
typedef enum
{
    PWM_CLK_NO_DIV = 0,
    PWM_CLK_DIV_2 = 1,
    PWM_CLK_DIV_4 = 2,
    PWM_CLK_DIV_8 = 3,
    PWM_CLK_DIV_16 = 4,
    PWM_CLK_DIV_32 = 5,
    PWM_CLK_DIV_64 = 6,
    PWM_CLK_DIV_128 = 7
} PWM_CLK_DIV_e;

/*************************************************************
    @brief      enum variable used for PWM work mode setting

*/
typedef enum
{
    PWM_CNT_UP = 0,
    PWM_CNT_UP_AND_DOWN = 1
} PWM_CNT_MODE_e;

/*************************************************************
    @brief      enum variable used for PWM output polarity setting

*/
typedef enum
{
    PWM_POLARITY_RISING = 0,
    PWM_POLARITY_FALLING = 1
} PWM_POLARITY_e;

/**************************************************************************************
    @fn          hal_pwm_init

    @brief       This function process for pwm initial

    input parameters

    @param       PWMN_e pwmN                     : pwm channel
                PWM_CLK_DIV_e pwmDiv            : clock prescaler of PWM channel
                PWM_CNT_MODE_e pwmMode          : count mode of PWM channel
                PWM_POLARITY_e pwmPolarity      : output polarity setting of PWM channel
                unsigned short cmpVal           : the compare value of PWM channel
                unsigned short cntTopVal        : the counter top value of PWM channel

    output parameters

    @param       None.

    @return      None.
 **************************************************************************************/
void hal_pwm_init(PWMN_e pwmN, PWM_CLK_DIV_e pwmDiv,
                  PWM_CNT_MODE_e pwmMode, PWM_POLARITY_e pwmPolarity);

/**************************************************************************************
    @fn          hal_pwm_open_channel

    @brief       This function process for pwm start working

    input parameters

    @param       PWMN_e pwmN                     : pwm channel
                gpio_pin_e pwmPin               : pwm pin number

    output parameters

    @param       None.

    @return      None.
 **************************************************************************************/
void hal_pwm_open_channel(PWMN_e pwmN,gpio_pin_e pwmPin);

/**************************************************************************************
    @fn          hal_pwm_close_channel

    @brief       This function process for pwm stop working

    input parameters

    @param       PWMN_e pwmN                     : pwm channel

    output parameters

    @param       None.

    @return      None.
 **************************************************************************************/
void hal_pwm_close_channel(PWMN_e pwmN);

/**************************************************************************************
    @fn          hal_pwm_destroy

    @brief       This function process for pwm clear and disable

    input parameters

    @param       PWMN_e pwmN                     : pwm channel

    output parameters

    @param       None.

    @return      None.
 **************************************************************************************/
void hal_pwm_destroy(PWMN_e pwmN);

/**************************************************************************************
    @fn          hal_pwm_set_count_val

    @brief       This function process for change pwm count value

    input parameters

    @param       PWMN_e pwmN                     : pwm channel
                uint16_t cmpVal                 : the compare value of PWM channel
                uint16_t cntTopVal              : the counter top value of PWM channel

    output parameters

    @param       None.

    @return      None.
 **************************************************************************************/
void hal_pwm_set_count_val(PWMN_e pwmN, uint16_t cmpVal, uint16_t cntTopVal);

/**************************************************************************************
    @fn          hal_pwm_start

    @brief       pwm start

    input parameters

    @param       None.

    output parameters

    @param       None.

    @return      None.
 **************************************************************************************/
void hal_pwm_start(void);

/**************************************************************************************
    @fn          hal_pwm_stop

    @brief       pwm stop

    input parameters

    @param       None.

    output parameters

    @param       None.

    @return      None.
 **************************************************************************************/
void hal_pwm_stop(void);

//new api,make use easily
typedef struct
{
    PWMN_e pwmN;
    gpio_pin_e pwmPin;
    PWM_CLK_DIV_e pwmDiv;
    PWM_CNT_MODE_e pwmMode;
    PWM_POLARITY_e pwmPolarity;
    uint16_t cmpVal;
    uint16_t cntTopVal;

} pwm_ch_t;

/**************************************************************************************
    @fn          hal_pwm_module_init

    @brief       init pwm global variables

    input parameters

    @param       None.

    output parameters

    @param       None.

    @return      None.
 **************************************************************************************/
void hal_pwm_module_init(void);

/**************************************************************************************
    @fn          hal_pwm_module_deinit

    @brief       deinit pwm global variables

    input parameters

    @param       None.

    output parameters

    @param       None.

    @return      None.
 **************************************************************************************/
void hal_pwm_module_deinit(void);

/**************************************************************************************
    @fn          hal_pwm_ch_start

    @brief       config and make a pwm start to work

    input parameters

    @param       pwm_ch_t ch: pwm channel

    output parameters

    @param       None.

    @return      None.
 **************************************************************************************/
void hal_pwm_ch_start(pwm_ch_t ch);

/**************************************************************************************
    @fn          hal_pwm_ch_stop

    @brief       make a pwm stop form working

    input parameters

    @param       pwm_ch_t ch: pwm channel

    output parameters

    @param       None.

    @return      None.
 **************************************************************************************/
void hal_pwm_ch_stop(pwm_ch_t ch);

#ifdef __cplusplus
}
#endif


#endif
