
/**
    \file appl_proxy.c


*/

/*
    Copyright (C) 2017. Mindtree Limited.
    All rights reserved.
*/

/* --------------------------------------------- Header File Inclusion */
#include "appl_main.h"
#include "blebrr.h"
#include "MS_net_api.h"

#ifdef MS_PROXY_SUPPORT
/* --------------------------------------------- External Global Variables */

/* --------------------------------------------- Exported Global Variables */

/* --------------------------------------------- Static Global Variables */
DECL_STATIC const char main_proxy_options[] = "\n\
========= Network Menu =============== \n\
    0.  Exit. \n\
    1.  Refresh \n\
 \n\
   10.  Register with Proxy Layer \n\
 \n\
   20.  Start Proxy ADV with Network ID \n\
   21.  Start Proxy ADV with Node Identity \n\
   22.  Stop Proxy ADV \n\
 \n\
   30.  Set WhiteList Filter Type \n\
   31.  Set BlackList Filter Type \n\
   32.  Add Address to Filter \n\
   33.  Remove Address from Filter \n\
 \n\
  200.  Enable Proxy \n\
  201.  Disable Proxy \n\
Your Option ? \0";

/* Global Network Interface Handle : Initialize to MAX Ifaces[Invalid Value] */
NETIF_HANDLE  g_appl_netif_hndl;

/* --------------------------------------------- Functions */
void appl_proxy_callback
(
    NETIF_HANDLE*        handle,
    UCHAR                p_evt,
    UCHAR*               data_param,
    UINT16               data_len
);
void appl_add_or_del_filter_addr(UCHAR opcode);
void appl_proxy_start_net_id_adv(MS_SUBNET_HANDLE subnet_handle);
void appl_proxy_start_node_identity_adv(MS_SUBNET_HANDLE subnet_handle);
void main_proxy_operations (void);

void main_proxy_operations (void)
{
    int choice;
    static UINT8 first_time = 0;

    if (0 == first_time)
    {
        g_appl_netif_hndl = (NETIF_HANDLE)MS_CONFIG_LIMITS(MS_NUM_NETWORK_INTERFACES);
        first_time = 1;
    }

    MS_LOOP_FOREVER()
    {
        printf("%s", main_proxy_options);
        scanf("%d", &choice);

        if (choice < 0)
        {
            printf("*** Invalid Choice. Try Again.\n");
            continue;
        }

        switch (choice)
        {
        case 0:
            return;

        case 1:
            break;

        case 10:
            /* Register with the Proxy Layer */
            MS_proxy_register(appl_proxy_callback);
            break;

        case 20:
            #ifdef MS_PROXY_SERVER
            /* Start Proxy ADV with Network ID */
            printf("\nEnter Subnet Handle :\n");
            scanf("%d", &choice);
            appl_proxy_start_net_id_adv((MS_SUBNET_HANDLE)choice);
            #else /* MS_PROXY_SERVER */
            CONSOLE_OUT(
                "Proxy Server Feature Currently Disabled!!!\n");
            #endif /* MS_PROXY_SERVER */
            break;

        case 21:
            #ifdef MS_PROXY_SERVER
            /* Start Proxy ADV with Node Identity */
            printf("\nEnter Subnet Handle :\n");
            scanf("%d", &choice);
            appl_proxy_start_node_identity_adv((MS_SUBNET_HANDLE)choice);
            #else /* MS_PROXY_SERVER */
            CONSOLE_OUT(
                "Proxy Server Feature Currently Disabled!!!\n");
            #endif /* MS_PROXY_SERVER */
            break;

        case 22:
            #ifdef MS_PROXY_SERVER
            /* Stop Proxy ADV */
            MS_proxy_server_adv_stop();
            #else /* MS_PROXY_SERVER */
            CONSOLE_OUT(
                "Proxy Server Feature Currently Disabled!!!\n");
            #endif /* MS_PROXY_SERVER */
            break;

        case 30:
            #ifdef MS_PROXY_CLIENT
            /* Set Filter Type as WhiteList */
            MS_proxy_set_whitelist_filter(&g_appl_netif_hndl, 0x0000);
            #else /* MS_PROXY_CLIENT */
            CONSOLE_OUT(
                "Proxy Client Feature Currently Disabled!!!\n");
            #endif /* MS_PROXY_CLIENT */
            break;

        case 31:
            #ifdef MS_PROXY_CLIENT
            /* Set Filter Type as BlackList */
            MS_proxy_set_blacklist_filter(&g_appl_netif_hndl, 0x0000);
            #else /* MS_PROXY_CLIENT */
            CONSOLE_OUT(
                "Proxy Client Feature Currently Disabled!!!\n");
            #endif /* MS_PROXY_CLIENT */
            break;

        case 32:
            /* Add Address to Filters */
            appl_add_or_del_filter_addr(MS_PROXY_ADD_TO_FILTER_OPCODE);
            break;

        case 33:
            /* Remove Address from Filters */
            appl_add_or_del_filter_addr(MS_PROXY_REM_FROM_FILTER_OPCODE);
            break;

        case 200:
            /* Enable Proxy */
            MS_ENABLE_PROXY_FEATURE();
            break;

        case 201:
            /* Disable Proxy */
            MS_DISABLE_PROXY_FEATURE();
            break;

        default:
            printf ("Invalid option %d\n", choice);
        }
    }
}

