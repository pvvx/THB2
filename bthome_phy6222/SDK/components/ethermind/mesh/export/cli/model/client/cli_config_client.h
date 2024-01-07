/**
    \file cli_configuration_client.h

    \brief This file defines the Mesh Configuration Model Application Interface
    - includes Data Structures and Methods for both Server and Client.
*/

/*
    Copyright (C) 2018. Mindtree Ltd.
    All rights reserved.
*/

#ifndef _H_CLI_CONFIG_CLIENT_
#define _H_CLI_CONFIG_CLIENT_


/* --------------------------------------------- Header File Inclusion */
#include "MS_config_api.h"
#include "cli_main.h"


/* --------------------------------------------- Global Definitions */


/* --------------------------------------------- Data Types/ Structures */


/* --------------------------------------------- Function */
/* configuration client CLI entry point */
CLI_CMD_HANDLER_DECL(cli_modelc_configuration);

/* configuration client CLI entry point */
CLI_CMD_HANDLER_DECL(cli_modelc_configuration_setup);

/* Send Config Model App Unbind */
CLI_CMD_HANDLER_DECL(cli_modelc_config_model_app_unbind);

/* Send Config Friend Get */
CLI_CMD_HANDLER_DECL(cli_modelc_config_friend_get);

/* Send Config Gatt Proxy Get */
CLI_CMD_HANDLER_DECL(cli_modelc_config_gatt_proxy_get);

/* Send Config Model Subscription Virtual Address Overwrite */
CLI_CMD_HANDLER_DECL(cli_modelc_config_model_subscription_virtual_address_overwrite);

/* Send Config Sig Model App Get */
CLI_CMD_HANDLER_DECL(cli_modelc_config_sig_model_app_get);

/* Send Config Model Subscription Add */
CLI_CMD_HANDLER_DECL(cli_modelc_config_model_subscription_add);

/* Send Config Network Transmit Set */
CLI_CMD_HANDLER_DECL(cli_modelc_config_network_transmit_set);

/* Send Config Model Subscription Delete */
CLI_CMD_HANDLER_DECL(cli_modelc_config_model_subscription_delete);

/* Send Config Relay Get */
CLI_CMD_HANDLER_DECL(cli_modelc_config_relay_get);

/* Send Config Heartbeat Subscription Set */
CLI_CMD_HANDLER_DECL(cli_modelc_config_heartbeat_subscription_set);

/* Send Config Beacon Set */
CLI_CMD_HANDLER_DECL(cli_modelc_config_beacon_set);

/* Send Config Node Reset */
CLI_CMD_HANDLER_DECL(cli_modelc_config_node_reset);

/* Send Config Relay Set */
CLI_CMD_HANDLER_DECL(cli_modelc_config_relay_set);

/* Send Config Key Refresh Phase Get */
CLI_CMD_HANDLER_DECL(cli_modelc_config_key_refresh_phase_get);

/* Send Config Model Subscription Virtual Address Delete */
CLI_CMD_HANDLER_DECL(cli_modelc_config_model_subscription_virtual_address_delete);

/* Send Config Model Publication Virtual Address Set */
CLI_CMD_HANDLER_DECL(cli_modelc_config_model_publication_virtual_address_set);

/* Send Config Gatt Proxy Set */
CLI_CMD_HANDLER_DECL(cli_modelc_config_gatt_proxy_set);

/* Send Config Model Subscription Virtual Address Add */
CLI_CMD_HANDLER_DECL(cli_modelc_config_model_subscription_virtual_address_add);

/* Send Config Default Ttl Get */
CLI_CMD_HANDLER_DECL(cli_modelc_config_default_ttl_get);

/* Send Config Model App Bind */
CLI_CMD_HANDLER_DECL(cli_modelc_config_model_app_bind);

/* Send Config Node Identity Set */
CLI_CMD_HANDLER_DECL(cli_modelc_config_node_identity_set);

/* Send Config Sig Model Subscription Get */
CLI_CMD_HANDLER_DECL(cli_modelc_config_sig_model_subscription_get);

/* Send Config Netkey Add */
CLI_CMD_HANDLER_DECL(cli_modelc_config_netkey_add);

/* Send Config Appkey Update */
CLI_CMD_HANDLER_DECL(cli_modelc_config_appkey_update);

/* Send Config Heartbeat Subscription Get */
CLI_CMD_HANDLER_DECL(cli_modelc_config_heartbeat_subscription_get);

/* Send Config Vendor Model Subscription Get */
CLI_CMD_HANDLER_DECL(cli_modelc_config_vendor_model_subscription_get);

/* Send Config Vendor Model App Get */
CLI_CMD_HANDLER_DECL(cli_modelc_config_vendor_model_app_get);

/* Send Config Model Subscription Overwrite */
CLI_CMD_HANDLER_DECL(cli_modelc_config_model_subscription_overwrite);

/* Send Config Beacon Get */
CLI_CMD_HANDLER_DECL(cli_modelc_config_beacon_get);

/* Send Config Model Subscription Delete All */
CLI_CMD_HANDLER_DECL(cli_modelc_config_model_subscription_delete_all);

/* Send Config Netkey Delete */
CLI_CMD_HANDLER_DECL(cli_modelc_config_netkey_delete);

/* Send Config Friend Set */
CLI_CMD_HANDLER_DECL(cli_modelc_config_friend_set);

/* Send Config Heartbeat Publication Set */
CLI_CMD_HANDLER_DECL(cli_modelc_config_heartbeat_publication_set);

/* Send Config Node Identity Get */
CLI_CMD_HANDLER_DECL(cli_modelc_config_node_identity_get);

/* Send Config Default Ttl Set */
CLI_CMD_HANDLER_DECL(cli_modelc_config_default_ttl_set);

/* Send Config Netkey Update */
CLI_CMD_HANDLER_DECL(cli_modelc_config_netkey_update);

/* Send Config Heartbeat Publication Get */
CLI_CMD_HANDLER_DECL(cli_modelc_config_heartbeat_publication_get);

/* Send Config Model Publication Set */
CLI_CMD_HANDLER_DECL(cli_modelc_config_model_publication_set);

/* Send Config Netkey Get */
CLI_CMD_HANDLER_DECL(cli_modelc_config_netkey_get);

/* Send Config Model Publication Get */
CLI_CMD_HANDLER_DECL(cli_modelc_config_model_publication_get);

/* Send Config Network Transmit Get */
CLI_CMD_HANDLER_DECL(cli_modelc_config_network_transmit_get);

/* Send Config Composition Data Get */
CLI_CMD_HANDLER_DECL(cli_modelc_config_composition_data_get);

/* Send Config Appkey Add */
CLI_CMD_HANDLER_DECL(cli_modelc_config_appkey_add);

/* Send Config Key Refresh Phase Set */
CLI_CMD_HANDLER_DECL(cli_modelc_config_key_refresh_phase_set);

/* Send Config Appkey Get */
CLI_CMD_HANDLER_DECL(cli_modelc_config_appkey_get);

/* Send Config Appkey Delete */
CLI_CMD_HANDLER_DECL(cli_modelc_config_appkey_delete);

/* Send Config Low Power Node Polltimeout Get */
CLI_CMD_HANDLER_DECL(cli_modelc_config_low_power_node_polltimeout_get);

/* Get Model Handle */
CLI_CMD_HANDLER_DECL(cli_modelc_configuration_get_model_handle);

/* Set Publish Address */
CLI_CMD_HANDLER_DECL(cli_modelc_configuration_set_publish_address);

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
);

#endif /*_H_CLI_CONFIG_CLIENT_ */
