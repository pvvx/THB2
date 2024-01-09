/*******************************************************************************
    @file     kscan.c
    @brief    Contains all functions support for key scan driver
    @version  0.0
    @date     13. Nov. 2017
    @author   Ding

	SDK_LICENSE

*******************************************************************************/
#include "rom_sym_def.h"
#include <string.h>
#include "clock.h"
#include "OSAL.h"
#include "kscan.h"
#include "pwrmgr.h"
#include "error.h"
#include "gpio.h"
#include "uart.h"
#include "bus_dev.h"
#include "log.h"
#include "jump_function.h"
typedef struct
{
    bool            enable;
    kscan_Cfg_t     cfg;
    uint16_t        key_state[MULTI_KEY_NUM<<1];
    uint8_t         pin_state[NUM_KEY_ROWS];
    uint8_t         kscan_task_id;
    uint16_t        timeout_event;
} kscan_Ctx_t;

static kscan_Ctx_t m_kscanCtx;
static kscan_Key_t m_keys[MAX_KEY_NUM];

static uint8_t reScan_flag=0;


//PRIVATE FUNCTIONS
static void kscan_hw_config(void);
static void hal_kscan_config_row(KSCAN_ROWS_e row);
static void hal_kscan_config_col(KSCAN_COLS_e col);
static void kscan_sleep_handler(void);
static void kscan_wakeup_handler(void);
static void get_key_matrix(uint16_t* key_matrix);
static void rmv_ghost_key(uint16_t* key_matrix);
static kscan_Evt_t  kscan_compare_key(uint16_t* key_pre, uint16_t* key_nxt);
static void hal_kscan_clear_config(void);
extern void hal_gpioin_set_flag(gpio_pin_e pin);

#define TIMEOUT_DELTA   10
/**************************************************************************************
    @fn          hal_kscan_init

    @brief       This function process for key scan initial

    input parameters

    @param       None.

    output parameters

    @param       None.

    @return      None.
 **************************************************************************************/
int hal_kscan_init(kscan_Cfg_t cfg, uint8 task_id, uint16 event)
{
    if(m_kscanCtx.enable)
        return PPlus_ERR_INVALID_STATE;

    m_kscanCtx.cfg = cfg;
    m_kscanCtx.kscan_task_id = task_id;
    m_kscanCtx.timeout_event = event;
    m_kscanCtx.enable = TRUE;
    kscan_hw_config();
    JUMP_FUNCTION(KSCAN_IRQ_HANDLER)   =   (uint32_t)&hal_KSCAN_IRQHandler;
    hal_pwrmgr_register(MOD_KSCAN, kscan_sleep_handler, kscan_wakeup_handler);
    return PPlus_SUCCESS;
}

/**************************************************************************************
    @fn          hal_kscan_clear_config

    @brief       This function process for key scan clear config

    input parameters

    @param       None.

    output parameters

    @param       None.

    @return      None.
 **************************************************************************************/
void hal_kscan_clear_config()
{
    subWriteReg(&(AP_IOMUX->keyscan_in_en),10,0,0);//iomux:key_scan_in_en and key_scan_out_en
    subWriteReg(&(AP_IOMUX->keyscan_out_en),11,0,0);
    subWriteReg((&AP_KSCAN->ctrl0), 13, 2, 0);//kscan:mattrix scan outputs and mattrix scan inputs
    subWriteReg((&AP_KSCAN->mk_in_en), 10, 0, 0);
}


