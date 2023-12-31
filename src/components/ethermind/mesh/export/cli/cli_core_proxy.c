
/**
    \file cli_core_proxy.c

    This File contains the "main" function for the CLI application,
    to exercise various functionalities of the Mindtree Mesh stack.
*/

/*
    Copyright (C) 2017. Mindtree Ltd.
    All rights reserved.
*/

/* ------------------------------- Header File Inclusion */
#include "cli_main.h"

#ifdef CLI_PROXY

/* --------------------------------------------- External Global Variables */
#ifdef MS_PROXY_SUPPORT
extern NETIF_HANDLE g_appl_netif_hndl;

/* ------------------------------- Global Variables */
/** Macro to hold configurabile limit of Address from CLI command */
#define CLI_PROXY_MAX_PROXY_ADDR_CNT              5

DECL_CONST CLI_COMMAND cli_core_proxy_cmd_list[] =
{
    /* Help */
    { "help", "Help", cli_help },

    /* Proxy Server Operation Setup */
    { "proxys", "Proxy Server Operation", cli_core_proxy_server_op},

    /* Proxy Client Opeartion Setup */
    { "proxyc", "Proxy Client Operation", cli_core_proxy_client_op },

    /* Back */
    { "back", "One Level Up", cli_back },

    /* Root */
    { "root", "Back to Root", cli_root }
};

DECL_CONST CLI_COMMAND cli_core_proxy_server_cmd_list[] =
{
    /* Help */
    { "help", "Help", cli_help },

    /* Start Advertising Proxy Service */
    {"start", "Start Proxy ADV", cli_core_start_proxy_adv},

    /* Stop Advertising proxy Service */
    { "stop", "Stop Proxy ADV", cli_core_stop_proxy_adv },

    /* Back */
    { "back", "One Level Up", cli_back },

    /* Root */
    { "root", "Back to Root", cli_root }
};

DECL_CONST CLI_COMMAND cli_core_proxy_client_cmd_list[] =
{
    /* Help */
    { "help", "Help", cli_help },

    /* Set WhiteList Filter Type */
    { "setwl", "Set White List Filter", cli_core_set_wl_filter },

    /* Set BlackList Filter Type */
    { "setbl", "Set Black List Filter", cli_core_set_bl_filter },

    /* Add Address to Filter */
    { "add_addr", "Add Address to Filter", cli_core_add_addr_to_filter },

    /* Remove Address from Filter */
    { "rm_addr", "Remove Address to Filter", cli_core_rm_addr_to_filter },

    /* Back */
    { "back", "One Level Up", cli_back },

    /* Root */
    { "root", "Back to Root", cli_root }
};
#endif /* MS_PROXY_SUPPORT */
/* ------------------------------- Functions */
API_RESULT cli_core_proxy(UINT32 argc, UCHAR* argv[])
{
    #ifdef MS_PROXY_SUPPORT
    /* Register with Proxy */
    appl_proxy_register();
    cli_cmd_stack_push((CLI_COMMAND*)cli_core_proxy_cmd_list, sizeof(cli_core_proxy_cmd_list) / sizeof(CLI_COMMAND));
    cli_help(argc, argv);
    return API_SUCCESS;
    #else /* MS_PROXY_SUPPORT */
    CONSOLE_OUT(
        "Proxy Support Feature Currently Disabled!!!\n");
    return API_FAILURE;
    #endif /* MS_PROXY_SUPPORT */
}

#ifdef MS_PROXY_SUPPORT
API_RESULT cli_core_proxy_server_op(UINT32 argc, UCHAR* argv[])
{
    cli_cmd_stack_push((CLI_COMMAND*)cli_core_proxy_server_cmd_list, sizeof(cli_core_proxy_server_cmd_list) / sizeof(CLI_COMMAND));
    cli_help(argc, argv);
    return API_SUCCESS;
}

API_RESULT cli_core_proxy_client_op(UINT32 argc, UCHAR* argv[])
{
    cli_cmd_stack_push((CLI_COMMAND*)cli_core_proxy_client_cmd_list, sizeof(cli_core_proxy_client_cmd_list) / sizeof(CLI_COMMAND));
    cli_help(argc, argv);
    return API_SUCCESS;
}

