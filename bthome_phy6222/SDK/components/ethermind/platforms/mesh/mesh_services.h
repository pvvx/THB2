
/**
    \file mesh_services.h

    Generic Module to handle both
    - Mesh Provisioning Service :: 0x1827
    - Mesh Proxy Service        :: 0x1828
*/

/*
    Copyright (C) 2018. Mindtree Limited.
    All rights reserved.
*/
#ifndef _MESH_SERVICES_H
#define _MESH_SERVICES_H

/*  ----------------------------------------------------------------------------
    If building with a C++ compiler, make all of the definitions in this header
    have a C binding.
    ------------------------------------------------------------------------- */
#ifdef __cplusplus
extern "C"
{
#endif

/*  ----------------------------------------------------------------------------
    Include files
    --------------------------------------------------------------------------*/
/* Mesh OS Specific Inclusion */
#include "EM_os.h"

/* BLE Stack Specific Inclusion */
#include "bcomdef.h"
#include "OSAL.h"
#include "linkdb.h"
#include "att.h"
#include "gatt.h"
#include "gatt_uuid.h"
#include "gattservapp.h"
#include "gapbondmgr.h"

/*  ----------------------------------------------------------------------------
    Defines
    --------------------------------------------------------------------------*/

/* Mesh GATT Bearer Related Service Assigned Numbers */
#define MESH_PROV_SERVICE_UUID128       { 0x27, 0x18, 0x00, 0x00, 0x00, 0x00, \
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
        0x00, 0x00, 0x00, 0x00 }
#define MESH_PROXY_SERVICE_UUID128      { 0x28, 0x18, 0x00, 0x00, 0x00, 0x00, \
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
        0x00, 0x00, 0x00, 0x00 }

#define MESH_PROV_DATA_IN_UUID128       { 0xDB, 0x2A, 0x00, 0x00, 0x00, 0x00, \
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
        0x00, 0x00, 0x00, 0x00 }
#define MESH_PROV_DATA_OUT_UUID128      { 0xDC, 0x2A, 0x00, 0x00, 0x00, 0x00, \
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
        0x00, 0x00, 0x00, 0x00 }
#define MESH_PROXY_DATA_IN_UUID128      { 0xDD, 0x2A, 0x00, 0x00, 0x00, 0x00, \
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
        0x00, 0x00, 0x00, 0x00 }
#define MESH_PROXY_DATA_OUT_UUID128     { 0xDE, 0x2A, 0x00, 0x00, 0x00, 0x00, \
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
        0x00, 0x00, 0x00, 0x00 }

/* 16 Bit UUID Defines */
#define MESH_PROV_SERVICE_UUID                    0x1827
#define MESH_PROXY_SERVICE_UUID                   0x1828

#define MESH_PROV_DATA_IN_UUID                    0x2ADB
#define MESH_PROV_DATA_OUT_UUID                   0x2ADC
#define MESH_PROXY_DATA_IN_UUID                   0x2ADD
#define MESH_PROXY_DATA_OUT_UUID                  0x2ADE

/* Mesh GATT Bearer Serivce Characterisitic Length Definitions */
#define MESH_PROV_DATA_IN_MAX_LENGTH              (ATT_MTU_SIZE - 3)
#define MESH_PROV_DATA_OUT_MAX_LENGTH             (ATT_MTU_SIZE - 3)
#define MESH_PROXY_DATA_IN_MAX_LENGTH             (ATT_MTU_SIZE - 3)
#define MESH_PROXY_DATA_OUT_MAX_LENGTH            (ATT_MTU_SIZE - 3)

enum mesh_prov_idx_att
{
    /* Mesh Provisioning Service Primary Service Declaration */
    MESH_PROV_SERV_DECL = 0x00,

    /* Mesh Provisioning Data IN Characteristic */
    MESH_PROV_DATA_IN_VALUE_CHAR,
    MESH_PROV_DATA_IN_VALUE_VAL,

    /* Mesh Provisioning Data OUT Characteristic */
    MESH_PROV_DATA_OUT_VALUE_CHAR,
    MESH_PROV_DATA_OUT_VALUE_VAL,
    MESH_PROV_DATA_OUT_VALUE_CCC,

    /* Max number of characteristics */
    MESH_PROV_IDX_NB,
};

