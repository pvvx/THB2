
/**
    \file blebrr_pl.c


*/

/*
    Copyright (C) 2018. Mindtree Limited.
    All rights reserved.
*/

/* --------------------------------------------- Header File Inclusion */
/* Platform Stack Headers */
#include <hci.h>
#include <gap.h>
#include <gatt.h>

#undef BLE_CLIENT_ROLE

/* Povisioning API headers */
#include "MS_prov_api.h"

/* BLE Bearer related Headers */
#include "blebrr.h"

#include "mesh_services.h"

#include "appl_main.h"

extern uint8             llState, llSecondaryState;

#ifdef BLE_CLIENT_ROLE
    #include "mesh_clients.h"
#endif /* BLE_CLIENT_ROLE */

/* Platform log to be mapped */
#define BLEBRRPL_LOG            printf
#define BLEBRRPL_dump_bytes     appl_dump_bytes

/*********************************************************************
    EXTERNAL FUNCTIONS
*/
bStatus_t BLE_gap_set_scan_enable(uint8_t scan_enable);
bStatus_t BLE_gap_connect(uint8_t whitelist, uint8_t* addr, uint8_t addr_type);
bStatus_t BLE_gap_set_advscanrsp_data(uint8_t type, uint8_t* adv_data, uint16_t adv_datalen);
bStatus_t BLE_gap_set_scan_params(uint8_t scan_type, uint16_t scan_interval, uint16_t scan_window, uint8_t scan_filterpolicy);
bStatus_t BLE_gap_set_adv_params(uint8_t adv_type, uint16_t adv_intervalmin, uint16_t adv_intervalmax, uint8_t adv_filterpolicy);
bStatus_t BLE_gap_disconnect(uint16_t conn_handle);
bStatus_t BLE_gap_set_adv_enable(uint8_t adv_enable);


API_RESULT blebrr_scan_cmd_handler_pl(UCHAR enable);

void blebrr_enable_mesh_serv_pl (UCHAR serv_type);
void blebrr_disable_mesh_serv_pl (UCHAR serv_type);

API_RESULT blebrr_handle_le_connection_pl
(
    uint16_t  conn_idx,
    uint16_t  conn_hndl,
    uint8_t   peer_addr_type,
    uint8_t*    peer_addr
);

API_RESULT blebrr_handle_le_disconnection_pl
(
    uint16_t  conn_idx,
    uint16_t  conn_hndl,
    uint8_t   reason
);

void blebrr_handle_evt_adv_complete (UINT8 enable);
void blebrr_handle_evt_adv_report (gapDeviceInfoEvent_t* adv);
void blebrr_handle_evt_scan_complete (UINT8 enable);

/**
    Value for invalid connection index

    Portable code should use this value wherever it's required to mark connection index as invalid.
*/
#define BLEBRR_CONN_IDX_INVALID    (0xFFFF)
#define BLEBRR_CONN_HNDL_INVALID   (0xFFFF)

/* --------------------------------------------- Global Definitions */
#define BLEBRR_ADVDATA_OFFSET                  0 /* 3 */

#define BLEBRR_NCON_ADVINTMIN               0xA0 /* ADV for NON_CONN_IND should be greater than 0x00A0 */
#define BLEBRR_NCON_ADVINTMAX               0xA0 /* ADV for NON_CONN_IND should be greater than 0x00A0 */
#define BLEBRR_NCON_ADVTYPE                 0x03
#define BLEBRR_NCON_DIRADDRTYPE             0x00
#define BLEBRR_NCON_ADVCHMAP                0x07
#define BLEBRR_NCON_ADVFILTERPOLICY         0x00

#define BLEBRR_CON_ADVINTMIN                0x320
#define BLEBRR_CON_ADVINTMAX                0x320
#define BLEBRR_CON_ADVTYPE                  0x00
#define BLEBRR_CON_DIRADDRTYPE              0x00
#define BLEBRR_CON_ADVCHMAP                 0x07
#define BLEBRR_CON_ADVFILTERPOLICY          0x00
#define BLEBRR_CON_SCANRSP_DATALEN          31

#define BLEBRR_SCANTYPE                     0x00
#define BLEBRR_SCANINTERVAL                 0x18            //0x18->15ms 0x20->20ms
#define BLEBRR_SCANWINDOW                   0x18            //0x18->15ms 0x20->20ms
#define BLEBRR_SCANFILTERPOLICY             0x00
#define BLEBRR_SCANFILTERDUPS               0x00

#define BLEBRR_CONN_FILTER_POLICY_WL        0x01
#define BLEBRR_CONN_FILTER_POLICY_NWL       0x00
#define BLEBRR_CONN_INTERVAL_MIN            0x0040
#define BLEBRR_CONN_INTERVAL_MAX            0x0040
#define BLEBRR_CONN_LATENCY                 0x0000
#define BLEBRR_CONN_SUPERVISION_TO          0x03BB
#define BLEBRR_CONN_MIN_CE_LEN              0x0000
#define BLEBRR_CONN_MAX_CE_LEN              0x0000

#define BLEBRR_OWNADDRTYPE                  0x00

#define BLEBRR_ADVSCANEN_TIMEOUT            50

/* Active connection handle used to send measurements */
static uint16_t active_conn_hndl = BLEBRR_CONN_HNDL_INVALID;

