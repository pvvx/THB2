/*******************************************************************************
    @file     pwm.c
    @brief    Contains all functions support for pwm driver
    @version  0.0
    @date     30. Oct. 2017
    @author   Ding

 SDK_LICENSE

*******************************************************************************/
#include "rom_sym_def.h"
#include "gpio.h"
#include "clock.h"
#include "pwm.h"
#include "pwrmgr.h"

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
void hal_pwm_init(
    PWMN_e pwmN,
    PWM_CLK_DIV_e pwmDiv,
    PWM_CNT_MODE_e pwmMode,
    PWM_POLARITY_e pwmPolarity)
{
    hal_clk_gate_enable(MOD_PWM);
    PWM_DISABLE_CH(pwmN);
    PWM_SET_DIV(pwmN, pwmDiv);
    PWM_SET_MODE(pwmN, pwmMode);
    PWM_SET_POL(pwmN, pwmPolarity);
    PWM_INSTANT_LOAD_CH(pwmN);
    hal_pwrmgr_register(MOD_PWM, NULL, NULL);
}

static gpio_pin_e pwm_gpio_map[]=
{
    GPIO_DUMMY,
    GPIO_DUMMY,
    GPIO_DUMMY,
    GPIO_DUMMY,
    GPIO_DUMMY,
    GPIO_DUMMY,
};
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
void hal_pwm_open_channel(PWMN_e pwmN,gpio_pin_e pwmPin)
{
    hal_gpio_fmux_set(pwmPin, (gpio_fmux_e)(FMUX_PWM0 + pwmN));
    PWM_ENABLE_CH(pwmN);
    pwm_gpio_map[pwmN] = pwmPin;
}

/**************************************************************************************
    @fn          hal_pwm_close_channel

    @brief       This function process for pwm stop working

    input parameters

    @param       PWMN_e pwmN                     : pwm channel

    output parameters

    @param       None.

    @return      None.
 **************************************************************************************/
void hal_pwm_close_channel(PWMN_e pwmN)
{
    if(pwm_gpio_map[pwmN] != GPIO_DUMMY)
    {
        hal_gpio_fmux(pwm_gpio_map[pwmN],Bit_DISABLE);
        pwm_gpio_map[pwmN] = GPIO_DUMMY;
    }

    PWM_DISABLE_CH(pwmN);
}

/**************************************************************************************
    @fn          hal_pwm_destroy

    @brief       This function process for pwm clear and disable

    input parameters

    @param       PWMN_e pwmN                     : pwm channel

    output parameters

    @param       None.

    @return      None.
 **************************************************************************************/
void hal_pwm_destroy(PWMN_e pwmN)
{
    PWM_DISABLE_CH(pwmN);
    PWM_NO_LOAD_CH(pwmN);
    PWM_NO_INSTANT_LOAD_CH(pwmN);
    PWM_SET_DIV(pwmN, 0);
    PWM_SET_MODE(pwmN, 0);
    PWM_SET_POL(pwmN, 0);
    PWM_SET_TOP_VAL(pwmN, 0);
    PWM_SET_CMP_VAL(pwmN, 0);
}

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
void hal_pwm_set_count_val(PWMN_e pwmN, uint16_t cmpVal, uint16_t cntTopVal)
{
    if(cmpVal > cntTopVal)
        return;

    PWM_NO_LOAD_CH(pwmN);
    PWM_SET_CMP_VAL(pwmN, cmpVal);
    PWM_SET_TOP_VAL(pwmN, cntTopVal);
    PWM_LOAD_CH(pwmN);
}

static unsigned int pwm_en = 0;
void hal_pwm_start(void)
{
    if(pwm_en == 0)
    {
        hal_pwrmgr_lock(MOD_PWM);
        PWM_ENABLE_ALL;
        pwm_en = 1;
    }
}