void appl_proxy_callback
(
    NETIF_HANDLE*        handle,
    UCHAR                p_evt,
    UCHAR*               data_param,
    UINT16               data_len
)
{
    PROXY_FILTER_TYPE filter_type;
    UINT16            count;
    UCHAR             role;
    MS_IGNORE_UNUSED_PARAM(data_len);

    switch(p_evt)
    {
    case MS_PROXY_UP_EVENT:
        CONSOLE_OUT(
            "\n\n[PROXY APPL]: MS_PROXY_UP_EVENT Received for NETIF Handle 0x%02X\n\n", *handle);
        g_appl_netif_hndl = *handle;
        CONSOLE_OUT(
            "\n[PROXY APPL]: Enabling Proxy Feature!!\n");

        if (NULL != data_param)
        {
            /* Catch the current role into a local */
            role = data_param[0];

            if (BRR_SERVER_ROLE == role)
            {
                /* Enable Proxy */
                MS_ENABLE_PROXY_FEATURE();
                /* Send Secure Network Beacons */
                MS_net_broadcast_secure_beacon(0x0000);
            }
        }

        break;

    case MS_PROXY_DOWN_EVENT:
        CONSOLE_OUT(
            "\n\n[PROXY APPL]: MS_PROXY_DOWN_EVENT Received for NETIF Handle 0x%02X\n\n", *handle);
        g_appl_netif_hndl = (NETIF_HANDLE)MS_CONFIG_LIMITS(MS_NUM_NETWORK_INTERFACES);
        CONSOLE_OUT(
            "\n[PROXY APPL]: Disabling Proxy Feature!!\n");
        /* Disable Proxy */
        MS_DISABLE_PROXY_FEATURE();
        break;

    case MS_PROXY_STATUS_EVENT:
        /* TODO: Length Check */
        /* Extract the Status contents */
        filter_type = data_param[0];
        MS_UNPACK_BE_2_BYTE(&count, &data_param[1]);
        CONSOLE_OUT(
            "\n\n[PROXY APPL]: MS_PROXY_STATUS_EVENT Received for NETIF Handle 0x%02X\n"
            "Filter Type :- %s\r\n Addr Count :- %04d\n\n",
            *handle,
            (MS_PROXY_WHITELIST_FILTER == filter_type)? "WhiteList":
            "BlackList",
            count);
        break;

    default:
        CONSOLE_OUT(
            "\n\n[PROXY APPL ERR]: Unknown Event Received for NETIF Handle 0x%02X!!\n\n", *handle);
        break;
    }
}

void appl_add_or_del_filter_addr(UCHAR opcode)
{
    #ifdef MS_PROXY_CLIENT
    PROXY_ADDR appl_proxy_addr_list[5];
    UINT16     addr_count;
    UINT32     index;
    int        read_in;
    /* Read and fill Addresses */
    CONSOLE_OUT("Enter Count of Address to be Added (in Hex)\n");
    CONSOLE_IN("%x", &read_in);
    addr_count = (UINT16)read_in;

    for (index = 0; index < addr_count; index++)
    {
        CONSOLE_OUT("Enter %d'th Address[in HEX]: \n", (index + 1));
        CONSOLE_IN("%x", &read_in);
        appl_proxy_addr_list[index] = (UINT8)read_in;
    }

    if (MS_PROXY_ADD_TO_FILTER_OPCODE == opcode)
    {
        MS_proxy_add_to_list
        (
            &g_appl_netif_hndl,
            0x0000,
            appl_proxy_addr_list,
            addr_count
        );
    }
    else
    {
        MS_proxy_del_from_list
        (
            &g_appl_netif_hndl,
            0x0000,
            appl_proxy_addr_list,
            addr_count
        );
    }

    #else /* MS_PROXY_CLIENT */
    MS_IGNORE_UNUSED_PARAM(opcode);
    CONSOLE_OUT(
        "Proxy Client Feature Currently Disabled!!!\n");
    #endif /* MS_PROXY_CLIENT */
}

void appl_proxy_register(void)
{
    MS_proxy_register(appl_proxy_callback);
}

void appl_proxy_start_net_id_adv(MS_SUBNET_HANDLE subnet_handle)
{
    #ifdef MS_PROXY_SERVER
    /* Set the role to Proxy with bearer */
    blebrr_gatt_mode_set(BLEBRR_GATT_PROXY_MODE);
    MS_proxy_server_adv_start
    (
        subnet_handle,
        MS_PROXY_NET_ID_ADV_MODE
    );
    #else /* MS_PROXY_SERVER */
    CONSOLE_OUT(
        "Proxy Server Feature Currently Disabled!!!\n");
    return API_FAILURE;
    #endif /* MS_PROXY_SERVER */
}

void appl_proxy_start_node_identity_adv(MS_SUBNET_HANDLE subnet_handle)
{
    #ifdef MS_PROXY_SERVER
    /* Set the role to Proxy with bearer */
    blebrr_gatt_mode_set(BLEBRR_GATT_PROXY_MODE);
    MS_proxy_server_adv_start
    (
        subnet_handle,
        MS_PROXY_NODE_ID_ADV_MODE
    );
    #else /* MS_PROXY_SERVER */
    CONSOLE_OUT(
        "Proxy Server Feature Currently Disabled!!!\n");
    return API_FAILURE;
    #endif /* MS_PROXY_SERVER */
}

void appl_proxy_adv
(
    UCHAR identification_type,
    MS_SUBNET_HANDLE subnet_handle
)
{
    #ifdef MS_PROXY_SERVER
    (MS_PROXY_NET_ID_ADV_MODE == identification_type) ?
    appl_proxy_start_net_id_adv(subnet_handle) : appl_proxy_start_node_identity_adv(subnet_handle);
    #else /* MS_PROXY_SERVER */
    CONSOLE_OUT(
        "Proxy Server Feature Currently Disabled!!!\n");
    return API_FAILURE;
    #endif /* MS_PROXY_SERVER */
}

#endif /* MS_PROXY_SUPPORT */