void __attribute__((used)) hal_KSCAN_IRQHandler()
{
    uint16_t key_nxt[MULTI_KEY_NUM<<1];

    if(reScan_flag==1)
        reScan_flag=0;

    osal_stop_timerEx(m_kscanCtx.kscan_task_id, m_kscanCtx.timeout_event);//todo
    get_key_matrix(key_nxt);

    if(m_kscanCtx.cfg.ghost_key_state == IGNORE_GHOST_KEY)
        rmv_ghost_key(key_nxt);

    if(m_kscanCtx.cfg.evt_handler)
    {
        kscan_Evt_t evt = kscan_compare_key(m_kscanCtx.key_state, key_nxt);

        if(evt.num>0)
            m_kscanCtx.cfg.evt_handler(&evt);
    }

    memcpy(m_kscanCtx.key_state, key_nxt, sizeof(uint16_t)*(MULTI_KEY_NUM<<1));
    osal_start_timerEx(m_kscanCtx.kscan_task_id, m_kscanCtx.timeout_event, (2*m_kscanCtx.cfg.interval+TIMEOUT_DELTA));//todo
}

void hal_kscan_timeout_handler()
{
    if(reScan_flag==0)
    {
        // LOG("kscan_reScan\n\r");
        hal_kscan_clear_config();
        reScan_flag=1;
        kscan_hw_config();
        osal_start_timerEx(m_kscanCtx.kscan_task_id, m_kscanCtx.timeout_event, m_kscanCtx.cfg.interval+TIMEOUT_DELTA);
    }
    else if(reScan_flag==1)
    {
        //LOG("kscan_timeout_handler\n\r");
        osal_stop_timerEx(m_kscanCtx.kscan_task_id, m_kscanCtx.timeout_event);
        uint16_t key_nxt[MULTI_KEY_NUM<<1];
        memset(&key_nxt[0],0,sizeof(uint16_t)*(MULTI_KEY_NUM<<1)); //all register must be 0.teedy add 2019/01/23

        //get_key_matrix(key_nxt); //no need to read the register,because keyScan didn't update the register .teedy add 2019/01/23

        if(m_kscanCtx.cfg.ghost_key_state == IGNORE_GHOST_KEY)
            rmv_ghost_key(key_nxt);

        if(m_kscanCtx.cfg.evt_handler)
        {
            kscan_Evt_t evt = kscan_compare_key(m_kscanCtx.key_state, key_nxt);
            m_kscanCtx.cfg.evt_handler(&evt);
        }

        memcpy(m_kscanCtx.key_state, key_nxt, sizeof(uint16_t)*(MULTI_KEY_NUM<<1));
        reScan_flag=0;
        hal_pwrmgr_unlock(MOD_KSCAN);
    }
}

static void kscan_hw_config(void)
{
    kscan_Cfg_t* cfg = &(m_kscanCtx.cfg);
    hal_clk_gate_enable(MOD_KSCAN);
    hal_kscan_clear_config();

    for(uint8_t i=0; i<NUM_KEY_ROWS; i++)
        hal_kscan_config_row(cfg->key_rows[i]);

    for(uint8_t i=0; i<NUM_KEY_COLS; i++)
        hal_kscan_config_col(cfg->key_cols[i]);

    subWriteReg((&AP_KSCAN->ctrl0),20,20,NOT_IGNORE_MULTI_KEY);
    subWriteReg((&AP_KSCAN->ctrl0),23,23,SENCE_LOW);//SENCE_HIGH
    subWriteReg((&AP_KSCAN->ctrl0),31,24,cfg->interval);
    NVIC_SetPriority((IRQn_Type)KSCAN_IRQn, IRQ_PRIO_HAL);
    NVIC_EnableIRQ((IRQn_Type)KSCAN_IRQn);
    subWriteReg((&AP_KSCAN->ctrl0),1,1,1);//kscan int enable
    subWriteReg((&AP_KSCAN->ctrl0),0,0,1);//kscan enable
}

/**************************************************************************************
    @fn          hal_kscan_config_row

    @brief       This function process for setting key row pin

    input parameters

    @param       KSCAN_ROWS_e row

    output parameters

    @param       None.

    @return      None.
 **************************************************************************************/