/* Call Back to Inform Application Layer about GATT Bearer Iface Events */
typedef void (* blebrr_gatt_iface_event_pl_cb)
(
    uint8_t  ev_name,
    uint8_t  status
);

API_RESULT blebrr_register_gatt_iface_event_pl
(
    blebrr_gatt_iface_event_pl_cb gatt_iface_evt_cb
);

/* --------------------------------------------- External Global Variables */

/* --------------------------------------------- Exported Global Variables */

/* --------------------------------------------- Static Global Variables */

/* Bearer Scan Response Data related */
DECL_STATIC UCHAR blebrr_scanrsp_data[BLEBRR_CON_SCANRSP_DATALEN];
DECL_STATIC UCHAR blebrr_scanrsp_datalen;

//DECL_STATIC UCHAR BLEBRR_DIRADDR[6];

DECL_STATIC UCHAR blebrr_advstate;
DECL_STATIC UCHAR blebrr_scanstate;

DECL_STATIC UCHAR curr_service;

UCHAR blebrr_advtype;


#ifdef BLE_CLIENT_ROLE
/* BLE Bearer GAP Connection State */
/**
    0x00 - Idle/Disconnected
    0x01 - To Initiate Connection
    0x02 - Connected Initiated
    0x03 - Connected
*/
static UCHAR blebrr_connect_state     = 0x00;
static UCHAR blebrr_addr_to_conn[6]   = {0};
static UCHAR blebrr_addr_type_to_conn = 0xFF;
#endif /* BLE_CLIENT_ROLE */

/* Global to hold the Role of GATT */
UCHAR blebrr_gatt_role;

BRR_HANDLE blebrr_gatt_handle_pl;

/* Mesh Provisioning service related data structures */
static uint16_t appl_mesh_prov_data_out_ccd_cb(uint16_t conn_hndl, uint8_t enabled);
static uint16_t appl_mesh_prov_data_in_wt_cb
(
    uint16_t conn_hndl,
    uint16_t offset,
    uint16_t length,
    uint8_t*   value
);
static mesh_prov_cb appl_mesh_prov_cb =
{
    .prov_data_in_cb      = appl_mesh_prov_data_in_wt_cb,
    .prov_data_out_ccd_cb = appl_mesh_prov_data_out_ccd_cb,
};

/* Mesh Proxy service related data structures */
static uint16_t appl_mesh_proxy_data_out_ccd_cb(uint16_t conn_hndl, uint8_t enabled);
static uint16_t appl_mesh_proxy_data_in_wt_cb
(
    uint16_t conn_hndl,
    uint16_t offset,
    uint16_t length,
    uint8_t*   value
);
static mesh_proxy_cb appl_mesh_proxy_cb =
{
    .proxy_data_in_cb      = appl_mesh_proxy_data_in_wt_cb,
    .proxy_data_out_ccd_cb = appl_mesh_proxy_data_out_ccd_cb,
};

/* Global to hold GATT Iface events Application Callback pointer */
DECL_STATIC BLEBRR_GATT_IFACE_EVENT_PL_CB  blebrr_gatt_iface_pl_cb;

/* --------------------------------------------- Functions */
API_RESULT blebrr_register_gatt_iface_event_pl
(
    BLEBRR_GATT_IFACE_EVENT_PL_CB gatt_iface_evt_cb
)
{
    if (NULL != gatt_iface_evt_cb)
    {
        blebrr_gatt_iface_pl_cb = gatt_iface_evt_cb;
        BLEBRRPL_LOG("\r\n Registered GATT Bearer Iface Events Appl Callback!\r\n");
        return API_SUCCESS;
    }

    /* If NULL callback is registered */
    return API_FAILURE;
}

void appl_dump_bytes(UCHAR* buffer, UINT16 length)
{
    char hex_stream[49];
    char char_stream[17];
    UINT32 i;
    UINT16 offset, count;
    UCHAR c;
    BLEBRRPL_LOG("\n");
    BLEBRRPL_LOG("-- Dumping %d Bytes --\n",
                 (int)length);
    BLEBRRPL_LOG(
        "-------------------------------------------------------------------\n");
    count = 0;
    offset = 0;

    for (i = 0; i < length; i++)
    {
        c = buffer[i];
        sprintf(hex_stream + offset, "%02X ", c);

        if ((c >= 0x20) && (c <= 0x7E))
        {
            char_stream[count] = c;
        }
        else
        {
            char_stream[count] = '.';
        }

        count++;
        offset += 3;

        if (16 == count)
        {
            char_stream[count] = '\0';
            count = 0;
            offset = 0;
            BLEBRRPL_LOG("%s   %s\n",
                         hex_stream, char_stream);
            EM_mem_set(hex_stream, 0, 49);
            EM_mem_set(char_stream, 0, 17);
        }
    }

    if (offset != 0)
    {
        char_stream[count] = '\0';
        /* Maintain the alignment */
        BLEBRRPL_LOG("%-48s   %s\n",
                     hex_stream, char_stream);
    }

    BLEBRRPL_LOG(
        "-------------------------------------------------------------------\n");
    BLEBRRPL_LOG("\n");
    return;
}