enum mesh_proxy_idx_att
{
    /* Mesh Proxy Service Primary Service Declaration */
    MESH_PROXY_SERV_DECL = 0x00,

    /* Mesh Proxy Data IN Characteristic */
    MESH_PROXY_DATA_IN_VALUE_CHAR,
    MESH_PROXY_DATA_IN_VALUE_VAL,

    /* Mesh Provisioning Data OUT Characteristic */
    MESH_PROXY_DATA_OUT_VALUE_CHAR,
    MESH_PROXY_DATA_OUT_VALUE_VAL,
    MESH_PROXY_DATA_OUT_VALUE_CCC,

    /* Max number of characteristics */
    MESH_PROXY_IDX_NB,
};

/* Define the available custom service states */
enum mesh_serv_state
{
    MESH_NO_SERVICES,
    MESH_PROV_SERVICE_DONE,
    MESH_PROXY_SERVICE_DONE,
    MESH_SERV_STATE_MAX
};

/*  ----------------------------------------------------------------------------
    Global variables and types
    --------------------------------------------------------------------------*/

typedef uint16 (* mesh_prov_data_in_wt_cb)
(
    uint16 conn_hndl,
    uint16 offset,
    uint16 length,
    uint8* value
);

typedef uint16 (* mesh_prov_data_out_ccd_cb)
(
    uint16 conn_hndl,
    uint8 enabled
);

/**
    Mesh Prov application callbacks
*/
typedef struct
{
    /** Provisioning Data IN Callback */
    mesh_prov_data_in_wt_cb              prov_data_in_cb;

    /** Provisioning Data OUT notif Changed */
    mesh_prov_data_out_ccd_cb            prov_data_out_ccd_cb;

} mesh_prov_cb;

typedef uint16 (* mesh_proxy_data_in_wt_cb)
(
    uint16 conn_hndl,
    uint16 offset,
    uint16 length,
    uint8* value
);

typedef uint16 (* mesh_proxy_data_out_ccd_cb)
(
    uint16 conn_hndl,
    uint8 enabled
);

/**
    Mesh Proxy application callbacks
*/
typedef struct
{
    /** Proxy Data IN Callback */
    mesh_proxy_data_in_wt_cb              proxy_data_in_cb;

    /** Proxy Data OUT notif Changed */
    mesh_proxy_data_out_ccd_cb            proxy_data_out_ccd_cb;

} mesh_proxy_cb;

/*  ----------------------------------------------------------------------------
    Function prototype definitions
    --------------------------------------------------------------------------*/

/**
    Register Mesh Provisioning Service Instance

    \param [in] cb        application callbacks
*/
bStatus_t mesh_prov_init (mesh_prov_cb* cb);

/**
    Deregister Mesh Provisioning Service Instance

*/
bStatus_t mesh_prov_deinit(void);

/**
    Mesh Provisioning Data out Notifications

    Notification will only be sent if given client enabled notifications before.

    \param [in] conn_hndl  Connection Identifier
    \param [in] attidx  Attribute Index
    \param [in] val     Pointer to Data to be sent
    \param [in] val_len Length of Data to be sent
*/
bStatus_t mesh_prov_notify_data_out
(
    uint16  conn_hndl,
    uint8   attidx,
    uint8*    val,
    uint8   val_len
);

/**
    Register Mesh Proxy Service Instance

    \param [in] cb        application callbacks
*/
bStatus_t mesh_proxy_init (mesh_proxy_cb* cb);

/**
    Deregister Mesh Proxy Service Instance

*/
bStatus_t mesh_proxy_deinit(void);

/**
    Mesh Proxy Data out Notifications

    Notification will only be sent if given client enabled notifications before.

    \param [in] conn_hndl  Connection Identifier
    \param [in] attidx  Attribute Index
    \param [in] val     Pointer to Data to be sent
    \param [in] val_len Length of Data to be sent
*/
bStatus_t mesh_proxy_notify_data_out
(
    uint16  conn_hndl,
    uint8   attidx,
    uint8*    val,
    uint8   val_len
);

/*  ----------------------------------------------------------------------------
    Close the 'extern "C"' block
    ------------------------------------------------------------------------- */
#ifdef __cplusplus
}
#endif

#endif /* _MESH_SERVICES_H */