static void hal_kscan_config_row(KSCAN_ROWS_e row)
{
    gpio_pin_e row_pin = (gpio_pin_e)KSCAN_ROW_GPIO[row];
    hal_gpio_fmux(row_pin, Bit_DISABLE);
    hal_gpio_pull_set(row_pin,GPIO_PULL_UP_S);
    subWriteReg(&(AP_IOMUX->keyscan_in_en),row,row,1);
    subWriteReg((&AP_KSCAN->mk_in_en),row,row, 1);
}

/**************************************************************************************
    @fn          hal_kscan_config_col

    @brief       This function process for setting key scan col pin

    input parameters

    @param       KSCAN_COLS_e col

    output parameters

    @param       None.

    @return      None.
 **************************************************************************************/
static void hal_kscan_config_col(KSCAN_COLS_e col)
{
    gpio_pin_e col_pin = (gpio_pin_e)KSCAN_COL_GPIO[col];
    hal_gpio_fmux(col_pin, Bit_DISABLE);
    hal_gpio_pull_set(col_pin,GPIO_PULL_UP_S);
    subWriteReg(&(AP_IOMUX->keyscan_out_en),col,col,1);
    subWriteReg((&AP_KSCAN->ctrl0),(col+2),(col+2), 1);
}

static void kscan_sleep_handler(void)
{
    gpio_polarity_e pol;
    hal_kscan_clear_config();

    for(uint8_t i=0; i<NUM_KEY_COLS; i++)
    {
        gpio_pin_e col_pin = (gpio_pin_e)KSCAN_COL_GPIO[m_kscanCtx.cfg.key_cols[i]];
        subWriteReg(&(AP_IOMUX->keyscan_out_en),m_kscanCtx.cfg.key_cols[i],m_kscanCtx.cfg.key_cols[i],0);
        hal_gpioin_set_flag(col_pin);
        hal_gpio_pull_set(col_pin, GPIO_PULL_DOWN);
        hal_gpio_pin_init(col_pin, GPIO_INPUT);
    }

    for(uint8_t i=0; i<NUM_KEY_ROWS; i++)
    {
        gpio_pin_e row_pin = (gpio_pin_e)KSCAN_ROW_GPIO[m_kscanCtx.cfg.key_rows[i]];
        subWriteReg(&(AP_IOMUX->keyscan_in_en),m_kscanCtx.cfg.key_rows[i],m_kscanCtx.cfg.key_rows[i],0);
        hal_gpioin_set_flag(row_pin);
        hal_gpio_pull_set(row_pin, GPIO_PULL_UP);
        hal_gpio_pin_init(row_pin, GPIO_INPUT);
        pol = hal_gpio_read(row_pin) ? POL_FALLING:POL_RISING;
        hal_gpio_wakeup_set(row_pin, pol);
        m_kscanCtx.pin_state[i] = pol;
    }
}