void blebrr_handle_evt_adv_report (gapDeviceInfoEvent_t* adv)
{
    UCHAR* pdata;
    UCHAR type;
    /* Reference the event type, data and datalength */
    type = (HCI_NONCONNECTABLE_UNDIRECTED_ADV == (UCHAR)adv->eventType)? BRR_BCON_PASSIVE: BRR_BCON_ACTIVE;
    pdata = (UCHAR*)adv->pEvtData;
    #if 0

    if (BRR_BCON_ACTIVE == type)
    {
        BLEBRRPL_LOG("Adv Report. Type: 0x%02X\r\n", adv->eventType);
        BLEBRRPL_LOG("BD Addr [0x%02X]: ", adv->addrType);
        BLEBRRPL_dump_bytes(adv->addr, 6);
        BLEBRRPL_LOG("Data [datalen]:");
        BLEBRRPL_dump_bytes(pdata, (UCHAR)adv->dataLen);
    }

    #endif /* 0 */

    /* Pass advertising data to the bearer */
    if ((MESH_AD_TYPE_BCON == pdata[1]) || (MESH_AD_TYPE_PB_ADV == pdata[1]) || (MESH_AD_TYPE_PKT == pdata[1]))
    {
        if (BRR_BCON_PASSIVE == type)
        {
            blebrr_pl_recv_advpacket (type, &pdata[1], pdata[0], (UCHAR)adv->rssi);
        }
    }
}

void blebrr_handle_evt_adv_complete (UINT8 enable)
{
    /* if (blebrr_advstate == enable) */
    {
        blebrr_pl_advertise_setup (blebrr_advstate);
    }
    #if 0
    else if (0x01 == blebrr_advstate)
    {
        blebrr_advstate = 0x00;
        blebrr_pl_advertise_end();
    }

    #endif /* 0 */
}

void blebrr_handle_evt_scan_complete (UINT8 enable)
{
    /* if (blebrr_scanstate == enable) */
    #ifdef BLE_CLIENT_ROLE
    bStatus_t ret;

    /*  BLEBRRPL_LOG ("\r\n blebrr_handle_evt_scan_complete with 0x%04X\r\n", enable);
        BLEBRRPL_LOG ("\r\n blebrr_connect_state is 0x%04X\r\n", blebrr_connect_state); */

    if (0x00 == enable)
    {
        /* Initiate Connection on Scan Disable */
        if (0x01 == blebrr_connect_state)
        {
            BLEBRRPL_LOG ("Scan Disabled! Initiating Connection...");
            ret = BLE_gap_connect
                  (
                      0x00,
                      blebrr_addr_to_conn,
                      blebrr_addr_type_to_conn
                  );
            BLEBRRPL_LOG("Initiating Connection to Address "
                         "0x%02X:0x%02X:0x%02X:0x%02X:0x%02X:0x%02X Type 0x%02X "
                         "with retval 0x%04X\r\n",
                         blebrr_addr_to_conn[0], blebrr_addr_to_conn[1], blebrr_addr_to_conn[2],
                         blebrr_addr_to_conn[3], blebrr_addr_to_conn[4], blebrr_addr_to_conn[5],
                         blebrr_addr_type_to_conn, ret);
            blebrr_connect_state = 0x02;
            /* Set GATT Role as Client */
            blebrr_gatt_role = BLEBRR_CLIENT_ROLE;
        }
        else
        {
            /* Indicate Scan disable to bearer */
            blebrr_pl_scan_setup (blebrr_scanstate);
        }
    }
    else
    #endif /* BLE_CLIENT_ROLE */
    {
        /* Indicate Scan disable to bearer */
        blebrr_pl_scan_setup (blebrr_scanstate);
    }
}

void blebrr_init_pl (void)
{
//    hciStatus_t ret;
    BLEBRR_LOG("Done.\n");
    /* Configure the local device address */
    GAP_ConfigDeviceAddr(ADDRTYPE_PUBLIC, NULL);
    /* Initialize */
    blebrr_gatt_iface_pl_cb = NULL;
    BLE_gap_set_scan_params
    (
        BLEBRR_SCANTYPE,
        BLEBRR_SCANINTERVAL,
        BLEBRR_SCANWINDOW,
        BLEBRR_SCANFILTERPOLICY
    );
    /**
        NOTE: Enabling Both the services at the start.

        TODO: Check if this needs to be flag protected.
    */
    BLEBRRPL_LOG ("Enabling Mesh Prov Service...\r\n");
    //mesh_prov_init((mesh_prov_cb *)&appl_mesh_prov_cb);
    /* Initialize the bearer handle */
    blebrr_gatt_handle_pl = BRR_HANDLE_INVALID;
}

