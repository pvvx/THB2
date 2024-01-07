
/**
    \file cli_model_server.c

    This File contains the "model server" handlers for the CLI application,
    to exercise various functionalities of the Mindtree Mesh stack.
*/

/*
    Copyright (C) 2017. Mindtree Ltd.
    All rights reserved.
*/

/* ------------------------------- Header File Inclusion */
#include "cli_main.h"

/* Model server application entry point */
void main_generic_battery_server_operations(/* IN */ UINT8 have_menu);
void main_generic_default_transition_time_server_operations(/* IN */ UINT8 have_menu);
void main_generic_level_server_operations(/* IN */ UINT8 have_menu);
void main_generic_location_server_operations(/* IN */ UINT8 have_menu);
void main_generic_onoff_server_operations(/* IN */ UINT8 have_menu);
void main_generic_power_level_server_operations(/* IN */ UINT8 have_menu);
void main_generic_power_onoff_server_operations(/* IN */ UINT8 have_menu);
void main_generic_property_server_operations(/* IN */ UINT8 have_menu);
void main_health_server_operations(/* IN */ UINT8 have_menu);
API_RESULT main_health_server_log_fault
(
    /* IN */ UINT8 test_id,
    /* IN */ UINT8 fault
);
void main_light_ctl_server_operations(/* IN */ UINT8 have_menu);
void main_light_hsl_server_operations(/* IN */ UINT8 have_menu);
void main_light_lc_server_operations(/* IN */ UINT8 have_menu);
void main_light_lightness_server_operations(/* IN */ UINT8 have_menu);
void main_light_xyl_server_operations(/* IN */ UINT8 have_menu);
void main_scene_server_operations(/* IN */ UINT8 have_menu);
API_RESULT appl_model_light_lc_server_set_default_trans_timeout_in_ms(/* IN */ UINT32 time_in_ms);

/* ------------------------------- Global Variables */
/* Level - Model - Server */
DECL_CONST CLI_COMMAND cli_models_cmd_list[] =
{
    /* Help */
    { "help", "Help", cli_help },

    /* Back */
    { "back", "One Level Up", cli_back },

    /* Root */
    { "root", "Back to Root", cli_root },

    /* Reset */
    { "reset", "Emulate Reset", cli_models_reset },

    /* Foundation Model Setup */
    { "foundation", "Foundation Server Models Initialization", cli_models_foundation },

    #ifdef CLI_HEALTH_SERVER_MODEL
    /* Health Server Model Setup */
    { "health", "Health Server Options", cli_models_health },
    #endif /* CLI_HEALTH_SERVER_MODEL */

    #ifdef CLI_GENERICS_SERVER_MODEL
    /* Generics */
    { "generics", "Generic Server Options", cli_models_generics },
    #endif /* CLI_GENERICS_SERVER_MODEL */

    #if (defined CLI_GENERICS_SERVER_MODEL || defined CLI_LIGHTINGS_SERVER_MODEL)
    /* Scene */
    { "scene", "Scene Server Model Initialization", cli_models_scene },
    #endif /* (defined CLI_GENERICS_SERVER_MODEL || defined CLI_LIGHTINGS_SERVER_MODEL) */

    #ifdef CLI_LIGHTINGS_SERVER_MODEL
    /* Light */
    { "light", "Light Server Options", cli_models_light },
    #endif /* CLI_LIGHTINGS_SERVER_MODEL */

    #ifdef HAVE_VENDOR_MODEL_EXAMPLE_1
    /* Vendor Specific */
    { "vendor", "Vendor Specific", cli_models_vendor },
    #endif /* HAVE_VENDOR_MODEL_EXAMPLE_1 */
};

#ifdef CLI_HEALTH_SERVER_MODEL
/* Level - Model = Server - Health */
DECL_CONST CLI_COMMAND cli_models_health_cmd_list[] =
{
    /* Help */
    { "help", "Help", cli_help },

    /* Back */
    { "back", "One Level Up", cli_back },

    /* Root */
    { "root", "Back to Root", cli_root },

    /* Model Server - Health - Log Fault */
    { "logfault", "Log Fault Values", cli_models_health_log_fault },

    /* Health - TODO: Temporary till model publication period is triggered */
    { "publishstatus", "Publish Health status", cli_health_status_publish }
};
#endif /* CLI_HEALTH_SERVER_MODEL */

