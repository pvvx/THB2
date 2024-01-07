/**
    \file appl_model_state_handler.c
*/

/*
    Copyright (C) 2017. Mindtree Ltd.
    All rights reserved.
*/

/* --------------------------------------------- Header File Inclusion */
#include "appl_model_state_handler.h"
#include "MS_common.h"

/* --------------------------------------------- Global Definitions */

/* --------------------------------------------- Data Types/ Structures */
#define MS_MAX_NUM_STATES    2
#define MS_MAX_LC_PROPERTIES 10

typedef struct _APPL_STATE_INFO_VAULT
{
    /* OnOff state */
    UINT8    onoff;

    /* LC Mode */
    UINT8    lc_mode;

    MS_STATE_LIGHT_LC_PROPERTY_STRUCT lc_property;

} APPL_STATE_INFO_VAULT;


static MS_STATE_LIGHT_LC_MODE_STRUCT appl_light_lc_mode[MS_MAX_NUM_STATES];
static MS_STATE_LIGHT_LIGHTNESS_STRUCT appl_light_lightness[MS_MAX_NUM_STATES];
static MS_STATE_LIGHT_LC_OM_STRUCT appl_light_lc_om[MS_MAX_NUM_STATES];
static MS_STATE_LIGHT_LC_LIGHT_ONOFF_STRUCT appl_light_lc_onoff[MS_MAX_NUM_STATES];
static MS_STATE_LIGHT_LC_PROPERTY_STRUCT appl_light_lc_property[MS_MAX_NUM_STATES][MS_MAX_LC_PROPERTIES];

static MS_STATE_GENERIC_ONOFF_STRUCT appl_generic_onoff[MS_MAX_NUM_STATES];

/* For simplicity keeping same number of state info, as the number of max scences */
static APPL_STATE_INFO_VAULT   appl_state_info_vault[16];

static UINT8       appl_current_property_id_index;
void appl_light_lightness_set_linear(UINT16 state_inst, UINT16 linear);
API_RESULT appl_model_light_lightness_state_set(UINT16 state_t, UINT16 state_inst, void* param, UINT8 direction);

/* --------------------------------------------- Function */
void appl_model_states_initialization(void)
{
    UINT32 index;
    EM_mem_set(appl_light_lc_mode, 0, sizeof(appl_light_lc_mode));
    EM_mem_set(appl_light_lc_om, 0, sizeof(appl_light_lc_om));
    EM_mem_set(appl_light_lc_onoff, 0, sizeof(appl_light_lc_onoff));
    EM_mem_set(appl_light_lc_property, 0, sizeof(appl_light_lc_property));

    for (index = 0; index < MS_MAX_LC_PROPERTIES; index++)
    {
        /* TODO: For PTS */
        appl_light_lc_property[0][index].property_id = index + 0x36 /* 1 */;
        /* Allocate and keep some property value */
        appl_light_lc_property[0][index].property_value_len = 3 /* 1 */;
        appl_light_lc_property[0][index].property_value = EM_alloc_mem(3 /* 1 */);
        /* TODO: Not checking for memeory allocation failure */
        EM_mem_set(appl_light_lc_property[0][index].property_value, (index + 3 /* 1 */), (3 /* 1 */));
    }

    EM_mem_set(appl_generic_onoff, 0, sizeof(appl_generic_onoff));
    EM_mem_set(appl_state_info_vault, 0, sizeof(appl_state_info_vault));
    EM_mem_set (appl_light_lightness, 0, sizeof(appl_light_lightness));
    appl_current_property_id_index = 0;
}


static void appl_generic_onoff_transition_start_cb(void* blob)
{
    /**
        Because binary states cannot support transitions, when changing to 0x01 (On),
        the Generic OnOff state shall change immediately when the transition starts,
        and when changing to 0x00, the state shall change when the transition finishes.
    */
    if (0 == appl_generic_onoff[0].onoff)
    {
        appl_generic_onoff[0].onoff = appl_generic_onoff[0].target_onoff;
    }
}

static void appl_generic_onoff_transition_complete_cb(void* blob)
{
    /* State Transition Complete */
    appl_generic_onoff[0].transition_time = 0;
    appl_generic_onoff[0].onoff = appl_generic_onoff[0].target_onoff;
    appl_generic_onoff[0].target_onoff = 0;
}