void blebrr_scan_pl (UCHAR enable)
{
    hciStatus_t ret;
    UCHAR prevstate;
    prevstate = blebrr_scanstate;

    /* Is request to enable? */
    if (MS_TRUE == enable)
    {
        /* Update global scan state */
        blebrr_scanstate = 0x01;
        #ifdef BLEBRR_ENABLE_SCAN_TRACE
        BLEBRRPL_LOG ("Enabling Scan...");
        #endif /* BLEBRR_ENABLE_SCAN_TRACE */
        /* Enable Scan */
        ret = BLE_gap_set_scan_enable (0x01);
    }
    else
    {
        /* Update global scan state */
        blebrr_scanstate = 0x00;
        #ifdef BLEBRR_ENABLE_SCAN_TRACE
        BLEBRRPL_LOG ("Disabling Scan...");
        #endif /* BLEBRR_ENABLE_SCAN_TRACE */
        /* Disable Scan */
        ret = BLE_gap_set_scan_enable (0x00);
    }

    /* Is operation failed? */
    if (0 != ret)
    {
//        BLEBRRPL_LOG ("Scan Operation (%d - %d) failed with reason 0x%04X", blebrr_scanstate, prevstate, ret);
        blebrr_scanstate = prevstate;

        if(blebrr_scanstate != prevstate)
            BLEBRRPL_LOG ("Scan Operation (%d - %d) failed with reason 0x%04X", blebrr_scanstate, prevstate, ret);
    }
    else
    {
        /* Update state */
        if(MS_TRUE == enable)
            BLEBRR_SET_STATE(BLEBRR_STATE_IN_SCAN_ENABLE);
        else
            BLEBRR_SET_STATE(BLEBRR_STATE_IN_SCAN_DISABLE);
    }
}

UCHAR blebrr_get_advdata_offset_pl (void)
{
    return BLEBRR_ADVDATA_OFFSET;
}

API_RESULT blebrr_set_adv_scanrsp_data_pl
(
    UCHAR* srp_data,
    UCHAR   srp_datalen
)
{
    /* Initialize the Globals */
    EM_mem_set
    (
        blebrr_scanrsp_data,
        0x0,
        sizeof(blebrr_scanrsp_data)
    );
    blebrr_scanrsp_datalen = 0;

    /* Set the application provided Scan Response Data to Global */
    if ((NULL != srp_data) && (0 != srp_datalen))
    {
        EM_mem_copy
        (
            blebrr_scanrsp_data,
            srp_data,
            srp_datalen
        );
        blebrr_scanrsp_datalen = srp_datalen;
        return API_SUCCESS;
    }

    return API_FAILURE;
}

void blebrr_advertise_data_pl (CHAR type, UCHAR* pdata, UINT16 pdatalen)
{
    blebrr_advtype = type;

    /* Is request to enable? */
    if ((NULL != pdata) && (0 != pdatalen))
    {
        /* Set Advertising Parameters */
        if (BRR_BCON_PASSIVE == type)
        {
            /* Set Non-Connectable Adv Params */
            BLE_gap_set_adv_params
            (
                BLEBRR_NCON_ADVTYPE,
                BLEBRR_NCON_ADVINTMIN,
                BLEBRR_NCON_ADVINTMAX,
                BLEBRR_NCON_ADVFILTERPOLICY
            );
        }
        else
        {
            /* Set Connectable Adv Params */
            BLE_gap_set_adv_params
            (
                BLEBRR_CON_ADVTYPE,
                BLEBRR_CON_ADVINTMIN,
                BLEBRR_CON_ADVINTMAX,
                BLEBRR_CON_ADVFILTERPOLICY
            );

            /* Set the Scan Response Data to Stack if length is valid */
            if (0 != blebrr_scanrsp_datalen)
            {
                BLE_gap_set_advscanrsp_data
                (
                    FALSE,
                    blebrr_scanrsp_data,
                    blebrr_scanrsp_datalen
                );
            }
        }

        /* Set Advertising Data */
        BLE_gap_set_advscanrsp_data(TRUE, pdata, pdatalen);
        //BLEBRRPL_LOG ("Adv Data - Retval: %d\r\n",ret);
        /* Enable Advertising */
        blebrr_advertise_pl(MS_TRUE);
    }
    else
    {
        /* Disable Advertising */
        blebrr_advertise_pl(MS_FALSE);
    }
}

extern UCHAR blebrr_state;

API_RESULT blebrr_advertise_pl (UCHAR state)   // HZF
{
    hciStatus_t ret = API_SUCCESS;    // HZF
    UCHAR prevstate;
    prevstate = blebrr_advstate;

    if (MS_TRUE == state)
    {
        #ifdef BLEBRR_ENABLE_ADV_TRACE
        BLEBRRPL_LOG ("Enabling Adv...");
        #endif /* BLEBRR_ENABLE_ADV_TRACE */
        /* Update global adv state */
        blebrr_advstate = 0x01;
        ret = BLE_gap_set_adv_enable(0x01);
    }
    else
    {
        #ifdef BLEBRR_ENABLE_ADV_TRACE
        BLEBRRPL_LOG ("Disabling Adv...");
        #endif /* BLEBRR_ENABLE_ADV_TRACE */
        /* Update global adv state */
        blebrr_advstate = 0x00;
        //BLEBRRPL_LOG ("Disabling Adv...");
        ret = BLE_gap_set_adv_enable(0x00);
    }

    /* Is operation failed? */
    if (0 != ret)
    {
        BLEBRRPL_LOG ("Adv Operation (%d - %d) failed with reason 0x%04X", blebrr_advstate, prevstate, ret);
        blebrr_advstate = prevstate;
    }
    else
    {
        /* Update state */
        if(MS_TRUE == state)
            BLEBRR_SET_STATE(BLEBRR_STATE_IN_ADV_ENABLE);
        else
            BLEBRR_SET_STATE(BLEBRR_STATE_IN_ADV_DISABLE);
    }

    return ret;
}