API_RESULT cli_core_start_proxy_adv(UINT32 argc, UCHAR* argv[])
{
    #ifdef MS_PROXY_SERVER
    UCHAR idtfn_type;
    MS_SUBNET_HANDLE sub_hndl;
    CONSOLE_OUT("Start Proxy ADV....\n");

    if (2 != argc)
    {
        CONSOLE_OUT("Usage: start <Type:[1 - Network ID, 2 - Node Identity]> <Subnet Handle>\n");
        return API_FAILURE;
    }

    idtfn_type = (UCHAR) CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
    sub_hndl   = (MS_SUBNET_HANDLE) CLI_strtoi(argv[1], CLI_strlen(argv[1]), 16);
    appl_proxy_adv(idtfn_type, sub_hndl);
    return API_SUCCESS;
    #else /* MS_PROXY_SERVER */
    CONSOLE_OUT(
        "Proxy Server Feature Currently Disabled!!!\n");
    return API_FAILURE;
    #endif /* MS_PROXY_SERVER */
}

API_RESULT cli_core_stop_proxy_adv(UINT32 argc, UCHAR* argv[])
{
    #ifdef MS_PROXY_SERVER
    API_RESULT retval;
    MS_IGNORE_UNUSED_PARAM(argc);
    MS_IGNORE_UNUSED_PARAM(argv);
    retval = MS_proxy_server_adv_stop();

    if (API_SUCCESS != retval)
    {
        CONSOLE_OUT("Failed to stop Adv\n");
        return retval;
    }

    CONSOLE_OUT("ADV Stopped successfully!\n");
    return retval;
    #else /* MS_PROXY_SERVER */
    CONSOLE_OUT(
        "Proxy Server Feature Currently Disabled!!!\n");
    return API_FAILURE;
    #endif /* MS_PROXY_SERVER */
}

API_RESULT cli_core_set_wl_filter(UINT32 argc, UCHAR* argv[])
{
    #ifdef MS_PROXY_CLIENT
    API_RESULT retval;
    MS_IGNORE_UNUSED_PARAM(argc);
    MS_IGNORE_UNUSED_PARAM(argv);
    retval = MS_proxy_set_whitelist_filter(&g_appl_netif_hndl, 0x0000);

    if (API_SUCCESS != retval)
    {
        CONSOLE_OUT("Failed to stop Adv");
        return retval;
    }

    CONSOLE_OUT("Set WhiteList filter Successfully\n\n");
    return retval;
    #else /* MS_PROXY_CLIENT */
    MS_IGNORE_UNUSED_PARAM(argc);
    MS_IGNORE_UNUSED_PARAM(argv);
    CONSOLE_OUT(
        "Proxy Client Feature Currently Disabled!!!\n");
    return API_FAILURE;
    #endif /* MS_PROXY_CLIENT */
}

API_RESULT cli_core_set_bl_filter(UINT32 argc, UCHAR* argv[])
{
    #ifdef MS_PROXY_CLIENT
    API_RESULT retval;
    MS_IGNORE_UNUSED_PARAM(argc);
    MS_IGNORE_UNUSED_PARAM(argv);
    retval = MS_proxy_set_blacklist_filter(&g_appl_netif_hndl, 0x0000);

    if (API_SUCCESS != retval)
    {
        CONSOLE_OUT("Failed to stop Adv");
        return retval;
    }

    CONSOLE_OUT("Set BlackList filter Successfully\n\n");
    return retval;
    #else /* MS_PROXY_CLIENT */
    MS_IGNORE_UNUSED_PARAM(argc);
    MS_IGNORE_UNUSED_PARAM(argv);
    CONSOLE_OUT(
        "Proxy Client Feature Currently Disabled!!!\n");
    return API_FAILURE;
    #endif /* MS_PROXY_CLIENT */
}

