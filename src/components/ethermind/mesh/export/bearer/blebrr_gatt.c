
/**
    \file blebrr_gatt.c

    This File contains the BLE Bearer interface for the GATT bearer over
    Mindtree Mesh stack.
*/

/*
    Copyright (C) 2016. Mindtree Ltd.
    All rights reserved.
*/

/* ------------------------------- Header File Inclusion */
#include "MS_brr_api.h"
#include "MS_prov_api.h"
#include "blebrr.h"

/* --------------------------------------------- External Global Variables */

/* --------------------------------------------- Global Definitions */

/* GATT Proxy Segmentation and Reassembly related states */
#define BLEBRR_GATT_SAR_COMPLETE_PKT        0x00
#define BLEBRR_GATT_SAR_START_PKT           0x01
#define BLEBRR_GATT_SAR_CONTINUE_PKT        0x02
#define BLEBRR_GATT_SAR_END_PKT             0x03

/* GATT Proxy Segmentation and Reassembly related state values */
#define BLEBRR_GATT_SAR_INIT_STATE          0x00
#define BLEBRR_GATT_SAR_TX_PROGRESS         0x01

/* GATT PDU Size in octets */
#define BLEBRR_GATT_PDU_SIZE                75

/* --------------------------------------------- Macros */

/* --------------------------------------------- Structures/Data Types */

/* --------------------------------------------- Global Variables */
/** GATT bearer Information */
DECL_STATIC BRR_BEARER_INFO blebrr_gatt;

/* TODO: Check if the MTU be global or part of specific context */
/** GATT bearer specific identifiers */
DECL_STATIC BRR_BEARER_CH_INFO blebrr_gatt_ch_info;

/** GATT Current mode identifier - PROV or PROXY */
DECL_STATIC UCHAR blebrr_gatt_mode = 0xFF;

/* ------------------------------- Functions */
/**
    \brief

    \par Description


    \param flag

    \return void
*/
void blebrr_gatt_mode_set(UCHAR flag)
{
    /**
        The valid values for 'flag' are:
        0x00: BLEBRR_GATT_PROV_MODE  GATT Provisioning Mode
        0x01: BLEBRR_GATT_PROXY_MODE GATT Proxy Mode
        All other values are RFU.
    */
    blebrr_gatt_mode = ((flag == BLEBRR_GATT_PROV_MODE) || (flag == BLEBRR_GATT_PROXY_MODE))?
                       flag: 0xFF;
    /* Notify GATT mode setting to blebrr pl */
    blebrr_set_gattmode_pl (blebrr_gatt_mode);
}

/**
    \brief

    \par Description

 * *
 * *  \param void

    \return void
*/
UCHAR blebrr_gatt_mode_get(void)
{
    /**
        The valid return values are:
        0x00: BLEBRR_GATT_PROV_MODE  GATT Provisioning Mode
        0x01: BLEBRR_GATT_PROXY_MODE GATT Proxy Mode
        All other values are RFU.
    */
    return blebrr_gatt_mode;
}

/**
    \brief

    \par Description


    \param handle
    \param type
    \param pdata
    \param datalen

    \return void
*/
DECL_STATIC API_RESULT blebrr_gatt_send
(
    BRR_HANDLE* handle,
    UCHAR type,
    void* pdata,
    UINT16 datalen
)
{
    API_RESULT retval;
    MS_IGNORE_UNUSED_PARAM(type);
    retval = blebrr_gatt_send_pl
             (
                 handle,
                 pdata,
                 datalen
             );
    return retval;
}

/**
    \brief

    \par Description


    \param handle
    \param type
    \param pdata
    \param pdatalen

    \return void
*/
API_RESULT blebrr_pl_recv_gattpacket
(
    BRR_HANDLE* handle,
    UCHAR* pdata,
    UINT16 pdatalen
)
{
    if (NULL != blebrr_gatt.bearer_recv)
    {
        blebrr_gatt.bearer_recv
        (
            handle,
            pdata,
            pdatalen,
            NULL
        );
    }

    return API_SUCCESS;
}

/**
    \brief

    \par Description


    \param type
    \param uuid
    \param handle

    \return void
*/
API_RESULT blebrr_pl_gatt_connection
(
    BRR_HANDLE* handle,
    UCHAR        role,
    UCHAR        mode,
    UINT16       mtu
)
{
    MS_BUFFER buffer;
    API_RESULT retval;
    /* Initialize */
    retval = API_FAILURE;
    /* Populate the Bearer GATT Channel info */
    blebrr_gatt_ch_info.role = role;
    blebrr_gatt_ch_info.mtu  = mtu;
    /* Add bearer to the mesh network */
    blebrr_gatt.bearer_send = blebrr_gatt_send;
    blebrr_gatt.binfo = &buffer;
    buffer.payload = (UCHAR*)&blebrr_gatt_ch_info;
    buffer.length = sizeof(blebrr_gatt_ch_info);
    retval = MS_brr_add_bearer(BRR_TYPE_GATT, &blebrr_gatt, handle);

    /* Check the PDU type received and Add bearer to Mesh stack */
    if (BLEBRR_SERVER_ROLE == role)
    {
        blebrr_pl_advertise_end();

        if (BLEBRR_GATT_PROXY_MODE == mode)
        {
            /* Start observing */
            blebrr_scan_enable();
        }
    }
    else if (BLEBRR_CLIENT_ROLE == role)
    {
        /* Do Nothing */
        /**
            Currently, not enabling scan for Proxy Client.
            Typically, Proxy Client supports only GATT Bearer.
            Hence, not initiating 'SCAN' on Bearer UP event.
        */
    }

    return retval;
}

/**
    \brief

    \par Description


    \param type
    \param handle

    \return void
*/
API_RESULT blebrr_pl_gatt_disconnection
(
    BRR_HANDLE* handle
)
{
    API_RESULT retval;
    retval = MS_brr_remove_bearer(BRR_TYPE_GATT, handle);
    return retval;
}

