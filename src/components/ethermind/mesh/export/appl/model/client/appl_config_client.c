/**
    \file appl_config_client.c

    \brief This file defines the Mesh Configuration Model Application Interface
    - includes Data Structures and Methods for both Server and Client.
*/

/*
    Copyright (C) 2018. Mindtree Ltd.
    All rights reserved.
*/



/* --------------------------------------------- Header File Inclusion */
#include "appl_config_client.h"


/* --------------------------------------------- Global Definitions */
/* #define APPL_CONFIG_CLIENT_PRINT_COMP_DATA */

#ifdef APPL_CONFIG_CLIENT_PRINT_COMP_DATA
void appl_config_client_print_composition_data
(
    /* IN */ UCHAR* comp_data,
    /* IN */ UINT16  data_len
);
#else
#define appl_config_client_print_composition_data(c, d)
#endif /* APPL_CONFIG_CLIENT_PRINT_COMP_DATA */


/* --------------------------------------------- Static Global Variables */
static const char main_config_client_options[] = "\n\
======== Configuration Client Menu ========\n\
     0. Exit. \n\
     1. Refresh. \n\
 \n\
    20. Send Config Beacon Set. \n\
    21. Send Config Beacon Get. \n\
    22. Send Config Composition Data Get. \n\
    23. Send Config Default Ttl Set. \n\
    24. Send Config Default Ttl Get. \n\
    25. Send Config Gatt Proxy Set. \n\
    26. Send Config Gatt Proxy Get. \n\
    27. Send Config Friend Set. \n\
    28. Send Config Friend Get. \n\
    29. Send Config Relay Set. \n\
    30. Send Config Relay Get. \n\
    31. Send Config Model Publication Set. \n\
    32. Send Config Model Publication Virtual Address Set. \n\
    33. Send Config Model Publication Get. \n\
    34. Send Config Model Subscription Add. \n\
    35. Send Config Model Subscription Virtual Address Add. \n\
    36. Send Config Model Subscription Overwrite. \n\
    37. Send Config Model Subscription Virtual Address Overwrite. \n\
    38. Send Config Model Subscription Delete. \n\
    39. Send Config Model Subscription Virtual Address Delete. \n\
    40. Send Config Model Subscription Delete All. \n\
    41. Send Config Sig Model Subscription Get. \n\
    42. Send Config Vendor Model Subscription Get. \n\
    43. Send Config Netkey Add. \n\
    44. Send Config Netkey Update. \n\
    45. Send Config Netkey Delete. \n\
    46. Send Config Netkey Get. \n\
    47. Send Config Appkey Add. \n\
    48. Send Config Appkey Update. \n\
    49. Send Config Appkey Delete. \n\
    50. Send Config Appkey Get. \n\
    51. Send Config Model App Bind. \n\
    52. Send Config Model App Unbind. \n\
    53. Send Config Sig Model App Get. \n\
    54. Send Config Vendor Model App Get. \n\
    55. Send Config Node Identity Set. \n\
    56. Send Config Node Identity Get. \n\
    57. Send Config Node Reset. \n\
    58. Send Config Heartbeat Publication Set. \n\
    59. Send Config Heartbeat Publication Get. \n\
    60. Send Config Heartbeat Subscription Set. \n\
    61. Send Config Heartbeat Subscription Get. \n\
    62. Send Config Network Transmit Set. \n\
    63. Send Config Network Transmit Get. \n\
    64. Send Config Low Power Node Polltimeout Get. \n\
    65. Send Config Key Refresh Phase Set. \n\
    66. Send Config Key Refresh Phase Get. \n\
 \n\
   100. Get Model Handle. \n\
   101. Set Publish Address. \n\
 \n\
Your Option ? \0";


/* --------------------------------------------- External Global Variables */
static MS_ACCESS_MODEL_HANDLE   appl_config_client_model_handle;


