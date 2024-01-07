/**
    \file cli_config_client.c

    \brief This file defines the Mesh Configuration Model Application Interface
    - includes Data Structures and Methods for both Server and Client.
*/

/*
    Copyright (C) 2018. Mindtree Ltd.
    All rights reserved.
*/



/* --------------------------------------------- Header File Inclusion */
#include "cli_config_client.h"

#ifdef CLI_CONFIG_CLIENT_MODEL

/* --------------------------------------------- Global Definitions */


/* --------------------------------------------- Static Global Variables */
DECL_CONST CLI_COMMAND cli_modelc_configuration_cmd_list[] =
{
    /* Help */
    { "help", "Help", cli_help },

    /* Back */
    { "back", "One Level Up", cli_back },

    /* Root */
    { "root", "Back to Root", cli_root },

    /* Setup */
    { "setup", "Model Configuration Setup", cli_modelc_configuration_setup },

    /* Get Model Handle */
    { "gethandle", "Get Model Handle", cli_modelc_configuration_get_model_handle },

    /* Set Publish Address */
    { "publishaddr", "Set Publish Address  <Peer Config Server Address (HEX 16-bit)>", cli_modelc_configuration_set_publish_address },

    /* Send Config Beacon Set */
    { "beaconset", "Send Config Beacon Set", cli_modelc_config_beacon_set},

    /* Send Config Beacon Get */
    { "beaconget", "Send Config Beacon Get", cli_modelc_config_beacon_get},

    /* Send Config Composition Data Get */
    { "compositiondataget", "Send Config Composition Data Get", cli_modelc_config_composition_data_get},

    /* Send Config Default Ttl Set */
    { "defaultttlset", "Send Config Default Ttl Set", cli_modelc_config_default_ttl_set},

    /* Send Config Default Ttl Get */
    { "defaultttlget", "Send Config Default Ttl Get", cli_modelc_config_default_ttl_get},

    /* Send Config Gatt Proxy Set */
    { "proxyset", "Send Config Gatt Proxy Set", cli_modelc_config_gatt_proxy_set},

    /* Send Config Gatt Proxy Get */
    { "proxyget", "Send Config Gatt Proxy Get", cli_modelc_config_gatt_proxy_get},

    /* Send Config Friend Set */
    { "friendset", "Send Config Friend Set", cli_modelc_config_friend_set},

    /* Send Config Friend Get */
    { "friendget", "Send Config Friend Get", cli_modelc_config_friend_get},

    /* Send Config Relay Set */
    { "relayset", "Send Config Relay Set", cli_modelc_config_relay_set},

    /* Send Config Relay Get */
    { "relayget", "Send Config Relay Get", cli_modelc_config_relay_get},

    /* Send Config Model Publication Set */
    { "modelpublicationset", "Send Config Model Publication Set", cli_modelc_config_model_publication_set},

    /* Send Config Model Publication Virtual Address Set */
    { "modelpublicationvirtualaddressset", "Send Config Model Publication Virtual Address Set", cli_modelc_config_model_publication_virtual_address_set},

    /* Send Config Model Publication Get */
    { "modelpublicationget", "Send Config Model Publication Get", cli_modelc_config_model_publication_get},

    /* Send Config Model Subscription Add */
    { "modelsubscriptionadd", "Send Config Model Subscription Add", cli_modelc_config_model_subscription_add},

    /* Send Config Model Subscription Virtual Address Add */
    { "modelsubscriptionvirtualaddressadd", "Send Config Model Subscription Virtual Address Add", cli_modelc_config_model_subscription_virtual_address_add},

    /* Send Config Model Subscription Overwrite */
    { "modelsubscriptionoverwrite", "Send Config Model Subscription Overwrite", cli_modelc_config_model_subscription_overwrite},

    /* Send Config Model Subscription Virtual Address Overwrite */
    { "modelsubscriptionvirtualaddressoverwrite", "Send Config Model Subscription Virtual Address Overwrite", cli_modelc_config_model_subscription_virtual_address_overwrite},

    /* Send Config Model Subscription Delete */
    { "modelsubscriptiondelete", "Send Config Model Subscription Delete", cli_modelc_config_model_subscription_delete},

    /* Send Config Model Subscription Virtual Address Delete */
    { "modelsubscriptionvirtualaddressdelete", "Send Config Model Subscription Virtual Address Delete", cli_modelc_config_model_subscription_virtual_address_delete},

    /* Send Config Model Subscription Delete All */
    { "modelsubscriptiondeleteall", "Send Config Model Subscription Delete All", cli_modelc_config_model_subscription_delete_all},

    /* Send Config Sig Model Subscription Get */
    { "sigmodelsubscriptionget", "Send Config Sig Model Subscription Get", cli_modelc_config_sig_model_subscription_get},

    /* Send Config Vendor Model Subscription Get */
    { "vendormodelsubscriptionget", "Send Config Vendor Model Subscription Get", cli_modelc_config_vendor_model_subscription_get},

    /* Send Config Netkey Add */
    { "netkeyadd", "Send Config Netkey Add", cli_modelc_config_netkey_add},

    /* Send Config Netkey Update */
    { "netkeyupdate", "Send Config Netkey Update", cli_modelc_config_netkey_update},

    /* Send Config Netkey Delete */
    { "netkeydelete", "Send Config Netkey Delete", cli_modelc_config_netkey_delete},

    /* Send Config Netkey Get */
    { "netkeyget", "Send Config Netkey Get", cli_modelc_config_netkey_get},

    /* Send Config Appkey Add */
    { "appkeyadd", "Send Config Appkey Add", cli_modelc_config_appkey_add},

    /* Send Config Appkey Update */
    { "appkeyupdate", "Send Config Appkey Update", cli_modelc_config_appkey_update},

    /* Send Config Appkey Delete */
    { "appkeydelete", "Send Config Appkey Delete", cli_modelc_config_appkey_delete},

    /* Send Config Appkey Get */
    { "appkeyget", "Send Config Appkey Get", cli_modelc_config_appkey_get},

    /* Send Config Model App Bind */
    { "bind", "Send Config Model App Bind", cli_modelc_config_model_app_bind},

    /* Send Config Model App Unbind */
    { "unbind", "Send Config Model App Unbind", cli_modelc_config_model_app_unbind},

    /* Send Config Sig Model App Get */
    { "sigmodelappget", "Send Config Sig Model App Get", cli_modelc_config_sig_model_app_get},

    /* Send Config Vendor Model App Get */
    { "vendormodelappget", "Send Config Vendor Model App Get", cli_modelc_config_vendor_model_app_get},

    /* Send Config Node Identity Set */
    { "nodeidentityset", "Send Config Node Identity Set", cli_modelc_config_node_identity_set},

    /* Send Config Node Identity Get */
    { "nodeidentityget", "Send Config Node Identity Get", cli_modelc_config_node_identity_get},

    /* Send Config Node Reset */
    { "nodereset", "Send Config Node Reset", cli_modelc_config_node_reset},

    /* Send Config Heartbeat Publication Set */
    { "heartbeatpublicationset", "Send Config Heartbeat Publication Set", cli_modelc_config_heartbeat_publication_set},

    /* Send Config Heartbeat Publication Get */
    { "heartbeatpublicationget", "Send Config Heartbeat Publication Get", cli_modelc_config_heartbeat_publication_get},

    /* Send Config Heartbeat Subscription Set */
    { "heartbeatsubscriptionset", "Send Config Heartbeat Subscription Set", cli_modelc_config_heartbeat_subscription_set},

    /* Send Config Heartbeat Subscription Get */
    { "heartbeatsubscriptionget", "Send Config Heartbeat Subscription Get", cli_modelc_config_heartbeat_subscription_get},

    /* Send Config Network Transmit Set */
    { "networktransmitset", "Send Config Network Transmit Set", cli_modelc_config_network_transmit_set},

    /* Send Config Network Transmit Get */
    { "networktransmitget", "Send Config Network Transmit Get", cli_modelc_config_network_transmit_get},

    /* Send Config Low Power Node Polltimeout Get */
    { "lpnpolltimeoutget", "Send Config Low Power Node Polltimeout Get", cli_modelc_config_low_power_node_polltimeout_get},

    /* Send Config Key Refresh Phase Set */
    { "keyrefreshphaseset", "Send Config Key Refresh Phase Set", cli_modelc_config_key_refresh_phase_set},

    /* Send Config Key Refresh Phase Get */
    { "keyrefreshphaseget", "Send Config Key Refresh Phase Get", cli_modelc_config_key_refresh_phase_get},

};