API_RESULT blebrr_gatt_send_pl(BRR_HANDLE* handle, UCHAR* data, UINT16 datalen)
{
    UCHAR      type;
    API_RESULT retval;
    /* TODO */
    /*  BLEBRR_LOG("\n >>>> GATT PL Data Tx:\n");
        appl_dump_bytes(data, datalen); */
    retval = API_SUCCESS;
    /* Get the current mode */
    /* TODO: See if we need to get specific mode for a GATT transport */
    type = blebrr_gatt_mode_get();

    /* Check the PDU type received and Add bearer to Mesh stack */
    if (BLEBRR_GATT_PROV_MODE == type)
    {
        /* BLEBRRPL_LOG("\r\nBLEBRR_GATT_PROV_MODE with role 0x%02X\r\n", blebrr_gatt_role); */
        if (BLEBRR_SERVER_ROLE == blebrr_gatt_role)
        {
            mesh_prov_notify_data_out
            (
                active_conn_hndl,
                MESH_PROV_DATA_OUT_VALUE_VAL,
                data,
                datalen
            );
        }

        #ifdef BLE_CLIENT_ROLE
        else
        {
            /* TODO SRIKKANTH */
            mesh_prov_client_data_in_write(active_conn_hndl, data, datalen);
            retval = API_SUCCESS;
        }

        #endif /* BLE_CLIENT_ROLE */
    }
    else
    {
        /* BLEBRRPL_LOG("\r\nBLEBRR_GATT_PROXY_MODE with role 0x%02X\r\n", blebrr_gatt_role); */
        if (BLEBRR_SERVER_ROLE == blebrr_gatt_role)
        {
            retval = mesh_proxy_notify_data_out
                     (
                         active_conn_hndl,
                         MESH_PROXY_DATA_OUT_VALUE_VAL,
                         data,
                         datalen
                     );

            if(retval)
            {
                return retval;
            }
        }

        #ifdef BLE_CLIENT_ROLE
        else
        {
            mesh_proxy_client_data_in_write(active_conn_hndl, data, datalen);
            retval = API_SUCCESS;
        }

        #endif /* BLE_CLIENT_ROLE */
    }

    return retval;
}

static API_RESULT blebrr_recv_mesh_packet_pl
(
    void*     handle,
    UINT16    attr_handle,
    UCHAR*    data,
    UINT16    data_len
)
{
    /**
        TODO: MAP the incoming handle to BLEBRR specific handle for
    */
    /*  BLEBRR_LOG("\n >>>> GATT PL Data Rx: %d bytes\n", data_len);
        appl_dump_bytes(data, data_len); */
    blebrr_pl_recv_gattpacket
    (
        &blebrr_gatt_handle_pl,
        data,
        data_len
    );
    return API_SUCCESS;
}

API_RESULT blebrr_handle_le_connection_pl
(
    uint16_t  conn_idx,
    uint16_t  conn_hndl,
    uint8_t   peer_addr_type,
    uint8_t*    peer_addr
)
{
    /**
        If Needed,
        Store the provided handle according to platform exposed data type
    */
    BLEBRR_LOG("Device Connected - Handle: 0x%04X\r\n", conn_hndl);

    if (active_conn_hndl == BLEBRR_CONN_HNDL_INVALID)
    {
        /* Store the incoming connection handle in global */
        active_conn_hndl = conn_hndl;
        #ifdef BLE_CLIENT_ROLE
        mesh_client_update_conidx(active_conn_hndl);
        /* Setting State To Connected */
        blebrr_connect_state = 0x03;
        #endif /* BLE_CLIENT_ROLE */
    }

    /* Advertisement is disable by connection mostly */
    blebrr_advstate = 0x00;
    blebrr_pl_advertise_end();

    /**
        Inform Application of GATT/BLE Link Layer Connection.
    */
    if (NULL != blebrr_gatt_iface_pl_cb)
    {
        blebrr_gatt_iface_pl_cb
        (
            BLEBRR_GATT_IFACE_UP, /* BLE Link Layer Connection */
            0x00  /* Status is Success */
        );
    }

    return API_SUCCESS;
}

