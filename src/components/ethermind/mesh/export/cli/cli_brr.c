/**
    \file cli_brr.c

    This File contains the "main" function for the CLI application,
    to exercise various functionalities of the Mindtree Mesh stack.
*/

/*
    Copyright (C) 2017. Mindtree Ltd.
    All rights reserved.
*/

/* ------------------------------- Header File Inclusion */
#include "cli_main.h"
#include "blebrr.h"

/* --------------------------------------------- External Global Variables */

/* ------------------------------- Global Variables */

DECL_CONST CLI_COMMAND cli_core_client_menu_cmd_list[] =
{
    /* Help */
    { "help", "Help", cli_help },

    /* Create GATT Connection to Peer */
    { "connect", "Create GATT Connection", cli_create_gatt_conn},

    /* Initiate GATT Disconnection with Peer */
    { "disconnect", "Terminate GATT Connection", cli_terminate_gatt_conn },

    /* Start or Stop Scan in Bearer */
    { "scan", "Enable/Disable Scanning", cli_scan_feature},

    /* Set Scan Response Data */
    { "srsp_set", "Set Scan Response Data", cli_scan_rsp_data_set},

    /* Discover GATT Bearer Services */
    {"discover", "Discover GATT Service", cli_discover_service},

    /* Enable/Disable GATT Bearer Services for Notfications */
    {"config_ntf", "Enable/Disable Notification", cli_config_ntf},

    /* Back */
    { "back", "One Level Up", cli_back },

    /* Root */
    { "root", "Back to Root", cli_root }
};

/* ------------------------------- Functions */
API_RESULT cli_brr(UINT32 argc, UCHAR* argv[])
{
    cli_cmd_stack_push((CLI_COMMAND*)cli_core_client_menu_cmd_list, sizeof(cli_core_client_menu_cmd_list) / sizeof(CLI_COMMAND));
    cli_help(argc, argv);
    return API_SUCCESS;
}

API_RESULT cli_create_gatt_conn(UINT32 argc, UCHAR* argv[])
{
    UCHAR peer_bd_addr_type;
    UCHAR peer_bd_addr[6];
    API_RESULT retval;

    if (2 != argc)
    {
        CONSOLE_OUT("Usage: connect <Peer Address Type> <Peer Address MSB-LSB>\n");
        return API_FAILURE;
    }

    peer_bd_addr_type = (UCHAR)CLI_strtoi(argv[0], (UINT8)CLI_strlen(argv[0]), 16);
    CLI_strtoarray
    (
        argv[1],
        CLI_strlen(argv[1]),
        &peer_bd_addr[0],
        6
    );
    retval = blebrr_create_gatt_conn_pl(peer_bd_addr_type, peer_bd_addr);
    CONSOLE_OUT("retval = 0x%04X\n", retval);
    return retval;
}

API_RESULT cli_scan_feature(UINT32 argc, UCHAR* argv[])
{
    CONSOLE_OUT("\n Scan Start Stop Feature to be extended for CLI\n");
    return API_FAILURE;
}

API_RESULT cli_scan_rsp_data_set(UINT32 argc, UCHAR* argv[])
{
    /**
        Currently setting MT-MESH-DEMO as Complete Device Name!
        This can be updated to each individual devices as per requirement.
    */
    UCHAR cli_brr_scanrsp_data[] =
    {
        /**
            Shortened Device Name: MT-MESH-DEMO
        */
        0x0D, 0x09, 'M', 'T', '-', 'M', 'E', 'S', 'H', '-', 'D', 'E', 'M', 'O'
    };
    CONSOLE_OUT("\n Setting MT-MESH-DEMO as Complete Device Name!\n");
    /* Set the Scan Response Data at the Bearer Layer */
    blebrr_set_adv_scanrsp_data_pl
    (
        cli_brr_scanrsp_data,
        sizeof(cli_brr_scanrsp_data)
    );
    return API_SUCCESS;
}

API_RESULT cli_terminate_gatt_conn(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    retval = blebrr_disconnect_pl();

    if (API_SUCCESS != retval)
    {
        CONSOLE_OUT("Failed to stop Adv");
        return retval;
    }

    CONSOLE_OUT("Disconnected successfully\n\n");
    return retval;
}

API_RESULT cli_discover_service(UINT32 argc, UCHAR* argv[])
{
    UCHAR service;

    if (1 != argc)
    {
        CONSOLE_OUT("Usage: discover <Service:[0-Mesh Provisioning service, 1-Mesh Proxy Service]>\n");
        return API_FAILURE;
    }

    service = (UCHAR)CLI_strtoi(argv[0], (UINT8)CLI_strlen(argv[0]), 16);
    return blebrr_discover_service_pl(service);
}

API_RESULT cli_config_ntf(UINT32 argc, UCHAR* argv[])
{
    UCHAR config_ntf;
    UCHAR mode;

    if (2 != argc)
    {
        CONSOLE_OUT("Usage: config_ntf < 0 - Disable, 1 - Enable> <Service:[0-Mesh Provisioning service, 1-Mesh Proxy Service]>\n");
        return API_FAILURE;
    }

    config_ntf = (UCHAR)CLI_strtoi(argv[0], (UINT8)CLI_strlen(argv[0]), 16);
    mode       = (UCHAR)CLI_strtoi(argv[1], (UINT8)CLI_strlen(argv[1]), 16);
    return blebrr_confige_ntf_pl(config_ntf, mode);
}

