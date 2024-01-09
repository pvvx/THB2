/*************
 phy_console.h
 SDK_LICENSE
***************/

#ifndef _PHY_CONSOLE_H
#define _PHY_CONSOLE_H

#include "bus_dev.h"

#define CONS_CMD_NUM_MAX  32
#define CONS_CMD_RXBUF_MAX  1024
#define CONS_PARAM_NUM_MAX  8

#define MOD_CONSOLE MOD_USR2

typedef void (*cons_callback_t)(uint16_t cmd_id, uint8_t argc, char** argv);


typedef struct
{
    uint16_t cmd_id;
    char*    cmd_name;
} cons_cmd_t;
int console_init(const cons_cmd_t* cmdlist, cons_callback_t callback);

#endif

