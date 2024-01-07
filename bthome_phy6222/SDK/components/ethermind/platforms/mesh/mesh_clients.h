
/**
    \file mesh_clients.h


*/

/*
    Copyright (C) 2018. Mindtree Limited.
    All rights reserved.
*/

#ifndef _MESH_CLIENTS_H
#define _MESH_CLIENTS_H

/* --------------------------------------------- Header File Inclusion */
#include "EM_os.h"
#include "blebrr.h"

#include "gatt.h"
#include "gatt_uuid.h"

#include "bleMesh.h"

/* --------------------------------------------- Global Definitions */

/* Mesh GATT Bearer Related Service Assigned Numbers as arrays */
#define MESH_PROV_SERVICE_UUID16        { 0x27, 0x18}
#define MESH_PROV_DATA_IN_UUID16        { 0xDB, 0x2A}
#define MESH_PROV_DATA_OUT_UUID16       { 0xDC, 0x2A}

#define MESH_PROXY_SERVICE_UUID16       { 0x28, 0x18}
#define MESH_PROXY_DATA_IN_UUID16       { 0xDD, 0x2A}
#define MESH_PROXY_DATA_OUT_UUID16      { 0xDE, 0x2A}

/* Mesh GATT Bearer Related Service Assigned Numbers */
/* Use the UUID from Mesh Asigned numbers headers */
#define UUID_MESH_PROVISIONING_SERVICE  (0x1827)
#define UUID_MESH_PROVISIONING_DATA_IN  (0x2ADB)
#define UUID_MESH_PROVISIONING_DATA_OUT (0x2ADC)
/* Use the UUID from Mesh Asigned numbers headers */
#define UUID_MESH_PROXY_SERVICE         (0x1828)
#define UUID_MESH_PROXY_DATA_IN         (0x2ADD)
#define UUID_MESH_PROXY_DATA_OUT        (0x2ADE)


/* --------------------------------------------- Structures/Data Types */
/**
    \brief Mesh Provisioning Data Out notification callback

    Called when Client received Mesh Provisioning data out
    notification from server.

    \param [in] client   client instance
    \param [in] length   length of data
    \param [in] value    mesh prov data out value from Server

*/
typedef void (* mesh_prov_client_data_out_cb)
(
    uint16_t  conidx,
    uint16_t  length,
    uint8_t*    value
);
/**
    \brief Set Notifications Enable Disable Callback

    \param [in] conidx           connection index
    \param [in] flag             Enable/Disable State Flag
    \param [in] status           operation status

*/
typedef void (* mesh_prov_client_data_out_ntf_status_cb)
(
    uint16_t  conidx,
    uint8_t   flag,
    uint8_t   status
);

/**
    \brief MEsh Provisioning Cliet application callbacks

*/
typedef struct
{
    /** Mesh Procv Client Data Out callback */
    mesh_prov_client_data_out_cb             mesh_prov_data_out_notif;

    /**
        Called once client enabled/disabled event
        characteristic notifications/indications
    */
    mesh_prov_client_data_out_ntf_status_cb  mesh_prov_ntf_status;

} mesh_prov_client_cb;

/**
    \brief Mesh Provisioning Data Out notification callback

    Called when Client received Mesh Provisioning data out
    notification from server.

    \param [in] client   client instance
    \param [in] length   length of data
    \param [in] value    mesh prov data out value from Server

*/
typedef void (* mesh_proxy_client_data_out_cb)
(
    uint16_t  conidx,
    uint16_t  length,
    uint8_t*    value
);
/**
    \brief Set Notifications Enable Disable Callback

    \param [in] conidx           connection index
    \param [in] flag             Enable/Disable State Flag
    \param [in] status           operation status

*/
typedef void (* mesh_proxy_client_data_out_ntf_status_cb)
(
    uint16_t  conidx,
    uint8_t   flag,
    uint8_t   status
);

