/*************
 app_datetime.c
 SDK_LICENSE
***************/

#include "phy_console.h"
#include "uart.h"
#include "error.h"
#include "OSAL.h"
#include "string.h"
#include "pwrmgr.h"
#include "log.h"


typedef enum
{
    CONS_ST_IDLE,
    CONS_ST_RX,
    CONS_ST_CMD
} cons_state_t;

typedef struct
{
    cons_state_t  state;
    uint16_t      rx_cnt;
    uint8_t       rx_buf[CONS_CMD_RXBUF_MAX];
    const cons_cmd_t*   cmd_list;
    cons_callback_t callback;
} cons_ctx_t;

cons_ctx_t s_cons_ctx;


void console_parse_cmd(void)
{
    uint16_t i,j;
    uint16_t cmd_id = 0;
    char* param_table[CONS_PARAM_NUM_MAX];
    cons_ctx_t* pctx = &s_cons_ctx;
    const cons_cmd_t*   cmd_list = pctx->cmd_list;
    uint8_t* prx = s_cons_ctx.rx_buf;
    char* pcmd = NULL;
    char* pparam = NULL;
    uint8_t param_num = 0;

    //LOG("\n");
    //for(i = 0; i< pctx->rx_cnt; i++)
    //  LOG("%x ",prx[i]);
    //  LOG("\n");

    //parse "at"
    for(i = 1; i< pctx->rx_cnt; i++)
    {
        if(prx[i-1] == 'a' && prx[i] == 't')
        {
            i++;
            break;
        }
    }

    if(i == pctx->rx_cnt)
    {
        LOG("Parse filed, did not found \"at\"\n");
        return;
    }

    //parse cmd
    {
        pcmd = (char*)prx+i;

        for(; i< pctx->rx_cnt; i++)
        {
            if(prx[i] == ' ' || prx[i] == '\0' )
            {
                prx[i] = '\0';
                break;
            }

            if(prx[i] < 0x20 || prx[i] > 0x7d)
            {
                LOG("Parse failed, cmd has illegal character\n");
                return;
            }
        }

        if(osal_strlen(pcmd) == 0)
        {
            LOG("Parse failed, cmd is empty\n");
            return;
        }

        for(j = 0; j<CONS_CMD_NUM_MAX; j++)
        {
            if(cmd_list[j].cmd_id == 0 || cmd_list[j].cmd_name == NULL )
            {
                LOG("Parse failed, cmd is not match cmdlist\n");
                return;
            }

            if(strcmp(pcmd, cmd_list[j].cmd_name) == 0)
            {
                //match
                cmd_id = cmd_list[j].cmd_id;
                break;
            }
        }

        if(cmd_id == 0)
        {
            LOG("Parse failed, cmd is not match cmdlist_1\n");
            return;
        }
    }
    i++;
    //parse parameter
    pparam = (char*)prx+i;

    for(; i< pctx->rx_cnt; i++)
    {
        if(prx[i] == ' ')
        {
            prx[i] = '\0';

            if(osal_strlen(pparam) == 0)
            {
                break;
            }

            param_table[param_num] = pparam;
            param_num++;

            if(param_num >= CONS_PARAM_NUM_MAX)
                break;

            pparam = (char*)prx+i+1;
            continue;
        }

        if(prx[i] == '\0')
        {
            if(osal_strlen(pparam) == 0)
            {
                break;
            }

            param_table[param_num] = pparam;
            param_num++;
            break;
        }

        if(prx[i] < 0x20 || prx[i] > 0x7d)
        {
            LOG("Parse failed, parameter has illegal character\n");
            return;
        }
    }

    pctx->callback(cmd_id, param_num, param_table);
}


void console_sleep_handler(void)
{
    hal_gpio_fmux(P10, Bit_DISABLE);      //enable fullmux fuction; enable or disable
    hal_gpio_wakeup_set(P10, NEGEDGE);
}

void console_wakeup_handler(void)
{
    int i;
    hal_gpio_write(P14, 0);
    hal_gpio_write(P14, 1);
    hal_gpio_write(P14, 0);

    for(i = 0; i< 20; i++)
    {
        if(hal_gpio_read(P10)==0)
        {
            hal_pwrmgr_lock(MOD_CONSOLE);
            break;
        }
    }

    hal_gpio_write(P14, 0);
    hal_gpio_write(P14, 1);
    hal_gpio_write(P14, 0);
    s_cons_ctx.state = CONS_ST_IDLE;
}

void console_rx_handler(uart_Evt_t* pev)
{
    cons_ctx_t* pctx = &s_cons_ctx;
    uint8_t* prx = s_cons_ctx.rx_buf;

    switch(pev->type)
    {
    case UART_EVT_TYPE_RX_DATA:
    case UART_EVT_TYPE_RX_DATA_TO:
    {
        uint8_t i;
        uint8_t* prx_msg = pev->data;

        //for(i = 0; i< pev->len; i++)
        //  LOG("%x ",pev->data[i]);
        //LOG("\n");
        if(pctx->state == CONS_ST_IDLE)
        {
            hal_pwrmgr_lock(MOD_CONSOLE);
            pctx->state = CONS_ST_RX;
        }

        if(pctx->state == CONS_ST_RX)
        {
            for(i = 0; i< pev->len; i++)
            {
                if(prx_msg[i] == '\r' ||prx_msg[i] == '\n')
                {
                    prx[pctx->rx_cnt] = 0;
                    pctx->rx_cnt++;
                    pctx->state = CONS_ST_CMD;
                    console_parse_cmd();
                    pctx->state = CONS_ST_IDLE;
                    pctx->rx_cnt = 0;
                    hal_pwrmgr_unlock(MOD_CONSOLE);
                    break;
                }

                prx[pctx->rx_cnt] = prx_msg[i];
                pctx->rx_cnt++;
            }
        }
        else
        {
            hal_pwrmgr_unlock(MOD_CONSOLE);
        }

        break;
    }

    default:
        break;
    }
}

int console_init(const cons_cmd_t* cmdlist, cons_callback_t callback)
{
    uart_Cfg_t cfg =
    {
        .tx_pin = P9,
        .rx_pin = P10,
        .rts_pin = GPIO_DUMMY,
        .cts_pin = GPIO_DUMMY,
        .baudrate = 115200,
        .use_fifo = TRUE,
        .hw_fwctrl = FALSE,
        .use_tx_buf = FALSE,
        .parity     = FALSE,
        .evt_handler = console_rx_handler,
    };

    if(callback == NULL)
        return PPlus_ERR_INVALID_PARAM;

    hal_pwrmgr_register(MOD_CONSOLE, console_sleep_handler, console_wakeup_handler);
    hal_uart_init(cfg,UART0);//uart init
    s_cons_ctx.cmd_list = cmdlist;
    s_cons_ctx.state = CONS_ST_IDLE;
    s_cons_ctx.rx_cnt = 0;
    s_cons_ctx.callback = callback;
    return PPlus_SUCCESS;
}