/* --------------------------------------------- External Global Variables */
static MS_ACCESS_MODEL_HANDLE   appl_configuration_client_model_handle;


/* --------------------------------------------- Function */
API_RESULT cli_modelc_config(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    CONSOLE_OUT("In Model Client - Configuration\n");
    cli_cmd_stack_push((CLI_COMMAND*)cli_modelc_configuration_cmd_list, sizeof(cli_modelc_configuration_cmd_list) / sizeof(CLI_COMMAND));
    retval = cli_help(argc, argv);
    return retval;
}

/* configuration client CLI entry point */
API_RESULT cli_modelc_configuration_setup(UINT32 argc, UCHAR* argv[])
{
    int choice;
    MS_ACCESS_ELEMENT_HANDLE element_handle;
    API_RESULT retval;
    static UCHAR model_initialized = 0x00;
    /**
        Register with Access Layer.
    */
    retval = API_FAILURE;

    if (0x00 == model_initialized)
    {
        /* Use Default Element Handle. Index 0 */
        element_handle = MS_ACCESS_DEFAULT_ELEMENT_HANDLE;
        retval = MS_config_client_init
                 (
                     element_handle,
                     &appl_configuration_client_model_handle,
                     appl_configuration_client_cb
                 );

        if (API_SUCCESS == retval)
        {
            CONSOLE_OUT(
                "Configuration Client Initialized. Model Handle: 0x%04X\n",
                appl_configuration_client_model_handle);
        }
        else
        {
            CONSOLE_OUT(
                "[ERR] Configuration Client Initialization Failed. Result: 0x%04X\n",
                retval);
        }

        model_initialized = 0x01;
    }

    return retval;
}