#ifdef CLI_GENERICS_SERVER_MODEL
/* Level - Model = Server - Generics */
DECL_CONST CLI_COMMAND cli_models_generics_cmd_list[] =
{
    /* Help */
    { "help", "Help", cli_help },

    /* Back */
    { "back", "One Level Up", cli_back },

    /* Root */
    { "root", "Back to Root", cli_root },

    #ifdef CLI_GENERICS_ONOFF_SERVER_MODEL
    /* Model Server - Generics - OnOff */
    { "onoff", "OnOff Server Model Initialization", cli_models_generics_onoff },
    #endif /* CLI_GENERICS_ONOFF_SERVER_MODEL */

    #ifdef CLI_GENERICS_LEVEL_SERVER_MODEL
    /* Model Server - Generics - Level */
    { "level", "Level Server Model Initialization", cli_models_generics_level },
    #endif /* CLI_GENERICS_LEVEL_SERVER_MODEL */

    #ifdef CLI_GENERICS_TRANSITIONTIME_SERVER_MODEL
    /* Model Server - Generics - Default Transition Time */
    { "transitiontime", "Default Transition Time Server Model Initialization", cli_models_generics_default_transition_time },
    #endif /* CLI_GENERICS_TRANSITIONTIME_SERVER_MODEL */

    #ifdef CLI_GENERICS_PWRONOFF_SERVER_MODEL
    /* Model Server - Generics - Power OnOff */
    { "poweronoff", "Power OnOff Server Model Initialization", cli_models_generics_power_onoff },
    #endif /* CLI_GENERICS_PWRONOFF_SERVER_MODEL */

    #ifdef CLI_GENERICS_PWRLEVEL_SERVER_MODEL
    /* Model Server - Generics - Power Level */
    { "powerlevel", "Power Level Server Model Initialization", cli_models_generics_power_level },
    #endif /* CLI_GENERICS_PWRLEVEL_SERVER_MODEL */

    #ifdef CLI_GENERICS_BATTERY_SERVER_MODEL
    /* Model Server - Generics - Battery */
    { "battery", "Battery Server Model Initialization", cli_models_generics_battery },
    #endif /* CLI_GENERICS_BATTERY_SERVER_MODEL */

    #ifdef CLI_GENERICS_LOCATION_SERVER_MODEL
    /* Model Server - Generics - Location */
    { "location", "Location Server Model Initialization", cli_models_generics_location },
    #endif /* CLI_GENERICS_LOCATION_SERVER_MODEL */

    #ifdef CLI_GENERICS_PROPERTY_SERVER_MODEL
    /* Model Server - Generics - Property */
    { "property", "Property Server Model Initialization", cli_models_generics_property },
    #endif /* CLI_GENERICS_PROPERTY_SERVER_MODEL */
};
#endif /* CLI_GENERICS_SERVER_MODEL */

#ifdef CLI_LIGHTINGS_SERVER_MODEL
/* Level - Model = Server - Light */
DECL_CONST CLI_COMMAND cli_models_light_cmd_list[] =
{
    /* Help */
    { "help", "Help", cli_help },

    /* Back */
    { "back", "One Level Up", cli_back },

    /* Root */
    { "root", "Back to Root", cli_root },

    #ifdef CLI_LIGHTINGS_LIGHTNESS_SERVER_MODEL
    /* Model Server - Light Lightness */
    { "lightness", "Lightness Server Model Initialization", cli_models_light_lightness },
    #endif /* CLI_LIGHTINGS_LIGHTNESS_SERVER_MODEL */

    #ifdef CLI_LIGHTINGS_CTL_SERVER_MODEL
    /* Model Server - Light CTL */
    { "ctl", "CTL Server Model Initialization", cli_models_light_ctl },
    #endif /* CLI_LIGHTINGS_CTL_SERVER_MODEL */

    #ifdef CLI_LIGHTINGS_HSL_SERVER_MODEL
    /* Model Server - Light HSL */
    { "hsl", "HSL Server Model Initialization", cli_models_light_hsl },
    #endif /* CLI_LIGHTINGS_HSL_SERVER_MODEL */

    #ifdef CLI_LIGHTINGS_XYL_SERVER_MODEL
    /* Model Server - Light xyL */
    { "xyl", "xyL Server Model Initialization", cli_models_light_xyl },
    #endif /* CLI_LIGHTINGS_XYL_SERVER_MODEL */

    #ifdef CLI_LIGHTINGS_LC_SERVER_MODEL
    /* Model Server - Light LC */
    { "lc", "LC Server Model Initialization", cli_models_light_lc },

    /* Model Server - Set Default Transition Time in ms */
    { "lcsettranstime", "LC Set Default Transition time in ms", cli_models_light_lc_set_default_trans_timeout_in_ms}
    #endif /* CLI_LIGHTINGS_LC_SERVER_MODEL */
};
#endif /* CLI_LIGHTINGS_SERVER_MODEL */

