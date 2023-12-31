
/**
    \file cli_model_client.c

    This File contains the "model client" handlers for the CLI application,
    to exercise various functionalities of the Mindtree Mesh stack.
*/

/*
    Copyright (C) 2017. Mindtree Ltd.
    All rights reserved.
*/

/* ------------------------------- Header File Inclusion */
#include "cli_main.h"

/* ------------------------------- Global Variables */
/* Level - Model - Client */
DECL_CONST CLI_COMMAND cli_modelc_cmd_list[] =
{
    /* Help */
    { "help", "Help", cli_help },

    /* Back */
    { "back", "One Level Up", cli_back },

    /* Root */
    { "root", "Back to Root", cli_root },

    #ifdef CLI_CONFIG_CLIENT_MODEL
    { "config", "Config Client Operations", cli_modelc_config },
    #endif /* CLI_CONFIG_CLIENT_MODEL */

    #ifdef CLI_HEALTH_CLIENT_MODEL
    { "health", "Health Client Operations", cli_modelc_health },
    #endif /* CLI_HEALTH_CLIENT_MODEL */

    #ifdef CLI_GENERICS_CLIENT_MODEL
    #ifdef CLI_GENERICS_ONOFF_CLIENT_MODEL
    { "onoff", "Generic Onoff Client Operations", cli_modelc_generic_onoff },
    #endif /* CLI_GENERICS_ONOFF_CLIENT_MODEL */
    #ifdef CLI_GENERICS_LEVEL_CLIENT_MODEL
    { "level", "Generic Level Client Operations", cli_modelc_generic_level },
    #endif /* CLI_GENERICS_LEVEL_CLIENT_MODEL */
    #ifdef CLI_GENERICS_TRANSITIONTIME_CLIENT_MODEL
    { "transitiontime", "Generic Default Transition Time Client Operations", cli_modelc_generic_default_transition_time },
    #endif /* CLI_GENERICS_TRANSITIONTIME_CLIENT_MODEL */
    #ifdef CLI_GENERICS_PWRONOFF_CLIENT_MODEL
    { "poweronoff", "Generic Power OnOff Client Operations", cli_modelc_generic_power_onoff },
    #endif /* CLI_GENERICS_PWRONOFF_CLIENT_MODEL */
    #ifdef CLI_GENERICS_PWRLEVEL_CLIENT_MODEL
    { "powerlevel", "Generic Power Level Client Operations", cli_modelc_generic_power_level },
    #endif /* CLI_GENERICS_PWRLEVEL_CLIENT_MODEL */
    #ifdef CLI_GENERICS_BATTERY_CLIENT_MODEL
    { "battery", "Generic Battery Client Operations", cli_modelc_generic_battery },
    #endif /* CLI_GENERICS_BATTERY_CLIENT_MODEL */
    #ifdef CLI_GENERICS_LOCATION_CLIENT_MODEL
    { "location", "Generic Location Client Operations", cli_modelc_generic_location },
    #endif /* CLI_GENERICS_LOCATION_CLIENT_MODEL */
    #ifdef CLI_GENERICS_PROPERTY_CLIENT_MODEL
    { "property", "Generic Property Client Operations", cli_modelc_generic_property },
    #endif /* CLI_GENERICS_PROPERTY_CLIENT_MODEL */
    #endif /* CLI_GENERICS_CLIENT_MODEL */

    #if (defined CLI_GENERICS_CLIENT_MODEL || defined CLI_LIGHTINGS_CLIENT_MODEL)
    { "scene", "Scene Client Operations", cli_modelc_scene },
    #endif /* (defined CLI_GENERICS_CLIENT_MODEL || defined CLI_LIGHTINGS_CLIENT_MODEL) */

    #ifdef CLI_LIGHTINGS_CLIENT_MODEL
    #ifdef CLI_LIGHTINGS_LIGHTNESS_CLIENT_MODEL
    { "lightness", "Light Lightness Client Operations", cli_modelc_light_lightness },
    #endif /* CLI_LIGHTINGS_LIGHTNESS_CLIENT_MODEL */
    #ifdef CLI_LIGHTINGS_CTL_CLIENT_MODEL
    { "ctl", "Light CTL Client Operations", cli_modelc_light_ctl },
    #endif /* CLI_LIGHTINGS_CTL_CLIENT_MODEL */
    #ifdef CLI_LIGHTINGS_HSL_CLIENT_MODEL
    { "hsl", "Light HSL Client Operations", cli_modelc_light_hsl },
    #endif /* CLI_LIGHTINGS_HSL_CLIENT_MODEL */
    #ifdef CLI_LIGHTINGS_XYL_CLIENT_MODEL
    { "xyl", "Light xyL Client Operations", cli_modelc_light_xyl },
    #endif /* CLI_LIGHTINGS_XYL_CLIENT_MODEL */
    #ifdef CLI_LIGHTINGS_LC_CLIENT_MODEL
    { "lc", "Light LC Client Operations", cli_modelc_light_lc },
    #endif /* CLI_LIGHTINGS_LC_CLIENT_MODEL */
    #endif /* CLI_LIGHTINGS_CLIENT_MODEL */
};

/* ------------------------------- Functions */
/* Model Client */
API_RESULT cli_model_client(UINT32 argc, UCHAR* argv[])
{
    CONSOLE_OUT("In Model Client \n");
    cli_cmd_stack_push((CLI_COMMAND*)cli_modelc_cmd_list, sizeof(cli_modelc_cmd_list) / sizeof(CLI_COMMAND));
    cli_help(argc, argv);
    return API_SUCCESS;
}

