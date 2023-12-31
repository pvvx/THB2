
/**
    \file mesh_clients.c


*/

/*
    Copyright (C) 2018. Mindtree Limited.
    All rights reserved.
*/

/* --------------------------------------------- Header File Inclusion */
#include "mesh_clients.h"

/* --------------------------------------------- Global Definitions */
/* Compilation Switch to have this module print trace on console */
#define MESH_CLIENT_CONSOLE_DEBUG

/* --------------------------------------------- Macros */
#ifdef MESH_CLIENT_CONSOLE_DEBUG
    #define MESH_CLIENT_TRC(...) printf(__VA_ARGS__)
#else /* MESH_CLIENT_CONSOLE_DEBUG */
    #define MESH_CLIENT_TRC(...)
#endif /* MESH_CLIENT_CONSOLE_DEBUG */

/* --------------------------------------------- External Global Variables */

/* --------------------------------------------- Exported Global Variables */

/* --------------------------------------------- Static Global Variables */
/**
    This Task ID is the identifier used by the GATT APIs to reference
    the BLE Mesh Application Task.
    The definition and usage of this is from the bleMesh.c application
    file.
*/
extern uint8_t bleMesh_TaskID;

/* --------------------------------------------- Functions */

#define MESH_MAX_CLI_ENV             1

/* Global variable definition */
struct mesh_cli_env_tag mesh_cli_env[MESH_MAX_CLI_ENV];

/* Mesh Provisioning Service Client Related Callbacks */
static mesh_prov_client_cb*    prov_cli_cb;

/* Mesh Proxy Service Client Related Callbacks */
static mesh_proxy_client_cb*   proxy_cli_cb;

/* Track current UUID being Discovered */
static uint16_t mesh_client_curr_dis_uuid;

/*  ----------------------------------------------------------------------------
    Function      : void mesh_client_init(void)
    ----------------------------------------------------------------------------
    Description   : Initialize Mesh Client environment
    Inputs        : None
    Outputs       : None
    Assumptions   : None
    ------------------------------------------------------------------------- */
void mesh_client_init(void)
{
    for (unsigned int i = 0; i < MESH_MAX_CLI_ENV; i++)
    {
        /* Reset the application manager environment */
        memset(&mesh_cli_env[i], 0, sizeof(struct mesh_cli_env_tag));
    }

    mesh_client_curr_dis_uuid = 0x0000;
    prov_cli_cb  = NULL;
    proxy_cli_cb = NULL;
}

/**
    Routine to update the connection identifier for Mesh.
*/
void mesh_client_update_conidx (uint16_t conidx)
{
    mesh_cli_env[0].conidx = conidx;
}

/**
    \brief Register Mesh Provisioning Client instance

    Function registers new Mesh Provisioning Client instance.

    \param [in] cb               client application callbacks

    \return None

*/
void mesh_prov_client_init(mesh_prov_client_cb* cb)
{
    /* Register the upper layer provided Provisioning Client Callbacks */
    if (NULL != cb)
    {
        prov_cli_cb = cb;
    }
}

/**
    \brief Register Mesh Proxy Client instance

    Function registers new Mesh Proxy Client instance.

    \param [in] cb               client application callbacks

    \return None

*/
void mesh_proxy_client_init(mesh_proxy_client_cb* cb)
{
    /* Register the upper layer provided Provisioning Client Callbacks */
    if (NULL != cb)
    {
        proxy_cli_cb = cb;
    }
}