static void appl_light_lc_onoff_transition_start_cb(void* blob)
{
    /**
        Because binary states cannot support transitions, when changing to 0x01 (On),
        the Generic OnOff state shall change immediately when the transition starts,
        and when changing to 0x00, the state shall change when the transition finishes.
    */
    if (0 == appl_light_lc_onoff[0].present_light_onoff)
    {
        appl_light_lc_onoff[0].present_light_onoff = appl_light_lc_onoff[0].target_light_onoff;
    }
}

static void appl_light_lc_onoff_transition_complete_cb(void* blob)
{
    /* State Transition Complete */
    appl_light_lc_onoff[0].transition_time = 0;
    appl_light_lc_onoff[0].present_light_onoff = appl_light_lc_onoff[0].target_light_onoff;
    appl_light_lc_onoff[0].target_light_onoff = 0;
}

void* appl_scene_save_current_state(/* IN */ UINT32 scene_index)
{
    /* Free previous context - if any */
    /* Empty right now - dummy returning the same scene index itself */
    /* Store current state */
    appl_state_info_vault[scene_index].onoff = appl_generic_onoff[0].onoff;
    appl_state_info_vault[scene_index].lc_mode = appl_light_lc_mode[0].present_mode;
    appl_state_info_vault[scene_index].lc_property.property_id = appl_light_lc_property[0][appl_current_property_id_index].property_id;
    appl_state_info_vault[scene_index].lc_property.property_value_len = appl_light_lc_property[0][appl_current_property_id_index].property_value_len;

    if (0 != appl_state_info_vault[scene_index].lc_property.property_value_len)
    {
        appl_state_info_vault[scene_index].lc_property.property_value = EM_alloc_mem(appl_state_info_vault[scene_index].lc_property.property_value_len);
        /* TODO: Not check malloc failure */
        EM_mem_copy(appl_state_info_vault[scene_index].lc_property.property_value, appl_light_lc_property[0][appl_current_property_id_index].property_value, appl_state_info_vault[scene_index].lc_property.property_value_len);
    }

    return (void*)scene_index;
}

void* appl_scene_delete_saved_state(/* IN */ UINT32 scene_index, /* IN */ void* context)
{
    /* Free context - if any */
    /* Empty right now - dummy returning NULL */
    /* Invalid value */
    appl_state_info_vault[scene_index].onoff = 0x02;
    appl_state_info_vault[scene_index].lc_mode = 0x00;
    appl_state_info_vault[scene_index].lc_property.property_id = 0x0000;

    if (0 != appl_state_info_vault[scene_index].lc_property.property_value_len)
    {
        EM_free_mem(appl_state_info_vault[scene_index].lc_property.property_value);
    }

    appl_state_info_vault[scene_index].lc_property.property_value = NULL;
    appl_state_info_vault[scene_index].lc_property.property_value_len = 0x0000;
    return (void*)NULL;
}

void* appl_scene_recall_saved_state(/* IN */ UINT32 scene_index, /* IN */ void* context)
{
    /* Recall context - if any */
    /* Empty right now - dummy returning NULL */
    appl_generic_onoff[0].onoff = appl_state_info_vault[scene_index].onoff;
    appl_light_lc_mode[0].present_mode = appl_state_info_vault[scene_index].lc_mode;
    appl_light_lc_property[0][appl_current_property_id_index].property_id = appl_state_info_vault[scene_index].lc_property.property_id;
    appl_light_lc_property[0][appl_current_property_id_index].property_value_len = appl_state_info_vault[scene_index].lc_property.property_value_len;

    if (0 != appl_state_info_vault[scene_index].lc_property.property_value_len)
    {
        /* appl_state_info_vault[scene_index].lc_property.property_value = EM_alloc_mem(appl_state_info_vault[scene_index].lc_property.property_value_len); */
        /* TODO: Not check malloc failure */
        /* EM_mem_copy(appl_state_info_vault[scene_index].lc_property.property_value, appl_light_lc_property[0][appl_current_property_id_index].property_value, appl_state_info_vault[scene_index].lc_property.property_value_len); */
        appl_light_lc_property[0][appl_current_property_id_index].property_value = appl_state_info_vault[scene_index].lc_property.property_value;
    }

    return (void*)NULL;
}

