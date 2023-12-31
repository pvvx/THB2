
/**
    \file cli_main.c

    This File contains the "main" function for the CLI application,
    to exercise various functionalities of the Mindtree Mesh stack.
*/

/*
    Copyright (C) 2017. Mindtree Ltd.
    All rights reserved.
*/

/* ------------------------------- Header File Inclusion */
#include "cli_main.h"

#ifdef CLI_PROVISION

/* ------------------------------- Global Variables */
/* Level - Core - Provision */
DECL_CONST CLI_COMMAND cli_core_provision_cmd_list[] =
{
    /* Help */
    { "help", "\tHelp", cli_help },

    /* Help */
    { "uuid", "\tUpdate 1st octet of UUID", cli_provision_uuid },

    /* Provision Setup */
    { "setup", "\tSetup", cli_core_provision_setup },

    /* Provision Bind */
    { "bind", "\tBind", cli_core_provision_bind },

    /* Provision Input Auth Val */
    { "auth_val", "\tInput Authentication Value", cli_core_provision_input_auth_val},

    /* Provision Set Authentication Action */
    { "auth_act", "\tSet Authentication Action", cli_core_provision_set_auth_action},

    /* Provision Set Dev Public Key */
    { "dev_pkey", "\tSet Device P256 ECDH Public Key", cli_core_provision_set_dev_pkey},

    /* Provision Get Local Public Key */
    { "get_pkey", "\tGet Local P256 ECDH Public Key", cli_core_provision_get_pkey},

    /* Get List of Provisioned Devices */
    { "dev_list", "\tGet List of Provisioned Devices", cli_core_provision_get_dev_list},

    /* Remove all Device Keys */
    { "rm_devkeys", "Delete All Device Keys", cli_core_provision_delete_all_dev_keys},

    /* Back */
    { "back", "\tOne Level Up", cli_back },

    /* Root */
    { "root", "\tBack to Root", cli_root }
};

/* ------------------------------- Functions */
/* Provision */
API_RESULT cli_core_provision(UINT32 argc, UCHAR* argv[])
{
    CONSOLE_OUT("In Core Provision\n");
    appl_prov_register();
    cli_cmd_stack_push((CLI_COMMAND*)cli_core_provision_cmd_list, sizeof(cli_core_provision_cmd_list) / sizeof(CLI_COMMAND));
    cli_help(argc, argv);
    return API_SUCCESS;
}

/* CLI Command Handlers. Level - Core - Provision */
API_RESULT cli_provision_uuid(UINT32 argc, UCHAR* argv[])
{
    UCHAR octet;
    CONSOLE_OUT("In Core Provision UUID\n");

    if (1 != argc)
    {
        CONSOLE_OUT("Usage: uuid <octet>\n");
        return API_FAILURE;
    }

    octet = (UCHAR)CLI_strtoi(argv[0], (UINT8)CLI_strlen(argv[0]), 16);
    appl_prov_update_uuid(octet);
    return API_SUCCESS;
}

/* Provision Setup */
API_RESULT cli_core_provision_setup(UINT32 argc, UCHAR* argv[])
{
    UCHAR role, brr;
    CONSOLE_OUT("In Core Provision Setup\n");

    if (2 != argc)
    {
        CONSOLE_OUT("Usage: setup <role:[1 - Device, 2 - Provisioner]> <bearer:[1 - Adv, 2 - GATT]>\n");
        return API_FAILURE;
    }

    role = (UCHAR)CLI_strtoi(argv[0], (UINT8)CLI_strlen(argv[0]), 16);
    brr  = (UCHAR)CLI_strtoi(argv[1], (UINT8)CLI_strlen(argv[1]), 16);
    appl_prov_setup(role, brr);
    return API_SUCCESS;
}

/* Provision Bind */
API_RESULT cli_core_provision_bind(UINT32 argc, UCHAR* argv[])
{
    UCHAR brr, indx;
    CONSOLE_OUT("In Core Provision Bind\n");

    if (2 != argc)
    {
        CONSOLE_OUT("Usage: bind <bearer:[1 - Adv, 2 - GATT]> <index>\n");
        return API_FAILURE;
    }

    brr  = (UCHAR)CLI_strtoi(argv[0], (UINT8)CLI_strlen(argv[0]), 16);
    indx = (UCHAR)CLI_strtoi(argv[1], (UINT8)CLI_strlen(argv[1]), 16);
    appl_prov_bind(brr, indx);
    return API_SUCCESS;
}

