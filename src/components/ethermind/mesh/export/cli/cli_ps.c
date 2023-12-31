
/**
    \file cli_ps.c

    This File contains the "Persistent Storage" handlers for the CLI application,
    to exercise various functionalities of the Mindtree Mesh stack.
*/

/*
    Copyright (C) 2017. Mindtree Ltd.
    All rights reserved.
*/

/* ------------------------------- Header File Inclusion */
#include "cli_main.h"

/* ------------------------------- Global Variables */
/* Level - Persistent Storage */
DECL_STATIC CLI_COMMAND cli_ps_cmd_list[] =
{
    /* Help */
    { "help", "Help", cli_help },

    /* Get Device Key */
    { "getdevkey", "Get Device Key", cli_ps_get_device_key },

    /* Get App Key */
    { "getappkey", "Get App Key", cli_ps_get_app_key },

    /* Get Network Key */
    { "getnetkey", "Get Network Key", cli_ps_get_net_key },

    /* Get Primary Unicast Address */
    { "getprimunicastaddr", "Get Primary Unicast Address", cli_ps_get_primary_unicast_addr },

    /* Back */
    { "back", "One Level Up", cli_back },

    /* Root */
    { "root", "Back to Root", cli_root }

};

/* ------------------------------- Functions */
/* Persistent Storage */
API_RESULT cli_ps(UINT32 argc, UCHAR* argv[])
{
    CONSOLE_OUT("In Persistent Storage\n");
    cli_cmd_stack_push(cli_ps_cmd_list, sizeof(cli_ps_cmd_list) / sizeof(CLI_COMMAND));
    cli_help(argc, argv);
    return API_SUCCESS;
}

/* Get Device Key */
API_RESULT cli_ps_get_device_key(UINT32 argc, UCHAR* argv[])
{
    UINT8   index;
    UINT8* key;
    API_RESULT retval;
    CONSOLE_OUT("In PS Get Device Key\n");

    if (1 != argc)
    {
        CONSOLE_OUT("Usage: getdevkey <Device Key Index>\n");
        return API_FAILURE;
    }

    index = (UINT8)CLI_strtoi(argv[0], (UINT8)CLI_strlen(argv[0]), 16);
    retval = MS_access_cm_get_device_key
             (
                 index,
                 &key
             );

    /* Check Retval. Print Device Key */
    if (API_SUCCESS == retval)
    {
        CONSOLE_OUT("Device Key[0x%02X]: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\n",
                    index, key[0], key[1], key[2], key[3], key[4], key[5], key[6], key[7],
                    key[8], key[9], key[10], key[11], key[12], key[13], key[14], key[15]);
    }
    else
    {
        CONSOLE_OUT("FAILED. Reason: 0x%04X\n", retval);
    }

    return API_SUCCESS;
}

/* Get App Key */
API_RESULT cli_ps_get_app_key(UINT32 argc, UCHAR* argv[])
{
    MS_APPKEY_HANDLE  handle;
    UINT8* key;
    UINT8   aid;
    API_RESULT retval;
    CONSOLE_OUT("In PS Get App Key\n");

    if (1 != argc)
    {
        CONSOLE_OUT("Usage: getappkey <App Key Handle>\n");
        return API_FAILURE;
    }

    handle = (MS_APPKEY_HANDLE)CLI_strtoi(argv[0], (UINT16)CLI_strlen(argv[0]), 16);
    retval = MS_access_cm_get_app_key
             (
                 handle,
                 &key,
                 &aid
             );

    /* Check Retval. Print App Key */
    if (API_SUCCESS == retval)
    {
        CONSOLE_OUT("App Key[0x%02X]: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\n",
                    handle, key[0], key[1], key[2], key[3], key[4], key[5], key[6], key[7],
                    key[8], key[9], key[10], key[11], key[12], key[13], key[14], key[15]);
    }
    else
    {
        CONSOLE_OUT("FAILED. Reason: 0x%04X\n", retval);
    }

    return API_SUCCESS;
}

/* Get Network Key */
API_RESULT cli_ps_get_net_key(UINT32 argc, UCHAR* argv[])
{
    MS_SUBNET_HANDLE  handle;
    UINT8   key[16];
    API_RESULT retval;
    CONSOLE_OUT("In PS Get Network Key\n");

    if (1 != argc)
    {
        CONSOLE_OUT("Usage: getnetkey <Subnet Handle>\n");
        return API_FAILURE;
    }

    handle = (MS_SUBNET_HANDLE)CLI_strtoi(argv[0], (UINT16)CLI_strlen(argv[0]), 16);
    retval = MS_access_cm_get_netkey_at_offset
             (
                 handle,
                 0,
                 key
             );

    /* Check Retval. Print Network Key */
    if (API_SUCCESS == retval)
    {
        CONSOLE_OUT("Net Key[0x%02X]: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\n",
                    handle, key[0], key[1], key[2], key[3], key[4], key[5], key[6], key[7],
                    key[8], key[9], key[10], key[11], key[12], key[13], key[14], key[15]);
    }
    else
    {
        CONSOLE_OUT("FAILED. Reason: 0x%04X\n", retval);
    }

    return API_SUCCESS;
}

/* Get Primary Unicast Address */
API_RESULT cli_ps_get_primary_unicast_addr(UINT32 argc, UCHAR* argv[])
{
    MS_NET_ADDR    addr;
    API_RESULT retval;
    CONSOLE_OUT("In PS Get Primary Unicast Address\n");
    retval = MS_access_cm_get_primary_unicast_address(&addr);

    /* Check Retval. Print Primary Unicast Address */
    if (API_SUCCESS == retval)
    {
        CONSOLE_OUT("Primary Unicast Address: 0x%04X\n", addr);
    }
    else
    {
        CONSOLE_OUT("FAILED. Reason: 0x%04X\n", retval);
    }

    return API_SUCCESS;
}

