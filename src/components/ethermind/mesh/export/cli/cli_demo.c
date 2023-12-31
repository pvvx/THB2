
/**
    \file cli_demo.c

    This File contains the "main" function for the Demo application.
*/

/*
    Copyright (C) 2017. Mindtree Ltd.
    All rights reserved.
*/

#ifdef DEMO

/* ------------------------------- Header File Inclusion */
#include "MS_access_api.h"
#include "MS_config_api.h"

#include "MS_generic_onoff_api.h"

#include "cliface.h"
#include "blebrr.h"


/* ------------------------------- Global Variables */
/* Console Input/Output */
#define CONSOLE_OUT(...)    printf(__VA_ARGS__)
#define CONSOLE_IN(...)     scanf(__VA_ARGS__)

/* Macro to declare a CLI Command Handler */
#define CLI_CMD_HANDLER_DECL(x) \
    API_RESULT (x) (UINT32 arc, UCHAR * arv[])

/* Macro to define an empty CLI Command Handler */
#define CLI_CMD_HANDLER_EMPTY_DEF(x) \
    API_RESULT (x) (UINT32 argc, UCHAR * argv[]) \
    { \
        UINT32 index; \
        \
        CLI_APP_TRC(#x ". Argc %d\n", argc); \
        \
        for (index = 0; index < argc; index++) \
        { \
            CLI_APP_TRC("Argv[%d]: %s\n", index, argv[index]); \
        } \
        \
        return API_SUCCESS; \
    }

/* CLI Command Handler declaration for configuration client */
CLI_CMD_HANDLER_DECL( cli_help );

CLI_CMD_HANDLER_DECL( cli_provisioning_mode_adv_set );
CLI_CMD_HANDLER_DECL( cli_provisioning_mode_gatt_set );

CLI_CMD_HANDLER_DECL( cli_proxy_mode_set );

CLI_CMD_HANDLER_DECL( cli_provisioner_mode_set );
CLI_CMD_HANDLER_DECL( cli_start_provisioning );

CLI_CMD_HANDLER_DECL( cli_clear_persistent );
CLI_CMD_HANDLER_DECL( cli_switchon );
CLI_CMD_HANDLER_DECL( cli_switchoff );

/* CLI Server command table */
DECL_STATIC CLI_COMMAND cli_server_cmd_list[] =
{
    /* Help */
    { "help", "Help Menu", cli_help },

    /* Set in provisioning mode */
    { "provadv", "Device for Provisioning over ADV", cli_provisioning_mode_adv_set },
    { "provgatt", "Device for Provisioning over GATT", cli_provisioning_mode_gatt_set },

    /* Switch to Proxy */
    { "proxy", "Device for Proxy", cli_proxy_mode_set},

    #ifdef DEMO_CLIENT
    { "on", "Switch ON Light", cli_switchon},
    { "off", "Switch OFF Light", cli_switchoff},
    #endif /* DEMO_CLIENT */

    /* Clear the Mesh Database from persistent store */
    { "reset", "Clear Persistent storage", cli_clear_persistent }
};

/* Debug Macros */
/* TBD: Mapped with debug sub-system */
#define CLI_APP_ERR(...)    printf(__VA_ARGS__)
#define CLI_APP_TRC(...)    printf(__VA_ARGS__)
#define CLI_APP_INF(...)    printf(__VA_ARGS__)

/* TODO: Remove */
/* Current Test Case ID */
UINT32 appl_mesh_test_id;

#ifdef DEMO_CLIENT
static MS_ACCESS_MODEL_HANDLE   appl_generic_onoff_client_model_handle;

API_RESULT appl_generic_onoff_client_cb
(
    /* IN */ MS_ACCESS_MODEL_HANDLE* handle,
    /* IN */ UINT32                   opcode,
    /* IN */ UCHAR*                   data_param,
    /* IN */ UINT16                   data_len
);
#endif /* DEMO_CLIENT */

/* ------------------------------- Functions */
void cli_input (char* cmd, int size)
{
    CLI_process_line
    (
        cmd,
        size,
        cli_server_cmd_list,
        (sizeof(cli_server_cmd_list) / sizeof(CLI_COMMAND))
    );
}

/* CLI Command Handler Defines */
API_RESULT cli_help(UINT32 argc, UCHAR* argv[])
{
    UINT32 index;

    /* Print all the available commands */
    for (index = 0; index < (sizeof(cli_server_cmd_list) / sizeof(CLI_COMMAND)); index++)
    {
        CONSOLE_OUT("    %s\n", cli_server_cmd_list[index].cmd);
    }

    return API_SUCCESS;
}

API_RESULT cli_demo_init(void)
{
    MS_ACCESS_NODE_ID node_id;
    MS_ACCESS_ELEMENT_HANDLE element_handle;
    API_RESULT retval;
    /* Create Node */
    retval = MS_access_create_node(&node_id);

    if (API_SUCCESS == retval)
    {
        /* Register the Models */
        main_register_models();
        #ifndef DEMO_CLIENT
        /* Initialize Generic ON/OFF Server */
        main_generic_onoff_server_operations(MS_FALSE);
        #else /* DEMO_CLIENT */
        cli_modelc_generic_onoff_setup(0, NULL);
        #endif /* DEMO_CLIENT */
    }

    CONSOLE_OUT("Model Registration Status: 0x%04X\n", retval);
    return API_SUCCESS;
}

API_RESULT cli_provisioning_mode_adv_set(UINT32 argc, UCHAR* argv[])
{
    appl_prov_register();
    appl_prov_setup(PROV_ROLE_DEVICE, PROV_BRR_ADV);
    return API_SUCCESS;
}

API_RESULT cli_provisioning_mode_gatt_set(UINT32 argc, UCHAR* argv[])
{
    appl_prov_register();
    /* Set the role to Prov with bearer */
    blebrr_gatt_mode_set(BLEBRR_GATT_PROV_MODE);
    appl_prov_setup(PROV_ROLE_DEVICE, PROV_BRR_GATT);
    return API_SUCCESS;
}

API_RESULT cli_proxy_mode_set(UINT32 argc, UCHAR* argv[])
{
    appl_proxy_register();
    appl_proxy_start_net_id_adv();
    return API_SUCCESS;
}

void appl_prov_register(void);
void appl_prov_setup_provisioner(void);
API_RESULT cli_provisioner_mode_set(UINT32 argc, UCHAR* argv[])
{
    appl_prov_register();
    appl_prov_setup_provisioner();
    return API_SUCCESS;
}

void appl_prov_start_provisioning(void);
API_RESULT cli_start_provisioning(UINT32 argc, UCHAR* argv[])
{
    appl_prov_start_provisioning();
    return API_SUCCESS;
}

API_RESULT cli_config_gatt_proxy_get(UINT32 argc, UCHAR* argv[])
{
    return MS_config_client_gatt_proxy_get();
}

API_RESULT cli_config_gatt_proxy_set(UINT32 argc, UCHAR* argv[])
{
    ACCESS_CONFIG_GATT_PROXY_SET_PARAM proxy;
    proxy.proxy = 0x01;
    return MS_config_client_gatt_proxy_set(&proxy);
}

API_RESULT cli_clear_persistent(UINT32 argc, UCHAR* argv[])
{
    nvs_reset();
    printf ("Persistent database Reset Success!");
    /* appl_set_prov_state(false); */
    return API_SUCCESS;
}


API_RESULT cli_switchon(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    MS_GENERIC_ONOFF_SET_STRUCT  param;
    param.onoff= 0x01;
    param.tid= 0;
    param.optional_fields_present = 0x00;
    printf ("Switch ON!");
    retval = MS_generic_onoff_set_unacknowledged(&param);
    printf ("Retval = 0x%04X\n", retval);
    return API_SUCCESS;
}

API_RESULT cli_switchoff(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    MS_GENERIC_ONOFF_SET_STRUCT  param;
    param.onoff= 0x00;
    param.tid= 0;
    param.optional_fields_present = 0x00;
    printf ("Switch OFF!");
    retval = MS_generic_onoff_set_unacknowledged(&param);
    printf ("Retval = 0x%04X\n", retval);
    return API_SUCCESS;
}

void appl_indicate_provisioning_state (void)
{
    MS_NET_ADDR addr;
    MS_access_cm_get_primary_unicast_address (&addr);

    if (0x0000 == addr)
    {
        printf ("Device Not Provisioned!");
        /* Put device in provisioning over GATT */
        cli_provisioning_mode_gatt_set(0, NULL);
    }
    else
    {
        printf ("Device Provisioned!");
        /* Put device in Proxy over GATT */
        /* cli_proxy_mode_set(0, NULL); */
    }
}
#endif /* DEMO */