static API_RESULT blebrr_gatt_com_channel_setup_pl
(
    UCHAR role,
    UCHAR mode,
    UCHAR evt
)
{
    API_RESULT retval;
    UINT16     mtu;
    /**
        Possible Values of Role are
        1. 0x00 - GATT Client ~ BLEBRR_CLIENT_ROLE
        2. 0x01 - GATT Server ~ BLEBRR_SERVER_ROLE
    */
    /**
        Possible Values of Mode are
        1. 0x00 - BLEBRR_GATT_PROV_MODE
        2. 0x01 - BLEBRR_GATT_PROXY_MODE
    */
    /**
        Possible Values of evt are
        1. 0x00 - BLEBRR_COM_CHANNEL_CONNECT
        2. 0x01 - BLEBRR_COM_CHANNEL_DISCONNECT
    */
    retval = API_FAILURE;

    if (BLEBRR_COM_CHANNEL_CONNECT == evt)
    {
        /* Store the gatt role to be used during write */
        blebrr_gatt_role = role;
        /* Initialie MTU */
        mtu = BLEBRR_GATT_MIN_MTU;

        if (BLEBRR_SERVER_ROLE == role)
        {
            /* Fetch MTU from ATT and adjust it for Mesh */
        }
        else if (BLEBRR_CLIENT_ROLE == role)
        {
            /* Fetch MTU from ATT and adjust it for Mesh */
        }
        else
        {
            /* Empty */
        }

        retval = blebrr_pl_gatt_connection
                 (
                     &blebrr_gatt_handle_pl,
                     role,
                     mode,
                     mtu
                 );

        if (NULL != blebrr_gatt_iface_pl_cb)
        {
            blebrr_gatt_iface_pl_cb
            (
                BLEBRR_GATT_IFACE_ENABLE,
                mode  /* BLEBRR_GATT_PROV_MODE or BLEBRR_GATT_PROXY_MODE */
            );
        }
    }
    else if (BLEBRR_COM_CHANNEL_DISCONNECT == evt)
    {
        /**
            Currently BLE Bearer GATT Channel Disconnection
            is called only from HCI/ACL link disconnection.
        */
        blebrr_gatt_role = 0xFF;
        /* Delete Device from the Bearer */
        retval = blebrr_pl_gatt_disconnection (&blebrr_gatt_handle_pl);
        blebrr_gatt_handle_pl = BRR_HANDLE_INVALID;

        if (NULL != blebrr_gatt_iface_pl_cb)
        {
            blebrr_gatt_iface_pl_cb
            (
                BLEBRR_GATT_IFACE_DISABLE,
                mode  /* BLEBRR_GATT_PROV_MODE or BLEBRR_GATT_PROXY_MODE */
            );
        }
    }
    else
    {
        /* Empty */
    }

    return retval;
}

static uint16_t appl_mesh_prov_data_out_ccd_cb(uint16_t conn_hndl, uint8_t enabled)
{
    /* Check the Current mode is not PROV */
    if (BLEBRR_GATT_PROV_MODE != blebrr_gatt_mode_get())
    {
        BLEBRRPL_LOG("Mesh Prov Out CCD being Written when PROV is not Active!\r\n");
        return 0xFFFF;
    }

    if (TRUE == enabled)
    {
        BLEBRRPL_LOG("Mesh Prov Out CCD Enabled");
    }
    else
    {
        BLEBRRPL_LOG("Mesh Prov Out CCD Disabled");
    }

    blebrr_gatt_com_channel_setup_pl
    (
        BLEBRR_SERVER_ROLE,
        BLEBRR_GATT_PROV_MODE,
        (enabled) ? BLEBRR_COM_CHANNEL_CONNECT : BLEBRR_COM_CHANNEL_DISCONNECT
    );
    return 0x0000;
}

static uint16_t appl_mesh_prov_data_in_wt_cb
(
    uint16_t conn_hndl,
    uint16_t offset,
    uint16_t length,
    uint8_t*   value
)
{
    if (NULL != value)
    {
        /*  BLEBRRPL_LOG("Mesh Prov Data IN received");
            appl_dump_bytes(value, length); */
        BLEBRRPL_LOG("Mesh Prov Data IN received");
        blebrr_recv_mesh_packet_pl
        (
            &conn_hndl,
            offset,
            value,
            length
        );
    }

    return 0x0000;
}

static uint16_t appl_mesh_proxy_data_out_ccd_cb(uint16_t conn_hndl, uint8_t enabled)
{
    /* Check the Current mode is not PROV */
    if (BLEBRR_GATT_PROXY_MODE != blebrr_gatt_mode_get())
    {
        BLEBRRPL_LOG("Mesh Proxy Out CCD being Written when PROXY is not Active!\r\n");
        return 0xFFFF;
    }

    if (TRUE == enabled)
    {
        BLEBRRPL_LOG("Mesh Proxy Out CCD Enabled");
        blebrr_scan_enable();
    }
    else
    {
        BLEBRRPL_LOG("Mesh Proxy Out CCD Disabled");
    }

    blebrr_gatt_com_channel_setup_pl
    (
        BLEBRR_SERVER_ROLE,
        BLEBRR_GATT_PROXY_MODE,
        (enabled) ? BLEBRR_COM_CHANNEL_CONNECT : BLEBRR_COM_CHANNEL_DISCONNECT
    );
    return 0x0000;
}

static uint16_t appl_mesh_proxy_data_in_wt_cb
(
    uint16_t conn_hndl,
    uint16_t offset,
    uint16_t length,
    uint8_t*   value
)
{
    if (NULL != value)
    {
        BLEBRRPL_LOG("Mesh Proxy Data IN received");
        //appl_dump_bytes(value, length);
        blebrr_recv_mesh_packet_pl
        (
            &conn_hndl,
            offset,
            value,
            length
        );
    }

    return 0x0000;
}

#ifdef BLE_CLIENT_ROLE
void appl_mesh_prov_data_out_notif_cb
(
    uint16_t  conidx,
    uint16_t  length,
    uint8_t*    value
)
{
    #if 0
    printf("\r\n Mesh PROV Data Out NTFs:\r\n");
    appl_dump_bytes
    (
        value,
        length
    );
    #endif /* 0 */
    blebrr_recv_mesh_packet_pl
    (
        &active_conn_hndl,
        0x0000,
        (UCHAR*)value,
        length
    );
}

