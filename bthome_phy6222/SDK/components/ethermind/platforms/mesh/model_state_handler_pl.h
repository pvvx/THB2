
/**
    \file model_state_handler_pl.h


*/

/*
    Copyright (C) 2013. Mindtree Limited.
    All rights reserved.
*/

#ifndef _H_MODEL_STATE_HANDLER_
#define _H_MODEL_STATE_HANDLER_

/* --------------------------------------------- Header File Inclusion */
#include "MS_common.h"

/* --------------------------------------------- Global Definitions */

/* --------------------------------------------- Structures/Data Types */

/* --------------------------------------------- Macros */

/* --------------------------------------------- Internal Functions */

/* --------------------------------------------- API Declarations */
void mesh_model_platform_init_pl(void);
void mesh_model_device_bootup_ind_pl(void);
void mesh_model_device_provisioned_ind_pl(void);
void generic_onoff_set_pl (UINT8 state);
void vendor_mode_mainlight_onoff_set_pl (UINT8 state);
void vendor_mode_backlight_onoff_set_pl (UINT8 state);
void light_lightness_set_pl (uint16_t ligtnessValue);
void light_ctl_set_pl (uint16_t ctlValue,uint16_t dltUV);
void light_hsl_set_pl (uint16_t H,uint16_t S,uint16_t L);

#endif /* _H_MODEL_STATE_HANDLER_ */