/* ------------------------------- Functions */
/* Model Server */
API_RESULT cli_model_server(UINT32 argc, UCHAR* argv[])
{
    CONSOLE_OUT("In Model Server\n");
    cli_cmd_stack_push((CLI_COMMAND*)cli_models_cmd_list, sizeof(cli_models_cmd_list) / sizeof(CLI_COMMAND));
    cli_help(argc, argv);
    return API_SUCCESS;
}

/* Model Server - Foundation Models */

#ifndef HSL_DONT_USE_MULTI_ELEMENTS
    /* Two additional elements are registerd for HSL Server */
    MS_ACCESS_ELEMENT_HANDLE sec_element_handle;
    MS_ACCESS_ELEMENT_HANDLE ter_element_handle;
#endif /* HSL_DONT_USE_MULTI_ELEMENTS */

API_RESULT cli_models_foundation(UINT32 argc, UCHAR* argv[])
{
    MS_ACCESS_NODE_ID node_id;
    MS_ACCESS_ELEMENT_DESC   element;
    MS_ACCESS_ELEMENT_HANDLE element_handle;
    MS_ACCESS_MODEL_HANDLE   config_server_model_handle;
    API_RESULT retval;
    CONSOLE_OUT("In Model Server - Foundation Models\n");
    /* Create Node */
    retval = MS_access_create_node(&node_id);
    /* Register Element */
    /**
        TBD: Define GATT Namespace Descriptions from
        https://www.bluetooth.com/specifications/assigned-numbers/gatt-namespace-descriptors

        Using 'main' (0x0106) as Location temporarily.
    */
    element.loc = 0x0106;
    retval = MS_access_register_element
             (
                 node_id,
                 &element,
                 &element_handle
             );
    #ifdef CLI_CONFIG_SERVER_MODEL

    if (API_SUCCESS == retval)
    {
        retval = MS_config_server_init(element_handle, &config_server_model_handle);
    }

    #endif /* CLI_CONFIG_SERVER_MODEL */
    CONSOLE_OUT("Model Registration Status: 0x%04X\n", retval);
    #ifdef CLI_HEALTH_SERVER_MODEL
    /* Health Server */
    main_health_server_operations(MS_FALSE);
    #endif /* CLI_HEALTH_SERVER_MODEL */
    #ifndef HSL_DONT_USE_MULTI_ELEMENTS
    /* Two additional elements are registerd for HSL Server */
    retval = MS_access_register_element
             (
                 node_id,
                 &element,
                 &sec_element_handle
             );

    if (API_SUCCESS == retval)
    {
        CONSOLE_OUT("Secondary Element Handle Registered @: 0x%04X\n", sec_element_handle);
    }

    retval = MS_access_register_element
             (
                 node_id,
                 &element,
                 &ter_element_handle
             );

    if (API_SUCCESS == retval)
    {
        CONSOLE_OUT("Tertiary Element Handle Registered @: 0x%04X\n", ter_element_handle);
    }

    #endif /* HSL_DONT_USE_MULTI_ELEMENTS */
    (void) config_server_model_handle;
    return API_SUCCESS;
}

#ifdef CLI_HEALTH_SERVER_MODEL
/* Model Server - Health */
API_RESULT cli_models_health(UINT32 argc, UCHAR* argv[])
{
    CONSOLE_OUT("In Model Server - Health\n");
    cli_cmd_stack_push((CLI_COMMAND*)cli_models_health_cmd_list, sizeof(cli_models_health_cmd_list) / sizeof(CLI_COMMAND));
    cli_help(argc, argv);
    return API_SUCCESS;
}