API_RESULT appl_model_light_lightness_state_get(UINT16 state_t, UINT16 state_inst, void* param, UINT8 direction)
{
    MS_STATE_LIGHT_LIGHTNESS_STRUCT* param_p;
    param_p = (MS_STATE_LIGHT_LIGHTNESS_STRUCT*)param;

    switch (state_t)
    {
    case MS_STATE_LIGHT_LIGHTNESS_DEFAULT_T:
    {
        /* Ignoring Instance and direction right now */
        param_p->light_lightness_default = appl_light_lightness[0].light_lightness_default;
    }
    break;

    case MS_STATE_LIGHT_LIGHTNESS_RANGE_T:
    {
        /* Ignoring Instance and direction right now */
        param_p->light_lightness_range = appl_light_lightness[0].light_lightness_range;
        param_p->range_status = 0x00;
    }
    break;

    case MS_STATE_LIGHT_LIGHTNESS_LINEAR_T:
    {
        /* Ignoring Instance and direction right now */
        param_p->light_lightness_linear = appl_light_lightness[0].light_lightness_linear;
    }
    break;

    case MS_STATE_LIGHT_LIGHTNESS_LAST_T:
    {
        /* Ignoring Instance and direction right now */
        param_p->light_lightness_last = appl_light_lightness[0].light_lightness_last;
    }
    break;

    case MS_STATE_LIGHT_LIGHTNESS_ACTUAL_T:
    {
        /* Ignoring Instance and direction right now */
        param_p->light_lightness_actual = appl_light_lightness[0].light_lightness_actual;
    }
    break;

    default:
        break;
    }

    return API_SUCCESS;
}

API_RESULT appl_model_state_get(UINT16 state_t, UINT16 state_inst, void* param, UINT8 direction)
{
    API_RESULT retval;
    retval = API_SUCCESS;

    switch(state_t)
    {
    case MS_STATE_LIGHT_LC_MODE_T:
    {
        MS_STATE_LIGHT_LC_MODE_STRUCT* param_p;
        param_p = (MS_STATE_LIGHT_LC_MODE_STRUCT*)param;
        /* Ignoring Instance and direction right now */
        param_p->present_mode = appl_light_lc_mode[0].present_mode;
    }
    break;

    case MS_STATE_LIGHT_LC_OM_T:
    {
        MS_STATE_LIGHT_LC_OM_STRUCT* param_p;
        param_p = (MS_STATE_LIGHT_LC_OM_STRUCT*)param;
        /* Ignoring Instance and direction right now */
        param_p->present_mode = appl_light_lc_om[0].present_mode;
    }
    break;

    case MS_STATE_LIGHT_LC_LIGHT_ONOFF_T:
    {
        MS_STATE_LIGHT_LC_LIGHT_ONOFF_STRUCT* param_p;
        param_p = (MS_STATE_LIGHT_LC_LIGHT_ONOFF_STRUCT*)param;
        /* Ignoring Instance and direction right now */
        *param_p = appl_light_lc_onoff[0];
    }
    break;

    case MS_STATE_LIGHT_LC_PROPERTY_T:
    {
        MS_STATE_LIGHT_LC_PROPERTY_STRUCT*   param_p;
        UINT32 index;
        param_p = (MS_STATE_LIGHT_LC_PROPERTY_STRUCT*)param;
        /* Ignoring Instance and direction right now */
        /* Search for matching property ID */
        param_p->property_value_len = 0x00;

        for (index = 0; index < MS_MAX_LC_PROPERTIES; index++)
        {
            if (param_p->property_id == appl_light_lc_property[0][index].property_id)
            {
                param_p->property_value_len = appl_light_lc_property[0][index].property_value_len;
                param_p->property_value = appl_light_lc_property[0][index].property_value;
                break;
            }
        }

        /* Else Return Error */
    }
    break;

    case MS_STATE_GENERIC_ONOFF_T:
    {
        MS_STATE_GENERIC_ONOFF_STRUCT* param_p;
        param_p = (MS_STATE_GENERIC_ONOFF_STRUCT*)param;
        /* Ignoring Instance and direction right now */
        *param_p = appl_generic_onoff[0];
    }
    break;

    case MS_STATE_LIGHT_LIGHTNESS_DEFAULT_T: /* Fall Through */
    case MS_STATE_LIGHT_LIGHTNESS_RANGE_T: /* Fall Through */
    case MS_STATE_LIGHT_LIGHTNESS_LINEAR_T: /* Fall Through */
    case MS_STATE_LIGHT_LIGHTNESS_LAST_T: /* Fall Through */
    case MS_STATE_LIGHT_LIGHTNESS_ACTUAL_T:
        retval = appl_model_light_lightness_state_get(state_t, state_inst, param, direction);
        break;

    default:
        break;
    }

    return retval;
}