void appl_mesh_prov_notif_config_status_cb
(
    uint16_t  conidx,
    uint8_t   flag,
    uint8_t   status
)
{
    if (status == 0x00)
    {
        printf("Mesh Provisioning Data Out notifications %s\r\n",
               flag ?  "enabled" : "disabled");
        blebrr_gatt_mode_set(BLEBRR_GATT_PROV_MODE);
        appl_prov_register();
        appl_prov_setup(PROV_ROLE_PROVISIONER, PROV_BRR_GATT);
        blebrr_gatt_com_channel_setup_pl
        (
            BLEBRR_CLIENT_ROLE,
            BLEBRR_GATT_PROV_MODE,
            (flag)? BLEBRR_COM_CHANNEL_CONNECT : BLEBRR_COM_CHANNEL_DISCONNECT
        );
    }
    else
    {
        printf("ERROR: failed to set notifications (0x%02x)\r\n", status);
    }
}

static mesh_prov_client_cb mesh_prov_callbacks =
{
    .mesh_prov_data_out_notif = appl_mesh_prov_data_out_notif_cb,
    .mesh_prov_ntf_status     = appl_mesh_prov_notif_config_status_cb,
};

void appl_mesh_proxy_data_out_notif_cb
(
    uint16_t  conidx,
    uint16_t  length,
    uint8_t*    value
)
{
    #if 0
    printf("\r\n Mesh PROXY Data Out NTFs:\r\n");
    appl_dump_bytes
    (
        value,
        length
    );
    #endif /* 0 */
    blebrr_recv_mesh_packet_pl
    (
        &active_conn_hndl,
        0x0000,
        (UCHAR*)value,
        length
    );
}

void appl_mesh_proxy_notif_config_status_cb
(
    uint16_t  conidx,
    uint8_t   flag,
    uint8_t   status
)
{
    if (status == 0x00)
    {
        printf("Mesh Proxy Data Out notifications %s\r\n",
               flag ?  "enabled" : "disabled");
        blebrr_gatt_mode_set(BLEBRR_GATT_PROXY_MODE);
        blebrr_gatt_com_channel_setup_pl
        (
            BLEBRR_CLIENT_ROLE,
            BLEBRR_GATT_PROXY_MODE,
            (flag)? BLEBRR_COM_CHANNEL_CONNECT : BLEBRR_COM_CHANNEL_DISCONNECT
        );
    }
    else
    {
        printf("ERROR: failed to set notifications (0x%02x)\r\n", status);
    }
}

static mesh_proxy_client_cb mesh_proxy_callbacks =
{
    .mesh_proxy_data_out_notif = appl_mesh_proxy_data_out_notif_cb,
    .mesh_proxy_ntf_status     = appl_mesh_proxy_notif_config_status_cb,
};
#endif /* BLE_CLIENT_ROLE */

/** Dummy Interfaces: To be filled for Client Role */
API_RESULT blebrr_scan_cmd_handler_pl(UCHAR enable)
{
    if (enable)
    {
        BLEBRRPL_LOG("\n Scan Start Feature to be extended for CLI\n");
    }
    else
    {
        BLE_gap_set_scan_enable (0x00);
    }

    return API_SUCCESS;
}

API_RESULT blebrr_create_gatt_conn_pl
(
    UCHAR p_bdaddr_type,
    UCHAR* p_bdaddr
)
{
    #ifdef BLE_CLIENT_ROLE
    bStatus_t ret;

    if (0x00 != blebrr_scanstate)
    {
        /* Update global scan state */
        blebrr_scanstate = 0x00;
        /* Disable Scan */
        ret = BLE_gap_set_scan_enable (0x00);
        BLEBRRPL_LOG ("Disabling Scan...with retval 0x%04X\r\n", ret);

        if (0x00 == ret)
        {
            /* Scan Disable Successful */
            /* Move to To Initiate Connection Phase */
            blebrr_connect_state = 0x01;
            /** Save the global Address */
            EM_mem_copy(blebrr_addr_to_conn, p_bdaddr, 6);
            blebrr_addr_type_to_conn = p_bdaddr_type;
        }
        else
        {
            /* Scan Stop Failed */
            BLEBRRPL_LOG ("Scan Disabled Failed wit ret 0x%02X", ret);
        }

        return (0 == ret) ? API_SUCCESS : API_FAILURE;
    }
    else
    {
        BLEBRRPL_LOG ("Scan already Disabled! Initiating Connection...");
        ret = BLE_gap_connect
              (
                  0x00,
                  p_bdaddr,
                  p_bdaddr_type
              );
        blebrr_connect_state = 0x02;
        /* Set GATT Role as Client */
        blebrr_gatt_role = BLEBRR_CLIENT_ROLE;
        BLEBRRPL_LOG("Initiating Connection to Address "
                     "0x%02X:0x%02X:0x%02X:0x%02X:0x%02X:0x%02X Type 0x%02X "
                     "with retval 0x%04X\r\n",
                     p_bdaddr[0], p_bdaddr[1], p_bdaddr[2], p_bdaddr[3],
                     p_bdaddr[4], p_bdaddr[5], p_bdaddr_type, ret);
        return (0 == ret) ? API_SUCCESS : API_FAILURE;
    }

    #else /* BLE_CLIENT_ROLE */
    return API_FAILURE;
    #endif /* BLE_CLIENT_ROLE */
}