static void kscan_wakeup_handler(void)
{
    for(uint8_t i=0; i<NUM_KEY_COLS; i++)
    {
        gpio_pin_e col_pin = (gpio_pin_e)KSCAN_COL_GPIO[m_kscanCtx.cfg.key_cols[i]];
        subWriteReg(&(AP_IOMUX->keyscan_out_en),m_kscanCtx.cfg.key_cols[i],m_kscanCtx.cfg.key_cols[i],0);
        hal_gpio_pull_set(col_pin, GPIO_PULL_DOWN);
        hal_gpio_pin_init(col_pin, GPIO_INPUT);
    }

    for(uint8_t i=0; i<NUM_KEY_ROWS; i++)
    {
        gpio_pin_e row_pin = (gpio_pin_e)KSCAN_ROW_GPIO[m_kscanCtx.cfg.key_rows[i]];
        subWriteReg(&(AP_IOMUX->keyscan_in_en),m_kscanCtx.cfg.key_rows[i],m_kscanCtx.cfg.key_rows[i],0);
        hal_gpio_pull_set(row_pin, GPIO_PULL_UP);//teddy add 20190122
        hal_gpio_pin_init(row_pin, GPIO_INPUT);
    }

    for(uint8_t i=0; i<NUM_KEY_ROWS; i++)
    {
        gpio_pin_e row_pin = (gpio_pin_e)KSCAN_ROW_GPIO[m_kscanCtx.cfg.key_rows[i]];
        hal_gpio_pin_init(row_pin, GPIO_INPUT);
        gpio_polarity_e pol = hal_gpio_read(row_pin) ? POL_RISING:POL_FALLING;

        if(pol == m_kscanCtx.pin_state[i])
        {
            break;
        }
        else if(i == (NUM_KEY_ROWS-1))
        {
            return;
        }
    }

    hal_pwrmgr_lock(MOD_KSCAN);

    for(uint8_t i=0; i<NUM_KEY_COLS; i++) //teddy add 20190122
    {
        gpio_pin_e col_pin = (gpio_pin_e)KSCAN_COL_GPIO[m_kscanCtx.cfg.key_cols[i]];
        hal_gpio_pull_set(col_pin, GPIO_PULL_UP);
        hal_gpio_pin_init(col_pin, GPIO_INPUT);
    }

    kscan_hw_config();
    reScan_flag=0;
    osal_start_timerEx(m_kscanCtx.kscan_task_id, m_kscanCtx.timeout_event, (m_kscanCtx.cfg.interval + TIMEOUT_DELTA));
}


/**************************************************************************************
    @fn          get_key_matrix

    @brief       This function process for reading key row and col

    input parameters

    @param       uint16_t* key_matrix

    output parameters

    @param       None.

    @return      None.
 **************************************************************************************/
static void get_key_matrix(uint16_t* key_matrix)
{
    for(uint8_t i=0; i < MULTI_KEY_NUM; i++)
    {
        uint16_t low  = (read_reg(&(AP_KSCAN->mkc[i])) & 0x0000FFFF);
        uint16_t high = (read_reg(&(AP_KSCAN->mkc[i])) & 0xFFFF0000) >> 16;
        key_matrix[i*2] = low;
        key_matrix[i*2+1] = high;
    }
}

/**************************************************************************************
    @fn          rmv_ghost_key

    @brief       This function process for removing ghost key

    input parameters

    @param       uint16_t* key_matrix

    output parameters

    @param       None.

    @return      None.
 **************************************************************************************/
static void rmv_ghost_key(uint16_t* key_matrix)
{
    uint16_t mix_final = 0;

    for (uint8_t i=0; i<MAX_KEY_COLS; ++i)
    {
        for (uint8_t j=i+1; j<MAX_KEY_ROWS; ++j)
        {
            uint16_t mix = key_matrix[i] & key_matrix[j];
            uint8_t bit_is_pow2 = (mix&(mix-1)) == 0;

            if (mix && !bit_is_pow2)
                mix_final |= mix;
        }

        key_matrix[i] &= ~mix_final;
    }
}

static kscan_Evt_t kscan_compare_key(uint16_t* key_pre, uint16_t* key_nxt)
{
    uint16_t multi_key_num = 0;

    for(uint8_t i=0; i<MAX_KEY_COLS; i++)
    {
        uint16_t chg_key = key_pre[i] ^ key_nxt[i];
        uint16_t key_sta = chg_key & key_nxt[i];

        if(chg_key != 0)
        {
            for(uint8_t j=0; j<MAX_KEY_ROWS; j++)
            {
                if((chg_key & BIT(j)) != 0)
                {
                    kscan_Key_t key_param;
                    key_param.row = j;
                    key_param.col = i;
                    key_param.type = (key_sta & BIT(j)) ? KEY_PRESSED:KEY_RELEASED;
                    m_keys[multi_key_num] = key_param;
                    multi_key_num++;
                }

                if(multi_key_num == MAX_KEY_NUM)
                    break;
            }
        }
    }

    kscan_Evt_t evt;
    evt.keys = m_keys;
    evt.num  = multi_key_num;
    return evt;
}