/* Generic OnOff Model Set Handlers */

void appl_light_lightness_set_linear(UINT16 state_inst, UINT16 linear)
{
    /* UINT16 actual; */
    /* UINT32 mul_val; */
    /* long double d; */
    /* appl_light_lightness[state_inst].light_lightness_linear.lightness_linear = linear; */
    /* mul_val = linear * 65535; */
    /* actual = (UINT16)sqrt(mul_val); */
    /* Light Lightness actual = sqrt(Linear * 65535) */
    /* appl_light_lightness_set_actual(state_inst, actual); */
    appl_light_lightness[state_inst].light_lightness_linear.lightness_linear = linear;
}


API_RESULT appl_model_light_lightness_state_set(UINT16 state_t, UINT16 state_inst, void* param, UINT8 direction)
{
    API_RESULT retval;
    MS_STATE_LIGHT_LIGHTNESS_STRUCT* param_p;
    param_p = (MS_STATE_LIGHT_LIGHTNESS_STRUCT*)param;
    retval = API_SUCCESS;

    switch(state_t)
    {
    case MS_STATE_LIGHT_LIGHTNESS_DEFAULT_T:
    {
        /* Ignoring Instance and direction right now */
        appl_light_lightness[0].light_lightness_default = param_p->light_lightness_default;
    }
    break;

    case MS_STATE_LIGHT_LIGHTNESS_RANGE_T:
    {
        /* Check range min and max */
        if (param_p->light_lightness_range.lightness_range_min > param_p->light_lightness_range.lightness_range_max)
        {
            /* TODO: add macro define */
            /**
                Table 7.2:
                0x00 - Success
                0x01 - Cannot Set Range Min
                0x02 - Cannot Set Range Max
            */
            param_p->range_status = 0x01;
            #if 0
            appl_light_lightness[0].light_lightness_range = param_p->light_lightness_range;
            param_p->light_lightness_range.lightness_range_min = 0x0000;
            param_p->light_lightness_range.lightness_range_max = 0x0000;
            #endif
        }
        else
        {
            /* Ignoring Instance and direction right now */
            appl_light_lightness[0].light_lightness_range = param_p->light_lightness_range;
            param_p->range_status = 0x00;
        }
    }
    break;

    case MS_STATE_LIGHT_LIGHTNESS_LINEAR_T:
    {
        if (0 != param_p->light_lightness_linear.transition_time)
        {
            #if 0
            MS_ACCESS_STATE_TRANSITION_TYPE   transition;
            UINT16                            transition_time_handle;
            appl_light_lightness[0].light_lightness_linear.lightness_target = param_p->light_lightness_linear.lightness_linear;
            appl_light_lightness[0].light_lightness_linear.transition_time = param_p->light_lightness_linear.transition_time;
            transition.delay = param_p->light_lightness_linear.delay;
            transition.transition_time = param_p->light_lightness_linear.transition_time;
            transition.blob = NULL;
            transition.transition_start_cb = appl_light_lightness_linear_transition_start_cb;
            transition.transition_complete_cb = appl_light_lightness_linear_transition_complete_cb;
            MS_common_start_transition_timer
            (
                &transition,
                &transition_time_handle
            );
            /* Return value to indicate not sending status right now */
            retval = API_FAILURE;
            #endif
        }
        else
        {
            /* Instantaneous Change */
            /* appl_generic_onoff[0].onoff = param_p->onoff; */
            appl_light_lightness_set_linear(0, param_p->light_lightness_linear.lightness_linear);
        }

        *param_p = appl_light_lightness[0];
        CONSOLE_OUT("[state] current: 0x%02X\n", param_p->light_lightness_linear.lightness_linear);
        CONSOLE_OUT("[state] target: 0x%02X\n", param_p->light_lightness_linear.lightness_target);
        CONSOLE_OUT("[state] remaining_time: 0x%02X\n", param_p->light_lightness_linear.transition_time);
        /* Ignoring Instance and direction right now */
    }
    break;

    case MS_STATE_LIGHT_LIGHTNESS_LAST_T:
    {
        /* Ignoring Instance and direction right now */
        appl_light_lightness[0].light_lightness_last = param_p->light_lightness_last;
    }
    break;

    case MS_STATE_LIGHT_LIGHTNESS_ACTUAL_T:
    {
        if (0 != param_p->light_lightness_actual.transition_time)
        {
            #if 0
            MS_ACCESS_STATE_TRANSITION_TYPE   transition;
            UINT16                            transition_time_handle;
            appl_light_lightness[0].light_lightness_actual.lightness_target = param_p->light_lightness_actual.lightness_actual;
            appl_light_lightness[0].light_lightness_actual.transition_time = param_p->light_lightness_actual.transition_time;
            transition.delay = param_p->light_lightness_actual.delay;
            transition.transition_time = param_p->light_lightness_actual.transition_time;
            transition.blob = NULL;
            transition.transition_start_cb = appl_light_lightness_transition_start_cb;
            transition.transition_complete_cb = appl_light_lightness_transition_complete_cb;
            MS_common_start_transition_timer
            (
                &transition,
                &transition_time_handle
            );
            /* Return value to indicate not sending status right now */
            retval = API_FAILURE;
            #endif
        }
        else
        {
            /* Instantaneous Change */
            /* appl_generic_onoff[0].onoff = param_p->onoff; */
            /* appl_light_lightness_set_actual(0, param_p->light_lightness_actual.lightness_actual); */
        }

        *param_p = appl_light_lightness[0];
        CONSOLE_OUT("[state] current: 0x%02X\n", param_p->light_lightness_actual.lightness_actual);
        CONSOLE_OUT("[state] target: 0x%02X\n", param_p->light_lightness_actual.lightness_target);
        CONSOLE_OUT("[state] remaining_time: 0x%02X\n", param_p->light_lightness_actual.transition_time);
        /* Ignoring Instance and direction right now */
        /* param_p->light_lightness_actual.lightness_actual = appl_light_lightness[0].light_lightness_actual.lightness_actual; */
    }
    break;

    default:
        break;
    }

    return retval;
}

