/**
    \file EM_platform.c


*/

/*
    Copyright (C) 2013. Mindtree Limited.
    All rights reserved.
*/

/* --------------------------------------------- Header File Inclusion */
#include "EM_platform.h"
#include "uart.h"

#pragma import(__use_no_semihosting_swi)

/* --------------------------------------------- Global Definitions */

/* --------------------------------------------- Static Global Variables */
struct __FILE
{
    int handle; /* whatever required */
};
FILE __stdout;
FILE __stdin;

/* --------------------------------------------- External Global Variables */

/* --------------------------------------------- Exported Global Variables */

/* --------------------------------------------- Function */
void EM_enter_sleep_pl(void)
{
}

void EM_exit_sleep_pl(void)
{
}

int fputc(int c, FILE* f)
{
    return hal_uart_send_buff(UART0,(uint8_t*)&c, 1);
}


int fgetc(FILE* f)
{
    return 0;
}


int ferror(FILE* f)
{
    /* Your implementation of ferror */
    return EOF;
}


void _ttywrch(int c)
{
    hal_uart_send_buff(UART0,(uint8_t*)&c, 1);
}


void _sys_exit(int return_code)
{
label:
    goto label;  /* endless loop */
}
