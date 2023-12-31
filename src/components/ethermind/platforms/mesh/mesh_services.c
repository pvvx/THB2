
/**
    \file mesh_services.c

    Generic Module to handle both
    - Mesh Provisioning Service :: 0x1827
    - Mesh Proxy Service        :: 0x1828
*/

/*
    Copyright (C) 2018. Mindtree Limited.
    All rights reserved.
*/

#include "mesh_services.h"

/*********************************************************************
    GLOBAL VARIABLES
*/
/* Mesh Prov Service UUID: 0x1827 */
CONST uint8 mesh_prov_UUID[ATT_BT_UUID_SIZE] =
{
    LO_UINT16(MESH_PROV_SERVICE_UUID), HI_UINT16(MESH_PROV_SERVICE_UUID)
};

/* Mesh Prov Data IN UUID: 0x2ADB */
CONST uint8 mesh_prov_data_in_UUID[ATT_BT_UUID_SIZE] =
{
    LO_UINT16(MESH_PROV_DATA_IN_UUID), HI_UINT16(MESH_PROV_DATA_IN_UUID)
};

/* Mesh Prov Data OUT UUID: 0x2ADC */
CONST uint8 mesh_prov_data_out_UUID[ATT_BT_UUID_SIZE] =
{
    LO_UINT16(MESH_PROV_DATA_OUT_UUID), HI_UINT16(MESH_PROV_DATA_OUT_UUID)
};

/* Mesh Proxy Service UUID: 0x1828 */
CONST uint8 mesh_proxy_UUID[ATT_BT_UUID_SIZE] =
{
    LO_UINT16(MESH_PROXY_SERVICE_UUID), HI_UINT16(MESH_PROXY_SERVICE_UUID)
};

/* Mesh Proxy Data IN UUID: 0x2ADD */
CONST uint8 mesh_proxy_data_in_UUID[ATT_BT_UUID_SIZE] =
{
    LO_UINT16(MESH_PROXY_DATA_IN_UUID), HI_UINT16(MESH_PROXY_DATA_IN_UUID)
};

/* Mesh Proxy Data OUT UUID: 0x2ADE */
CONST uint8 mesh_proxy_data_out_UUID[ATT_BT_UUID_SIZE] =
{
    LO_UINT16(MESH_PROXY_DATA_OUT_UUID), HI_UINT16(MESH_PROXY_DATA_OUT_UUID)
};

/*********************************************************************
    EXTERNAL VARIABLES
*/

/*********************************************************************
    EXTERNAL FUNCTIONS
*/

/*********************************************************************
    LOCAL VARIABLES
*/
/* Mesh Provisioning Service Related Callbacks */
static mesh_prov_cb*    prov_cb;

/* Mesh Proxy Service Related Callbacks */
static mesh_proxy_cb*   proxy_cb;

/*********************************************************************
    Profile Attributes - variables
*/

/* Mesh Provisioning Service */
static CONST gattAttrType_t mesh_prov_service =
{ATT_BT_UUID_SIZE, mesh_prov_UUID};

/* Mesh Prov Data IN Properties */
static uint8 mesh_prov_data_in_props = GATT_PROP_WRITE_NO_RSP;

/* Mesh Prov Data IN Value */
static uint8 mesh_prov_data_in_val[20];

/* Mesh Prov Data OUT Properties */
static uint8 mesh_prov_data_out_props = GATT_PROP_NOTIFY;

/* Mesh Prov Data OUT Value */
static uint8 mesh_prov_data_out_val[20];

/* Mesh Prov Data OUT CCCD value array */
static gattCharCfg_t mesh_prov_data_out_cccd[GATT_MAX_NUM_CONN];

/* Mesh Proxy Service */
static CONST gattAttrType_t mesh_proxy_service =
{ATT_BT_UUID_SIZE, mesh_proxy_UUID};

/* Mesh Proxy Data IN Properties */
static uint8 mesh_proxy_data_in_props = GATT_PROP_WRITE_NO_RSP;

/* Mesh Proxy Data IN Value */
static uint8 mesh_proxy_data_in_val[20];

/* Mesh Proxy Data OUT Properties */
static uint8 mesh_proxy_data_out_props = GATT_PROP_NOTIFY;

/* Mesh Proxy Data OUT Value */
static uint8 mesh_proxy_data_out_val[20];

