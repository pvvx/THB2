/**
    \file appl_model_state_handler.h
*/

/*
    Copyright (C) 2017. Mindtree Ltd.
    All rights reserved.
*/

#ifndef _H_APPL_MODEL_STATE_HANDLER_
#define _H_APPL_MODEL_STATE_HANDLER_


/* --------------------------------------------- Header File Inclusion */
#include "appl_main.h"
#include "MS_model_states.h"


/* --------------------------------------------- Global Definitions */


/* --------------------------------------------- Data Types/ Structures */


/* --------------------------------------------- Function */
void appl_model_states_initialization(void);

void appl_model_state_get(UINT16 state_t, UINT16 state_inst, void* param, UINT8 direction);

void appl_model_state_set(UINT16 state_t, UINT16 state_inst, void* param, UINT8 direction);

#endif /*_H_APPL_MODEL_STATE_HANDLER_ */
