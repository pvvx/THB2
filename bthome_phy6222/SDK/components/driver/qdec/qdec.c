/*******************************************************************************
    @file     qdec.c
    @brief    Contains all functions support for key scan driver
    @version  0.0
    @date     13. Nov. 2017
    @author   Ding

 SDK_LICENSE

*******************************************************************************/
#include "rom_sym_def.h"
#include <string.h>
#include "bus_dev.h"

#include "OSAL.h"
#include "qdec.h"
#include "gpio.h"
#include "uart.h"
#include "log.h"
#include "pwrmgr.h"
#include "error.h"
#include "clock.h"


qdec_Ctx_t m_qdecCtx;


void __attribute__((used)) hal_QDEC_IRQHandler()
{
    hal_gpio_pin_init(P20, OEN);
    hal_gpio_write(P20,1);
    osal_stop_timerEx(m_qdecCtx.qdec_task_id, m_qdecCtx.timeout_event);
    WaitMs(1);
    int32_t delta = GET_CNT_QUAN(m_qdecCtx.cfg.qdec_chn);
    m_qdecCtx.count += delta;

    if(m_qdecCtx.cfg.evt_handler)
    {
        qdec_Evt_t evt;
        evt.count = m_qdecCtx.count;
        m_qdecCtx.cfg.evt_handler(&evt);
    }

    CLR_INT_QUAN(m_qdecCtx.cfg.qdec_chn);
    hal_pwrmgr_unlock(MOD_QDEC);
    hal_gpio_pin_init(P20, OEN);
    hal_gpio_write(P20,0);
}

void hal_qdec_timeout_handler()
{
    osal_stop_timerEx(m_qdecCtx.qdec_task_id, m_qdecCtx.timeout_event);
    hal_pwrmgr_unlock(MOD_QDEC);
}

/**************************************************************************************
    @fn          hal_qdec_set_cha

    @brief       This function process for qdec initial

    input parameters

    @param       None.

    output parameters

    @param       None.

    @return      None.
 **************************************************************************************/
static void hal_qdec_set_cha(QDEC_CHN_e qdecCHN,gpio_pin_e pin)
{
    hal_gpio_pull_set(pin, PULL_DOWN);
    hal_gpio_fmux_set(pin, (Fmux_Type_e)(FMUX_CHAX + (qdecCHN*3)));
}

/**************************************************************************************
    @fn          hal_qdec_set_chb

    @brief       This function process for qdec initial

    input parameters

    @param       None.

    output parameters

    @param       None.

    @return      None.
 **************************************************************************************/
static void hal_qdec_set_chb(QDEC_CHN_e qdecCHN,gpio_pin_e pin)
{
    hal_gpio_pull_set(pin, PULL_DOWN);
    hal_gpio_fmux_set(pin, (Fmux_Type_e)(FMUX_CHBX + (qdecCHN*3)));
}

/**************************************************************************************
    @fn          hal_qdec_set_chi

    @brief       This function process for qdec initial

    input parameters

    @param       None.

    output parameters

    @param       None.

    @return      None.
 **************************************************************************************/
static void hal_qdec_set_chi(QDEC_CHN_e qdecCHN,gpio_pin_e pin)
{
    hal_gpio_pull_set(pin, PULL_DOWN);
    hal_gpio_fmux_set(pin, (Fmux_Type_e)(FMUX_CHIX + (qdecCHN*3)));
}

/**************************************************************************************
    @fn          hal_qdec_init

    @brief       This function process for qdec initial

    input parameters

    @param       qdec_Cfg_t cfg

    output parameters

    @param       None.

    @return      None.
 **************************************************************************************/
int hal_qdec_init(qdec_Cfg_t cfg, uint8 task_id, uint16 event)
{
    if(m_qdecCtx.enable)
        return PPlus_ERR_INVALID_STATE;

    uint8_t pins[3] = {cfg.cha_pin, cfg.chb_pin, cfg.chi_pin};
    memcpy(m_qdecCtx.pin_arr, pins, sizeof(uint8_t)*3);
    m_qdecCtx.cfg = cfg;
    m_qdecCtx.qdec_task_id = task_id;
    m_qdecCtx.timeout_event = event;
    m_qdecCtx.enable = TRUE;
    qdec_hw_config();
    hal_pwrmgr_register(MOD_QDEC, qdec_sleep_handler, qdec_wakeup_handler);
    return PPlus_SUCCESS;
}