/* --------------------------------------------- Function */
/* config client application entry point */
void main_config_client_operations(void)
{
    int choice;
    MS_ACCESS_ELEMENT_HANDLE element_handle;
    static UCHAR model_initialized = 0x00;

    /**
        Register with Access Layer.
    */
    if (0x00 == model_initialized)
    {
        API_RESULT retval;
        /* Use Default Element Handle. Index 0 */
        element_handle = MS_ACCESS_DEFAULT_ELEMENT_HANDLE;
        retval = MS_config_client_init
                 (
                     element_handle,
                     &appl_config_client_model_handle,
                     appl_config_client_cb
                 );

        if (API_SUCCESS == retval)
        {
            CONSOLE_OUT(
                "Configuration Client Initialized. Model Handle: 0x%04X\n",
                appl_config_client_model_handle);
        }
        else
        {
            CONSOLE_OUT(
                "[ERR] Configuration Client Initialization Failed. Result: 0x%04X\n",
                retval);
        }

        model_initialized = 0x01;
    }

    MS_LOOP_FOREVER()
    {
        CONSOLE_OUT
        ("%s", main_config_client_options);
        CONSOLE_IN
        ("%d", &choice);

        if (choice < 0)
        {
            CONSOLE_OUT
            ("*** Invalid Choice. Try Again.\n");
            continue;
        }

        switch (choice)
        {
        case 0:
            return;

        case 1:
            break;

        case 20: /* Send Config Beacon Set */
            appl_send_config_beacon_set();
            break;

        case 21: /* Send Config Beacon Get */
            appl_send_config_beacon_get();
            break;

        case 22: /* Send Config Composition Data Get */
            appl_send_config_composition_data_get();
            break;

        case 23: /* Send Config Default Ttl Set */
            appl_send_config_default_ttl_set();
            break;

        case 24: /* Send Config Default Ttl Get */
            appl_send_config_default_ttl_get();
            break;

        case 25: /* Send Config Gatt Proxy Set */
            appl_send_config_gatt_proxy_set();
            break;

        case 26: /* Send Config Gatt Proxy Get */
            appl_send_config_gatt_proxy_get();
            break;

        case 27: /* Send Config Friend Set */
            appl_send_config_friend_set();
            break;

        case 28: /* Send Config Friend Get */
            appl_send_config_friend_get();
            break;

        case 29: /* Send Config Relay Set */
            appl_send_config_relay_set();
            break;

        case 30: /* Send Config Relay Get */
            appl_send_config_relay_get();
            break;

        case 31: /* Send Config Model Publication Set */
            appl_send_config_model_publication_set();
            break;

        case 32: /* Send Config Model Publication Virtual Address Set */
            appl_send_config_model_publication_virtual_address_set();
            break;

        case 33: /* Send Config Model Publication Get */
            appl_send_config_model_publication_get();
            break;

        case 34: /* Send Config Model Subscription Add */
            appl_send_config_model_subscription_add();
            break;

        case 35: /* Send Config Model Subscription Virtual Address Add */
            appl_send_config_model_subscription_virtual_address_add();
            break;

        case 36: /* Send Config Model Subscription Overwrite */
            appl_send_config_model_subscription_overwrite();
            break;

        case 37: /* Send Config Model Subscription Virtual Address Overwrite */
            appl_send_config_model_subscription_virtual_address_overwrite();
            break;

        case 38: /* Send Config Model Subscription Delete */
            appl_send_config_model_subscription_delete();
            break;

        case 39: /* Send Config Model Subscription Virtual Address Delete */
            appl_send_config_model_subscription_virtual_address_delete();
            break;

        case 40: /* Send Config Model Subscription Delete All */
            appl_send_config_model_subscription_delete_all();
            break;

        case 41: /* Send Config Sig Model Subscription Get */
            appl_send_config_sig_model_subscription_get();
            break;

        case 42: /* Send Config Vendor Model Subscription Get */
            appl_send_config_vendor_model_subscription_get();
            break;

        case 43: /* Send Config Netkey Add */
            appl_send_config_netkey_add();
            break;

        case 44: /* Send Config Netkey Update */
            appl_send_config_netkey_update();
            break;

        case 45: /* Send Config Netkey Delete */
            appl_send_config_netkey_delete();
            break;

        case 46: /* Send Config Netkey Get */
            appl_send_config_netkey_get();
            break;

        case 47: /* Send Config Appkey Add */
            appl_send_config_appkey_add();
            break;

        case 48: /* Send Config Appkey Update */
            appl_send_config_appkey_update();
            break;

        case 49: /* Send Config Appkey Delete */
            appl_send_config_appkey_delete();
            break;

        case 50: /* Send Config Appkey Get */
            appl_send_config_appkey_get();
            break;

        case 51: /* Send Config Model App Bind */
            appl_send_config_model_app_bind();
            break;

        case 52: /* Send Config Model App Unbind */
            appl_send_config_model_app_unbind();
            break;

        case 53: /* Send Config Sig Model App Get */
            appl_send_config_sig_model_app_get();
            break;

        case 54: /* Send Config Vendor Model App Get */
            appl_send_config_vendor_model_app_get();
            break;

        case 55: /* Send Config Node Identity Set */
            appl_send_config_node_identity_set();
            break;

        case 56: /* Send Config Node Identity Get */
            appl_send_config_node_identity_get();
            break;

        case 57: /* Send Config Node Reset */
            appl_send_config_node_reset();
            break;

        case 58: /* Send Config Heartbeat Publication Set */
            appl_send_config_heartbeat_publication_set();
            break;

        case 59: /* Send Config Heartbeat Publication Get */
            appl_send_config_heartbeat_publication_get();
            break;

        case 60: /* Send Config Heartbeat Subscription Set */
            appl_send_config_heartbeat_subscription_set();
            break;

        case 61: /* Send Config Heartbeat Subscription Get */
            appl_send_config_heartbeat_subscription_get();
            break;

        case 62: /* Send Config Network Transmit Set */
            appl_send_config_network_transmit_set();
            break;

        case 63: /* Send Config Network Transmit Get */
            appl_send_config_network_transmit_get();
            break;

        case 64: /* Send Config Low Power Node Polltimeout Get */
            appl_send_config_low_power_node_polltimeout_get();
            break;

        case 65: /* Send Config Key Refresh Phase Set */
            appl_send_config_key_refresh_phase_set();
            break;

        case 66: /* Send Config Key Refresh Phase Get */
            appl_send_config_key_refresh_phase_get();
            break;

        case 100: /* Get Model Handle */
            appl_config_client_get_model_handle();
            break;

        case 101: /* Set Publish Address */
            appl_config_client_set_publish_address();
            break;
        }
    }
}

