/**
    \file appl_config_client.h

    \brief This file defines the Mesh Configuration Model Application Interface
    - includes Data Structures and Methods for both Server and Client.
*/

/*
    Copyright (C) 2018. Mindtree Ltd.
    All rights reserved.
*/

#ifndef _H_APPL_CONFIG_CLIENT_
#define _H_APPL_CONFIG_CLIENT_


/* --------------------------------------------- Header File Inclusion */
#include "MS_config_api.h"
#include "appl_main.h"


/* --------------------------------------------- Global Definitions */


/* --------------------------------------------- Data Types/ Structures */


/* --------------------------------------------- Function */
/* config client application entry point */
void main_config_client_operations(void);

/* Send Config Model App Unbind */
void appl_send_config_model_app_unbind(void);

/* Send Config Friend Get */
void appl_send_config_friend_get(void);

/* Send Config Gatt Proxy Get */
void appl_send_config_gatt_proxy_get(void);

/* Send Config Model Subscription Virtual Address Overwrite */
void appl_send_config_model_subscription_virtual_address_overwrite(void);

/* Send Config Sig Model App Get */
void appl_send_config_sig_model_app_get(void);

/* Send Config Model Subscription Add */
void appl_send_config_model_subscription_add(void);

/* Send Config Network Transmit Set */
void appl_send_config_network_transmit_set(void);

/* Send Config Model Subscription Delete */
void appl_send_config_model_subscription_delete(void);

/* Send Config Relay Get */
void appl_send_config_relay_get(void);

/* Send Config Heartbeat Subscription Set */
void appl_send_config_heartbeat_subscription_set(void);

/* Send Config Beacon Set */
void appl_send_config_beacon_set(void);

/* Send Config Node Reset */
void appl_send_config_node_reset(void);

/* Send Config Relay Set */
void appl_send_config_relay_set(void);

/* Send Config Key Refresh Phase Get */
void appl_send_config_key_refresh_phase_get(void);

/* Send Config Model Subscription Virtual Address Delete */
void appl_send_config_model_subscription_virtual_address_delete(void);

/* Send Config Model Publication Virtual Address Set */
void appl_send_config_model_publication_virtual_address_set(void);

/* Send Config Gatt Proxy Set */
void appl_send_config_gatt_proxy_set(void);

/* Send Config Model Subscription Virtual Address Add */
void appl_send_config_model_subscription_virtual_address_add(void);

/* Send Config Default Ttl Get */
void appl_send_config_default_ttl_get(void);

/* Send Config Model App Bind */
void appl_send_config_model_app_bind(void);

/* Send Config Node Identity Set */
void appl_send_config_node_identity_set(void);

/* Send Config Sig Model Subscription Get */
void appl_send_config_sig_model_subscription_get(void);

/* Send Config Netkey Add */
void appl_send_config_netkey_add(void);

/* Send Config Appkey Update */
void appl_send_config_appkey_update(void);

/* Send Config Heartbeat Subscription Get */
void appl_send_config_heartbeat_subscription_get(void);

/* Send Config Vendor Model Subscription Get */
void appl_send_config_vendor_model_subscription_get(void);

/* Send Config Vendor Model App Get */
void appl_send_config_vendor_model_app_get(void);

/* Send Config Model Subscription Overwrite */
void appl_send_config_model_subscription_overwrite(void);

/* Send Config Beacon Get */
void appl_send_config_beacon_get(void);

/* Send Config Model Subscription Delete All */
void appl_send_config_model_subscription_delete_all(void);

/* Send Config Netkey Delete */
void appl_send_config_netkey_delete(void);

/* Send Config Friend Set */
void appl_send_config_friend_set(void);

/* Send Config Heartbeat Publication Set */
void appl_send_config_heartbeat_publication_set(void);

/* Send Config Node Identity Get */
void appl_send_config_node_identity_get(void);

/* Send Config Default Ttl Set */
void appl_send_config_default_ttl_set(void);

/* Send Config Netkey Update */
void appl_send_config_netkey_update(void);

/* Send Config Heartbeat Publication Get */
void appl_send_config_heartbeat_publication_get(void);

/* Send Config Model Publication Set */
void appl_send_config_model_publication_set(void);

/* Send Config Netkey Get */
void appl_send_config_netkey_get(void);

/* Send Config Model Publication Get */
void appl_send_config_model_publication_get(void);

/* Send Config Network Transmit Get */
void appl_send_config_network_transmit_get(void);

/* Send Config Composition Data Get */
void appl_send_config_composition_data_get(void);

/* Send Config Appkey Add */
void appl_send_config_appkey_add(void);

/* Send Config Key Refresh Phase Set */
void appl_send_config_key_refresh_phase_set(void);

/* Send Config Appkey Get */
void appl_send_config_appkey_get(void);

/* Send Config Appkey Delete */
void appl_send_config_appkey_delete(void);

/* Send Config Low Power Node Polltimeout Get */
void appl_send_config_low_power_node_polltimeout_get(void);

/* Get Model Handle */
void appl_config_client_get_model_handle(void);

/* Set Publish Address */
void appl_config_client_set_publish_address(void);

/**
    \brief Client Application Asynchronous Notification Callback.

    \par Description
    Configuration client calls the registered callback to indicate events occurred to the application.

    \param [in] handle        Model Handle.
    \param [in] opcode        Opcode.
    \param [in] data_param    Data associated with the event if any or NULL.
    \param [in] data_len      Size of the event data. 0 if event data is NULL.
*/
API_RESULT appl_config_client_cb
(
    /* IN */ MS_ACCESS_MODEL_HANDLE* handle,
    /* IN */ UINT32                   opcode,
    /* IN */ UCHAR*                   data_param,
    /* IN */ UINT16                   data_len
);

#endif /*_H_APPL_CONFIG_CLIENT_ */