API_RESULT appl_model_state_set(UINT16 state_t, UINT16 state_inst, void* param, UINT8 direction)
{
    UINT32 index;
    API_RESULT retval;
    retval = API_SUCCESS;

    switch(state_t)
    {
    case MS_STATE_LIGHT_LC_MODE_T:
    {
        MS_STATE_LIGHT_LC_MODE_STRUCT* param_p;
        param_p = (MS_STATE_LIGHT_LC_MODE_STRUCT*)param;
        /* Ignoring Instance and direction right now */
        appl_light_lc_mode[0].present_mode = param_p->target_mode;
        param_p->present_mode = appl_light_lc_mode[0].present_mode;
    }
    break;

    case MS_STATE_LIGHT_LC_OM_T:
    {
        MS_STATE_LIGHT_LC_OM_STRUCT* param_p;
        param_p = (MS_STATE_LIGHT_LC_OM_STRUCT*)param;
        /* Ignoring Instance and direction right now */
        appl_light_lc_om[0].present_mode = param_p->target_mode;
        param_p->present_mode = appl_light_lc_om[0].present_mode;
    }
    break;

    case MS_STATE_LIGHT_LC_LIGHT_ONOFF_T:
    {
        MS_STATE_LIGHT_LC_LIGHT_ONOFF_STRUCT* param_p;
        param_p = (MS_STATE_LIGHT_LC_LIGHT_ONOFF_STRUCT*)param;
        /* TODO: Hack for test case MMDL/SR/LLC/BV-04-C */
        #if 0

        if (0 == param_p->transition_time)
        {
            param_p->transition_time = 0x54;
            param_p->delay = 0x05;
        }

        #endif /* 0 */

        /* Check if state transition is specified */
        if (0 != param_p->transition_time)
        {
            MS_ACCESS_STATE_TRANSITION_TYPE   transition;
            UINT16                            transition_time_handle;
            appl_light_lc_onoff[0].target_light_onoff = param_p->target_light_onoff;
            appl_light_lc_onoff[0].transition_time = param_p->transition_time;
            transition.delay = param_p->delay;
            transition.transition_time = param_p->transition_time;
            transition.blob = NULL;
            transition.transition_start_cb = appl_light_lc_onoff_transition_start_cb;
            transition.transition_complete_cb = appl_light_lc_onoff_transition_complete_cb;
            MS_common_start_transition_timer
            (
                &transition,
                &transition_time_handle
            );
            /* Return value to indicate not sending status right now */
            retval = API_FAILURE;
        }
        else
        {
            /* Instantaneous Change */
            /* Ignoring Instance and direction right now */
            appl_light_lc_onoff[0].present_light_onoff = param_p->target_light_onoff;
            appl_generic_onoff[0].onoff = param_p->target_light_onoff;
            appl_light_lightness_set_linear(0, param_p->target_light_onoff);
        }

        *param_p = appl_light_lc_onoff[0];
        CONSOLE_OUT("[state] current: 0x%02X\n", appl_light_lc_onoff[0].present_light_onoff);
        CONSOLE_OUT("[state] target: 0x%02X\n", appl_light_lc_onoff[0].target_light_onoff);
        CONSOLE_OUT("[state] remaining_time: 0x%02X\n", appl_light_lc_onoff[0].transition_time);
        /* TODO: Check for timer and start */
    }
    break;

    case MS_STATE_LIGHT_LC_PROPERTY_T:
    {
        MS_STATE_LIGHT_LC_PROPERTY_STRUCT* param_p;
        param_p = (MS_STATE_LIGHT_LC_PROPERTY_STRUCT*)param;

        for (index = 0; index < MS_MAX_LC_PROPERTIES; index++)
        {
            if (param_p->property_id == appl_light_lc_property[0][index].property_id)
            {
                if (appl_light_lc_property[0][index].property_value_len == param_p->property_value_len)
                {
                    EM_mem_copy(appl_light_lc_property[0][index].property_value, param_p->property_value, param_p->property_value_len);
                    appl_current_property_id_index = index;
                }
                else
                {
                    param_p->property_value_len = 0x00;
                }

                break;
            }
        }

        /* TODO: */
    }
    break;

    case MS_STATE_GENERIC_ONOFF_T:
    {
        MS_STATE_GENERIC_ONOFF_STRUCT* param_p;
        param_p = (MS_STATE_GENERIC_ONOFF_STRUCT*)param;

        /* Check if state transition is specified */
        if (0 != param_p->transition_time)
        {
            MS_ACCESS_STATE_TRANSITION_TYPE   transition;
            UINT16                            transition_time_handle;
            appl_generic_onoff[0].target_onoff = param_p->onoff;
            appl_generic_onoff[0].transition_time = param_p->transition_time;
            transition.delay = param_p->delay;
            transition.transition_time = param_p->transition_time;
            transition.blob = NULL;
            transition.transition_start_cb = appl_generic_onoff_transition_start_cb;
            transition.transition_complete_cb = appl_generic_onoff_transition_complete_cb;
            MS_common_start_transition_timer
            (
                &transition,
                &transition_time_handle
            );
            /* Return value to indicate not sending status right now */
            retval = API_FAILURE;
        }
        else
        {
            /* Instantaneous Change */
            appl_generic_onoff[0].onoff = param_p->onoff;
            appl_light_lc_onoff[0].present_light_onoff = param_p->onoff;
        }

        *param_p = appl_generic_onoff[0];
        CONSOLE_OUT("[state] current: 0x%02X\n", appl_generic_onoff[0].onoff);
        CONSOLE_OUT("[state] target: 0x%02X\n", appl_generic_onoff[0].target_onoff);
        CONSOLE_OUT("[state] remaining_time: 0x%02X\n", appl_generic_onoff[0].transition_time);
        /* Ignoring Instance and direction right now */
    }
    break;

    case MS_STATE_LIGHT_LIGHTNESS_DEFAULT_T: /* Fall Through */
    case MS_STATE_LIGHT_LIGHTNESS_RANGE_T: /* Fall Through */
    case MS_STATE_LIGHT_LIGHTNESS_LINEAR_T: /* Fall Through */
    case MS_STATE_LIGHT_LIGHTNESS_LAST_T: /* Fall Through */
    case MS_STATE_LIGHT_LIGHTNESS_ACTUAL_T:
        retval = appl_model_light_lightness_state_set(state_t, state_inst, param, direction);
        break;

    default:
        break;
    }

    return retval;
}