API_RESULT cli_core_add_addr_to_filter(UINT32 argc, UCHAR* argv[])
{
    #ifdef MS_PROXY_CLIENT
    PROXY_ADDR       appl_proxy_addr_list[CLI_PROXY_MAX_PROXY_ADDR_CNT];
    UINT32           i;
    MS_SUBNET_HANDLE sub_hndl;
    UCHAR            appl_proxy_addr_count;

    if ((0 == argc) || (argc > (CLI_PROXY_MAX_PROXY_ADDR_CNT + 1)))
    {
        CONSOLE_OUT("Usage  : add_addr <subnet handle> <Addresses[Max of 5 addresses]>\n");
        CONSOLE_OUT("Example: add_addr 0 1001 1002 ... 1005]>\n");
        return API_FAILURE;
    }

    /* Initialize */
    EM_mem_set(appl_proxy_addr_list, 0x0, sizeof(appl_proxy_addr_list));
    appl_proxy_addr_count = 0;
    sub_hndl              = 0xFFFF;
    sub_hndl              = (MS_SUBNET_HANDLE)CLI_strtoi(argv[0], (UINT16)CLI_strlen(argv[0]), 16);
    appl_proxy_addr_count = argc - 1;

    for (i = 0; i < appl_proxy_addr_count; i++)
    {
        appl_proxy_addr_list[i] = CLI_strtoi(argv[i + 1], CLI_strlen(argv[i + 1]), 16);
    }

    MS_proxy_add_to_list
    (
        &g_appl_netif_hndl,
        sub_hndl,
        appl_proxy_addr_list,
        appl_proxy_addr_count
    );
    return API_SUCCESS;
    #else /* MS_PROXY_CLIENT */
    MS_IGNORE_UNUSED_PARAM(argc);
    MS_IGNORE_UNUSED_PARAM(argv);
    CONSOLE_OUT(
        "Proxy Client Feature Currently Disabled!!!\n");
    return API_FAILURE;
    #endif /* MS_PROXY_CLIENT */
}

API_RESULT cli_core_rm_addr_to_filter(UINT32 argc, UCHAR* argv[])
{
    #ifdef MS_PROXY_CLIENT
    PROXY_ADDR       appl_proxy_addr_list[CLI_PROXY_MAX_PROXY_ADDR_CNT];
    UINT32           i;
    MS_SUBNET_HANDLE sub_hndl;
    UCHAR            appl_proxy_addr_count;

    if ((0 == argc) || (argc > (CLI_PROXY_MAX_PROXY_ADDR_CNT + 1)))
    {
        CONSOLE_OUT("Usage  : rm_addr <subnet handle> <Addresses[Max of 5 addresses]>\n");
        CONSOLE_OUT("Example: rm_addr 0 1001 1002 ... 1005]>\n");
        return API_FAILURE;
    }

    /* Initialize */
    EM_mem_set(appl_proxy_addr_list, 0x0,sizeof(appl_proxy_addr_list));
    appl_proxy_addr_count = 0;
    sub_hndl              = 0xFFFF;
    sub_hndl              = (MS_SUBNET_HANDLE)CLI_strtoi(argv[0], (UINT16)CLI_strlen(argv[0]), 16);
    appl_proxy_addr_count = argc - 1;

    for (i = 0; i < appl_proxy_addr_count; i++)
    {
        appl_proxy_addr_list[i] = CLI_strtoi(argv[i + 1], CLI_strlen(argv[i + 1]), 16);
    }

    MS_proxy_del_from_list
    (
        &g_appl_netif_hndl,
        sub_hndl,
        appl_proxy_addr_list,
        appl_proxy_addr_count
    );
    return API_SUCCESS;
    #else /* MS_PROXY_CLIENT */
    MS_IGNORE_UNUSED_PARAM(argc);
    MS_IGNORE_UNUSED_PARAM(argv);
    CONSOLE_OUT(
        "Proxy Client Feature Currently Disabled!!!\n");
    return API_FAILURE;
    #endif /* MS_PROXY_CLIENT */
}

#endif /* MS_PROXY_SUPPORT */

#endif /* CLI_PROXY */