void hal_pwm_stop(void)
{
    if(pwm_en == 1)
    {
        hal_pwrmgr_unlock(MOD_PWM);
        PWM_DISABLE_ALL;
        pwm_en = 0;
        hal_clk_gate_disable(MOD_PWM);
    }
}

//------------------------------------------------------------
//new api,make use easily
typedef struct
{
    bool          enable;
    bool          ch_en[6];
    pwm_ch_t      ch[6];
} pwm_Ctx_t;

static pwm_Ctx_t pwmCtx =
{
    .enable = FALSE,
    .ch_en = {FALSE,FALSE,FALSE,FALSE,FALSE,FALSE},
};

void hal_pwm_module_init(void)
{
    int i = 0;

    if(pwmCtx.enable == TRUE)
        return;

    pwmCtx.enable = TRUE;

    for(i = 0; i < 6; i++)
    {
        pwmCtx.ch_en[i] = FALSE;
        pwmCtx.ch[i].pwmN = (PWMN_e)i;
        pwmCtx.ch[i].pwmPin = GPIO_DUMMY;
        pwmCtx.ch[i].pwmDiv = PWM_CLK_NO_DIV;
        pwmCtx.ch[i].pwmMode = PWM_CNT_UP;
        pwmCtx.ch[i].pwmPolarity = PWM_POLARITY_RISING;
        pwmCtx.ch[i].cmpVal = 0;
        pwmCtx.ch[i].cntTopVal = 0;
        hal_pwm_destroy((PWMN_e)i);
    }

    hal_pwm_stop();
}

void hal_pwm_module_deinit(void)
{
    int i = 0;

    if(pwmCtx.enable == FALSE)
        return;

    pwmCtx.enable = FALSE;

    for(i = 0; i < 6; i++)
    {
        pwmCtx.ch_en[i] = FALSE;
        pwmCtx.ch[i].pwmN = (PWMN_e)i;
        pwmCtx.ch[i].pwmPin = GPIO_DUMMY;
        pwmCtx.ch[i].pwmDiv = PWM_CLK_NO_DIV;
        pwmCtx.ch[i].pwmMode = PWM_CNT_UP;
        pwmCtx.ch[i].pwmPolarity = PWM_POLARITY_RISING;
        pwmCtx.ch[i].cmpVal = 0;
        pwmCtx.ch[i].cntTopVal = 0;
        hal_pwm_close_channel((PWMN_e)i);
        hal_pwm_destroy((PWMN_e)i);
    }

    hal_pwm_stop();
}

void hal_pwm_ch_start(pwm_ch_t ch)
{
    if(pwmCtx.enable == FALSE)
        return;

    if(pwmCtx.ch_en[ch.pwmN] == TRUE)
    {
        hal_pwm_set_count_val(ch.pwmN,ch.cmpVal,ch.cntTopVal);
        PWM_SET_DIV(ch.pwmN, ch.pwmDiv);
    }
    else
    {
        hal_pwm_init(ch.pwmN,ch.pwmDiv,ch.pwmMode,ch.pwmPolarity);
        hal_pwm_set_count_val(ch.pwmN,ch.cmpVal,ch.cntTopVal);
        hal_pwm_open_channel(ch.pwmN,ch.pwmPin);
        pwmCtx.ch_en[ch.pwmN] = TRUE;
        hal_pwm_start();
    }
}

void hal_pwm_ch_stop(pwm_ch_t ch)
{
    if(pwmCtx.ch_en[ch.pwmN] == FALSE)
        return;
    else
    {
        pwmCtx.ch_en[ch.pwmN] = FALSE;
        hal_pwm_destroy(ch.pwmN);
        hal_pwm_close_channel(ch.pwmN);
    }
}

bool hal_pwm_ch_enable(PWMN_e pwmN)
{
    return pwmCtx.ch_en[pwmN];
}

pwm_ch_t hal_pwm_ch_reg(PWMN_e pwmN)
{
    return pwmCtx.ch[pwmN];
}