API_RESULT blebrr_disconnect_pl(void)
{
    bStatus_t ret;
    ret = BLE_gap_disconnect(active_conn_hndl);
    BLEBRRPL_LOG("\r\n Initiating Disconnection with Connection Handle 0x%04X"
                 "with retval 0x%04X\r\n", active_conn_hndl, ret);
    return (0 == ret) ? API_SUCCESS : API_FAILURE;
}

API_RESULT blebrr_discover_service_pl(UCHAR serv)
{
    /* Set the mode with bearer */
    (serv == 0) ? blebrr_gatt_mode_set(BLEBRR_GATT_PROV_MODE) :
    blebrr_gatt_mode_set(BLEBRR_GATT_PROXY_MODE);
    #ifdef BLE_CLIENT_ROLE
    /* Register the corresponding Callbacks */
    (serv == BLEBRR_GATT_PROV_MODE) ?             \
    mesh_prov_client_init(&mesh_prov_callbacks) : \
    mesh_proxy_client_init(&mesh_proxy_callbacks);
    return mesh_client_discover_services
           (
               active_conn_hndl,
               serv
           );
    #else
    return API_FAILURE;
    #endif /* BLE_CLIENT_ROLE */
}

API_RESULT blebrr_confige_ntf_pl(UCHAR config_ntf, UCHAR mode)
{
    #ifdef BLE_CLIENT_ROLE
    return  mesh_client_config_ntf(active_conn_hndl, mode, (0x00 == config_ntf) ? false:true);
    #else /* BLE_CLIENT_ROLE */
    BLEBRRPL_LOG("\r\n BLE_CLIENT_ROLE Disabled!\r\n");
    return API_FAILURE;
    #endif /* BLE_CLIENT_ROLE */
}

API_RESULT blebrr_handle_le_disconnection_pl
(
    uint16_t  conn_idx,
    uint16_t  conn_hndl,
    uint8_t   reason
)
{
    API_RESULT retval;
    retval = API_FAILURE;
    BLEBRRPL_LOG("Device Disconnected - Handle: 0x%04X with Reason: 0x%02X\r\n", conn_hndl, reason);

    if (active_conn_hndl == conn_hndl)
    {
        /* Reinitialize Connection Handle */
        active_conn_hndl = BLEBRR_CONN_HNDL_INVALID;
        #ifdef BLE_CLIENT_ROLE
        mesh_client_update_conidx(active_conn_hndl);
        /* Setting State to Idle/Disconnected */
        blebrr_connect_state = 0x00;
        #endif /* BLE_CLIENT_ROLE */
    }

    /* Inform Disconnection to GATT Bearer */
    blebrr_gatt_role = 0xFF;
    retval  = blebrr_pl_gatt_disconnection
              (
                  &blebrr_gatt_handle_pl
              );
    blebrr_gatt_handle_pl = BRR_HANDLE_INVALID;

    /**
        Inform Application of GATT/BLE Link Layer Connection.
    */
    if (NULL != blebrr_gatt_iface_pl_cb)
    {
        blebrr_gatt_iface_pl_cb
        (
            BLEBRR_GATT_IFACE_DOWN, /* BLE Link Layer Disconnection */
            0x00  /* Status is Success */
        );
    }

    return retval;
}

void blebrr_enable_mesh_serv_pl (UCHAR serv_type)
{
    BLEBRRPL_LOG ("Serv Enable called with 0x%02X", serv_type);

    /*  If serv_type :
        BLEBRR_GATT_PROVILINK - Mesh Prov
        BLEBRR_GATT_PROXYLINK - Mesh Proxy
    */
    if (BLEBRR_GATT_PROV_MODE == serv_type)
    {
        BLEBRRPL_LOG ("Enabling Mesh Prov Service...\r\n");
        mesh_prov_init(&appl_mesh_prov_cb);
    }
    else
    {
        BLEBRRPL_LOG ("Enabling Mesh Proxy Service...\r\n");
        mesh_proxy_init(&appl_mesh_proxy_cb);
    }
}

void blebrr_disable_mesh_serv_pl (UCHAR serv_type)
{
    BLEBRRPL_LOG ("Serv Disable called with 0x%02X", serv_type);

    /*  If serv_type :
        BLEBRR_GATT_PROVILINK - Mesh Prov
        BLEBRR_GATT_PROXYLINK - Mesh Proxy
    */
    if (BLEBRR_GATT_PROV_MODE == serv_type)
    {
        BLEBRRPL_LOG ("Disabling Mesh Prov Service...\r\n");
        /*
            Disable Mesh Provisioing Serivce
        */
        mesh_prov_deinit();
    }
    else
    {
        BLEBRRPL_LOG ("Disabling Mesh Proxy Service...\r\n");
        /*
            Disable Mesh Proxy Serivce
        */
        mesh_proxy_deinit();
    }

    BLEBRRPL_LOG("Service Disable yet to be Supported\r\n");
}

void blebrr_set_gattmode_pl (UCHAR flag)
{
    /* Setting Provisioning or Proxy Mode */
    if (0xFF != flag)
    {
        blebrr_disable_mesh_serv_pl(curr_service);
        blebrr_enable_mesh_serv_pl(flag);
        curr_service = flag;
    }
    else
    {
        /* Do Nothing */
    }
}