/* Mesh Proxy Data OUT CCCD value array */
static gattCharCfg_t mesh_proxy_data_out_cccd[GATT_MAX_NUM_CONN];

/*********************************************************************
    Profile Attributes - Table
*/
/* Mesh Provisioning Service Attribute Table */
static gattAttribute_t mesh_prov_attr_tbl[MESH_PROV_IDX_NB] =
{
    /* Mesh Provisioning Service Declaration */
    {
        { ATT_BT_UUID_SIZE, primaryServiceUUID }, /* type */
        GATT_PERMIT_READ,                         /* permissions */
        0,                                        /* handle */
        (uint8*)& mesh_prov_service               /* pValue */
    },

    /* Mesh Prov Data IN Characteristic Declaration */
    {
        { ATT_BT_UUID_SIZE, characterUUID },
        GATT_PERMIT_READ,
        0,
        &mesh_prov_data_in_props
    },

    /* Mesh Prov Data IN Characteristic Value */
    {
        { ATT_BT_UUID_SIZE, mesh_prov_data_in_UUID },
        GATT_PERMIT_WRITE,
        0,
        &mesh_prov_data_in_val[0]
    },

    /* Mesh Prov Data OUT Characteristic Declaration */
    {
        { ATT_BT_UUID_SIZE, characterUUID },
        GATT_PERMIT_READ,
        0,
        &mesh_prov_data_out_props
    },

    /* Mesh Prov Data OUT Characteristic Value */
    {
        { ATT_BT_UUID_SIZE, mesh_prov_data_out_UUID },
        0,
        0,
        &mesh_prov_data_out_val[0]
    },

    /* Mesh Prov Data OUT CCCD */
    {
        { ATT_BT_UUID_SIZE, clientCharCfgUUID },
        GATT_PERMIT_READ | GATT_PERMIT_WRITE,
        0,
        (uint8*)& mesh_prov_data_out_cccd
    },
};

/* Mesh Proxy Service Attribute Table */
static gattAttribute_t mesh_proxy_attr_tbl[MESH_PROXY_IDX_NB] =
{
    /* Mesh Proxy Service Declaration */
    {
        { ATT_BT_UUID_SIZE, primaryServiceUUID }, /* type */
        GATT_PERMIT_READ,                         /* permissions */
        0,                                        /* handle */
        (uint8*)& mesh_proxy_service               /* pValue */
    },

    /* Mesh Proxy Data IN Characteristic Declaration */
    {
        { ATT_BT_UUID_SIZE, characterUUID },
        GATT_PERMIT_READ,
        0,
        &mesh_proxy_data_in_props
    },

    /* Mesh Proxy Data IN Characteristic Value */
    {
        { ATT_BT_UUID_SIZE, mesh_proxy_data_in_UUID },
        GATT_PERMIT_WRITE,
        0,
        &mesh_proxy_data_in_val[0]
    },

    /* Mesh Proxy Data OUT Characteristic Declaration */
    {
        { ATT_BT_UUID_SIZE, characterUUID },
        GATT_PERMIT_READ,
        0,
        &mesh_proxy_data_out_props
    },

    /* Mesh Proxy Data OUT Characteristic Value */
    {
        { ATT_BT_UUID_SIZE, mesh_proxy_data_out_UUID },
        0,
        0,
        &mesh_proxy_data_out_val[0]
    },

    /* Mesh Proxy Data OUT CCCD */
    {
        { ATT_BT_UUID_SIZE, clientCharCfgUUID },
        GATT_PERMIT_READ | GATT_PERMIT_WRITE,
        0,
        (uint8*)& mesh_proxy_data_out_cccd
    },
};

/*********************************************************************
    LOCAL FUNCTIONS
*/
static bStatus_t mesh_prov_write_cb
(
    uint16 connHandle,
    gattAttribute_t* pAttr,
    uint8* pValue,
    uint16 len,
    uint16 offset
);

static bStatus_t mesh_proxy_write_cb
(
    uint16 connHandle,
    gattAttribute_t* pAttr,
    uint8* pValue,
    uint16 len,
    uint16 offset
);

static void mesh_prov_handle_conn
(
    uint16 connHandle,
    uint8 changeType
);

static void mesh_proxy_handle_conn
(
    uint16 connHandle,
    uint8 changeType
);