/* Send Config Model App Unbind */
void appl_send_config_model_app_unbind(void)
{
    API_RESULT retval;
    int  choice;
    ACCESS_CONFIG_MODEL_APP_UNBIND_PARAM  param;
    CONSOLE_OUT
    (">> Send Config Model App Unbind\n");
    CONSOLE_OUT
    ("Enter ElementAddress (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.element_address = (UINT16)choice;
    CONSOLE_OUT
    ("Enter AppKeyIndex (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.appkey_index = (UINT16)choice;
    CONSOLE_OUT
    ("Enter ModelIdentifier\n");
    CONSOLE_OUT
    ("Enter Model ID Type. (0 -> SIG, 1 -> Vendor\n");
    CONSOLE_IN
    ("%x", &choice);
    param.model.type = (UCHAR)choice;
    CONSOLE_OUT
    ("Enter Model ID.\n");
    CONSOLE_IN
    ("%x", &choice);
    param.model.id = (UINT32)choice;
    CONSOLE_OUT
    ("Enter Corresponding Local ModelIdentifier\n");
    CONSOLE_OUT
    ("Enter Model ID Type. (0 -> SIG, 1 -> Vendor\n");
    CONSOLE_IN
    ("%x", &choice);
    param.client_model.type = (UCHAR)choice;
    CONSOLE_OUT
    ("Enter Model ID.\n");
    CONSOLE_IN
    ("%x", &choice);
    param.client_model.id = (UINT32)choice;
    retval = MS_config_client_model_app_unbind(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Config Friend Get */
void appl_send_config_friend_get(void)
{
    API_RESULT retval;
    CONSOLE_OUT
    (">> Send Config Friend Get\n");
    retval = MS_config_client_friend_get();
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Config Gatt Proxy Get */
void appl_send_config_gatt_proxy_get(void)
{
    API_RESULT retval;
    CONSOLE_OUT
    (">> Send Config Gatt Proxy Get\n");
    retval = MS_config_client_gatt_proxy_get();
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Config Model Subscription Virtual Address Overwrite */
void appl_send_config_model_subscription_virtual_address_overwrite(void)
{
    API_RESULT retval;
    int  choice;
    ACCESS_CONFIG_MODELSUB_VADDR_OVERWRITE_PARAM  param;
    CONSOLE_OUT
    (">> Send Config Model Subscription Virtual Address Overwrite\n");
    CONSOLE_OUT
    ("Enter ElementAddress (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.element_address = (UINT16)choice;
    CONSOLE_OUT
    ("Enter Address (16-octets in HEX)\n");
    {
        UINT16 index;

        for(index = 0; index < 16; index++)
        {
            CONSOLE_IN
            ("%x", &choice);
            param.label[index] = (UCHAR)choice;
        }
    }
    CONSOLE_OUT
    ("Enter ModelIdentifier\n");
    CONSOLE_OUT
    ("Enter Model ID Type. (0 -> SIG, 1 -> Vendor\n");
    CONSOLE_IN
    ("%x", &choice);
    param.model.type = (UCHAR)choice;
    CONSOLE_OUT
    ("Enter Model ID.\n");
    CONSOLE_IN
    ("%x", &choice);
    param.model.id = (UINT32)choice;
    retval = MS_config_client_model_subscription_vaddr_overwrite(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Config Sig Model App Get */
void appl_send_config_sig_model_app_get(void)
{
    API_RESULT retval;
    int  choice;
    ACCESS_CONFIG_SIG_MODEL_APP_GET_PARAM  param;
    CONSOLE_OUT
    (">> Send Config Sig Model App Get\n");
    CONSOLE_OUT
    ("Enter ElementAddress (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.element_address = (UINT16)choice;
    CONSOLE_OUT
    ("Enter ModelIdentifier (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.model_id = (UINT16)choice;
    retval = MS_config_client_sig_model_app_get(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Config Model Subscription Add */
void appl_send_config_model_subscription_add(void)
{
    API_RESULT retval;
    int  choice;
    ACCESS_CONFIG_MODELSUB_ADD_PARAM  param;
    CONSOLE_OUT
    (">> Send Config Model Subscription Add\n");
    CONSOLE_OUT
    ("Enter ElementAddress (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.element_address= (UINT16)choice;
    CONSOLE_OUT
    ("Enter Address (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.address = (UINT16)choice;
    CONSOLE_OUT
    ("Enter ModelIdentifier\n");
    CONSOLE_OUT
    ("Enter Model ID Type. (0 -> SIG, 1 -> Vendor\n");
    CONSOLE_IN
    ("%x", &choice);
    param.model.type = (UCHAR)choice;
    CONSOLE_OUT
    ("Enter Model ID.\n");
    CONSOLE_IN
    ("%x", &choice);
    param.model.id = (UINT32)choice;
    retval = MS_config_client_model_subscription_add(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Config Network Transmit Set */
void appl_send_config_network_transmit_set(void)
{
    API_RESULT retval;
    int  choice;
    ACCESS_CONFIG_NETWORK_TRANSMIT_SET_PARAM  param;
    CONSOLE_OUT
    (">> Send Config Network Transmit Set\n");
    CONSOLE_OUT
    ("Enter NetWork Transmit Count (in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.net_tx_count = (UINT16)choice;
    CONSOLE_OUT
    ("Enter NetWork Transmit Interval Steps (in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.net_tx_interval_steps = (UINT16)choice;
    retval = MS_config_client_network_transmit_set(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Config Model Subscription Delete */
void appl_send_config_model_subscription_delete(void)
{
    API_RESULT retval;
    int  choice;
    ACCESS_CONFIG_MODELSUB_DEL_PARAM  param;
    CONSOLE_OUT
    (">> Send Config Model Subscription Delete\n");
    CONSOLE_OUT
    ("Enter ElementAddress (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.element_address = (UINT16)choice;
    CONSOLE_OUT
    ("Enter Address (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.address = (UINT16)choice;
    CONSOLE_OUT
    ("Enter ModelIdentifier\n");
    CONSOLE_OUT
    ("Enter Model ID Type. (0 -> SIG, 1 -> Vendor\n");
    CONSOLE_IN
    ("%x", &choice);
    param.model.type = (UCHAR)choice;
    CONSOLE_OUT
    ("Enter Model ID.\n");
    CONSOLE_IN
    ("%x", &choice);
    param.model.id = (UINT32)choice;
    retval = MS_config_client_model_subscription_delete(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Config Relay Get */
void appl_send_config_relay_get(void)
{
    API_RESULT retval;
    CONSOLE_OUT
    (">> Send Config Relay Get\n");
    retval = MS_config_client_relay_get();
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Config Heartbeat Subscription Set */
void appl_send_config_heartbeat_subscription_set(void)
{
    API_RESULT retval;
    int  choice;
    ACCESS_CONFIG_HEARTBEATSUB_SET_PARAM  param;
    CONSOLE_OUT
    (">> Send Config Heartbeat Subscription Set\n");
    CONSOLE_OUT
    ("Enter Source (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.source = (UINT16)choice;
    CONSOLE_OUT
    ("Enter Destination (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.destination = (UINT16)choice;
    CONSOLE_OUT
    ("Enter PeriodLog (8-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.periodlog = (UCHAR)choice;
    retval = MS_config_client_heartbeat_subscription_set(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Config Beacon Set */
void appl_send_config_beacon_set(void)
{
    API_RESULT retval;
    int  choice;
    ACCESS_CONFIG_BEACON_SET_PARAM  param;
    CONSOLE_OUT
    (">> Send Config Beacon Set\n");
    CONSOLE_OUT
    ("Enter Beacon (8-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.beacon = (UCHAR)choice;
    retval = MS_config_client_beacon_set(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Config Node Reset */
void appl_send_config_node_reset(void)
{
    API_RESULT retval;
    CONSOLE_OUT
    (">> Send Config Node Reset\n");
    retval = MS_config_client_node_reset();
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Config Relay Set */
void appl_send_config_relay_set(void)
{
    API_RESULT retval;
    int  choice;
    ACCESS_CONFIG_RELAY_SET_PARAM  param;
    CONSOLE_OUT
    (">> Send Config Relay Set\n");
    CONSOLE_OUT
    ("Enter Relay (8-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.relay = (UCHAR)choice;
    CONSOLE_OUT
    ("Enter RelayRetransmitCount (3-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.relay_rtx_count = (UCHAR)choice;
    CONSOLE_OUT
    ("Enter RelayRetransmitIntervalSteps (5-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.relay_rtx_interval_steps = (UCHAR)choice;
    retval = MS_config_client_relay_set(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Config Key Refresh Phase Get */
void appl_send_config_key_refresh_phase_get(void)
{
    API_RESULT retval;
    int  choice;
    ACCESS_CONFIG_KEYREFRESH_PHASE_GET_PARAM  param;
    CONSOLE_OUT
    (">> Send Config Key Refresh Phase Get\n");
    CONSOLE_OUT
    ("Enter NetKeyIndex (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.netkey_index = (UINT16)choice;
    retval = MS_config_client_keyrefresh_phase_get(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Config Model Subscription Virtual Address Delete */
void appl_send_config_model_subscription_virtual_address_delete(void)
{
    API_RESULT retval;
    int  choice;
    ACCESS_CONFIG_MODELSUB_VADDR_DEL_PARAM  param;
    CONSOLE_OUT
    (">> Send Config Model Subscription Virtual Address Delete\n");
    CONSOLE_OUT
    ("Enter ElementAddress (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.element_address = (UINT16)choice;
    CONSOLE_OUT
    ("Enter Label (16-octets in HEX)\n");
    {
        UINT16 index;

        for(index = 0; index < 16; index++)
        {
            CONSOLE_IN
            ("%x", &choice);
            param.label[index] = (UCHAR)choice;
        }
    }
    CONSOLE_OUT
    ("Enter ModelIdentifier\n");
    CONSOLE_OUT
    ("Enter Model ID Type. (0 -> SIG, 1 -> Vendor\n");
    CONSOLE_IN
    ("%x", &choice);
    param.model.type = (UCHAR)choice;
    CONSOLE_OUT
    ("Enter Model ID.\n");
    CONSOLE_IN
    ("%x", &choice);
    param.model.id = (UINT32)choice;
    retval = MS_config_client_model_subscription_vaddr_delete(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Config Model Publication Virtual Address Set */
void appl_send_config_model_publication_virtual_address_set(void)
{
    API_RESULT retval;
    int  choice;
    ACCESS_CONFIG_MODELPUB_VADDR_SET_PARAM  param;
    CONSOLE_OUT
    (">> Send Config Model Publication Virtual Address Set\n");
    CONSOLE_OUT
    ("Enter ElementAddress (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.element_address = (UINT16)choice;
    CONSOLE_OUT
    ("Enter PublishAddress (16-octets in HEX)\n");
    {
        UINT16 index;

        for(index = 0; index < 16; index++)
        {
            CONSOLE_IN
            ("%x", &choice);
            param.publish_address[index] = (UCHAR)choice;
        }
    }
    CONSOLE_OUT
    ("Enter AppKeyIndex (12-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.appkey_index = (UINT16)choice;
    CONSOLE_OUT
    ("Enter CredentialFlag (1-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.credential_flag = (UCHAR)choice;
    CONSOLE_OUT
    ("Enter PublishTTL (8-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.publish_ttl = (UCHAR)choice;
    CONSOLE_OUT
    ("Enter PublishPeriod (8-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.publish_period = (UCHAR)choice;
    CONSOLE_OUT
    ("Enter PublishRetransmitCount (3-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.publish_rtx_count = (UCHAR)choice;
    CONSOLE_OUT
    ("Enter PublishRetransmitIntervalSteps (5-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.publish_rtx_interval_steps = (UCHAR)choice;
    CONSOLE_OUT
    ("Enter ModelIdentifier\n");
    CONSOLE_OUT
    ("Enter Model ID Type. (0 -> SIG, 1 -> Vendor\n");
    CONSOLE_IN
    ("%x", &choice);
    param.model.type = (UCHAR)choice;
    CONSOLE_OUT
    ("Enter Model ID.\n");
    CONSOLE_IN
    ("%x", &choice);
    param.model.id = (UINT32)choice;
    retval = MS_config_client_model_publication_vaddr_set(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Config Gatt Proxy Set */
void appl_send_config_gatt_proxy_set(void)
{
    API_RESULT retval;
    int  choice;
    ACCESS_CONFIG_GATT_PROXY_SET_PARAM  param;
    CONSOLE_OUT
    (">> Send Config Gatt Proxy Set\n");
    CONSOLE_OUT
    ("Enter GATTProxy (8-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.proxy = (UCHAR)choice;
    retval = MS_config_client_gatt_proxy_set(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Config Model Subscription Virtual Address Add */
void appl_send_config_model_subscription_virtual_address_add(void)
{
    API_RESULT retval;
    int  choice;
    ACCESS_CONFIG_MODELSUB_VADDR_ADD_PARAM  param;
    CONSOLE_OUT
    (">> Send Config Model Subscription Virtual Address Add\n");
    CONSOLE_OUT
    ("Enter ElementAddress (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.element_address = (UINT16)choice;
    CONSOLE_OUT
    ("Enter Label (16-octets in HEX)\n");
    {
        UINT16 index;

        for(index = 0; index < 16; index++)
        {
            CONSOLE_IN
            ("%x", &choice);
            param.label[index] = (UCHAR)choice;
        }
    }
    CONSOLE_OUT
    ("Enter ModelIdentifier\n");
    CONSOLE_OUT
    ("Enter Model ID Type. (0 -> SIG, 1 -> Vendor\n");
    CONSOLE_IN
    ("%x", &choice);
    param.model.type = (UCHAR)choice;
    CONSOLE_OUT
    ("Enter Model ID.\n");
    CONSOLE_IN
    ("%x", &choice);
    param.model.id = (UINT32)choice;
    retval = MS_config_client_model_subscription_vaddr_add(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Config Default Ttl Get */
void appl_send_config_default_ttl_get(void)
{
    API_RESULT retval;
    CONSOLE_OUT
    (">> Send Config Default Ttl Get\n");
    retval = MS_config_client_default_ttl_get();
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Config Model App Bind */
void appl_send_config_model_app_bind(void)
{
    API_RESULT retval;
    int  choice;
    ACCESS_CONFIG_MODEL_APP_BIND_PARAM  param;
    CONSOLE_OUT
    (">> Send Config Model App Bind\n");
    CONSOLE_OUT
    ("Enter ElementAddress (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.element_address = (UINT16)choice;
    CONSOLE_OUT
    ("Enter AppKeyIndex (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.appkey_index = (UINT16)choice;
    CONSOLE_OUT
    ("Enter ModelIdentifier\n");
    CONSOLE_OUT
    ("Enter Model ID Type. (0 -> SIG, 1 -> Vendor\n");
    CONSOLE_IN
    ("%x", &choice);
    param.model.type = (UCHAR)choice;
    CONSOLE_OUT
    ("Enter Model ID.\n");
    CONSOLE_IN
    ("%x", &choice);
    param.model.id = (UINT32)choice;
    CONSOLE_OUT
    ("Enter Corresponding Local ModelIdentifier\n");
    CONSOLE_OUT
    ("Enter Model ID Type. (0 -> SIG, 1 -> Vendor\n");
    CONSOLE_IN
    ("%x", &choice);
    param.client_model.type = (UCHAR)choice;
    CONSOLE_OUT
    ("Enter Model ID.\n");
    CONSOLE_IN
    ("%x", &choice);
    param.client_model.id = (UINT32)choice;
    retval = MS_config_client_model_app_bind(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Config Node Identity Set */
void appl_send_config_node_identity_set(void)
{
    API_RESULT retval;
    int  choice;
    ACCESS_CONFIG_NODEID_SET_PARAM  param;
    CONSOLE_OUT
    (">> Send Config Node Identity Set\n");
    CONSOLE_OUT
    ("Enter NetKeyIndex (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.netkey_index = (UINT16)choice;
    CONSOLE_OUT
    ("Enter Identity (8-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.identity = (UCHAR)choice;
    retval = MS_config_client_node_identity_set(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Config Sig Model Subscription Get */
void appl_send_config_sig_model_subscription_get(void)
{
    API_RESULT retval;
    int  choice;
    ACCESS_CONFIG_SIGMODELSUB_GET_PARAM  param;
    CONSOLE_OUT
    (">> Send Config Sig Model Subscription Get\n");
    CONSOLE_OUT
    ("Enter ElementAddress (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.element_address = (UINT16)choice;
    CONSOLE_OUT
    ("Enter ModelIdentifier (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.model_id = (UINT16)choice;
    retval = MS_config_client_sig_model_subscription_get(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Config Netkey Add */
void appl_send_config_netkey_add(void)
{
    API_RESULT retval;
    int  choice;
    ACCESS_CONFIG_NETKEY_ADD_PARAM  param;
    CONSOLE_OUT
    (">> Send Config Netkey Add\n");
    CONSOLE_OUT
    ("Enter NetKeyIndex (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.netkey_index = (UINT16)choice;
    CONSOLE_OUT
    ("Enter NetKey (16-octets in HEX)\n");
    {
        UINT16 index;

        for(index = 0; index < 16; index++)
        {
            CONSOLE_IN
            ("%x", &choice);
            param.netkey[index] = (UCHAR)choice;
        }
    }
    retval = MS_config_client_netkey_add(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Config Appkey Update */
void appl_send_config_appkey_update(void)
{
    API_RESULT retval;
    int  choice;
    ACCESS_CONFIG_APPKEY_UPDATE_PARAM  param;
    CONSOLE_OUT
    (">> Send Config Appkey Update\n");
    CONSOLE_OUT
    ("Enter NetKeyIndex (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.netkey_index = (UINT16)choice;
    CONSOLE_OUT
    ("Enter AppKeyIndex (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.appkey_index = (UINT16)choice;
    CONSOLE_OUT
    ("Enter AppKey (16-octets in HEX)\n");
    {
        UINT16 index;

        for(index = 0; index < 16; index++)
        {
            CONSOLE_IN
            ("%x", &choice);
            param.appkey[index] = (UCHAR)choice;
        }
    }
    retval = MS_config_client_appkey_update(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Config Heartbeat Subscription Get */
void appl_send_config_heartbeat_subscription_get(void)
{
    API_RESULT retval;
    CONSOLE_OUT
    (">> Send Config Heartbeat Subscription Get\n");
    retval = MS_config_client_heartbeat_subscription_get();
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Config Vendor Model Subscription Get */
void appl_send_config_vendor_model_subscription_get(void)
{
    API_RESULT retval;
    int  choice;
    ACCESS_CONFIG_VENDORMODELSUB_GET_PARAM  param;
    CONSOLE_OUT
    (">> Send Config Vendor Model Subscription Get\n");
    CONSOLE_OUT
    ("Enter ElementAddress (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.element_address= (UINT16)choice;
    CONSOLE_OUT
    ("Enter ModelIdentifier (32-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.model_id = (UINT32)choice;
    retval = MS_config_client_vendor_model_subscription_get(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Config Vendor Model App Get */
void appl_send_config_vendor_model_app_get(void)
{
    API_RESULT retval;
    int  choice;
    ACCESS_CONFIG_VENDOR_MODEL_APP_GET_PARAM  param;
    CONSOLE_OUT
    (">> Send Config Vendor Model App Get\n");
    CONSOLE_OUT
    ("Enter ElementAddress (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.element_address= (UINT16)choice;
    CONSOLE_OUT
    ("Enter ModelIdentifier (32-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.model_id = (UINT32)choice;
    retval = MS_config_client_vendor_model_app_get(&param);
}

/* Send Config Model Subscription Overwrite */
void appl_send_config_model_subscription_overwrite(void)
{
    API_RESULT retval;
    int  choice;
    ACCESS_CONFIG_MODELSUB_OVERWRITE_PARAM  param;
    CONSOLE_OUT
    (">> Send Config Model Subscription Overwrite\n");
    CONSOLE_OUT
    ("Enter ElementAddress (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.element_address = (UINT16)choice;
    CONSOLE_OUT
    ("Enter Address (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.address = (UINT16)choice;
    CONSOLE_OUT
    ("Enter ModelIdentifier\n");
    CONSOLE_OUT
    ("Enter Model ID Type. (0 -> SIG, 1 -> Vendor\n");
    CONSOLE_IN
    ("%x", &choice);
    param.model.type = (UCHAR)choice;
    CONSOLE_OUT
    ("Enter Model ID.\n");
    CONSOLE_IN
    ("%x", &choice);
    param.model.id = (UINT32)choice;
    retval = MS_config_client_model_subscription_overwrite(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Config Beacon Get */
void appl_send_config_beacon_get(void)
{
    API_RESULT retval;
    CONSOLE_OUT
    (">> Send Config Beacon Get\n");
    retval = MS_config_client_beacon_get();
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Config Model Subscription Delete All */
void appl_send_config_model_subscription_delete_all(void)
{
    API_RESULT retval;
    int  choice;
    ACCESS_CONFIG_MODELSUB_DELETEALL_PARAM  param;
    CONSOLE_OUT
    (">> Send Config Model Subscription Delete All\n");
    CONSOLE_OUT
    ("Enter ElementAddress (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.element_address = (UINT16)choice;
    CONSOLE_OUT
    ("Enter ModelIdentifier\n");
    CONSOLE_OUT
    ("Enter Model ID Type. (0 -> SIG, 1 -> Vendor\n");
    CONSOLE_IN
    ("%x", &choice);
    param.model.type = (UCHAR)choice;
    CONSOLE_OUT
    ("Enter Model ID.\n");
    CONSOLE_IN
    ("%x", &choice);
    param.model.id = (UINT32)choice;
    retval = MS_config_client_model_subscription_delete_all(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Config Netkey Delete */
void appl_send_config_netkey_delete(void)
{
    API_RESULT retval;
    int  choice;
    ACCESS_CONFIG_NETKEY_DELETE_PARAM  param;
    CONSOLE_OUT
    (">> Send Config Netkey Delete\n");
    CONSOLE_OUT
    ("Enter NetKeyIndex (12-bit in Hex)\n");
    CONSOLE_IN
    ("%d", &choice);
    param.netkey_index = (UINT16)choice;
    retval = MS_config_client_netkey_delete(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Config Friend Set */
void appl_send_config_friend_set(void)
{
    API_RESULT retval;
    int  choice;
    ACCESS_CONFIG_FRIEND_SET_PARAM  param;
    CONSOLE_OUT
    (">> Send Config Friend Set\n");
    CONSOLE_OUT
    ("Enter Friend (8-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.friend = (UCHAR)choice;
    retval = MS_config_client_friend_set(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Config Heartbeat Publication Set */
void appl_send_config_heartbeat_publication_set(void)
{
    API_RESULT retval;
    int  choice;
    ACCESS_CONFIG_HEARTBEATPUB_SET_PARAM  param;
    CONSOLE_OUT
    (">> Send Config Heartbeat Publication Set\n");
    CONSOLE_OUT
    ("Enter Destination (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.destination = (UINT16)choice;
    CONSOLE_OUT
    ("Enter CountLog (8-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.countlog = (UCHAR)choice;
    CONSOLE_OUT
    ("Enter PeriodLog (8-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.periodlog = (UCHAR)choice;
    CONSOLE_OUT
    ("Enter TTL (8-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.ttl = (UCHAR)choice;
    CONSOLE_OUT
    ("Enter Features (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.features = (UINT16)choice;
    CONSOLE_OUT
    ("Enter NetKeyIndex (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.netkey_index = (UINT16)choice;
    retval = MS_config_client_heartbeat_publication_set(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Config Node Identity Get */
void appl_send_config_node_identity_get(void)
{
    API_RESULT retval;
    int  choice;
    ACCESS_CONFIG_NODEID_GET_PARAM  param;
    CONSOLE_OUT
    (">> Send Config Node Identity Get\n");
    CONSOLE_OUT
    ("Enter NetKeyIndex (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.netkey_index = (UINT16)choice;
    retval = MS_config_client_node_identity_get(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Config Default Ttl Set */
void appl_send_config_default_ttl_set(void)
{
    API_RESULT retval;
    int  choice;
    ACCESS_CONFIG_DEFAULT_TTL_SET_PARAM  param;
    CONSOLE_OUT
    (">> Send Config Default Ttl Set\n");
    CONSOLE_OUT
    ("Enter TTL (8-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.ttl = (UCHAR)choice;
    retval = MS_config_client_default_ttl_set(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Config Netkey Update */
void appl_send_config_netkey_update(void)
{
    API_RESULT retval;
    int  choice;
    ACCESS_CONFIG_NETKEY_UPDATE_PARAM  param;
    CONSOLE_OUT
    (">> Send Config Netkey Update\n");
    CONSOLE_OUT
    ("Enter NetKeyIndex (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.netkey_index = (UINT16)choice;
    CONSOLE_OUT
    ("Enter NetKey (16-octets in HEX)\n");
    {
        UINT16 index;

        for(index = 0; index < 16; index++)
        {
            CONSOLE_IN
            ("%x", &choice);
            param.netkey[index] = (UCHAR)choice;
        }
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
}

/* Send Config Heartbeat Publication Get */
void appl_send_config_heartbeat_publication_get(void)
{
    API_RESULT retval;
    CONSOLE_OUT
    (">> Send Config Heartbeat Publication Get\n");
    retval = MS_config_client_heartbeat_publication_get();
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Config Model Publication Set */
void appl_send_config_model_publication_set(void)
{
    API_RESULT retval;
    int  choice;
    ACCESS_CONFIG_MODELPUB_SET_PARAM  param;
    CONSOLE_OUT
    (">> Send Config Model Publication Set\n");
    CONSOLE_OUT
    ("Enter ElementAddress (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.element_address = (UINT16)choice;
    CONSOLE_OUT
    ("Enter PublishAddress (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.publish_address = (UINT16)choice;
    CONSOLE_OUT
    ("Enter AppKeyIndex (12-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.appkey_index = (UINT16)choice;
    CONSOLE_OUT
    ("Enter CredentialFlag (1-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.credential_flag = (UCHAR)choice;
    CONSOLE_OUT
    ("Enter PublishTTL (8-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.publish_ttl = (UCHAR)choice;
    CONSOLE_OUT
    ("Enter PublishPeriod (8-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.publish_period = (UCHAR)choice;
    CONSOLE_OUT
    ("Enter PublishRetransmitCount (3-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.publish_rtx_count = (UCHAR)choice;
    CONSOLE_OUT
    ("Enter PublishRetransmitIntervalSteps (5-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.publish_rtx_interval_steps = (UCHAR)choice;
    CONSOLE_OUT
    ("Enter ModelIdentifier\n");
    CONSOLE_OUT
    ("Enter Model ID Type. (0 -> SIG, 1 -> Vendor\n");
    CONSOLE_IN
    ("%x", &choice);
    param.model.type = (UCHAR)choice;
    CONSOLE_OUT
    ("Enter Model ID.\n");
    CONSOLE_IN
    ("%x", &choice);
    param.model.id = (UINT32)choice;
    retval = MS_config_client_model_publication_set(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Config Netkey Get */
void appl_send_config_netkey_get(void)
{
    API_RESULT retval;
    CONSOLE_OUT
    (">> Send Config Netkey Get\n");
    retval = MS_config_client_netkey_get();
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Config Model Publication Get */
void appl_send_config_model_publication_get(void)
{
    API_RESULT retval;
    int  choice;
    ACCESS_CONFIG_MODELPUB_GET_PARAM  param;
    CONSOLE_OUT
    (">> Send Config Model Publication Get\n");
    CONSOLE_OUT
    ("Enter ElementAddress (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.element_address = (UINT16)choice;
    CONSOLE_OUT
    ("Enter ModelIdentifier\n");
    CONSOLE_OUT
    ("Enter Model ID Type. (0 -> SIG, 1 -> Vendor\n");
    CONSOLE_IN
    ("%x", &choice);
    param.model.type = (UCHAR)choice;
    CONSOLE_OUT
    ("Enter Model ID.\n");
    CONSOLE_IN
    ("%x", &choice);
    param.model.id = (UINT32)choice;
    retval = MS_config_client_model_publication_get(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Config Network Transmit Get */
void appl_send_config_network_transmit_get(void)
{
    API_RESULT retval;
    CONSOLE_OUT
    (">> Send Config Network Transmit Get\n");
    retval = MS_config_client_network_transmit_get();
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Config Composition Data Get */
void appl_send_config_composition_data_get(void)
{
    API_RESULT retval;
    int  choice;
    ACCESS_CONFIG_COMPDATA_GET_PARAM  param;
    CONSOLE_OUT
    (">> Send Config Composition Data Get\n");
    CONSOLE_OUT
    ("Enter Page (8-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.page = (UCHAR)choice;
    retval = MS_config_client_composition_data_get(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Config Appkey Add */
void appl_send_config_appkey_add(void)
{
    API_RESULT retval;
    int  choice;
    ACCESS_CONFIG_APPKEY_ADD_PARAM  param;
    CONSOLE_OUT
    (">> Send Config Appkey Add\n");
    CONSOLE_OUT
    ("Enter NetKeyIndex (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.netkey_index = (UINT16)choice;
    CONSOLE_OUT
    ("Enter AppKeyIndex (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.appkey_index = (UINT16)choice;
    CONSOLE_OUT
    ("Enter AppKey (16-octets in HEX)\n");
    {
        UINT16 index;

        for(index = 0; index < 16; index++)
        {
            CONSOLE_IN
            ("%x", &choice);
            param.appkey[index] = (UCHAR)choice;
        }
    }
    retval = MS_config_client_appkey_add(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Config Key Refresh Phase Set */
void appl_send_config_key_refresh_phase_set(void)
{
    API_RESULT retval;
    int  choice;
    ACCESS_CONFIG_KEYREFRESH_PHASE_SET_PARAM  param;
    CONSOLE_OUT
    (">> Send Config Key Refresh Phase Set\n");
    CONSOLE_OUT
    ("Enter NetKeyIndex (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.netkey_index = (UINT16)choice;
    CONSOLE_OUT
    ("Enter Transition (8-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.transition = (UCHAR)choice;
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
}

/* Send Config Appkey Get */
void appl_send_config_appkey_get(void)
{
    API_RESULT retval;
    int  choice;
    ACCESS_CONFIG_APPKEY_GET_PARAM  param;
    CONSOLE_OUT
    (">> Send Config Appkey Get\n");
    CONSOLE_OUT
    ("Enter NetKeyIndex (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.netkey_index = (UINT16)choice;
    retval = MS_config_client_appkey_get(&param);
}

/* Send Config Appkey Delete */
void appl_send_config_appkey_delete(void)
{
    API_RESULT retval;
    int  choice;
    ACCESS_CONFIG_APPKEY_DELETE_PARAM  param;
    CONSOLE_OUT
    (">> Send Config Appkey Delete\n");
    CONSOLE_OUT
    ("Enter NetKeyIndex (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.netkey_index = (UINT16)choice;
    CONSOLE_OUT
    ("Enter AppKeyIndex (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.appkey_index = (UINT16)choice;
    retval = MS_config_client_appkey_delete(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Config Low Power Node Polltimeout Get */
void appl_send_config_low_power_node_polltimeout_get(void)
{
    API_RESULT retval;
    int  choice;
    ACCESS_CONFIG_LPNPOLLTIMEOUT_GET_PARAM  param;
    CONSOLE_OUT
    (">> Send Config Low Power Node Polltimeout Get\n");
    CONSOLE_OUT
    ("Enter LPNAddress (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.lpn_address = (UINT16)choice;
    retval = MS_config_client_lpn_polltimeout_get(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}


/* Get Model Handle */
void appl_config_client_get_model_handle(void)
{
    #if 0
    API_RESULT retval;
    MS_ACCESS_MODEL_HANDLE model_handle;
    retval = MS_config_client_get_model_handle(&model_handle);

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

    #endif /* 0 */
    return;
}


/* Set Publish Address */
void appl_config_client_set_publish_address(void)
{
    int  choice;
    API_RESULT retval;
    MS_ACCESS_MODEL_HANDLE model_handle;
    MS_ACCESS_PUBLISH_INFO  publish_info;
    /* Set Publish Information */
    EM_mem_set(&publish_info, 0, sizeof(publish_info));
    publish_info.addr.use_label = MS_FALSE;
    #if 0
    CONSOLE_OUT
    ("Enter Model Handle (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    model_handle = (UINT16)choice;
    #else
    model_handle = appl_config_client_model_handle;
    #endif /* 0 */
    CONSOLE_OUT
    ("Enter Configuration client Server Address (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    publish_info.addr.addr = (UINT16)choice;
    #if 0
    CONSOLE_OUT
    ("Enter AppKey Index\n");
    CONSOLE_IN
    ("%x", &choice);
    publish_info.appkey_index = choice;
    #else
    publish_info.appkey_index = MS_CONFIG_LIMITS(MS_MAX_APPS);
    #endif /* 0 */
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

    return;
}

/* Decode and print composition data */
#ifdef APPL_CONFIG_CLIENT_PRINT_COMP_DATA
void appl_config_client_print_composition_data
(
    /* IN */ UCHAR* comp_data,
    /* IN */ UINT16  data_len
)
{
    UINT16  marker;
    UINT16  u16_val, elem_index;
    marker = 0;
    CONSOLE_OUT("[Composition Data]\n");
    appl_dump_bytes(comp_data, data_len);
    /* Page 0 */
    CONSOLE_OUT("Page 0: 0x%02X\n", comp_data[marker]);
    marker++;
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
    elem_index = 0;

    while (marker < data_len)
    {
        UINT32 u32_val;
        UINT16 nums, numv, index;
        /* LOC */
        MS_UNPACK_LE_2_BYTE(&u16_val, &comp_data[marker]);
        CONSOLE_OUT("LOC: 0x%04X\n", u16_val);
        marker += 2;
        /* NumS */
        MS_UNPACK_LE_2_BYTE(&nums, &comp_data[marker]);
        CONSOLE_OUT("NumS: 0x%04X\n", nums);
        marker += 2;
        /* NumV */
        MS_UNPACK_LE_2_BYTE(&numv, &comp_data[marker]);
        CONSOLE_OUT("NumV: 0x%04X\n", numv);
        marker += 2;

        /* Print SIG Model IDs */
        for (index = 0; index < nums; index++)
        {
            MS_UNPACK_LE_2_BYTE(&u16_val, &comp_data[marker]);
            CONSOLE_OUT("SIG Model[%d]: 0x%04X\n", index, u16_val);
            marker += 2;
        }

        /* Print Vendor Model IDs */
        for (index = 0; index < numv; index++)
        {
            MS_UNPACK_LE_4_BYTE(&u32_val, &comp_data[marker]);
            CONSOLE_OUT("Vendor Model[%d]: 0x%08X\n", index, u32_val);
            marker += 4;
        }

        elem_index++;
        continue;
    }

    return;
}
#endif /* APPL_CONFIG_CLIENT_PRINT_COMP_DATA */

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
)
{
    API_RESULT retval;
    retval = API_SUCCESS;

//    CONSOLE_OUT (
//    "[CONFIG_CLIENT] Callback. Opcode 0x%04X\n", opcode);

//    appl_dump_bytes(data_param, data_len);

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