/* Model Server - Health - Log Fault */
API_RESULT cli_models_health_log_fault(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    UINT8  test_id, fault;
    CONSOLE_OUT("In Model Server - Health - Log Fault\n");
    retval = API_FAILURE;

    if (2 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        test_id = (UINT8)choice;
        CONSOLE_OUT("Test ID (8-bit in HEX): 0x%02X\n", test_id);
        choice = CLI_strtoi(argv[1], CLI_strlen(argv[1]), 16);
        fault = (UINT8)choice;
        CONSOLE_OUT("Fault Value (8-bit in HEX): 0x%02X\n", fault);
        retval = main_health_server_log_fault(test_id, fault);
    }
    else
    {
        CONSOLE_OUT("Usage: logfault <test_id> <fault_code>\n");
        retval = API_FAILURE;
    }

    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Health - TODO: Temporary till model publication period is triggered */
API_RESULT cli_health_status_publish(UINT32 argc, UCHAR* argv[])
{
    UCHAR current_status[4] = { 0x00, 0x6A, 0x00, 0x00 };

    if (1 != argc)
    {
        CONSOLE_OUT("Usage: publishstatus <current status>\n");
        return API_FAILURE;
    }

    current_status[0] = (UCHAR)CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
    return MS_health_server_publish_current_status(current_status, sizeof(current_status));
}
#endif /* CLI_HEALTH_SERVER_MODEL */

#ifdef CLI_GENERICS_SERVER_MODEL
/* Model Server - Generics */
API_RESULT cli_models_generics(UINT32 argc, UCHAR* argv[])
{
    CONSOLE_OUT("In Model Server - Generics\n");
    cli_cmd_stack_push((CLI_COMMAND*)cli_models_generics_cmd_list, sizeof(cli_models_generics_cmd_list) / sizeof(CLI_COMMAND));
    cli_help(argc, argv);
    return API_SUCCESS;
}

/* Model Server - Generics - OnOff */
API_RESULT cli_models_generics_onoff(UINT32 argc, UCHAR* argv[])
{
    CONSOLE_OUT("In Model Server - Generics - OnOff\n");
    main_generic_onoff_server_operations(MS_FALSE);
    return API_SUCCESS;
}

/* Model Server - Generics - Level */
API_RESULT cli_models_generics_level(UINT32 argc, UCHAR* argv[])
{
    CONSOLE_OUT("In Model Server - Generics - Level\n");
    main_generic_level_server_operations(MS_FALSE);
    return API_SUCCESS;
}

/* Model Server - Generics - Default Transition Time */
API_RESULT cli_models_generics_default_transition_time(UINT32 argc, UCHAR* argv[])
{
    CONSOLE_OUT("In Model Server - Generics - Default Transition Time\n");
    main_generic_default_transition_time_server_operations(MS_FALSE);
    return API_SUCCESS;
}

/* Model Server - Generics - Power OnOff */
API_RESULT cli_models_generics_power_onoff(UINT32 argc, UCHAR* argv[])
{
    CONSOLE_OUT("In Model Server - Generics - Power OnOff\n");
    main_generic_power_onoff_server_operations(MS_FALSE);
    return API_SUCCESS;
}

/* Model Server - Generics - Power Level */
API_RESULT cli_models_generics_power_level(UINT32 argc, UCHAR* argv[])
{
    CONSOLE_OUT("In Model Server - Generics - Power Level\n");
    main_generic_power_level_server_operations(MS_FALSE);
    return API_SUCCESS;
}

/* Model Server - Generics - Battery */
API_RESULT cli_models_generics_battery(UINT32 argc, UCHAR* argv[])
{
    CONSOLE_OUT("In Model Server - Generics - Battery\n");
    main_generic_battery_server_operations(MS_FALSE);
    return API_SUCCESS;
}

/* Model Server - Generics - Location */
API_RESULT cli_models_generics_location(UINT32 argc, UCHAR* argv[])
{
    CONSOLE_OUT("In Model Server - Generics - Location\n");
    main_generic_location_server_operations(MS_FALSE);
    return API_SUCCESS;
}

/* Model Server - Generics - Property */
API_RESULT cli_models_generics_property(UINT32 argc, UCHAR* argv[])
{
    CONSOLE_OUT("In Model Server - Generics - Property\n");
    main_generic_property_server_operations(MS_FALSE);
    return API_SUCCESS;
}
#endif /* CLI_GENERICS_SERVER_MODEL */

#if (defined CLI_GENERICS_SERVER_MODEL || defined CLI_LIGHTINGS_SERVER_MODEL)
/* Model Server - Time and Scene */
/* Model Server - Scene */
API_RESULT cli_models_scene(UINT32 argc, UCHAR* argv[])
{
    CONSOLE_OUT("In Model Server - Scene\n");
    main_scene_server_operations(MS_FALSE);
    return API_SUCCESS;
}
#endif /* (defined CLI_GENERICS_SERVER_MODEL || defined CLI_LIGHTINGS_SERVER_MODEL) */

#ifdef CLI_LIGHTINGS_SERVER_MODEL
/* Model Server - Light */
API_RESULT cli_models_light(UINT32 argc, UCHAR* argv[])
{
    CONSOLE_OUT("In Model Server - Light\n");
    cli_cmd_stack_push((CLI_COMMAND*)cli_models_light_cmd_list, sizeof(cli_models_light_cmd_list) / sizeof(CLI_COMMAND));
    cli_help(argc, argv);
    return API_SUCCESS;
}

/* Model Server - Light Lightness */
API_RESULT cli_models_light_lightness(UINT32 argc, UCHAR* argv[])
{
    CONSOLE_OUT("In Model Server - Light - Lightness\n");
    main_light_lightness_server_operations(MS_FALSE);
    return API_SUCCESS;
}

/* Model Server - Light CTL */
API_RESULT cli_models_light_ctl(UINT32 argc, UCHAR* argv[])
{
    CONSOLE_OUT("In Model Server - Light - CTL\n");
    main_light_ctl_server_operations(MS_FALSE);
    return API_SUCCESS;
}

/* Model Server - Light HSL */
API_RESULT cli_models_light_hsl(UINT32 argc, UCHAR* argv[])
{
    CONSOLE_OUT("In Model Server - Light - HSL\n");
    main_light_hsl_server_operations(MS_FALSE);
    return API_SUCCESS;
}

/* Model Server - Light xyL */
API_RESULT cli_models_light_xyl(UINT32 argc, UCHAR* argv[])
{
    CONSOLE_OUT("In Model Server - Light - xyL\n");
    main_light_xyl_server_operations(MS_FALSE);
    return API_SUCCESS;
}

/* Model Server - Light LC */
API_RESULT cli_models_light_lc(UINT32 argc, UCHAR* argv[])
{
    CONSOLE_OUT("In Model Server - Light - LC\n");
    main_light_lc_server_operations(MS_FALSE);
    return API_SUCCESS;
}

/* Model Server - Set Default Transition Time in ms */
API_RESULT cli_models_light_lc_set_default_trans_timeout_in_ms(UINT32 argc, UCHAR* argv[])
{
    UINT32 time_in_ms;
    CONSOLE_OUT("In Model Server - Light - LC - Set Default Timeout in ms\n");
    CONSOLE_OUT("In Core Provision Setup\n");

    if (1 != argc)
    {
        CONSOLE_OUT("Usage: lcsettranstime <timeout in ms in HEX>\n");
        return API_FAILURE;
    }

    time_in_ms = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
    appl_model_light_lc_server_set_default_trans_timeout_in_ms(time_in_ms);
    return API_SUCCESS;
}
#endif /* CLI_LIGHTINGS_SERVER_MODEL */

#ifdef HAVE_VENDOR_MODEL_EXAMPLE_1
/* Model Server - Vendor Example 1 */
API_RESULT cli_models_vendor(UINT32 argc, UCHAR* argv[])
{
    CONSOLE_OUT("In Model Server - Vendor\n");
    main_vendor_example_1_server_operations(MS_FALSE);
    return API_SUCCESS;
}
#endif /* HAVE_VENDOR_MODEL_EXAMPLE_1 */

/* Reset */
/* TODO: Shall be part of common reset called from root */
API_RESULT cli_models_reset(UINT32 argc, UCHAR* argv[])
{
    CONSOLE_OUT("In Model Server - calling RESET\n");
    appl_model_power_cycle();
    return API_SUCCESS;
}