API_RESULT mesh_client_discover_services
(
    uint16_t conidx,
    uint8_t serv_mode
)
{
    uint8_t svc_uuid[2];
    bStatus_t ret;

    if (BLEBRR_GATT_PROV_MODE == serv_mode)
    {
        /* UUID is Provisioning Service */
        svc_uuid[0] = (uint8_t)(UUID_MESH_PROVISIONING_SERVICE);
        svc_uuid[1] = (uint8_t)(UUID_MESH_PROVISIONING_SERVICE >> 8);
        /* EM_mem_copy(svc_uuid, MESH_PROV_SERVICE_UUID128, 16); */
        mesh_client_curr_dis_uuid = UUID_MESH_PROVISIONING_SERVICE;
    }
    else
    {
        /* UUID is Proxy Service */
        svc_uuid[0] = (uint8_t)(UUID_MESH_PROXY_SERVICE);
        svc_uuid[1] = (uint8_t)(UUID_MESH_PROXY_SERVICE >> 8);
        /* EM_mem_copy(svc_uuid, MESH_PROXY_SERVICE_UUID128, 16); */
        mesh_client_curr_dis_uuid = UUID_MESH_PROVISIONING_SERVICE;
    }

    /* Discover service by UUID */
    ret = GATT_DiscPrimaryServiceByUUID
          (
              conidx,
              svc_uuid,
              ATT_BT_UUID_SIZE,
              bleMesh_TaskID
          );
    MESH_CLIENT_TRC(
        "Discovery Primiary Service UUID 0x%04X "
        "returned with retval 0x%04X\r\n",
        mesh_client_curr_dis_uuid, ret);
    return (0 == ret) ? API_SUCCESS : API_FAILURE;
}

/*  ----------------------------------------------------------------------------
    Function      : void mesh_client_send_wwr(uint8_t conidx, uint8_t *value,
                                                  uint16_t handle, uint8_t offset,
                                                  uint16_t length, uint8_t type)
    ----------------------------------------------------------------------------
    Description   : Send a write command or request to the client device
    Inputs        : - conidx       - Connection index
                   - value        - Pointer to value
                   - handle       - Attribute handle
                   - length       - Length of value
                   - type         - Type of write message
    Outputs       : None
    Assumptions   : None
    ------------------------------------------------------------------------- */
void mesh_client_send_wwr
(
    uint16_t  conidx,
    uint8_t*    value,
    uint16_t  length,
    uint8_t   serv_pref
)
{
    attWriteReq_t req;
    bStatus_t     ret;
    /* Assign the Handle Based on the Service Preference for Write */
    req.handle = (BLEBRR_GATT_PROV_MODE == serv_pref) ? \
                 mesh_cli_env[0].prov_data_in_hdl :    \
                 mesh_cli_env[0].proxy_data_in_hdl;
    req.len    = length;
    req.sig    = 0x00;
    req.cmd    = 0x01; /* Write witout response */
    osal_memcpy(req.value, value, length);
    ret = GATT_WriteNoRsp(conidx, &req);
    MESH_CLIENT_TRC(
        "Writing data : len 0x%04X : handle 0x%02X : ret 0x%04X\n",
        req.len, req.handle, ret);
    return;
}

/*  ----------------------------------------------------------------------------
    Function      : void mesh_client_config_ntf
                        (
                            uint16_t  conidx,
                            uint8_t   serv_pref,
                            uint8_t   flag
                        )
    ----------------------------------------------------------------------------
    Description   : Send a write command or request to the client device
    Inputs        : - conidx    - Connection index
                   - serv_pref - Provisioning or Proxy Service
                   - flag      - enable or disable
    Outputs       : None
    Assumptions   : None
    ------------------------------------------------------------------------- */
API_RESULT mesh_client_config_ntf
(
    uint16_t  conidx,
    uint8_t   serv_pref,
    uint8_t   flag
)
{
    attWriteReq_t req;
    bStatus_t     ret;
    /* Assign the Handle Based on the Service Preference for Write */
    /**
        Increment the Handle by 1 as the CCCD is present in the handle next to
        the Value handle
    */
    req.handle   =  (BLEBRR_GATT_PROV_MODE == serv_pref) ?    \
                    mesh_cli_env[0].prov_data_out_cccd_hdl : \
                    mesh_cli_env[0].proxy_data_out_cccd_hdl;
    req.len      = 0x02;
    req.sig      = 0x00;
    req.cmd      = 0x00;
    req.value[0] = (true == flag) ? 0x01 : 0x00;
    req.value[1] = 0x00;
    /* Store CCCD Mode and State in global */
    mesh_cli_env[0].curr_notif_state = flag;
    mesh_cli_env[0].curr_notif_mode  = (BLEBRR_GATT_PROV_MODE == serv_pref) ?    \
                                       BLEBRR_GATT_PROV_MODE :                   \
                                       BLEBRR_GATT_PROXY_MODE;
    ret = GATT_WriteCharValue(mesh_cli_env[0].conidx, &req, bleMesh_TaskID);
    MESH_CLIENT_TRC
    ("Writing %d to CCCD 0x%04X for Mode 0x%02X\n",
     flag, req.handle, serv_pref);
    return (0 == ret) ? API_SUCCESS : API_FAILURE;
}

