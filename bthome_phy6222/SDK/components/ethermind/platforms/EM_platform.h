
/**
    \file EM_platform.h


*/

/*
    Copyright (C) 2013. Mindtree Limited.
    All rights reserved.
*/

#ifndef _H_EM_PLATFORM_
#define _H_EM_PLATFORM_

/* --------------------------------------------- Header File Inclusion */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "osal.h"

/* --------------------------------------------- Global Definitions */
#define EM_HAVE_STATIC_DECL
#define EM_HAVE_CONST_DECL

/* --------------------------------------------- Structures/Data Types */

/* --------------------------------------------- Macros */
#define printf(...)     {printf (__VA_ARGS__); printf("\r\n"); fflush(stdout);}
#define scanf(...)

/* --------------------------------------------- Internal Functions */

/* --------------------------------------------- API Declarations */
void EM_enter_sleep_pl(void);
void EM_exit_sleep_pl(void);

int _write (int fd, char* ptr, int len);
int _read (int fd, char* ptr, int len);
int _close (int fd);
int _fstat (int fd);
int _isatty (int fd);
int _lseek (int fd);

void HardFault_Handler(void);
void debugHardfault(uint32_t* sp);

#endif /* _H_EM_PLATFORM_ */