/*********************************************************************
    PROFILE CALLBACKS
*/
/* Mesh Provisioning Service Internal Callbacks */
CONST gattServiceCBs_t mesh_prov_internal_cbs =
{
    NULL,               // Read callback function pointer
    mesh_prov_write_cb, // Write callback function pointer
    NULL                // Authorization callback function pointer
};

/* Mesh Proxy Service Internal Callbacks */
CONST gattServiceCBs_t mesh_proxy_internal_cbs =
{
    NULL,                // Read callback function pointer
    mesh_proxy_write_cb, // Write callback function pointer
    NULL                 // Authorization callback function pointer
};


/*  ----------------------------------------------------------------------------
    Function      : bStatus_t mesh_prov_init(mesh_prov_cb *cb)
    ----------------------------------------------------------------------------
    Description   : Send request to add Mesh Provisioning Service[0x1827]
                   into the attribute database.
                   Defines the different access functions
    .*                 (setter/getter commands to access the different
                   characteristic attributes).
    Inputs        : Pointer to mesh_prov_cb.
    Outputs       : bStatus_t : SUCCESS or failure reasons
    Assumptions   : None
    ------------------------------------------------------------------------- */
bStatus_t mesh_prov_init(mesh_prov_cb* cb)
{
    uint8 status = FAILURE;
    /* Initialize Client Characteristic Configuration attributes */
    GATTServApp_InitCharCfg
    (
        INVALID_CONNHANDLE,
        mesh_prov_data_out_cccd
    );
    /* Register with Link DB to receive link status change callback */
    VOID linkDB_Register(mesh_prov_handle_conn);
    /* Register GATT attribute list and CBs with GATT Server App */
    status = GATTServApp_RegisterService
             (
                 mesh_prov_attr_tbl,
                 GATT_NUM_ATTRS(mesh_prov_attr_tbl),
                 &mesh_prov_internal_cbs
             );
    /* Save the Callback Provided */
    prov_cb = cb;
    return (status);
}

/*  ----------------------------------------------------------------------------
    Function      : bStatus_t mesh_proxy_init(mesh_proxy_cb *cb)
    ----------------------------------------------------------------------------
    Description   : Send request to add Mesh Proxy Service[0x1828]
                   into the attribute database.
                   Defines the different access functions
    .*                 (setter/getter commands to access the different
                   characteristic attributes).
    Inputs        : Pointer to mesh_proxy_cb.
    Outputs       : bStatus_t : SUCCESS or failure reasons
    Assumptions   : None
    ------------------------------------------------------------------------- */
bStatus_t mesh_proxy_init(mesh_proxy_cb* cb)
{
    uint8 status = FAILURE;
    /* Initialize Client Characteristic Configuration attributes */
    GATTServApp_InitCharCfg
    (
        INVALID_CONNHANDLE,
        mesh_proxy_data_out_cccd
    );
    /* Register with Link DB to receive link status change callback */
    VOID linkDB_Register(mesh_proxy_handle_conn);
    /* Register GATT attribute list and CBs with GATT Server App */
    status = GATTServApp_RegisterService
             (
                 mesh_proxy_attr_tbl,
                 GATT_NUM_ATTRS(mesh_proxy_attr_tbl),
                 &mesh_proxy_internal_cbs
             );
    /* Save the Callback Provided */
    proxy_cb = cb;
    return (status);
}

/*  ----------------------------------------------------------------------------
    Function      : bStatus_t mesh_prov_deinit(void)
    ----------------------------------------------------------------------------
    Description   : Send request to delete Mesh Provisioning Service[0x1827]
                   from the attribute database.
    Inputs        : None
    Outputs       : bStatus_t : SUCCESS or failure reasons
    Assumptions   : None
    ------------------------------------------------------------------------- */
bStatus_t mesh_prov_deinit(void)
{
    uint8 status = SUCCESS;
    /* Reset the Callback Provided */
    prov_cb = NULL;
    /**
        Deregister Mesh Prov Service attribute list and
        CBs from GATT Server Application
    */
    status = GATTServApp_DeregisterService
             (
                 GATT_SERVICE_HANDLE(mesh_prov_attr_tbl),
                 NULL
             );
    return ( status );
}

/*  ----------------------------------------------------------------------------
    Function      : bStatus_t mesh_proxy_deinit(void)
    ----------------------------------------------------------------------------
    Description   : Send request to delete Mesh Proxy Service[0x1828]
                   from the attribute database.
    Inputs        : None
    Outputs       : bStatus_t : SUCCESS or failure reasons
    Assumptions   : None
    ------------------------------------------------------------------------- */