void mesh_client_process_gattMsg
(
    gattMsgEvent_t* pMsg,
    uint8_t        t_id
)
{
    bStatus_t ret;
    uint16_t  serv_start_hndl, serv_end_hndl;
    MESH_CLIENT_TRC("Processing GATT Messages:\n");
    MESH_CLIENT_TRC("pMsg->method: 0x%X\n",pMsg->method);
    MESH_CLIENT_TRC("pMsg->hdr.status: 0x%X\n",pMsg->hdr.status);
    /* Cache the Task ID */
    bleMesh_TaskID = t_id;

    /* Process the GATT server message */
    switch ( pMsg->method )
    {
    case ATT_EXCHANGE_MTU_RSP:
        break;

    case ATT_FIND_BY_TYPE_VALUE_RSP:
    {
        if (bleProcedureComplete == pMsg->hdr.status)
        {
            if (UUID_MESH_PROVISIONING_SERVICE == mesh_client_curr_dis_uuid)
            {
                serv_start_hndl = mesh_cli_env[0].prov_start_hdl;
                serv_end_hndl   = mesh_cli_env[0].prov_end_hdl;
            }
            else if (UUID_MESH_PROXY_SERVICE == mesh_client_curr_dis_uuid)
            {
                serv_start_hndl = mesh_cli_env[0].proxy_start_hdl;
                serv_end_hndl   = mesh_cli_env[0].proxy_end_hdl;
            }
            else
            {
                MESH_CLIENT_TRC( "Mesh Services not Present\r\n");
            }

            /* Start Charactertistic Discovery */
            ret = GATT_DiscAllChars
                  (
                      mesh_cli_env[0].conidx,
                      serv_start_hndl,
                      serv_end_hndl,
                      bleMesh_TaskID
                  );
            MESH_CLIENT_TRC(
                "Disc Characteristics btw Handles 0x%04X :: 0x%04X"
                "returned with retval 0x%04X\r\n",
                serv_start_hndl, serv_end_hndl, ret);
        }
        else
        {
            if (pMsg->msg.findByTypeValueRsp.numInfo > 0)
            {
                MESH_CLIENT_TRC(
                    "\nServices found :%d\n",
                    pMsg->msg.findByTypeValueRsp.numInfo);
                MESH_CLIENT_TRC(
                    "Service Handles are 0x%04X :: 0x%04X",
                    pMsg->msg.findByTypeValueRsp.handlesInfo[0].handle,
                    pMsg->msg.findByTypeValueRsp.handlesInfo[0].grpEndHandle);

                if (mesh_client_curr_dis_uuid == UUID_MESH_PROVISIONING_SERVICE)
                {
                    mesh_cli_env[0].prov_start_hdl   = pMsg->msg.findByTypeValueRsp.handlesInfo[0].handle;
                    mesh_cli_env[0].prov_end_hdl     = pMsg->msg.findByTypeValueRsp.handlesInfo[0].grpEndHandle;
                    MESH_CLIENT_TRC(
                        "\r\n Prov Service Start Handle is 0x%04X\r\n",
                        mesh_cli_env[0].prov_start_hdl);
                    MESH_CLIENT_TRC(
                        "\r\n Prov Service End Handle is 0x%04X\r\n",
                        mesh_cli_env[0].prov_end_hdl);
                }
                else if (mesh_client_curr_dis_uuid == UUID_MESH_PROXY_SERVICE)
                {
                    mesh_cli_env[0].proxy_start_hdl   = pMsg->msg.findByTypeValueRsp.handlesInfo[0].handle;
                    mesh_cli_env[0].proxy_end_hdl     = pMsg->msg.findByTypeValueRsp.handlesInfo[0].grpEndHandle;
                    MESH_CLIENT_TRC(
                        "\r\nProxy Service Start Handle is 0x%04X\r\n",
                        mesh_cli_env[0].proxy_start_hdl);
                    MESH_CLIENT_TRC(
                        "\r\nProxy Service End Handle is 0x%04X\r\n",
                        mesh_cli_env[0].proxy_end_hdl);
                }
            }
            else
            {
                MESH_CLIENT_TRC("\n No Services Found!\n");
                /**
                    TODO: Provide Discovery completion Callback.
                */
            }
        }
    }
    break;

    case ATT_READ_BY_TYPE_RSP:
    {
        if (bleProcedureComplete == pMsg->hdr.status)
        {
            uint16_t t_char_start_hndl;

            if (UUID_MESH_PROVISIONING_SERVICE == mesh_client_curr_dis_uuid)
            {
                t_char_start_hndl = mesh_cli_env[0].prov_data_out_hdl;
                serv_end_hndl     = mesh_cli_env[0].prov_end_hdl;
            }
            else if (UUID_MESH_PROXY_SERVICE == mesh_client_curr_dis_uuid)
            {
                t_char_start_hndl = mesh_cli_env[0].proxy_data_out_hdl;
                serv_end_hndl   = mesh_cli_env[0].proxy_end_hdl;
            }

            ret = GATT_DiscAllCharDescs
                  (
                      mesh_cli_env[0].conidx,
                      t_char_start_hndl,
                      serv_end_hndl,
                      bleMesh_TaskID
                  );
            MESH_CLIENT_TRC(
                "Disc Char Desc btw Handles 0x%04X :: 0x%04X"
                "returned with retval 0x%04X\r\n",
                t_char_start_hndl, serv_end_hndl, ret);
        }
        else
        {
            // Pointer to the pair list data in the GATT response.
            uint8_t*   pCharPairList;
            // Will store the start and end handles of the current pair.
            uint16_t  charStartHandle;
            // Stores to the UUID of the current pair.
            uint16_t  charUuid;
            // Stores what pair the loop is currently processing.
            uint8_t   currentCharIndex;
            // Set the pair pointer to the first pair.
            pCharPairList = pMsg->msg.readByTypeRsp.dataList;

            // Iterate through all three pairs found.
            for(currentCharIndex = 0; currentCharIndex < pMsg->msg.readByTypeRsp.numPairs ; currentCharIndex++)
            {
                /* uint16_t* p_chars_hdl = pservie->chars_hdl; */
                // Extract the starting handle, ending handle, and UUID of the current characteristic.
                charStartHandle = BUILD_UINT16(pCharPairList[3], pCharPairList[4]);
                charUuid        = BUILD_UINT16(pCharPairList[5], pCharPairList[6]);
                MESH_CLIENT_TRC(
                    "Chars found handle is %d, uuid is %x\n",
                    charStartHandle, charUuid);

                switch (charUuid)
                {
                /* "Mesh Provisioning Data IN" */
                case UUID_MESH_PROVISIONING_DATA_IN:
                    mesh_cli_env[0].prov_data_in_hdl = charStartHandle;
                    break;

                /* "Mesh Provisioning Data OUT" */
                case UUID_MESH_PROVISIONING_DATA_OUT:
                    mesh_cli_env[0].prov_data_out_hdl = charStartHandle;
                    mesh_cli_env[0].prov_data_out_cccd_hdl = charStartHandle + 1;
                    break;

                /* "Mesh Proxy Data IN" */
                case UUID_MESH_PROXY_DATA_IN:
                    mesh_cli_env[0].proxy_data_in_hdl = charStartHandle;
                    break;

                /* "Mesh Proxy Data OUT" */
                case UUID_MESH_PROXY_DATA_OUT:
                    mesh_cli_env[0].proxy_data_out_hdl = charStartHandle;
                    mesh_cli_env[0].proxy_data_out_cccd_hdl = charStartHandle + 1;
                    break;
                }

                // Increment the pair pointer to the next pair.
                pCharPairList += 5 + 2;
            }
        }
    }
    break;

    case ATT_FIND_INFO_RSP:
    {
        if (bleProcedureComplete == pMsg->hdr.status)
        {
            /**
                TODO: Provide Discovery completion Callback.
            */
        }
        else
        {
            if ( (pMsg->msg.findInfoRsp.numInfo > 0) &&
                    (pMsg->msg.findInfoRsp.format == ATT_HANDLE_BT_UUID_TYPE) )
            {
                // This will keep track of the current pair being processed.
                uint8_t currentPair;

                // Iterate through the pair list.
                for(currentPair = 0; currentPair < pMsg->msg.findInfoRsp.numInfo; currentPair++)
                {
                    // Check if the pair is a CCCD.
                    uint16_t uuid = BUILD_UINT16(pMsg->msg.findInfoRsp.info.btPair[currentPair].uuid[0], pMsg->msg.findInfoRsp.info.btPair[currentPair].uuid[1]);

                    if (uuid == GATT_CLIENT_CHAR_CFG_UUID)
                    {
                        MESH_CLIENT_TRC(
                            "CCCD is found at handle 0x%X for Mode %d\n",
                            pMsg->msg.findInfoRsp.info.btPair[currentPair].handle, blebrr_gatt_mode_get());

                        if (BLEBRR_GATT_PROV_MODE == blebrr_gatt_mode_get())
                        {
                            mesh_cli_env[0].prov_data_out_cccd_hdl = pMsg->msg.findInfoRsp.info.btPair[currentPair].handle;
                        }
                        else
                        {
                            mesh_cli_env[0].proxy_data_out_cccd_hdl = pMsg->msg.findInfoRsp.info.btPair[currentPair].handle;
                        }
                    }
                }
            }
        }
    }
    break;

    case ATT_HANDLE_VALUE_NOTI:
    {
        uint16_t notifHandle = pMsg->msg.handleValueNoti.handle;

        if (pMsg->msg.handleValueNoti.len > 0)
        {
            /*  Check the Attribute Handle:
                - If Provisioning Data Out Handle: then call the Prov Callback
                - If Proxy Data Out Handle: the ncall the Proxy Callback
            */
            if (mesh_cli_env[0].prov_data_out_hdl == notifHandle)
            {
                if (NULL != prov_cli_cb)
                {
                    prov_cli_cb->mesh_prov_data_out_notif
                    (
                        mesh_cli_env[0].conidx,
                        pMsg->msg.handleValueNoti.len,
                        pMsg->msg.handleValueNoti.value
                    );
                }
            }
            else if (mesh_cli_env[0].proxy_data_out_hdl == notifHandle)
            {
                if (NULL != proxy_cli_cb)
                {
                    proxy_cli_cb->mesh_proxy_data_out_notif
                    (
                        mesh_cli_env[0].conidx,
                        pMsg->msg.handleValueNoti.len,
                        pMsg->msg.handleValueNoti.value
                    );
                }
            }
        }
    }
    break;

    case ATT_WRITE_RSP:
    {
        if (BLEBRR_GATT_PROV_MODE == mesh_cli_env[0].curr_notif_mode)
        {
            /* Call the Prov CCCD ntf complete Callback */
            if (NULL != prov_cli_cb)
            {
                prov_cli_cb->mesh_prov_ntf_status
                (
                    mesh_cli_env[0].conidx,
                    mesh_cli_env[0].curr_notif_state,
                    0x00
                );
            }
        }
        else if (BLEBRR_GATT_PROXY_MODE == mesh_cli_env[0].curr_notif_mode)
        {
            /* Call the Proxy CCCD ntf complete Callback */
            if (NULL != proxy_cli_cb)
            {
                proxy_cli_cb->mesh_proxy_ntf_status
                (
                    mesh_cli_env[0].conidx,
                    mesh_cli_env[0].curr_notif_state,
                    0x00
                );
            }
        }
        else
        {
            /* DO Nothing ! */
        }
    }
    break;

    case ATT_ERROR_RSP:
    {
        attErrorRsp_t* perr = &(pMsg->msg.errorRsp);
        MESH_CLIENT_TRC(
            "\nATT_ERROR_RSP 0x%x, 0x%x, 0x%x\n",
            perr->errCode, perr->handle, perr->reqOpcode);
    }
    break;

    default:
        break;
    }

    (void)ret;
}

