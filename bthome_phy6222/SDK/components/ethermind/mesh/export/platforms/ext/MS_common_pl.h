
/**
    \file MS_common_pl.h

    This file contains the Function Declaration, and Constant Definitions
    for the EtherMind Mesh Stack in Windows User Mode.
*/

/*
    Copyright (C) 2016. Mindtree Ltd.
    All rights reserved.
*/

#ifndef _H_MS_COMMON_PL_
#define _H_MS_COMMON_PL_

/* ------------------------------------------- Header File Inclusion */

/* ------------------------------------------- Common PL Debug */
#define PL_ERR(...)                             MS_debug_error(ms_debug_fd, __VA_ARGS__)

#ifdef PL_DEBUG

    #define PL_TRC(...)                             MS_debug_trace(ms_debug_fd, __VA_ARGS__)
    #define PL_INF(...)                             MS_debug_info(ms_debug_fd, __VA_ARGS__)

#else  /* PL_DEBUG */

    #define PL_TRC                                  MS_debug_null
    #define PL_INF                                  MS_debug_null

#endif /* PL_DEBUG */


/* ------------------------------------------- Global Definitions/Macros */
/* EtherMind Configuration File */
#define MS_CONFIG_FILE                          "ethermind.conf"


/* ------------------------------------------- Data Structures */


/* ------------------------------------------- Function Declarations */
/* EtherMind-Init: Platform Handler */
void ms_init_pl(void);

__ATTR_SECTION_XIP__ UINT8  MS_common_reset(void);


/* Mesh Shutdown: Platform Handler */
void ms_shutdown_pl(void);

#endif /* _H_MS_COMMON_PL_ */