bStatus_t mesh_proxy_deinit(void)
{
    uint8 status = SUCCESS;
    /* Reset the Callback Provided */
    proxy_cb = NULL;
    /**
        Deregister Mesh Prov Service attribute list and
        CBs from GATT Server Application
    */
    status = GATTServApp_DeregisterService
             (
                 GATT_SERVICE_HANDLE(mesh_proxy_attr_tbl),
                 NULL
             );
    return ( status );
}


/*  ----------------------------------------------------------------------------
    Function      : bStatus_t mesh_prov_notify_data_out(uint16 conn_hndl,
                                 uint8 attidx, uint8 *val, uint8 val_len)
    ----------------------------------------------------------------------------
    Description   : Send Mesh Provisiong Data Out notification to Peer
    Inputs        : - conn_hndl       - connection index
                   - attidx       - index to attributes in the service
                   - val          - pointer to value
                   - val_len      - length of value
    Outputs       : bStatus_t : SUCCESS or failure reasons
    Assumptions   : None
    ------------------------------------------------------------------------- */
bStatus_t mesh_prov_notify_data_out
(
    uint16  conn_hndl,
    uint8   attidx,
    uint8*    val,
    uint8   val_len
)
{
    attHandleValueNoti_t mesh_prov_notif;
    uint16 value = GATTServApp_ReadCharCfg
                   (
                       conn_hndl,
                       mesh_prov_data_out_cccd
                   );
    /* NOTE: Currently the attidx is ignored */
    (void)attidx;
    /* Assign the Mesh Prov Data Out Value Handle */
    mesh_prov_notif.handle =\
                            mesh_prov_attr_tbl[MESH_PROV_DATA_OUT_VALUE_VAL].handle;
    /* Copy the Value and Length */
    mesh_prov_notif.len = val_len;
    osal_memcpy(mesh_prov_notif.value, val, val_len);

    /* If notifications enabled */
    if ( value & GATT_CLIENT_CFG_NOTIFY )
    {
        /* Send the notification */
        return GATT_Notification( conn_hndl, &mesh_prov_notif, FALSE );
    }

    return bleIncorrectMode;
}

/*  ----------------------------------------------------------------------------
    Function      : bStatus_t mesh_proxy_notify_data_out(uint16 conn_hndl,
                                 uint8 attidx, uint8 *val, uint8 val_len)
    ----------------------------------------------------------------------------
    Description   : Send Mesh Proxy Data Out notification to Peer
    Inputs        : - conn_hndl       - connection index
                   - attidx       - index to attributes in the service
                   - val          - pointer to value
                   - val_len      - length of value
    Outputs       : bStatus_t : SUCCESS or failure reasons
    Assumptions   : None
    ------------------------------------------------------------------------- */
bStatus_t mesh_proxy_notify_data_out
(
    uint16  conn_hndl,
    uint8   attidx,
    uint8*    val,
    uint8   val_len
)
{
    attHandleValueNoti_t mesh_proxy_notif;
    uint16 value = GATTServApp_ReadCharCfg
                   (
                       conn_hndl,
                       mesh_proxy_data_out_cccd
                   );
    /* NOTE: Currently the attidx is ignored */
    (void)attidx;
    /* Assign the Mesh Prov Data Out Value Handle */
    mesh_proxy_notif.handle =\
                             mesh_proxy_attr_tbl[MESH_PROV_DATA_OUT_VALUE_VAL].handle;
    /* Copy the Value and Length */
    mesh_proxy_notif.len = val_len;
    osal_memcpy(mesh_proxy_notif.value, val, val_len);

    /* If notifications enabled */
    if ( value & GATT_CLIENT_CFG_NOTIFY )
    {
        /* Send the notification */
        return GATT_Notification( conn_hndl, &mesh_proxy_notif, FALSE );
    }

    return bleIncorrectMode;
}

/*  ----------------------------------------------------------------------------
    Function      : bStatus_t mesh_prov_write_cb (uint16 connHandle,
                             gattAttribute_t * pAttr, uint8 * pValue,
                             uint8 len, uint16 offset)
    ----------------------------------------------------------------------------
    Description   : Validate attribute data prior to a write operation
    Inputs        : - connHandle - connection Handle
                   - pAttr      - pointer to attribute
                   - pValue     - pointer to data to be written
                   - len        - length of data
                   - offset     - offset of the first octet to be written
    Outputs       : Success or Failure
    Assumptions   : None
    ------------------------------------------------------------------------- */
