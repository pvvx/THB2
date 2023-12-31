
/**
    \file cli_core.c

    This File contains the "Core" Layer handlers for the CLI application,
    to exercise various functionalities of the Mindtree Mesh stack.
*/

/*
    Copyright (C) 2017. Mindtree Ltd.
    All rights reserved.
*/

/* ------------------------------- Header File Inclusion */
#include "cli_main.h"
#include "nvsto.h"
#include "blebrr.h"
#include "access_extern.h"

/* ------------------------------- Global Variables */
/* Level - Core */
DECL_CONST CLI_COMMAND cli_core_cmd_list[] =
{
    /* Help */
    { "help", "Help", cli_help },

    /* Setup */
    { "setup", "Setup", cli_core_setup },

    #ifdef CLI_PROVISION
    /* Provision */
    { "provision", "Provisioning Menu", cli_core_provision },
    #endif /* CLI_PROVISION */

    #ifdef CLI_PROXY
    /* Proxy */
    { "proxy", "Start Proxy", cli_core_proxy },
    #endif /* CLI_PROXY */

    #ifdef CLI_TRANSPORT
    /* Transport */
    { "transport", "Transport Menu", cli_core_transport },
    #endif /* CLI_TRANSPORT */

    #ifdef CLI_NETWORK
    /* Network */
    { "network", "Network Menu", cli_core_network },
    #endif /* CLI_NETWORK */

    /* Enable */
    { "enable", "Enable relay/proxy/friend/lpn", cli_core_enable_feature },

    /* Enable */
    { "disable", "Disable relay/proxy/friend/lpn", cli_core_disable_feature },

    /* Back */
    { "back", "One Level Up", cli_back },

    /* Root */
    { "root", "Back to Root", cli_root }

};

/* ------------------------------- Functions */
/* CLI Command Handlers. Level - Root */

/* Core */
API_RESULT cli_core(UINT32 argc, UCHAR* argv[])
{
    CONSOLE_OUT("In Core\n");
    cli_cmd_stack_push((CLI_COMMAND*)cli_core_cmd_list, sizeof(cli_core_cmd_list) / sizeof(CLI_COMMAND));
    cli_help(argc, argv);
    return API_SUCCESS;
}

/* CLI Command Handlers. Level - Core */
/* Setup */
API_RESULT cli_core_setup(UINT32 argc, UCHAR* argv[])
{
    MS_CONFIG* config_ptr;
    #ifdef MS_HAVE_DYNAMIC_CONFIG
    MS_CONFIG  config;
    /* Initialize dynamic configuration */
    MS_INIT_CONFIG(config);
    config_ptr = &config;
    #else
    config_ptr = NULL;
    #endif /* MS_HAVE_DYNAMIC_CONFIG */
    #ifdef MEMWATCH
    mwInit();
    #endif /* MEMWATCH */
    CONSOLE_OUT("In Core Setup\n");
    /* Initialize OSAL */
    EM_os_init();
    /* Initialize Debug Module */
    EM_debug_init();
    /* Initialize Timer Module */
    EM_timer_init();
    timer_em_init();
    /* Initialize utilities */
    nvsto_init(NVS_FLASH_BASE1,NVS_FLASH_BASE2);
    /* Initialize Mesh Stack */
    MS_init(config_ptr);
    /* Register with underlying BLE stack */
    blebrr_register();
    /* Register GATT Bearer Connection/Disconnection Event Hook */
    blebrr_register_gatt_iface_event_pl(cli_gatt_bearer_iface_event_pl_cb);

    /* Enable bearer scan if device provisioned and configured */
    if (API_SUCCESS == appl_is_configured())
    {
        CONSOLE_OUT ("Device Provisioned and Configured\n");
        blebrr_scan_enable();
    }
    else
    {
        CONSOLE_OUT ("Provision and Configure the Device!\n");
    }

    return API_SUCCESS;
}

/* Enable Feature */
API_RESULT cli_core_enable_feature(UINT32 argc, UCHAR* argv[])
{
    UINT32  index;
    UCHAR* features[4] = { "relay", "proxy", "friend", "lpn" };
    CONSOLE_OUT("In Enable Feature\n");

    if (1 == argc)
    {
        for (index = 0; index < 4; index++)
        {
            if (0 == CLI_STR_COMPARE(argv[0], features[index]))
            {
                break;
            }
        }

        if (index < 4)
        {
            MS_access_cm_set_features_field(MS_ENABLE, index);
            CONSOLE_OUT("Enable Feature:%s\n", features[index]);
        }
        else
        {
            CONSOLE_OUT("Invalid Argument: %s\n", argv[0]);
        }
    }
    else
    {
        CONSOLE_OUT("Invalid Number of Arguments:0x%04X\n", argc);
    }

    return API_SUCCESS;
}

/* Disable Feature */
API_RESULT cli_core_disable_feature(UINT32 argc, UCHAR* argv[])
{
    UINT32  index;
    UCHAR* features[4] = { "relay", "proxy", "friend", "lpn" };
    CONSOLE_OUT("In Disable Feature\n");

    if (1 == argc)
    {
        for (index = 0; index < 4; index++)
        {
            if (0 == CLI_STR_COMPARE(argv[0], features[index]))
            {
                break;
            }
        }

        if (index < 4)
        {
            MS_access_cm_set_features_field(MS_DISABLE, index);
            CONSOLE_OUT("Disable Feature:%s\n", features[index]);
        }
        else
        {
            CONSOLE_OUT("Invalid Argument: %s\n", argv[0]);
        }
    }
    else
    {
        CONSOLE_OUT("Invalid Number of Arguments:0x%04X\n", argc);
    }

    return API_SUCCESS;
}