/* Send Config Model App Unbind */
API_RESULT cli_modelc_config_model_app_unbind(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    ACCESS_CONFIG_MODEL_APP_UNBIND_PARAM  param;
    CONSOLE_OUT
    (">> Send Config Model App Unbind\n");

    if (4 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.element_address = (UINT16)choice;
        CONSOLE_OUT("ElementAddress (16-bit in HEX): 0x%04X\n", param.element_address);
        choice = CLI_strtoi(argv[1], CLI_strlen(argv[1]), 16);
        param.appkey_index = (UINT16)choice;
        CONSOLE_OUT("AppKeyIndex (16-bit in HEX): 0x%04X\n", param.appkey_index);

        if (4 < CLI_strlen(argv[2]))
        {
            CONSOLE_OUT("Model ID Type: Vendor\n");
            param.model.type = 0x01;
        }
        else
        {
            CONSOLE_OUT("Model ID Type: SIG\n");
            param.model.type = 0x00;
        }

        choice = CLI_strtoi(argv[2], CLI_strlen(argv[2]), 16);
        param.model.id = (UINT32)choice;
        CONSOLE_OUT("ModelIdentifier: 0x%08X\n", param.model.id);

        /* Corresponding Local Model ID */
        if (4 < CLI_strlen(argv[3]))
        {
            param.client_model.type = 0x01;
            CONSOLE_OUT("Model ID Type: Vendor\n");
        }
        else
        {
            param.client_model.type = 0x00;
            CONSOLE_OUT("Model ID Type: SIG\n");
        }

        choice = CLI_strtoi(argv[3], CLI_strlen(argv[3]), 16);
        param.client_model.id = (UINT32)choice;
        CONSOLE_OUT("Local ModelIdentifier: 0x%08X\n", param.client_model.id);
    }
    else
    {
        CONSOLE_OUT("Invalid Number of Arguments:0x%04X. Returning.\n", argc);
        CONSOLE_OUT("Usage: unbind <ElementAddress> <AppKeyIndex> <Model ID to bind> <Correponding Local Model ID>\n");
        return API_FAILURE;
    }

    retval = MS_config_client_model_app_unbind(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Config Friend Get */
API_RESULT cli_modelc_config_friend_get(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    CONSOLE_OUT
    (">> Send Config Friend Get\n");
    retval = MS_config_client_friend_get();
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Config Gatt Proxy Get */
API_RESULT cli_modelc_config_gatt_proxy_get(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    CONSOLE_OUT
    (">> Send Config Gatt Proxy Get\n");
    retval = MS_config_client_gatt_proxy_get();
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Config Model Subscription Virtual Address Overwrite */
API_RESULT cli_modelc_config_model_subscription_virtual_address_overwrite(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    ACCESS_CONFIG_MODELSUB_VADDR_OVERWRITE_PARAM  param;
    CONSOLE_OUT
    (">> Send Config Model Subscription Virtual Address Overwrite\n");

    if (3 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.element_address = (UINT16)choice;
        CONSOLE_OUT("ElementAddress (16-bit in HEX): 0x%04X\n", param.element_address);
        CLI_strtoarray
        (
            argv[1],
            CLI_strlen(argv[1]),
            &param.label[0],
            16
        );

        if (4 < CLI_strlen(argv[2]))
        {
            param.model.type = 0x01;
            CONSOLE_OUT("Model ID Type: Vendor\n");
        }
        else
        {
            param.model.type = 0x00;
            CONSOLE_OUT("Model ID Type: SIG\n");
        }

        choice = CLI_strtoi(argv[2], CLI_strlen(argv[2]), 16);
        param.model.id = (UINT32)choice;
        CONSOLE_OUT("ModelIdentifier: 0x%08X\n", param.model.id);
    }

    retval = MS_config_client_model_subscription_vaddr_overwrite(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Config Sig Model App Get */
API_RESULT cli_modelc_config_sig_model_app_get(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    ACCESS_CONFIG_SIG_MODEL_APP_GET_PARAM  param;
    CONSOLE_OUT
    (">> Send Config Sig Model App Get\n");

    if (2 == argc)
    {
        choice = CLI_strtoi(argv[1], CLI_strlen(argv[0]), 16);
        param.element_address = (UINT16)choice;
        CONSOLE_OUT("ElementAddress (16-bit in HEX): 0x%04X\n", param.element_address);
        choice = CLI_strtoi(argv[1], CLI_strlen(argv[1]), 16);
        param.model_id = (UINT16)choice;
        CONSOLE_OUT("ModelIdentifier (16-bit in HEX): 0x%04X\n", param.model_id);
    }

    retval = MS_config_client_sig_model_app_get(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Config Model Subscription Add */
API_RESULT cli_modelc_config_model_subscription_add(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    ACCESS_CONFIG_MODELSUB_ADD_PARAM  param;
    CONSOLE_OUT
    (">> Send Config Model Subscription Add\n");

    if (3 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.element_address = (UINT16)choice;
        CONSOLE_OUT("ElementAddress (16-bit in HEX): 0x%04X\n", param.element_address);
        choice = CLI_strtoi(argv[1], CLI_strlen(argv[1]), 16);
        param.address = (UINT16)choice;
        CONSOLE_OUT("Address (16-bit in HEX): 0x%04X\n", param.address);

        if (4 < CLI_strlen(argv[2]))
        {
            param.model.type = 0x01;
            CONSOLE_OUT("Model ID Type: Vendor\n");
        }
        else
        {
            param.model.type = 0x00;
            CONSOLE_OUT("Model ID Type: SIG\n");
        }

        choice = CLI_strtoi(argv[2], CLI_strlen(argv[2]), 16);
        param.model.id = (UINT32)choice;
        CONSOLE_OUT("ModelIdentifier: 0x%08X\n", param.model.id);
    }

    retval = MS_config_client_model_subscription_add(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Config Network Transmit Set */
API_RESULT cli_modelc_config_network_transmit_set(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    ACCESS_CONFIG_NETWORK_TRANSMIT_SET_PARAM  param;
    CONSOLE_OUT
    (">> Send Config Network Transmit Set\n");

    if (2 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.net_tx_count = (UCHAR)choice;
        CONSOLE_OUT("NetWork Transmit Count (5-bit in HEX): 0x%02X\n", param.net_tx_count);
        choice = CLI_strtoi(argv[1], CLI_strlen(argv[1]), 16);
        param.net_tx_interval_steps = (UCHAR)choice;
        CONSOLE_OUT("NetWork Transmit Interval Steps (3-bit in HEX): 0x%02X\n", param.net_tx_interval_steps);
    }

    retval = MS_config_client_network_transmit_set(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Config Model Subscription Delete */
API_RESULT cli_modelc_config_model_subscription_delete(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    ACCESS_CONFIG_MODELSUB_DEL_PARAM  param;
    CONSOLE_OUT
    (">> Send Config Model Subscription Delete\n");

    if (3 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.element_address = (UINT16)choice;
        CONSOLE_OUT("ElementAddress (16-bit in HEX): 0x%04X\n", param.element_address);
        choice = CLI_strtoi(argv[1], CLI_strlen(argv[1]), 16);
        param.address = (UINT16)choice;
        CONSOLE_OUT("Address (16-bit in HEX): 0x%04X\n", param.address);

        if (4 < CLI_strlen(argv[2]))
        {
            param.model.type = 0x01;
            CONSOLE_OUT("Model ID Type: Vendor\n");
        }
        else
        {
            param.model.type = 0x00;
            CONSOLE_OUT("Model ID Type: SIG\n");
        }

        choice = CLI_strtoi(argv[2], CLI_strlen(argv[2]), 16);
        param.model.id = (UINT32)choice;
        CONSOLE_OUT("ModelIdentifier: 0x%08X\n", param.model.id);
    }

    retval = MS_config_client_model_subscription_delete(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Config Relay Get */
API_RESULT cli_modelc_config_relay_get(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    CONSOLE_OUT
    (">> Send Config Relay Get\n");
    retval = MS_config_client_relay_get();
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Config Heartbeat Subscription Set */
API_RESULT cli_modelc_config_heartbeat_subscription_set(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    ACCESS_CONFIG_HEARTBEATSUB_SET_PARAM  param;
    CONSOLE_OUT
    (">> Send Config Heartbeat Subscription Set\n");

    if (3 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.source = (UINT16)choice;
        CONSOLE_OUT("Source (16-bit in HEX): 0x%04X\n", param.source);
        choice = CLI_strtoi(argv[1], CLI_strlen(argv[1]), 16);
        param.destination = (UINT16)choice;
        CONSOLE_OUT("Destination (16-bit in HEX): 0x%04X\n", param.destination);
        choice = CLI_strtoi(argv[2], CLI_strlen(argv[2]), 16);
        param.periodlog = (UCHAR)choice;
        CONSOLE_OUT("PeriodLog (8-bit in HEX): 0x%02X\n", param.periodlog);
    }

    retval = MS_config_client_heartbeat_subscription_set(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Config Beacon Set */
API_RESULT cli_modelc_config_beacon_set(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    ACCESS_CONFIG_BEACON_SET_PARAM  param;
    CONSOLE_OUT
    (">> Send Config Beacon Set\n");

    if (1 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.beacon = (UCHAR)choice;
        CONSOLE_OUT("Beacon (8-bit in HEX): 0x%02X\n", param.beacon);
    }
    else
    {
        CONSOLE_OUT("Invalid Number of Arguments:0x%04X. Returning.\n", argc);
        return API_FAILURE;
    }

    retval = MS_config_client_beacon_set(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Config Node Reset */
API_RESULT cli_modelc_config_node_reset(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    CONSOLE_OUT
    (">> Send Config Node Reset\n");
    retval = MS_config_client_node_reset();
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Config Relay Set */
API_RESULT cli_modelc_config_relay_set(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    ACCESS_CONFIG_RELAY_SET_PARAM  param;
    CONSOLE_OUT
    (">> Send Config Relay Set\n");

    if (3 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.relay = (UCHAR)choice;
        CONSOLE_OUT("Relay (8-bit in HEX): 0x%02X\n", param.relay);
        choice = CLI_strtoi(argv[1], CLI_strlen(argv[1]), 16);
        param.relay_rtx_count = (UCHAR)choice;
        CONSOLE_OUT("RelayRetransmitCount (3-bit in HEX): 0x%02X\n", param.relay_rtx_count);
        choice = CLI_strtoi(argv[2], CLI_strlen(argv[2]), 16);
        param.relay_rtx_interval_steps = (UCHAR)choice;
        CONSOLE_OUT("RelayRetransmitIntervalSteps (5-bit in HEX): 0x%02X\n", param.relay_rtx_interval_steps);
    }
    else
    {
        CONSOLE_OUT("Invalid Number of Arguments:0x%04X. Returning.\n", argc);
        return API_FAILURE;
    }

    retval = MS_config_client_relay_set(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Config Key Refresh Phase Get */
API_RESULT cli_modelc_config_key_refresh_phase_get(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    ACCESS_CONFIG_KEYREFRESH_PHASE_GET_PARAM  param;
    CONSOLE_OUT
    (">> Send Config Key Refresh Phase Get\n");

    if (1 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.netkey_index = (UINT16)choice;
        CONSOLE_OUT("NetKeyIndex (16-bit in HEX): 0x%04X\n", param.netkey_index);
    }

    retval = MS_config_client_keyrefresh_phase_get(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Config Model Subscription Virtual Address Delete */
API_RESULT cli_modelc_config_model_subscription_virtual_address_delete(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    ACCESS_CONFIG_MODELSUB_VADDR_DEL_PARAM  param;
    CONSOLE_OUT
    (">> Send Config Model Subscription Virtual Address Delete\n");

    if (3 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.element_address = (UINT16)choice;
        CONSOLE_OUT("ElementAddress (16-bit in HEX): 0x%04X\n", param.element_address);
        CLI_strtoarray
        (
            argv[1],
            CLI_strlen(argv[1]),
            &param.label[0],
            16
        );

        if (4 < CLI_strlen(argv[2]))
        {
            param.model.type = 0x01;
            CONSOLE_OUT("Model ID Type: Vendor\n");
        }
        else
        {
            param.model.type = 0x00;
            CONSOLE_OUT("Model ID Type: SIG\n");
        }

        choice = CLI_strtoi(argv[2], CLI_strlen(argv[2]), 16);
        param.model.id = (UINT32)choice;
        CONSOLE_OUT("ModelIdentifier: 0x%08X\n", param.model.id);
    }
    else
    {
        CONSOLE_OUT("Invalid Number of Arguments:0x%04X. Returning.\n", argc);
        return API_FAILURE;
    }

    retval = MS_config_client_model_subscription_vaddr_delete(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Config Model Publication Virtual Address Set */
API_RESULT cli_modelc_config_model_publication_virtual_address_set(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    ACCESS_CONFIG_MODELPUB_VADDR_SET_PARAM  param;
    CONSOLE_OUT
    (">> Send Config Model Publication Virtual Address Set\n");

    if (9 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.element_address = (UINT16)choice;
        CONSOLE_OUT("ElementAddress (16-bit in HEX): 0x%04X\n", param.element_address);
        CLI_strtoarray
        (
            argv[1],
            CLI_strlen(argv[1]),
            &param.publish_address[0],
            16
        );
        choice = CLI_strtoi(argv[2], CLI_strlen(argv[2]), 16);
        param.appkey_index = (UINT16)choice;
        CONSOLE_OUT("AppKeyIndex (12-bit in HEX): 0x%04X\n", param.appkey_index);
        choice = CLI_strtoi(argv[3], CLI_strlen(argv[3]), 16);
        param.credential_flag = (UCHAR)choice;
        CONSOLE_OUT("CredentialFlag (1-bit in HEX): 0x%02X\n", param.credential_flag);
        choice = CLI_strtoi(argv[4], CLI_strlen(argv[4]), 16);
        param.publish_ttl = (UCHAR)choice;
        CONSOLE_OUT("PublishTTL (8-bit in HEX): 0x%02X\n", param.publish_ttl);
        choice = CLI_strtoi(argv[5], CLI_strlen(argv[5]), 16);
        param.publish_period = (UCHAR)choice;
        CONSOLE_OUT("PublishPeriod (8-bit in HEX): 0x%02X\n", param.publish_period);
        choice = CLI_strtoi(argv[6], CLI_strlen(argv[6]), 16);
        param.publish_rtx_count = (UCHAR)choice;
        CONSOLE_OUT("PublishRetransmitCount (3-bit in HEX): 0x%02X\n", param.publish_rtx_count);
        choice = CLI_strtoi(argv[7], CLI_strlen(argv[7]), 16);
        param.publish_rtx_interval_steps = (UCHAR)choice;
        CONSOLE_OUT("PublishRetransmitIntervalSteps (5-bit in HEX): 0x%02X\n", param.publish_rtx_interval_steps);

        if (4 < CLI_strlen(argv[8]))
        {
            param.model.type = 0x01;
            CONSOLE_OUT("Model ID Type: Vendor\n");
        }
        else
        {
            param.model.type = 0x00;
            CONSOLE_OUT("Model ID Type: SIG\n");
        }

        choice = CLI_strtoi(argv[8], CLI_strlen(argv[8]), 16);
        param.model.id = (UINT32)choice;
        CONSOLE_OUT("ModelIdentifier: 0x%08X\n", param.model.id);
    }
    else
    {
        CONSOLE_OUT("Invalid Number of Arguments:0x%04X. Returning.\n", argc);
        return API_FAILURE;
    }

    retval = MS_config_client_model_publication_vaddr_set(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Config Gatt Proxy Set */
API_RESULT cli_modelc_config_gatt_proxy_set(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    ACCESS_CONFIG_GATT_PROXY_SET_PARAM  param;
    CONSOLE_OUT
    (">> Send Config Gatt Proxy Set\n");

    if (1 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.proxy = (UCHAR)choice;
        CONSOLE_OUT("GATTProxy (8-bit in HEX): 0x%02X\n", param.proxy);
    }

    retval = MS_config_client_gatt_proxy_set(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Config Model Subscription Virtual Address Add */
API_RESULT cli_modelc_config_model_subscription_virtual_address_add(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    ACCESS_CONFIG_MODELSUB_VADDR_ADD_PARAM  param;
    CONSOLE_OUT
    (">> Send Config Model Subscription Virtual Address Add\n");

    if (3 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.element_address = (UINT16)choice;
        CONSOLE_OUT("ElementAddress (16-bit in HEX): 0x%04X\n", param.element_address);
        CLI_strtoarray
        (
            argv[1],
            CLI_strlen(argv[1]),
            &param.label[0],
            16
        );

        if (4 < CLI_strlen(argv[2]))
        {
            param.model.type = 0x01;
            CONSOLE_OUT("Model ID Type: Vendor\n");
        }
        else
        {
            param.model.type = 0x00;
            CONSOLE_OUT("Model ID Type: SIG\n");
        }

        choice = CLI_strtoi(argv[2], CLI_strlen(argv[2]), 16);
        param.model.id = (UINT32)choice;
        CONSOLE_OUT("ModelIdentifier: 0x%08X\n", param.model.id);
    }
    else
    {
        CONSOLE_OUT("Invalid Number of Arguments:0x%04X. Returning.\n", argc);
        return API_FAILURE;
    }

    retval = MS_config_client_model_subscription_vaddr_add(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Config Default Ttl Get */
API_RESULT cli_modelc_config_default_ttl_get(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    CONSOLE_OUT
    (">> Send Config Default Ttl Get\n");
    retval = MS_config_client_default_ttl_get();
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Config Model App Bind */
API_RESULT cli_modelc_config_model_app_bind(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    ACCESS_CONFIG_MODEL_APP_BIND_PARAM  param;
    CONSOLE_OUT
    (">> Send Config Model App Bind\n");

    if (4 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.element_address = (UINT16)choice;
        CONSOLE_OUT("ElementAddress (16-bit in HEX): 0x%04X\n", param.element_address);
        choice = CLI_strtoi(argv[1], CLI_strlen(argv[1]), 16);
        param.appkey_index = (UINT16)choice;
        CONSOLE_OUT("AppKeyIndex (16-bit in HEX): 0x%04X\n", param.appkey_index);

        if (4 < CLI_strlen(argv[2]))
        {
            param.model.type = 0x01;
            CONSOLE_OUT("Model ID Type: Vendor\n");
        }
        else
        {
            param.model.type = 0x00;
            CONSOLE_OUT("Model ID Type: SIG\n");
        }

        choice = CLI_strtoi(argv[2], CLI_strlen(argv[2]), 16);
        param.model.id = (UINT32)choice;

        /* Corresponding Local Model ID */
        if (4 < CLI_strlen(argv[3]))
        {
            param.client_model.type = 0x01;
            CONSOLE_OUT("Model ID Type: Vendor\n");
        }
        else
        {
            param.client_model.type = 0x00;
            CONSOLE_OUT("Model ID Type: SIG\n");
        }

        choice = CLI_strtoi(argv[3], CLI_strlen(argv[3]), 16);
        param.client_model.id = (UINT32)choice;
        CONSOLE_OUT("Local ModelIdentifier: 0x%08X\n", param.client_model.id);
    }
    else
    {
        CONSOLE_OUT("Invalid Number of Arguments:0x%04X. Returning.\n", argc);
        CONSOLE_OUT("Usage: bind <ElementAddress> <AppKeyIndex> <Model ID to bind> <Correponding Local Model ID>\n");
        return API_FAILURE;
    }

    retval = MS_config_client_model_app_bind(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Config Node Identity Set */
API_RESULT cli_modelc_config_node_identity_set(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    ACCESS_CONFIG_NODEID_SET_PARAM  param;
    CONSOLE_OUT
    (">> Send Config Node Identity Set\n");

    if (2 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.netkey_index = (UINT16)choice;
        CONSOLE_OUT("NetKeyIndex (16-bit in HEX): 0x%04X\n", param.netkey_index);
        choice = CLI_strtoi(argv[1], CLI_strlen(argv[1]), 16);
        param.identity = (UCHAR)choice;
        CONSOLE_OUT("Identity (8-bit in HEX): 0x%02X\n", param.identity);
    }
    else
    {
        CONSOLE_OUT("Invalid Number of Arguments:0x%04X. Returning.\n", argc);
        return API_FAILURE;
    }

    retval = MS_config_client_node_identity_set(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Config Sig Model Subscription Get */
API_RESULT cli_modelc_config_sig_model_subscription_get(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    ACCESS_CONFIG_SIGMODELSUB_GET_PARAM  param;
    CONSOLE_OUT
    (">> Send Config Sig Model Subscription Get\n");

    if (2 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.element_address = (UINT16)choice;
        CONSOLE_OUT("ElementAddress (16-bit in HEX): 0x%04X\n", param.element_address);
        choice = CLI_strtoi(argv[1], CLI_strlen(argv[1]), 16);
        param.model_id = (UINT16)choice;
        CONSOLE_OUT("ModelIdentifier (16-bit in HEX): 0x%04X\n", param.model_id);
    }
    else
    {
        CONSOLE_OUT("Invalid Number of Arguments:0x%04X. Returning.\n", argc);
        return API_FAILURE;
    }

    retval = MS_config_client_sig_model_subscription_get(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Config Netkey Add */
API_RESULT cli_modelc_config_netkey_add(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    ACCESS_CONFIG_NETKEY_ADD_PARAM  param;
    CONSOLE_OUT
    (">> Send Config Netkey Add\n");

    if (2 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.netkey_index = (UINT16)choice;
        CONSOLE_OUT("NetKeyIndex (16-bit in HEX): 0x%04X\n", param.netkey_index);
        CLI_strtoarray
        (
            argv[1],
            CLI_strlen(argv[1]),
            &param.netkey[0],
            16
        );
    }
    else
    {
        CONSOLE_OUT("Invalid Number of Arguments:0x%04X. Returning.\n", argc);
        return API_FAILURE;
    }

    retval = MS_config_client_netkey_add(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Config Appkey Update */
API_RESULT cli_modelc_config_appkey_update(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    ACCESS_CONFIG_APPKEY_UPDATE_PARAM  param;
    CONSOLE_OUT
    (">> Send Config Appkey Update\n");

    if (3 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.netkey_index = (UINT16)choice;
        CONSOLE_OUT("NetKeyIndex (12-bit in HEX): 0x%04X\n", param.netkey_index);
        choice = CLI_strtoi(argv[1], CLI_strlen(argv[1]), 16);
        param.appkey_index = (UINT16)choice;
        CONSOLE_OUT("AppKeyIndex (12-bit in HEX): 0x%48X\n", param.appkey_index);
        CLI_strtoarray
        (
            argv[2],
            CLI_strlen(argv[2]),
            &param.appkey[0],
            16
        );
    }
    else
    {
        CONSOLE_OUT("Invalid Number of Arguments:0x%04X. Returning.\n", argc);
        return API_FAILURE;
    }

    retval = MS_config_client_appkey_update(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Config Heartbeat Subscription Get */
API_RESULT cli_modelc_config_heartbeat_subscription_get(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    CONSOLE_OUT
    (">> Send Config Heartbeat Subscription Get\n");
    retval = MS_config_client_heartbeat_subscription_get();
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Config Vendor Model Subscription Get */
API_RESULT cli_modelc_config_vendor_model_subscription_get(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    ACCESS_CONFIG_VENDORMODELSUB_GET_PARAM  param;
    CONSOLE_OUT
    (">> Send Config Vendor Model Subscription Get\n");

    if (2 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.element_address = (UINT16)choice;
        CONSOLE_OUT("ElementAddress (16-bit in HEX): 0x%04X\n", param.element_address);
        choice = CLI_strtoi(argv[1], CLI_strlen(argv[1]), 16);
        param.model_id = (UINT32)choice;
        CONSOLE_OUT("ModelIdentifier (32-bit in HEX): 0x%08X\n", param.model_id);
    }
    else
    {
        CONSOLE_OUT("Invalid Number of Arguments:0x%04X. Returning.\n", argc);
        return API_FAILURE;
    }

    retval = MS_config_client_vendor_model_subscription_get(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Config Vendor Model App Get */
API_RESULT cli_modelc_config_vendor_model_app_get(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    ACCESS_CONFIG_VENDOR_MODEL_APP_GET_PARAM  param;
    CONSOLE_OUT
    (">> Send Config Vendor Model App Get\n");

    if (2 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.element_address = (UINT16)choice;
        CONSOLE_OUT("ElementAddress (16-bit in HEX): 0x%04X\n", param.element_address);
        choice = CLI_strtoi(argv[1], CLI_strlen(argv[1]), 16);
        param.model_id = (UINT32)choice;
        CONSOLE_OUT("ModelIdentifier (32-bit in HEX): 0x%08X\n", param.model_id);
    }
    else
    {
        CONSOLE_OUT("Invalid Number of Arguments:0x%04X. Returning.\n", argc);
        return API_FAILURE;
    }

    retval = MS_config_client_vendor_model_app_get(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Config Model Subscription Overwrite */
API_RESULT cli_modelc_config_model_subscription_overwrite(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    ACCESS_CONFIG_MODELSUB_OVERWRITE_PARAM  param;
    CONSOLE_OUT
    (">> Send Config Model Subscription Overwrite\n");

    if (3 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.element_address = (UINT16)choice;
        CONSOLE_OUT("ElementAddress (16-bit in HEX): 0x%04X\n", param.element_address);
        choice = CLI_strtoi(argv[1], CLI_strlen(argv[1]), 16);
        param.address = (UINT16)choice;
        CONSOLE_OUT("Address (16-bit in HEX): 0x%04X\n", param.address);

        if (4 < CLI_strlen(argv[2]))
        {
            param.model.type = 0x01;
            CONSOLE_OUT("Model ID Type: Vendor\n");
        }
        else
        {
            param.model.type = 0x00;
            CONSOLE_OUT("Model ID Type: SIG\n");
        }

        choice = CLI_strtoi(argv[2], CLI_strlen(argv[2]), 16);
        param.model.id = (UINT32)choice;
        CONSOLE_OUT("ModelIdentifier: 0x%08X\n", param.model.id);
    }
    else
    {
        CONSOLE_OUT("Invalid Number of Arguments:0x%04X. Returning.\n", argc);
        return API_FAILURE;
    }

    retval = MS_config_client_model_subscription_overwrite(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Config Beacon Get */
API_RESULT cli_modelc_config_beacon_get(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    CONSOLE_OUT
    (">> Send Config Beacon Get\n");
    retval = MS_config_client_beacon_get();
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Config Model Subscription Delete All */
API_RESULT cli_modelc_config_model_subscription_delete_all(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    ACCESS_CONFIG_MODELSUB_DELETEALL_PARAM  param;
    CONSOLE_OUT
    (">> Send Config Model Subscription Delete All\n");

    if (2 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.element_address = (UINT16)choice;
        CONSOLE_OUT("ElementAddress (16-bit in HEX): 0x%04X\n", param.element_address);

        if (4 < CLI_strlen(argv[1]))
        {
            param.model.type = 0x01;
            CONSOLE_OUT("Model ID Type: Vendor\n");
        }
        else
        {
            param.model.type = 0x00;
            CONSOLE_OUT("Model ID Type: SIG\n");
        }

        choice = CLI_strtoi(argv[1], CLI_strlen(argv[1]), 16);
        param.model.id = (UINT32)choice;
        CONSOLE_OUT("ModelIdentifier: 0x%08X\n", param.model.id);
    }
    else
    {
        CONSOLE_OUT("Invalid Number of Arguments:0x%04X. Returning.\n", argc);
        return API_FAILURE;
    }

    retval = MS_config_client_model_subscription_delete_all(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Config Netkey Delete */
API_RESULT cli_modelc_config_netkey_delete(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    ACCESS_CONFIG_NETKEY_DELETE_PARAM  param;
    CONSOLE_OUT
    (">> Send Config Netkey Delete\n");

    if (1 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.netkey_index = (UINT16)choice;
        CONSOLE_OUT("NetKey (12-bit in HEX): 0x%04X\n", param.netkey_index);
    }
    else
    {
        CONSOLE_OUT("Invalid Number of Arguments:0x%04X. Returning.\n", argc);
        return API_FAILURE;
    }

    retval = MS_config_client_netkey_delete(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Config Friend Set */
API_RESULT cli_modelc_config_friend_set(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    ACCESS_CONFIG_FRIEND_SET_PARAM  param;
    CONSOLE_OUT
    (">> Send Config Friend Set\n");

    if (1 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.friend = (UCHAR)choice;
        CONSOLE_OUT("Friend (8-bit in HEX): 0x%02X\n", param.friend);
    }
    else
    {
        CONSOLE_OUT("Invalid Number of Arguments:0x%04X. Returning.\n", argc);
        return API_FAILURE;
    }

    retval = MS_config_client_friend_set(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Config Heartbeat Publication Set */
API_RESULT cli_modelc_config_heartbeat_publication_set(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    ACCESS_CONFIG_HEARTBEATPUB_SET_PARAM  param;
    CONSOLE_OUT
    (">> Send Config Heartbeat Publication Set\n");

    if (6 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.destination = (UINT16)choice;
        CONSOLE_OUT("Destination (16-bit in HEX): 0x%04X\n", param.destination);
        choice = CLI_strtoi(argv[1], CLI_strlen(argv[1]), 16);
        param.countlog = (UCHAR)choice;
        CONSOLE_OUT("CountLog (8-bit in HEX): 0x%02X\n", param.countlog);
        choice = CLI_strtoi(argv[2], CLI_strlen(argv[2]), 16);
        param.periodlog = (UCHAR)choice;
        CONSOLE_OUT("PeriodLog (8-bit in HEX): 0x%02X\n", param.periodlog);
        choice = CLI_strtoi(argv[3], CLI_strlen(argv[3]), 16);
        param.ttl = (UCHAR)choice;
        CONSOLE_OUT("TTL (8-bit in HEX): 0x%02X\n", param.ttl);
        choice = CLI_strtoi(argv[4], CLI_strlen(argv[4]), 16);
        param.features = (UINT16)choice;
        CONSOLE_OUT("Features (16-bit in HEX): 0x%04X\n", param.features);
        choice = CLI_strtoi(argv[5], CLI_strlen(argv[5]), 16);
        param.netkey_index = (UINT16)choice;
        CONSOLE_OUT("NetKeyIndex (16-bit in HEX): 0x%04X\n", param.netkey_index);
    }
    else
    {
        CONSOLE_OUT("Invalid Number of Arguments:0x%04X. Returning.\n", argc);
        return API_FAILURE;
    }

    retval = MS_config_client_heartbeat_publication_set(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Config Node Identity Get */
API_RESULT cli_modelc_config_node_identity_get(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    ACCESS_CONFIG_NODEID_GET_PARAM  param;
    CONSOLE_OUT
    (">> Send Config Node Identity Get\n");

    if (1 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.netkey_index = (UINT16)choice;
        CONSOLE_OUT("NetKeyIndex (16-bit in HEX): 0x%04X\n", param.netkey_index);
    }
    else
    {
        CONSOLE_OUT("Invalid Number of Arguments:0x%04X. Returning.\n", argc);
        return API_FAILURE;
    }

    retval = MS_config_client_node_identity_get(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Config Default Ttl Set */
API_RESULT cli_modelc_config_default_ttl_set(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    ACCESS_CONFIG_DEFAULT_TTL_SET_PARAM  param;
    CONSOLE_OUT
    (">> Send Config Default Ttl Set\n");

    if (1 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.ttl = (UCHAR)choice;
        CONSOLE_OUT("TTL (8-bit in HEX): 0x%02X\n", param.ttl);
    }
    else
    {
        CONSOLE_OUT("Invalid Number of Arguments:0x%04X. Returning.\n", argc);
        return API_FAILURE;
    }

    retval = MS_config_client_default_ttl_set(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Config Netkey Update */
API_RESULT cli_modelc_config_netkey_update(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    ACCESS_CONFIG_NETKEY_UPDATE_PARAM  param;
    CONSOLE_OUT
    (">> Send Config Netkey Update\n");

    if (2 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.netkey_index = (UINT16)choice;
        CONSOLE_OUT("NetKeyIndex (16-bit in HEX): 0x%04X\n", param.netkey_index);
        CLI_strtoarray
        (
            argv[1],
            CLI_strlen(argv[1]),
            &param.netkey[0],
            16
        );
    }
    else
    {
        CONSOLE_OUT("Invalid Number of Arguments:0x%04X. Returning.\n", argc);
        return API_FAILURE;
    }

    /* Set Local NetKey */
    MS_access_cm_add_update_netkey
    (
        0, /* netkey_index */
        MS_ACCESS_CONFIG_NETKEY_UPDATE_OPCODE, /* opcode */
        &param.netkey[0] /* net_key */
    );
    retval = MS_config_client_netkey_update(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Config Heartbeat Publication Get */
API_RESULT cli_modelc_config_heartbeat_publication_get(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    CONSOLE_OUT
    (">> Send Config Heartbeat Publication Get\n");
    retval = MS_config_client_heartbeat_publication_get();
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Config Model Publication Set */
API_RESULT cli_modelc_config_model_publication_set(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    ACCESS_CONFIG_MODELPUB_SET_PARAM  param;
    CONSOLE_OUT
    (">> Send Config Model Publication Set\n");

    if (9 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.element_address = (UINT16)choice;
        CONSOLE_OUT("ElementAddress (16-bit in HEX): 0x%04X\n", param.element_address);
        choice = CLI_strtoi(argv[1], CLI_strlen(argv[1]), 16);
        param.publish_address = (UINT16)choice;
        CONSOLE_OUT("PublishAddress (16-bit in HEX): 0x%04X\n", param.publish_address);
        choice = CLI_strtoi(argv[2], CLI_strlen(argv[2]), 16);
        param.appkey_index = (UINT16)choice;
        CONSOLE_OUT("AppKeyIndex (12-bit in HEX): 0x%04X\n", param.appkey_index);
        choice = CLI_strtoi(argv[3], CLI_strlen(argv[3]), 16);
        param.credential_flag = (UCHAR)choice;
        CONSOLE_OUT("CredentialFlag (1-bit in HEX): 0x%02X\n", param.credential_flag);
        choice = CLI_strtoi(argv[4], CLI_strlen(argv[4]), 16);
        param.publish_ttl = (UCHAR)choice;
        CONSOLE_OUT("PublishTTL (8-bit in HEX): 0x%02X\n", param.publish_ttl);
        choice = CLI_strtoi(argv[5], CLI_strlen(argv[5]), 16);
        param.publish_period = (UCHAR)choice;
        CONSOLE_OUT("PublishPeriod (8-bit in HEX): 0x%02X\n", param.publish_period);
        choice = CLI_strtoi(argv[6], CLI_strlen(argv[6]), 16);
        param.publish_rtx_count = (UCHAR)choice;
        CONSOLE_OUT("PublishRetransmitCount (3-bit in HEX): 0x%02X\n", param.publish_rtx_count);
        choice = CLI_strtoi(argv[7], CLI_strlen(argv[7]), 16);
        param.publish_rtx_interval_steps = (UCHAR)choice;
        CONSOLE_OUT("PublishRetransmitIntervalSteps (5-bit in HEX): 0x%02X\n", param.publish_rtx_interval_steps);

        if (4 < CLI_strlen(argv[8]))
        {
            param.model.type = 0x01;
            CONSOLE_OUT("Model ID Type: Vendor\n");
        }
        else
        {
            param.model.type = 0x00;
            CONSOLE_OUT("Model ID Type: SIG\n");
        }

        choice = CLI_strtoi(argv[8], CLI_strlen(argv[8]), 16);
        param.model.id = (UINT32)choice;
        CONSOLE_OUT("ModelIdentifier: 0x%08X\n", param.model.id);
    }
    else
    {
        CONSOLE_OUT("Invalid Number of Arguments:0x%04X. Returning.\n", argc);
        return API_FAILURE;
    }

    retval = MS_config_client_model_publication_set(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Config Netkey Get */
API_RESULT cli_modelc_config_netkey_get(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    CONSOLE_OUT
    (">> Send Config Netkey Get\n");
    retval = MS_config_client_netkey_get();
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Config Model Publication Get */
API_RESULT cli_modelc_config_model_publication_get(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    ACCESS_CONFIG_MODELPUB_GET_PARAM  param;
    CONSOLE_OUT
    (">> Send Config Model Publication Get\n");

    if (2 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.element_address = (UINT16)choice;
        CONSOLE_OUT("ElementAddress (16-bit in HEX): 0x%04X\n", param.element_address);

        if (4 < CLI_strlen(argv[1]))
        {
            param.model.type = 0x01;
            CONSOLE_OUT("Model ID Type: Vendor\n");
        }
        else
        {
            param.model.type = 0x00;
            CONSOLE_OUT("Model ID Type: SIG\n");
        }

        choice = CLI_strtoi(argv[1], CLI_strlen(argv[1]), 16);
        param.model.id = (UINT32)choice;
        CONSOLE_OUT("ModelIdentifier: 0x%08X\n", param.model.id);
    }
    else
    {
        CONSOLE_OUT("Invalid Number of Arguments:0x%04X. Returning.\n", argc);
        return API_FAILURE;
    }

    retval = MS_config_client_model_publication_get(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Config Network Transmit Get */
API_RESULT cli_modelc_config_network_transmit_get(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    CONSOLE_OUT
    (">> Send Config Network Transmit Get\n");
    retval = MS_config_client_network_transmit_get();
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Config Composition Data Get */
API_RESULT cli_modelc_config_composition_data_get(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    ACCESS_CONFIG_COMPDATA_GET_PARAM  param;
    CONSOLE_OUT
    (">> Send Config Composition Data Get (Page No <in HEX>)\n");

    if (1 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.page = (UCHAR)choice;
        CONSOLE_OUT("Page (8-bit in HEX): 0x%02X\n", param.page);
    }
    else
    {
        CONSOLE_OUT("Invalid Number of Arguments:0x%04X. Returning.\n", argc);
        return API_FAILURE;
    }

    retval = MS_config_client_composition_data_get(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Config Appkey Add */
API_RESULT cli_modelc_config_appkey_add(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    ACCESS_CONFIG_APPKEY_ADD_PARAM  param;
    CONSOLE_OUT
    (">> Send Config Appkey Add\n");

    if (3 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.netkey_index = (UINT16)choice;
        CONSOLE_OUT("NetKeyIndex (12-bit in HEX): 0x%04X\n", param.netkey_index);
        choice = CLI_strtoi(argv[1], CLI_strlen(argv[1]), 16);
        param.appkey_index = (UINT16)choice;
        CONSOLE_OUT("AppKeyIndex (12-bit in HEX): 0x%04X\n", param.appkey_index);
        CLI_strtoarray
        (
            argv[2],
            CLI_strlen(argv[2]),
            &param.appkey[0],
            16
        );
    }
    else
    {
        CONSOLE_OUT("Invalid Number of Arguments:0x%04X. Returning.\n", argc);
        return API_FAILURE;
    }

    retval = MS_config_client_appkey_add(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Config Key Refresh Phase Set */
API_RESULT cli_modelc_config_key_refresh_phase_set(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    ACCESS_CONFIG_KEYREFRESH_PHASE_SET_PARAM  param;
    CONSOLE_OUT
    (">> Send Config Key Refresh Phase Set\n");

    if (2 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.netkey_index = (UINT16)choice;
        CONSOLE_OUT("NetKeyIndex (16-bit in HEX): 0x%04X\n", param.netkey_index);
        choice = CLI_strtoi(argv[1], CLI_strlen(argv[1]), 16);
        param.transition = (UCHAR)choice;
        CONSOLE_OUT("Transition (8-bit in HEX): 0x%02X\n", param.transition);
    }
    else
    {
        CONSOLE_OUT("Invalid Number of Arguments:0x%04X. Returning.\n", argc);
        return API_FAILURE;
    }

    /* Change Local State as well */
    MS_access_cm_set_key_refresh_phase
    (
        0, /* subnet_handle */
        &param.transition /* key_refresh_state */
    );
    param.transition = (UCHAR)choice;
    retval = MS_config_client_keyrefresh_phase_set(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Config Appkey Get */
API_RESULT cli_modelc_config_appkey_get(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    ACCESS_CONFIG_APPKEY_GET_PARAM  param;
    CONSOLE_OUT
    (">> Send Config Appkey Get\n");

    if (1 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.netkey_index = (UINT16)choice;
        CONSOLE_OUT("NetKeyIndex (12-bit in HEX): 0x%04X\n", param.netkey_index);
    }
    else
    {
        CONSOLE_OUT("Invalid Number of Arguments:0x%04X. Returning.\n", argc);
        return API_FAILURE;
    }

    retval = MS_config_client_appkey_get(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Config Appkey Delete */
API_RESULT cli_modelc_config_appkey_delete(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    ACCESS_CONFIG_APPKEY_DELETE_PARAM  param;
    CONSOLE_OUT
    (">> Send Config Appkey Delete\n");

    if (2 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.netkey_index = (UINT16)choice;
        CONSOLE_OUT("NetKeyIndex (12-bit in HEX): 0x%04X\n", param.netkey_index);
        choice = CLI_strtoi(argv[1], CLI_strlen(argv[1]), 16);
        param.appkey_index = (UINT16)choice;
        CONSOLE_OUT("AppKeyIndex (12-bit in HEX): 0x%04X\n", param.appkey_index);
    }
    else
    {
        CONSOLE_OUT("Invalid Number of Arguments:0x%04X. Returning.\n", argc);
        return API_FAILURE;
    }

    retval = MS_config_client_appkey_delete(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Config Low Power Node Polltimeout Get */
API_RESULT cli_modelc_config_low_power_node_polltimeout_get(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    ACCESS_CONFIG_LPNPOLLTIMEOUT_GET_PARAM  param;
    CONSOLE_OUT
    (">> Send Config Low Power Node Polltimeout Get\n");

    if (1 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.lpn_address = (UINT16)choice;
        CONSOLE_OUT("LPNAddress (16-bit in HEX): 0x%04X\n", param.lpn_address);
    }
    else
    {
        CONSOLE_OUT("Invalid Number of Arguments:0x%04X. Returning.\n", argc);
        return API_FAILURE;
    }

    retval = MS_config_client_lpn_polltimeout_get(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Get Model Handle */
API_RESULT cli_modelc_configuration_get_model_handle(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    MS_ACCESS_MODEL_HANDLE model_handle;
    #if 0
    retval = MS_configuration_get_model_handle(&model_handle);

    if (API_SUCCESS == retval)
    {
        CONSOLE_OUT
        (">> Model Handle 0x%04X\n", model_handle);
    }
    else
    {
        CONSOLE_OUT
        (">> Get Model Handle Failed. Status 0x%04X\n", retval);
    }

    #else
    retval = API_FAILURE;
    CONSOLE_OUT("To be implemented\n");
    #endif /* 0 */
    return retval;
}

/* Set Publish Address */
API_RESULT cli_modelc_configuration_set_publish_address(UINT32 argc, UCHAR* argv[])
{
    int  choice;
    API_RESULT retval;
    MS_ACCESS_MODEL_HANDLE   model_handle;
    MS_ACCESS_PUBLISH_INFO   publish_info;
    MS_ACCESS_DEV_KEY_HANDLE dev_key_handle;
    /* Set Publish Information */
    EM_mem_set(&publish_info, 0, sizeof(publish_info));
    publish_info.addr.use_label = MS_FALSE;
    model_handle = appl_configuration_client_model_handle;
    CONSOLE_OUT("Model Handle (16-bit in HEX): 0x%04X\n", model_handle);

    /* Check Number of Arguments */
    if (1 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        publish_info.addr.addr = (UINT16)choice;
        CONSOLE_OUT("Config Server Address (16-bit in HEX): 0x%04X\n", publish_info.addr.addr);
        retval = MS_access_cm_get_device_key_handle
                 (
                     publish_info.addr.addr,
                     &dev_key_handle
                 );

        if (API_SUCCESS == retval)
        {
            publish_info.appkey_index = MS_CONFIG_LIMITS(MS_MAX_APPS) + dev_key_handle;
            CONSOLE_OUT("DevKey -> AppKey Index: 0x%04X\n", publish_info.appkey_index);
        }
        else
        {
            CONSOLE_OUT(
                "\nDevice Key Entry not found for 0x%04X Address, retval 0x%04X\n",
                publish_info.addr.addr, retval);
            /**
                NOTE:
                Currently assigning default Device Key Index/Handle if there is no device key
                mapped with the provided "Config Server Address".
                This possible in scenarios where DUT is the provisioned device but also a Config
                client. Then, the stack is not aware of the Provisioner's unicast address and the
                default device key handle which holds the device key of the local device with respect
                to the provisioner shall be used.
            */
            CONSOLE_OUT(
                "\nUsing Default Index 0x%04X for Device Key\n", MS_CONFIG_LIMITS(MS_MAX_APPS));
            publish_info.appkey_index = MS_CONFIG_LIMITS(MS_MAX_APPS);
        }
    }
    else
    {
        CONSOLE_OUT("Invalid Number of Arguments:0x%04X. Returning.\n", argc);
        return API_FAILURE;
    }

    publish_info.remote = MS_FALSE;
    retval = MS_access_cm_set_model_publication
             (
                 model_handle,
                 &publish_info
             );

    if (API_SUCCESS == retval)
    {
        CONSOLE_OUT
        (">> Publish Address is set Successfully.\n");
    }
    else
    {
        CONSOLE_OUT
        (">> Failed to set publish address. Status 0x%04X\n", retval);
    }

    return retval;
}

static void  appl_config_client_print_composition_data
(
    /* IN */ UCHAR*                   comp_data,
    /* IN */ UINT16                   data_len
)
{
    UINT32 marker, u32_val;
    UINT16 u16_val;
    UINT8  nums, numv, index;
    UINT8  elem_index;

    if ((NULL == comp_data) || (15 > data_len))
    {
        CONSOLE_OUT("Invalid Composition Data of Length:0x%02X\n", data_len);
        return;
    }

    /* Page Number */
    marker = 0;
    CONSOLE_OUT("Page: 0x%02X\n", comp_data[marker]);
    marker += 1;
    /* CID */
    MS_UNPACK_LE_2_BYTE(&u16_val, &comp_data[marker]);
    CONSOLE_OUT("CID: 0x%04X\n", u16_val);
    marker += 2;
    /* PID */
    MS_UNPACK_LE_2_BYTE(&u16_val, &comp_data[marker]);
    CONSOLE_OUT("PID: 0x%04X\n", u16_val);
    marker += 2;
    /* VID */
    MS_UNPACK_LE_2_BYTE(&u16_val, &comp_data[marker]);
    CONSOLE_OUT("VID: 0x%04X\n", u16_val);
    marker += 2;
    /* CRPL */
    MS_UNPACK_LE_2_BYTE(&u16_val, &comp_data[marker]);
    CONSOLE_OUT("CRPL: 0x%04X\n", u16_val);
    marker += 2;
    /* Features */
    MS_UNPACK_LE_2_BYTE(&u16_val, &comp_data[marker]);
    CONSOLE_OUT("Features: 0x%04X\n", u16_val);
    marker += 2;
    /* Parse Elements */
    elem_index = 0;

    while (4 < (data_len - marker))
    {
        CONSOLE_OUT("Element #0x%02X\n", elem_index);
        /* LOC */
        MS_UNPACK_LE_2_BYTE(&u16_val, &comp_data[marker]);
        CONSOLE_OUT("LOC: 0x%04X\n", u16_val);
        marker += 2;
        /* NumS */
        nums = comp_data[marker];
        CONSOLE_OUT("NumS: 0x%02X\n", nums);
        marker += 1;
        /* NumV */
        numv = comp_data[marker];
        CONSOLE_OUT("NumV: 0x%02X\n", numv);
        marker += 1;

        /* Check Length based on Model ID */
        if ((2 * nums + 4 * numv) > (data_len - marker))
        {
            break;
        }

        /* Print SIG Model IDs */
        CONSOLE_OUT("SIG MODEL ID(s):\n");

        for (index = 0; index < nums; index ++)
        {
            MS_UNPACK_LE_2_BYTE(&u16_val, &comp_data[marker]);
            CONSOLE_OUT("#0x%02X: 0x%04X\n", index, u16_val);
            marker += 2;
        }

        /* Print Vendor Model IDs */
        CONSOLE_OUT("Vendor MODEL ID(s):\n");

        for (index = 0; index < numv; index ++)
        {
            MS_UNPACK_LE_4_BYTE(&u32_val, &comp_data[marker]);
            CONSOLE_OUT("#0x%02X: 0x%08X\n", index, u32_val);
            marker += 4;
        }

        elem_index++;
    }

    /* Check if all the data is parsed */
    if (marker != data_len)
    {
        CONSOLE_OUT("Invalid Composition Data. Parsing is not complete\n");
    }
}

/**
    \brief Client Application Asynchronous Notification Callback.

    \par Description
    Configuration client calls the registered callback to indicate events occurred to the application.

    \param [in] handle        Model Handle.
    \param [in] opcode        Opcode.
    \param [in] data_param    Data associated with the event if any or NULL.
    \param [in] data_len      Size of the event data. 0 if event data is NULL.
*/
API_RESULT appl_configuration_client_cb
(
    /* IN */ MS_ACCESS_MODEL_HANDLE* handle,
    /* IN */ UINT32                   opcode,
    /* IN */ UCHAR*                   data_param,
    /* IN */ UINT16                   data_len
)
{
    API_RESULT retval;
    retval = API_SUCCESS;
    CONSOLE_OUT (
        "[CONFIGURATION_CLIENT] Callback. Opcode 0x%04X\n", opcode);
    appl_dump_bytes(data_param, data_len);

    switch(opcode)
    {
    case MS_ACCESS_CONFIG_GATT_PROXY_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_CONFIG_GATT_PROXY_STATUS_OPCODE\n");
    }
    break;

    case MS_ACCESS_CONFIG_HEARTBEAT_PUBLICATION_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_CONFIG_HEARTBEAT_PUBLICATION_STATUS_OPCODE\n");
    }
    break;

    case MS_ACCESS_CONFIG_SIG_MODEL_APP_LIST_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_CONFIG_SIG_MODEL_APP_LIST_OPCODE\n");
    }
    break;

    case MS_ACCESS_CONFIG_COMPOSITION_DATA_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_CONFIG_COMPOSITION_DATA_STATUS_OPCODE\n");
        appl_config_client_print_composition_data(data_param, data_len);
    }
    break;

    case MS_ACCESS_CONFIG_MODEL_SUBSCRIPTION_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_CONFIG_MODEL_SUBSCRIPTION_STATUS_OPCODE\n");
    }
    break;

    case MS_ACCESS_CONFIG_NETKEY_LIST_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_CONFIG_NETKEY_LIST_OPCODE\n");
    }
    break;

    case MS_ACCESS_CONFIG_LOW_POWER_NODE_POLLTIMEOUT_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_CONFIG_LOW_POWER_NODE_POLLTIMEOUT_STATUS_OPCODE\n");
    }
    break;

    case MS_ACCESS_CONFIG_SIG_MODEL_SUBSCRIPTION_LIST_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_CONFIG_SIG_MODEL_SUBSCRIPTION_LIST_OPCODE\n");
    }
    break;

    case MS_ACCESS_CONFIG_VENDOR_MODEL_SUBSCRIPTION_LIST_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_CONFIG_VENDOR_MODEL_SUBSCRIPTION_LIST_OPCODE\n");
    }
    break;

    case MS_ACCESS_CONFIG_APPKEY_LIST_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_CONFIG_APPKEY_LIST_OPCODE\n");
    }
    break;

    case MS_ACCESS_CONFIG_APPKEY_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_CONFIG_APPKEY_STATUS_OPCODE\n");
    }
    break;

    case MS_ACCESS_CONFIG_RELAY_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_CONFIG_RELAY_STATUS_OPCODE\n");
    }
    break;

    case MS_ACCESS_CONFIG_FRIEND_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_CONFIG_FRIEND_STATUS_OPCODE\n");
    }
    break;

    case MS_ACCESS_CONFIG_MODEL_APP_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_CONFIG_MODEL_APP_STATUS_OPCODE\n");
    }
    break;

    case MS_ACCESS_CONFIG_NODE_RESET_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_CONFIG_NODE_RESET_STATUS_OPCODE\n");
    }
    break;
    #if 0

    case MS_ACCESS_CONFIG_MODEL_SUBSCRIPTION_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_CONFIG_MODEL_SUBSCRIPTION_STATUS_OPCODE\n");
    }
    break;
        #endif /* 0 */

    case MS_ACCESS_CONFIG_DEFAULT_TTL_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_CONFIG_DEFAULT_TTL_STATUS_OPCODE\n");
    }
    break;

    case MS_ACCESS_CONFIG_NODE_IDENTITY_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_CONFIG_NODE_IDENTITY_STATUS_OPCODE\n");
    }
    break;

    case MS_ACCESS_CONFIG_HEARTBEAT_SUBSCRIPTION_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_CONFIG_HEARTBEAT_SUBSCRIPTION_STATUS_OPCODE\n");
    }
    break;

    case MS_ACCESS_CONFIG_KEY_REFRESH_PHASE_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_CONFIG_KEY_REFRESH_PHASE_STATUS_OPCODE\n");
    }
    break;

    case MS_ACCESS_CONFIG_NETWORK_TRANSMIT_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_CONFIG_NETWORK_TRANSMIT_STATUS_OPCODE\n");
    }
    break;

    case MS_ACCESS_CONFIG_BEACON_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_CONFIG_BEACON_STATUS_OPCODE\n");
    }
    break;

    case MS_ACCESS_CONFIG_MODEL_PUBLICATION_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_CONFIG_MODEL_PUBLICATION_STATUS_OPCODE\n");
    }
    break;

    case MS_ACCESS_CONFIG_NETKEY_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_CONFIG_NETKEY_STATUS_OPCODE\n");
    }
    break;

    case MS_ACCESS_CONFIG_VENDOR_MODEL_APP_LIST_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_CONFIG_VENDOR_MODEL_APP_LIST_OPCODE\n");
    }
    break;
    }

    return retval;
}

#endif /* CLI_CONFIG_CLIENT_MODEL */