/**
    \brief Mesh Provisioning Cliet application callbacks

*/
typedef struct
{
    /** Mesh Procv Client Data Out callback */
    mesh_proxy_client_data_out_cb             mesh_proxy_data_out_notif;

    /**
        Called once client enabled/disabled event
        characteristic notifications/indications
    */
    mesh_proxy_client_data_out_ntf_status_cb  mesh_proxy_ntf_status;

} mesh_proxy_client_cb;

struct mesh_cli_env_tag
{
    /* Connection index */
    uint16_t conidx;

    /* Provisioning Service Related Handles */
    uint16_t prov_start_hdl;
    uint16_t prov_end_hdl;
    uint16_t prov_data_in_hdl;
    uint16_t prov_data_out_hdl;
    uint16_t prov_data_out_cccd_hdl;

    /* Proxy Service Related Handles */
    uint16_t proxy_start_hdl;
    uint16_t proxy_end_hdl;
    uint16_t proxy_data_in_hdl;
    uint16_t proxy_data_out_hdl;
    uint16_t proxy_data_out_cccd_hdl;

    /* Current Notification Mode and State */
    uint16_t curr_notif_state;
    uint8_t  curr_notif_mode;

};

extern struct mesh_cli_env_tag mesh_cli_env[];

/* --------------------------------------------- Macros */
#define mesh_prov_client_discover_serv(cidx) \
    mesh_client_discover_services((BLEBRR_GATT_PROV_MODE), (cidx));

#define mesh_proxy_client_discover_serv(cidx) \
    mesh_client_discover_services((BLEBRR_GATT_PROXY_MODE), (cidx));

#define mesh_prov_client_data_in_write(cidx, val, len) \
    mesh_client_send_wwr((cidx), (val), (len), BLEBRR_GATT_PROV_MODE);

#define mesh_proxy_client_data_in_write(cidx, val, len) \
    mesh_client_send_wwr((cidx), (val), (len), BLEBRR_GATT_PROXY_MODE);

#define mesh_prov_client_enable_data_out(cidx) \
    mesh_client_config_ntf((cidx), BLEBRR_GATT_PROV_MODE, (true));

#define mesh_proxy_client_enable_data_out(cidx) \
    mesh_client_config_ntf((cidx), BLEBRR_GATT_PROXY_MODE, (true));

#define mesh_prov_client_disable_data_out(cidx) \
    mesh_client_config_ntf((cidx), BLEBRR_GATT_PROV_MODE, (false));

#define mesh_proxy_client_disable_data_out(cidx) \
    mesh_client_config_ntf((cidx), BLEBRR_GATT_PROXY_MODE, (false));

/* --------------------------------------------- Internal Functions */
void mesh_client_process_gattMsg
(
    gattMsgEvent_t* pMsg,
    uint8_t        t_id
);

/* --------------------------------------------- API Declarations */
/**
    \brief Register Mesh Provisioning Client instance

    Function registers new Mesh Provisioning Client instance.

    \param [in] cb               client application callbacks

    \return None

*/
void mesh_prov_client_init
(
    mesh_prov_client_cb* cb
);

/**
    \brief Register Mesh Proxy Client instance

    Function registers new Mesh Proxy Client instance.

    \param [in] cb               client application callbacks

    \return None

*/
void mesh_proxy_client_init
(
    mesh_proxy_client_cb* cb
);

void mesh_client_send_wwr
(
    uint16_t  conidx,
    uint8_t*    value,
    uint16_t  length,
    uint8_t   serv_pref
);

API_RESULT mesh_client_config_ntf
(
    uint16_t  conidx,
    uint8_t   serv_pref,
    uint8_t   flag
);

void mesh_client_init(void);
API_RESULT mesh_client_discover_services(uint16_t conidx, uint8_t serv_mode);
void mesh_client_update_conidx (uint16_t conidx);

#endif /* _MESH_CLIENTS_H */