void qdec_hw_config()
{
    qdec_Cfg_t* cfg = &(m_qdecCtx.cfg);
    hal_clk_gate_enable(MOD_QDEC);
    hal_qdec_set_cha(cfg->qdec_chn, cfg->cha_pin);
    hal_qdec_set_chb(cfg->qdec_chn, cfg->chb_pin);
    DIS_INT_INCN(cfg->qdec_chn);
    DIS_INT_QUAN(cfg->qdec_chn);
    DIS_INT_02F_QUAN(cfg->qdec_chn);
    DIS_INT_F20_QUAN(cfg->qdec_chn);
    SET_MODE_QUAN(cfg->qdec_chn, cfg->quaMode);
    hal_qdec_set_qua_irq(cfg->qdec_chn, cfg->intMode);

    if(cfg->use_inc)
    {
        hal_qdec_set_chi(cfg->qdec_chn, cfg->chi_pin);

        if(cfg->use_inc_irq)
        {
            hal_qdec_set_inc_irq(cfg->qdec_chn, cfg->incMode, cfg->intMode);
        }
    }

    QDEC_IRQ_ENABLE;
    ENABLE_CHN(cfg->qdec_chn);
}

static void qdec_sleep_handler(void)
{
    uint8_t pin_num;
    pin_num = m_qdecCtx.cfg.use_inc ? 3:2;

    for(uint8_t i=0; i<pin_num; i++)
    {
        GPIO_Wakeup_Pol_e pol;
        gpio_pin_e pin = m_qdecCtx.pin_arr[i];
        hal_gpio_pin_init(pin, IE);
        pol = hal_gpio_read(pin) ? NEGEDGE:POSEDGE;
        hal_gpio_wakeup_set(pin, pol);
        m_qdecCtx.pin_state[i] = pol;
    }

    DISABLE_CHN(m_qdecCtx.cfg.qdec_chn);
    hal_gpio_pin_init(P23, OEN);
    hal_gpio_write(P23,0);
}

static void qdec_wakeup_handler(void)
{
    hal_gpio_pin_init(P23, OEN);
    hal_gpio_write(P23,1);
    uint8_t pin_num;
    pin_num = m_qdecCtx.cfg.use_inc ? 3:2;
    GPIO_Wakeup_Pol_e pol;

    for(uint8_t i=0; i<pin_num; i++)
    {
        hal_gpio_pin_init(m_qdecCtx.pin_arr[i], IE);
        pol = hal_gpio_read(m_qdecCtx.pin_arr[i]) ? POSEDGE:NEGEDGE;

        if(pol == m_qdecCtx.pin_state[i])
        {
            break;
        }
        else if(i == pin_num-1)
        {
            return;
        }
    }

    int32_t delta = GET_CNT_QUAN(m_qdecCtx.cfg.qdec_chn);
    hal_gpio_pin_init(P31, OEN);
    hal_gpio_write(P31,0);
    hal_pwrmgr_lock(MOD_QDEC);
    qdec_hw_config();
    hal_gpio_pin_init(P31, OEN);
    hal_gpio_write(P31,1);
    osal_start_timerEx(m_qdecCtx.qdec_task_id, m_qdecCtx.timeout_event, 150);
}

/**************************************************************************************
    @fn          hal_qdec_set_qua_irq

    @brief       This function process for setting qdecode counter interupt mode

    input parameters

    @param       QDEC_CHN_e chn
    @param       QDEC_INT_MODE_e intMode

    output parameters

    @param       None.

    @return      None.
 **************************************************************************************/
static void hal_qdec_set_qua_irq(QDEC_CHN_e chn, QDEC_INT_MODE_e intMode)
{
    SET_INT_MODE_QUAN(chn, intMode);
    EN_INT_QUAN(chn);
}

/**************************************************************************************
    @fn          hal_qdec_set_inc_irq

    @brief       This function process for setting qdecode index count mode and interupt mode

    input parameters

    @param       QDEC_CHN_e chn
    @param       QDEC_INT_MODE_e intMode
    @param       QDEC_INC_MODE_e incMode

    output parameters

    @param       None.

    @return      None.
 **************************************************************************************/
static void hal_qdec_set_inc_irq(QDEC_CHN_e chn, QDEC_INC_MODE_e incMode, QDEC_INT_MODE_e intMode)
{
    SET_MODE_INCN(chn, incMode);
    SET_INT_MODE_INCN(chn, intMode);
    EN_INT_INCN(chn);
}

