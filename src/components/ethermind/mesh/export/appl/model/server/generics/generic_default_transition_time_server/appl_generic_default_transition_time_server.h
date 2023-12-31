/**
    \file appl_generic_default_transition_time_server.h
*/

/*
    Copyright (C) 2017. Mindtree Ltd.
    All rights reserved.
*/

#ifndef _H_APPL_GENERIC_DEFAULT_TRANSITION_TIME_SERVER_
#define _H_APPL_GENERIC_DEFAULT_TRANSITION_TIME_SERVER_


/* --------------------------------------------- Header File Inclusion */
#include "MS_generic_default_transition_time_api.h"
#include "appl_main.h"


/* --------------------------------------------- Global Definitions */


/* --------------------------------------------- Data Types/ Structures */


/* --------------------------------------------- Function */
/* generic_default_transition_time server application entry point */
void main_generic_default_transition_time_server_operations(/* IN */ UINT8 have_menu);

/* Get Model Handle */
void appl_generic_default_transition_time_server_get_model_handle(void);

/* Set Publish Address */
void appl_generic_default_transition_time_server_set_publish_address(void);

#endif /*_H_APPL_GENERIC_DEFAULT_TRANSITION_TIME_SERVER_ */