static bStatus_t mesh_prov_write_cb
(
    uint16            connHandle,
    gattAttribute_t* pAttr,
    uint8*            pValue,
    uint16             len,
    uint16            offset
)
{
    bStatus_t status = SUCCESS;

    /* If attribute permissions require authorization to write, return error */
    if ( gattPermitAuthorWrite( pAttr->permissions ) )
    {
        /* Insufficient authorization */
        return ( ATT_ERR_INSUFFICIENT_AUTHOR );
    }

    if ( pAttr->type.len == ATT_BT_UUID_SIZE )
    {
        /* 16-bit UUID */
        uint16 uuid = BUILD_UINT16( pAttr->type.uuid[0], pAttr->type.uuid[1]);

        switch (uuid)
        {
        case MESH_PROV_DATA_IN_UUID:

            /* Validate the value */
            /* Make sure it's not a blob oper */
            /* dbg_printf("\r\n MESH_PROV_DATA_IN WWR of length %d\r\n", len); */
            if ( offset != 0 )
            {
                status = ATT_ERR_ATTR_NOT_LONG;
            }

            /* Write the value */
            if (SUCCESS == status)
            {
                /* TODO: Check if this is needed */
                /*  len = (len <= MESH_PROV_DATA_IN_MAX_LENGTH) ?\
                      len : MESH_PROV_DATA_IN_MAX_LENGTH; */

                /* Notify Data IN Write Callback */
                /* Invoke Data IN Callback */
                if (NULL != prov_cb)
                {
                    prov_cb->prov_data_in_cb
                    (
                        connHandle,
                        offset,
                        len,
                        pValue
                    );
                }
            }

            break;

        case GATT_CLIENT_CHAR_CFG_UUID:
            /* dbg_printf("\r\n MESH_PROV_DATA_OUT CCCD Write for %d Attr Handle of length %d\r\n", pAttr->handle, len); */
            status = GATTServApp_ProcessCCCWriteReq
                     (
                         connHandle,
                         pAttr,
                         pValue,
                         len,
                         offset,
                         GATT_CLIENT_CFG_NOTIFY
                     );

            if ( status == SUCCESS )
            {
                uint16 t_cccd_val = BUILD_UINT16( pValue[0], pValue[1] );
                t_cccd_val = (GATT_CLIENT_CFG_NOTIFY == t_cccd_val) ?\
                             TRUE : FALSE;

                /* Invoke CCCD Update Callback */
                if (NULL != prov_cb)
                {
                    prov_cb->prov_data_out_ccd_cb
                    (
                        connHandle,
                        t_cccd_val
                    );
                }
            }

            break;

        default:
            status = ATT_ERR_ATTR_NOT_FOUND;
            break;
        }
    }
    else
    {
        // 128-bit UUID
        status = ATT_ERR_INVALID_HANDLE;
    }

    return ( status );
}

/*  ----------------------------------------------------------------------------
    Function      : bStatus_t mesh_proxy_write_cb (uint16 connHandle,
                             gattAttribute_t * pAttr, uint8 * pValue,
                             uint8 len, uint16 offset)
    ----------------------------------------------------------------------------
    Description   : Validate attribute data prior to a write operation
    Inputs        : - connHandle - connection Handle
                   - pAttr      - pointer to attribute
                   - pValue     - pointer to data to be written
                   - len        - length of data
                   - offset     - offset of the first octet to be written
    Outputs       : Success or Failure
    Assumptions   : None
    ------------------------------------------------------------------------- */
