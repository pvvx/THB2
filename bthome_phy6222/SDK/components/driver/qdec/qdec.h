/*******************************************************************************
    @file     qdec.h
    @brief    Contains all functions support for key scan driver
    @version  0.0
    @date     13. Nov. 2017
    @author   Ding

 SDK_LICENSE

*******************************************************************************/
#ifndef __QDEC__H__
#define __QDEC__H__

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"
#include "gpio.h"

#define     QDEC_IRQ_ENABLE             *(volatile unsigned int *) 0xe000e100 |= BIT(30)

#define     ENABLE_CHN(n)               *(volatile unsigned int *) 0x4000B000 |= BIT(4*n)
#define     DISABLE_CHN(n)              *(volatile unsigned int *) 0x4000B000 &= ~BIT(4*n)

#define     EN_INT_QUAN(n)              *(volatile unsigned int *) 0x4000B004 |= BIT(8*n)
#define     EN_INT_INCN(n)              *(volatile unsigned int *) 0x4000B004 |= BIT(4+8*n)
#define     DIS_INT_QUAN(n)             *(volatile unsigned int *) 0x4000B004 &= ~BIT(8*n)
#define     DIS_INT_INCN(n)             *(volatile unsigned int *) 0x4000B004 &= ~BIT(4+8*n)
#define     SET_INT_MODE_QUAN(chn,mod)  subWriteReg(0x4000B004,2+8*chn,2+8*chn,mod)
#define     SET_INT_MODE_INCN(chn,mod)  subWriteReg(0x4000B004,6+8*chn,6+8*chn,mod)

#define     EN_INT_F20_QUAN(n)          *(volatile unsigned int *) 0x4000B004 |= BIT(24+2*n)
#define     EN_INT_02F_QUAN(n)          *(volatile unsigned int *) 0x4000B004 |= BIT(25+2*n)
#define     DIS_INT_F20_QUAN(n)         *(volatile unsigned int *) 0x4000B004 &= ~BIT(24+2*n)
#define     DIS_INT_02F_QUAN(n)         *(volatile unsigned int *) 0x4000B004 &= ~BIT(25+2*n)

#define     CLR_INT_F20_QUAN(n)         *(volatile unsigned int *) 0x4000B008 |= BIT(24+2*n)
#define     CLR_INT_02F_QUAN(n)         *(volatile unsigned int *) 0x4000B008 |= BIT(25+2*n)
#define     CLR_INT_QUAN(n)             *(volatile unsigned int *) 0x4000B008 |= BIT(8*n)
#define     CLR_INT_INCN(n)             *(volatile unsigned int *) 0x4000B008 |= BIT(4+8*n)

#define     STATUS_INT_QUAN(n)          read_reg(0x4000B00C) & BIT(8*n)
#define     STATUS_INT_INCN(n)          read_reg(0x4000B00C) & BIT(4+8*n)
#define     STATUS_INT_F20_QUAN(n)      read_reg(0x4000B00C) & BIT(24+2*n)
#define     STATUS_INT_02F_QUAN(n)      read_reg(0x4000B00C) & BIT(25+2*n)

#define     SET_MODE_QUAN(chn,mod)      subWriteReg(0x4000B010 + 0x14*chn,1,0, mod)
#define     SET_MODE_INCN(chn,mod)      subWriteReg(0x4000B010 + 0x14*chn,17,16,mod)
#define     SET_HIT_QUAN(chn,mod)       write_reg(0x4000B014 + 0x14*chn,mod)
#define     SET_HIT_INCN(chn,mod)       write_reg(0x4000B018 + 0x14*chn,mod)
#define     GET_CNT_QUAN(n)             read_reg(0x4000B01C + 0x14*n)
#define     GET_CNT_INCN(n)             read_reg(0x4000B020 + 0x14*n)

/*************************************************************
    @brief      enum variable used for setting channel

*/
typedef enum
{

    QDEC_CHX   =   0,
    QDEC_CHY   =   1,
    QDEC_CHZ   =   2

} QDEC_CHN_e;

/*************************************************************
    @brief      enum variable used for setting quadrature count mode

*/
typedef enum
{

    QDEC_MODE_1X = 1,
    QDEC_MODE_2X = 2,
    QDEC_MODE_4X = 3

} QDEC_QUA_MODE_e;

/*************************************************************
    @brief      enum variable used for setting index count mode

*/
typedef enum
{

    HIGH_LEVEL = 0,
    POS_EDGE = 1,
    NEG_EDGE = 2,
    POS_OR_NEG_EDGE = 3,

} QDEC_INC_MODE_e;

/*************************************************************
    @brief      enum variable used for setting interupt mode

*/
typedef enum
{

    INT_BY_CHANGE = 0,
    INT_BY_HIT = 1

} QDEC_INT_MODE_e;

typedef struct
{
    int32_t         count;
} qdec_Evt_t;

typedef void (*qdec_Hdl_t)(qdec_Evt_t* pev);

typedef struct
{
    gpio_pin_e      cha_pin;
    gpio_pin_e      chb_pin;
    QDEC_CHN_e      qdec_chn;
    QDEC_QUA_MODE_e quaMode;
    QDEC_INT_MODE_e intMode;
    qdec_Hdl_t      evt_handler;
    bool            use_inc;
    bool            use_inc_irq;
    gpio_pin_e      chi_pin;
    QDEC_INC_MODE_e incMode;
} qdec_Cfg_t;

typedef struct
{
    bool        enable;
    qdec_Cfg_t  cfg;
    int32_t     count;
    uint8_t     pin_state[3];
    gpio_pin_e  pin_arr[3];
    uint8_t     qdec_task_id;
    uint16_t    timeout_event;
} qdec_Ctx_t;

static void qdec_hw_config(void);
static void qdec_sleep_handler(void);
static void qdec_wakeup_handler(void);
static void hal_qdec_set_cha(QDEC_CHN_e qdecCHN,gpio_pin_e pin);
static void hal_qdec_set_chb(QDEC_CHN_e qdecCHN,gpio_pin_e pin);
static void hal_qdec_set_chi(QDEC_CHN_e qdecCHN,gpio_pin_e pin);
static void hal_qdec_set_qua_irq(QDEC_CHN_e chn, QDEC_INT_MODE_e intMode);
static void hal_qdec_set_inc_irq(QDEC_CHN_e chn, QDEC_INC_MODE_e incMode, QDEC_INT_MODE_e intMode);

int hal_qdec_init(qdec_Cfg_t cfg, uint8 task_id, uint16 event);
void hal_qdec_timeout_handler(void);
void hal_qdec_IRQHandler(void);

#ifdef __cplusplus
}
#endif

#endif