/* Provision Input Auth Val */
API_RESULT cli_core_provision_input_auth_val(UINT32 argc, UCHAR* argv[])
{
    UCHAR  mode;
    UINT32 auth_val_num;

    if (2 != argc)
    {
        CONSOLE_OUT("Usage: auth_val <mode:[0 - Numeric, 1 - Alphanumeric]> <Auth-Value>\n");
        return API_FAILURE;
    }

    /**
        MODE = 0 -> Numeric
        MODE = 1 -> Alphanumeric
    */
    /* Copy "Mode" from the CLI stream */
    mode = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);

    if (0x00 != mode)
    {
        appl_prov_input_auth_val(mode, argv[1], CLI_strlen(argv[1]));
    }
    else
    {
        /* Copy the Numeric Value */
        auth_val_num = CLI_strtoi(argv[1], CLI_strlen(argv[1]), 10);
        appl_prov_input_auth_val(mode, &auth_val_num, sizeof(auth_val_num));
    }

    return API_SUCCESS;
}

/* Provision Set Authentication Action */
API_RESULT cli_core_provision_set_auth_action(UINT32 argc, UCHAR* argv[])
{
    UCHAR mode, oob_act, oob_sz;
    UCHAR s_oob_val[16];

    if ((3 > argc) || (4 < argc))
    {
        CONSOLE_OUT("Usage: auth_act <mode:[0-3]> <OOB Action:[0-3]> <OOB Size:[1-8]> <Static OOB{Optional}:[16]>\n");
        return API_FAILURE;
    }

    /**
        Valid Values of Mode for OOB usage are:
        0x00 - None
        0x01 - Static
        0x02 - Output
        0x03 - Input
    */
    /* Copy "Mode" from the CLI stream */
    mode    = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
    /* Copy "OOB Action" from the CLI stream */
    oob_act = CLI_strtoi(argv[1], CLI_strlen(argv[1]), 16);
    /* Copy "OOB size" from the CLI stream */
    oob_sz  = CLI_strtoi(argv[2], CLI_strlen(argv[2]), 16);
    EM_mem_set(s_oob_val, 0x0, sizeof(s_oob_val));

    if (4 == argc)
    {
        CLI_strtoarray
        (
            argv[3],
            CLI_strlen(argv[3]),
            &s_oob_val[0],
            sizeof(s_oob_val)
        );
    }

    appl_prov_set_auth_action
    (
        mode,
        s_oob_val,
        oob_act,
        oob_sz
    );
    return API_SUCCESS;
}

/* Provision Set Dev Public Key */
API_RESULT cli_core_provision_set_dev_pkey(UINT32 argc, UCHAR* argv[])
{
    /**
        TODO: Extended the CLI to have values for generic Peer Device
             Currently, this CLI command is making use of PTS provided
             PublicKeys by default.
    */
    appl_prov_set_dev_public_key();
    return API_SUCCESS;
}

/* Provision Get Local Public Key */
API_RESULT cli_core_provision_get_pkey(UINT32 argc, UCHAR* argv[])
{
    appl_prov_get_local_public_key();
    return API_SUCCESS;
}

/* Get List of Provisioned Devices */
API_RESULT cli_core_provision_get_dev_list(UINT32 argc, UCHAR* argv[])
{
    API_RESULT        retval;
    UINT32            index;
    MS_PROV_DEV_ENTRY cli_prov_dev_list[MS_MAX_DEV_KEYS];
    UINT16            cli_req_entries;
    UINT16            cli_pointer_entries;
    cli_req_entries = MS_MAX_DEV_KEYS;
    retval = MS_access_cm_get_prov_devices_list
             (
                 &cli_prov_dev_list[0],
                 &cli_req_entries,
                 &cli_pointer_entries
             );

    if (API_SUCCESS == retval)
    {
        CONSOLE_OUT("\r\n=============================================\r\n");
        CONSOLE_OUT("         PROVISONED DEVICE LIST:-\r\n");

        for (index = 0; index < cli_req_entries; index++)
        {
            CONSOLE_OUT(
                " %02d Device : Primary Element Addr 0x%04X [%d elements]\r\n",
                (index + 1),
                cli_prov_dev_list[index].uaddr,
                cli_prov_dev_list[index].num_elements);
        }

        CONSOLE_OUT("=============================================\r\n");
    }

    return retval;
}

/* Delete all Device Keys */
API_RESULT cli_core_provision_delete_all_dev_keys(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    retval = MS_access_cm_remove_all_device_keys();
    CONSOLE_OUT("\n Deleting ALL Device Keys, retval 0x%04X\n", retval);
    return retval;
}

#endif /* CLI_PROVISION */