static bStatus_t mesh_proxy_write_cb
(
    uint16            connHandle,
    gattAttribute_t* pAttr,
    uint8*            pValue,
    uint16             len,
    uint16            offset
)
{
    bStatus_t status = SUCCESS;

    /* If attribute permissions require authorization to write, return error */
    if ( gattPermitAuthorWrite( pAttr->permissions ) )
    {
        /* Insufficient authorization */
        return ( ATT_ERR_INSUFFICIENT_AUTHOR );
    }

    if ( pAttr->type.len == ATT_BT_UUID_SIZE )
    {
        /* 16-bit UUID */
        uint16 uuid = BUILD_UINT16( pAttr->type.uuid[0], pAttr->type.uuid[1]);

        switch (uuid)
        {
        case MESH_PROXY_DATA_IN_UUID:

            /* Validate the value */
            /* Make sure it's not a blob oper */
            /* dbg_printf("\r\n MESH_PROXY_DATA_IN WWR of length %d\r\n", len); */
            if ( offset != 0 )
            {
                status = ATT_ERR_ATTR_NOT_LONG;
            }

            /* Write the value */
            if (SUCCESS == status)
            {
                /* TODO: Check if this is needed */
                /*  len = (len <= MESH_PROXY_DATA_IN_MAX_LENGTH) ?\
                      len : MESH_PROXY_DATA_IN_MAX_LENGTH; */

                /* Notify Data IN Write Callback */
                /* Invoke Data IN Callback */
                if (NULL != proxy_cb)
                {
                    proxy_cb->proxy_data_in_cb
                    (
                        connHandle,
                        offset,
                        len,
                        pValue
                    );
                }
            }

            break;

        case GATT_CLIENT_CHAR_CFG_UUID:
            /* dbg_printf("\r\n MESH_PROXY_DATA_OUT CCCD Write for %d Attr Handle of length %d\r\n", pAttr->handle, len); */
            status = GATTServApp_ProcessCCCWriteReq
                     (
                         connHandle,
                         pAttr,
                         pValue,
                         len,
                         offset,
                         GATT_CLIENT_CFG_NOTIFY
                     );

            if ( status == SUCCESS )
            {
                uint16 t_cccd_val = BUILD_UINT16( pValue[0], pValue[1] );
                t_cccd_val = (GATT_CLIENT_CFG_NOTIFY == t_cccd_val) ?\
                             TRUE : FALSE;

                /* Invoke CCCD Update Callback */
                if (NULL != proxy_cb)
                {
                    proxy_cb->proxy_data_out_ccd_cb
                    (
                        connHandle,
                        t_cccd_val
                    );
                }
            }

            break;

        default:
            status = ATT_ERR_ATTR_NOT_FOUND;
            break;
        }
    }
    else
    {
        // 128-bit UUID
        status = ATT_ERR_INVALID_HANDLE;
    }

    return ( status );
}

/*  ----------------------------------------------------------------------------
    Function      : void mesh_prov_handle_conn(uint16 connHandle,
                                  uint8 changeType)
    ----------------------------------------------------------------------------
    Description   : Callback to Handle Mesh Prov Connection event
    Inputs        : - connHandle   - connection Handle
                   - changeType   - type of change
    Outputs       : None
    Assumptions   : None
    ------------------------------------------------------------------------- */
static void mesh_prov_handle_conn( uint16 connHandle, uint8 changeType )
{
    // Make sure this is not loopback connection
    if ( connHandle != LOOPBACK_CONNHANDLE )
    {
        // Reset Client Char Config if connection has dropped
        if ( ( changeType == LINKDB_STATUS_UPDATE_REMOVED )      ||
                ( ( changeType == LINKDB_STATUS_UPDATE_STATEFLAGS ) &&
                  ( !linkDB_Up( connHandle ) ) ) )
        {
            GATTServApp_InitCharCfg( connHandle, mesh_prov_data_out_cccd );
        }
    }
}

/*  ----------------------------------------------------------------------------
    Function      : void mesh_proxy_handle_conn(uint16 connHandle,
                                  uint8 changeType)
    ----------------------------------------------------------------------------
    Description   : Callback to Handle Mesh Proxy Connection event
    Inputs        : - connHandle   - connection Handle
                   - changeType   - type of change
    Outputs       : None
    Assumptions   : None
    ------------------------------------------------------------------------- */
static void mesh_proxy_handle_conn( uint16 connHandle, uint8 changeType )
{
    // Make sure this is not loopback connection
    if ( connHandle != LOOPBACK_CONNHANDLE )
    {
        // Reset Client Char Config if connection has dropped
        if ( ( changeType == LINKDB_STATUS_UPDATE_REMOVED )      ||
                ( ( changeType == LINKDB_STATUS_UPDATE_STATEFLAGS ) &&
                  ( !linkDB_Up( connHandle ) ) ) )
        {
            GATTServApp_InitCharCfg( connHandle, mesh_proxy_data_out_cccd );
        }
    }
}


