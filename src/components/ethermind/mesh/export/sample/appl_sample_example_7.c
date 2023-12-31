
/**
    \file appl_sample_example_7.c

    Source File for Generic OnOff Client Standalone application without CLI or
    menu based console input interface.
*/

/*
    Copyright (C) 2018. Mindtree Ltd.
    All rights reserved.
*/

#if (MESH_STANDALONE == 7)

/* ----------------------------------------- Header File Inclusion */
#include "MS_common.h"
#include "MS_access_api.h"
#include "MS_config_api.h"
#include "MS_generic_onoff_api.h"
#include "blebrr.h"
#include "nvsto.h"

/* Console Input/Output */
#define CONSOLE_OUT(...)    printf(__VA_ARGS__)
#define CONSOLE_IN(...)     scanf(__VA_ARGS__)

void appl_dump_bytes(UCHAR* buffer, UINT16 length);
void appl_mesh_sample (void);

/* Provisioner */
#define UI_PROV_OUTPUT_OOB_ACTIONS \
    (PROV_MASK_OOOB_ACTION_BLINK | PROV_MASK_OOOB_ACTION_BEEP | \
     PROV_MASK_OOOB_ACTION_VIBRATE | PROV_MASK_OOOB_ACTION_NUMERIC | \
     PROV_MASK_OOOB_ACTION_ALPHANUMERIC)

/** Output OOB Maximum size supported */
#define UI_PROV_OUTPUT_OOB_SIZE               0x08

/** Input OOB Actions supported */
#define UI_PROV_INPUT_OOB_ACTIONS \
    (PROV_MASK_IOOB_ACTION_PUSH | PROV_MASK_IOOB_ACTION_TWIST | \
     PROV_MASK_IOOB_ACTION_NUMERIC | PROV_MASK_IOOB_ACTION_ALPHANUMERIC)

/** Input OOB Maximum size supported */
#define UI_PROV_INPUT_OOB_SIZE                0x08

/** Beacon setup timeout in seconds */
#define UI_PROV_SETUP_TIMEOUT_SECS            30

/** Attention timeout for device in seconds */
#define UI_PROV_DEVICE_ATTENTION_TIMEOUT      30

#define PROV_AUTHVAL_SIZE_PL                  16

/** Authentication values for OOB Display - To be made random */
#define UI_DISPLAY_AUTH_DIGIT                 3
#define UI_DISPLAY_AUTH_NUMERIC               35007
#define UI_DISPLAY_AUTH_STRING                "f00l"

#define UI_DEVICE_UUID      {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF}

/* AppKey to be used by the configuration client */
#define UI_APPKEY           {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11}

/* ----------------------------------------- External Global Variables */


/* ----------------------------------------- Exported Global Variables */


/* ----------------------------------------- Static Global Variables */
/** Provisioning capabilities of local device */
DECL_STATIC PROV_CAPABILITIES_S UI_prov_capab =
{
    /** Number of Elements */
    0x01,

    /** Supported algorithms */
    PROV_MASK_ALGO_EC_FIPS_P256,

    /** Public key type */
    PROV_MASK_PUBKEY_OOBINFO,

    /** Static OOB type */
    PROV_MASK_STATIC_OOBINFO,

    /** Output OOB information */
    { UI_PROV_OUTPUT_OOB_ACTIONS, UI_PROV_OUTPUT_OOB_SIZE },

    /** Input OOB information */
    { UI_PROV_INPUT_OOB_ACTIONS, UI_PROV_INPUT_OOB_SIZE },
};

/** Data exchanged during Provisiong procedure */
DECL_STATIC PROV_DATA_S UI_prov_data =
{
    /** NetKey */
    { 0x45, 0x74, 0x68, 0x65, 0x72, 0x4d, 0x69, 0x6e, 0x64, 0x4e, 0x65, 0x74, 0x4b, 0x65, 0x79, 0x00 },

    /** Index of the NetKey */
    0x0000,

    /** Flags bitmask */
    0x00,

    /** Current value of the IV index */
    0x00000000,

    /** Unicast address of the primary element */
    0x0002
};

/** Default Provisioning method to start */
DECL_STATIC PROV_METHOD_S UI_prov_method =
{
    /** Algorithm */
    PROV_ALGO_EC_FIPS_P256,

    /** Public Key */
    PROV_PUBKEY_NO_OOB,

    /** Authentication Method */
    PROV_AUTH_OOB_NONE,

    /** OOB Information */
    {0x0000, 0x00}
};

/** Current role of application - Provisioner/Device */
DECL_STATIC UCHAR UI_prov_role;

/** Provisioning Handle */
DECL_STATIC PROV_HANDLE UI_prov_handle;

/** Device UUID Identifier */
DECL_STATIC UCHAR UI_device_uuid[MS_DEVICE_UUID_SIZE] = UI_DEVICE_UUID;

/* Set provision started */
DECL_STATIC UCHAR UI_prov_started;

/* Configuration Client Model Handle */
DECL_STATIC MS_ACCESS_MODEL_HANDLE   UI_config_client_model_handle;

/** Appkey to be used for model binding */
DECL_STATIC UCHAR UI_appkey[MS_ACCESS_APPKEY_SIZE] = UI_APPKEY;

/* Generic ONOFF Client Model Handle */
DECL_STATIC MS_ACCESS_MODEL_HANDLE   UI_generic_onoff_client_model_handle;

/* ----------------------------------------- Functions */
/* Model Client - Configuration Models */
/* Send Config Composition Data Get */
void UI_config_client_get_composition_data(UCHAR page)
{
    API_RESULT retval;
    ACCESS_CONFIG_COMPDATA_GET_PARAM  param;
    CONSOLE_OUT
    ("Send Config Composition Data Get\n");
    param.page = page;
    retval = MS_config_client_composition_data_get(&param);
    CONSOLE_OUT
    ("Retval - 0x%04X\n", retval);
}

/* Send Config Appkey Add */
void UI_config_client_appkey_add(UINT16 netkey_index, UINT16 appkey_index, UCHAR* appkey)
{
    API_RESULT retval;
    ACCESS_CONFIG_APPKEY_ADD_PARAM  param;
    CONSOLE_OUT
    ("Send Config Appkey Add\n");
    param.netkey_index = netkey_index;
    param.appkey_index = appkey_index;
    EM_mem_copy(param.appkey, appkey, MS_ACCESS_APPKEY_SIZE);
    /* Update local database */
    MS_access_cm_add_appkey
    (
        0, /* subnet_handle */
        param.appkey_index, /* appkey_index */
        &param.appkey[0] /* app_key */
    );
    retval = MS_config_client_appkey_add(&param);
    CONSOLE_OUT
    ("Retval - 0x%04X\n", retval);
}

void UI_config_client_model_app_bind(UINT16 addr, UINT16 appkey_index, UCHAR model_type, UINT32 model_id)
{
    API_RESULT retval;
    ACCESS_CONFIG_MODEL_APP_BIND_PARAM  param;
    CONSOLE_OUT
    ("Send Config Model App Bind\n");
    param.element_address = addr;
    param.appkey_index = appkey_index;
    param.model.type = model_type;
    param.model.id = model_id;
    retval = MS_config_client_model_app_bind(&param);
    CONSOLE_OUT
    ("Retval - 0x%04X\n", retval);
}

/* Set Publish Address */
void UI_set_publish_address(UINT16 addr, MS_ACCESS_MODEL_HANDLE model_handle)
{
    API_RESULT retval;
    MS_ACCESS_PUBLISH_INFO  publish_info;
    /* Set Publish Information */
    EM_mem_set(&publish_info, 0, sizeof(publish_info));
    publish_info.addr.use_label = MS_FALSE;
    publish_info.appkey_index = MS_CONFIG_LIMITS(MS_MAX_APPS);
    publish_info.remote = MS_FALSE;
    publish_info.addr.addr = addr;
    retval = MS_access_cm_set_model_publication
             (
                 model_handle,
                 &publish_info
             );

    if (API_SUCCESS == retval)
    {
        CONSOLE_OUT
        ("Publish Address is set Successfully.\n");
    }
    else
    {
        CONSOLE_OUT
        ("Failed to set publish address. Status 0x%04X\n", retval);
    }

    return;
}

void UI_generic_onoff_set(UCHAR state)
{
    API_RESULT retval;
    MS_GENERIC_ONOFF_SET_STRUCT  param;
    CONSOLE_OUT
    ("Send Generic Onoff Set\n");
    param.onoff = state;
    param.tid = 0;
    param.optional_fields_present = 0x00;
    retval = MS_generic_onoff_set(&param);
    CONSOLE_OUT
    ("Retval - 0x%04X\n", retval);
}

/**
    \brief Client Application Asynchronous Notification Callback.

    \par Description
    Configuration client calls the registered callback to indicate events occurred to the application.

    \param [in] handle        Model Handle.
    \param [in] opcode        Opcode.
    \param [in] data_param    Data associated with the event if any or NULL.
    \param [in] data_len      Size of the event data. 0 if event data is NULL.
*/
API_RESULT UI_config_client_cb
(
    /* IN */ MS_ACCESS_MODEL_HANDLE* handle,
    /* IN */ UINT32                   opcode,
    /* IN */ UCHAR*                   data_param,
    /* IN */ UINT16                   data_len
)
{
    API_RESULT retval;
    retval = API_SUCCESS;
    CONSOLE_OUT (
        "[CONFIG_CLIENT] Callback. Opcode 0x%04X\n", opcode);
    appl_dump_bytes(data_param, data_len);

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
        /* Add Appkey */
        UI_config_client_appkey_add(0, 0, UI_appkey);
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
        /* Bind the model to Appkey */
        UI_config_client_model_app_bind(UI_prov_data.uaddr, 0, MS_ACCESS_MODEL_TYPE_SIG, MS_MODEL_ID_GENERIC_ONOFF_SERVER);
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
        /* Set the Publish address for Config Client */
        UI_set_publish_address(UI_prov_data.uaddr, UI_generic_onoff_client_model_handle);
        /* Send a Generic ON */
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

API_RESULT UI_register_config_model_client
(
    MS_ACCESS_ELEMENT_HANDLE element_handle
)
{
    /* Configuration Client */
    API_RESULT retval;
    CONSOLE_OUT("In Model Client - Configuration Models\n");
    retval = MS_config_client_init
             (
                 element_handle,
                 &UI_config_client_model_handle,
                 UI_config_client_cb
             );
    CONSOLE_OUT("Config Model Client Registration Status: 0x%04X\n", retval);
    return retval;
}


/* ---- Generic OnOff Handlers */

/* Generic OnOff Model Client */
/**
    \brief Client Application Asynchronous Notification Callback.

    \par Description
    Generic_Onoff client calls the registered callback to indicate events occurred to the application.

    \param [in] handle        Model Handle.
    \param [in] opcode        Opcode.
    \param [in] data_param    Data associated with the event if any or NULL.
    \param [in] data_len      Size of the event data. 0 if event data is NULL.
*/
API_RESULT UI_generic_onoff_client_cb
(
    /* IN */ MS_ACCESS_MODEL_HANDLE* handle,
    /* IN */ UINT32                   opcode,
    /* IN */ UCHAR*                   data_param,
    /* IN */ UINT16                   data_len
)
{
    API_RESULT retval;
    retval = API_SUCCESS;
    CONSOLE_OUT (
        "[GENERIC_ONOFF_CLIENT] Callback. Opcode 0x%04X\n", opcode);
    appl_dump_bytes(data_param, data_len);

    switch(opcode)
    {
    case MS_ACCESS_GENERIC_ONOFF_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_GENERIC_ONOFF_STATUS_OPCODE\n");
    }
    break;
    }

    return retval;
}


API_RESULT UI_register_generic_onoff_model_client
(
    MS_ACCESS_ELEMENT_HANDLE element_handle
)
{
    /* Generic OnOff Client */
    API_RESULT retval;
    CONSOLE_OUT("In Generic OnOff Model Client\n");
    retval = MS_generic_onoff_client_init
             (
                 element_handle,
                 &UI_generic_onoff_client_model_handle,
                 UI_generic_onoff_client_cb
             );

    if (API_SUCCESS == retval)
    {
        CONSOLE_OUT(
            "Generic Onoff Client Initialized. Model Handle: 0x%04X\n",
            UI_generic_onoff_client_model_handle);
    }
    else
    {
        CONSOLE_OUT(
            "[ERR] Generic Onoff Client Initialization Failed. Result: 0x%04X\n",
            retval);
    }

    return retval;
}

API_RESULT UI_prov_callback
(
    PROV_HANDLE* phandle,
    UCHAR         event_type,
    API_RESULT    event_result,
    void*         event_data,
    UINT16        event_datalen
)
{
    PROV_DEVICE_S* rdev;
    PROV_CAPABILITIES_S* rcap;
    PROV_DATA_S* rdata;
    PROV_OOB_TYPE_S* oob_info;
    API_RESULT retval;
    UCHAR i;
    UCHAR authstr[PROV_AUTHVAL_SIZE_PL << 1];
    UINT32 authnum;
    UCHAR authtype;
    UCHAR* pauth;
    UINT16 authsize;
    UCHAR  pdata[(MS_DEVICE_UUID_SIZE * 2) + 1];
    UCHAR*   t_data;

    switch (event_type)
    {
    case PROV_EVT_UNPROVISIONED_BEACON:
        /* Reference the beacon pointer */
        rdev = (PROV_DEVICE_S*)event_data;
        retval = API_SUCCESS;

        if (0 != EM_mem_cmp(rdev->uuid, UI_device_uuid, 16))
        {
            /* Beacon not from device of interest. Do no process */
            break;
        }

        CONSOLE_OUT ("Recvd PROV_EVT_UNPROVISIONED_BEACON\n");
        CONSOLE_OUT ("Status - 0x%04X\n", event_result);
        CONSOLE_OUT("\nUUID : [");
        EM_mem_set(pdata, 0x0, sizeof(pdata));
        t_data = pdata;

        for (i = 0; i < (MS_DEVICE_UUID_SIZE); i++)
        {
            sprintf(t_data,"%02X", rdev->uuid[i]);
            t_data += 2;
        }

        CONSOLE_OUT(" %s ", pdata);
        CONSOLE_OUT("]");
        CONSOLE_OUT("\nOOB  : 0x%04X", rdev->oob);
        CONSOLE_OUT("\nURI  : 0x%08X", rdev->uri);
        CONSOLE_OUT("\n\n");
        /* Bind to the device */
        retval = MS_prov_bind(PROV_BRR_ADV, rdev, UI_PROV_DEVICE_ATTENTION_TIMEOUT, phandle);
        CONSOLE_OUT("Retval - 0x%04X\n", retval);
        /* Set provision started */
        UI_prov_started = MS_TRUE;
        break;

    case PROV_EVT_PROVISIONING_SETUP:
        CONSOLE_OUT("Recvd PROV_EVT_PROVISIONING_SETUP\n");
        CONSOLE_OUT("Status - 0x%04X\n", event_result);

        /* Decipher the data based on the role */
        if (PROV_ROLE_PROVISIONER == UI_prov_role)
        {
            /* Display the capabilities */
            rcap = (PROV_CAPABILITIES_S*)event_data;
            CONSOLE_OUT ("Remote Device Capabilities:\n");
            CONSOLE_OUT ("\tNum Elements     - %d\n", rcap->num_elements);
            CONSOLE_OUT ("\tSupp Algorithms  - 0x%04X\n", rcap->supp_algorithms);
            CONSOLE_OUT ("\tSupp PublicKey   - 0x%02X\n", rcap->supp_pubkey);
            CONSOLE_OUT ("\tSupp Static OOB  - 0x%02X\n", rcap->supp_soob);
            CONSOLE_OUT ("\tOutput OOB Size  - %d\n", rcap->ooob.size);
            CONSOLE_OUT ("\tOutput OOB Action- 0x%04X\n", rcap->ooob.action);
            CONSOLE_OUT ("\tInput OOB Size   - %d\n", rcap->ioob.size);
            CONSOLE_OUT ("\tInput OOB Action - 0x%04X\n", rcap->ioob.action);
            /* Start with default method */
            CONSOLE_OUT ("Start Provisioning with default method...\n");
            retval = MS_prov_start (phandle, &UI_prov_method);
            CONSOLE_OUT ("Retval - 0x%04X\n", retval);
        }
        else
        {
            /* Display the attention timeout */
            CONSOLE_OUT("Attention TImeout - %d\n", *((UCHAR*)event_data));
        }

        break;

    case PROV_EVT_OOB_DISPLAY:
        CONSOLE_OUT("Recvd PROV_EVT_OOB_DISPLAY\n");
        CONSOLE_OUT("Status - 0x%04X\n", event_result);
        /* Reference the Authenticatio Type information */
        oob_info = (PROV_OOB_TYPE_S*)event_data;
        CONSOLE_OUT("Authenticaion Action - 0x%02X\n", oob_info->action);
        CONSOLE_OUT("Authenticaion Size - 0x%02X\n", oob_info->size);

        /* If role is Device, the action is of Output OOB, else Input OOB */
        if (PROV_ROLE_DEVICE == UI_prov_role)
        {
            if (PROV_OOOB_ACTION_ALPHANUMERIC == oob_info->action)
            {
                authtype = 1;
            }
            else if (PROV_OOOB_ACTION_NUMERIC == oob_info->action)
            {
                authtype = 2;
            }
            else
            {
                authtype = 0;
            }
        }
        else
        {
            if (PROV_IOOB_ACTION_ALPHANUMERIC == oob_info->action)
            {
                authtype = 1;
            }
            else if (PROV_IOOB_ACTION_NUMERIC == oob_info->action)
            {
                authtype = 2;
            }
            else
            {
                authtype = 0;
            }
        }

        if (1 == authtype)
        {
            EM_str_copy (authstr, UI_DISPLAY_AUTH_STRING);
            CONSOLE_OUT("\n\n>>> AuthVal - %s <<<\n\n", authstr);
            pauth = authstr;
            authsize = EM_str_len(authstr);
        }
        else if (2 == authtype)
        {
            authnum = (UINT32)UI_DISPLAY_AUTH_NUMERIC;
            CONSOLE_OUT("\n\n>>> AuthVal - %d <<<\n\n", authnum);
            pauth = (UCHAR*)&authnum;
            authsize = sizeof(UINT32);
        }
        else
        {
            authnum = (UINT32)UI_DISPLAY_AUTH_DIGIT;
            CONSOLE_OUT("\n\n>>> AuthVal - %d <<<\n\n", authnum);
            pauth = (UCHAR*)&authnum;
            authsize = sizeof(UINT32);
        }

        /* Call to input the oob */
        CONSOLE_OUT("Setting the Authval...\n");
        retval = MS_prov_set_authval(phandle, pauth, authsize);
        CONSOLE_OUT("Retval - 0x%04X\n", retval);
        break;

    case PROV_EVT_OOB_ENTRY:
        CONSOLE_OUT("Recvd PROV_EVT_OOB_ENTRY\n");
        CONSOLE_OUT("Status - 0x%04X\n", event_result);
        /* Reference the Authenticatio Type information */
        oob_info = (PROV_OOB_TYPE_S*)event_data;
        CONSOLE_OUT("Authenticaion Action - 0x%02X\n", oob_info->action);
        CONSOLE_OUT("Authenticaion Size - 0x%02X\n", oob_info->size);
        break;

    case PROV_EVT_DEVINPUT_COMPLETE:
        CONSOLE_OUT("Recvd PROV_EVT_DEVINPUT_COMPLETE\n");
        CONSOLE_OUT("Status - 0x%04X\n", event_result);
        break;

    case PROV_EVT_PROVDATA_INFO_REQ:
        CONSOLE_OUT ("Recvd PROV_EVT_PROVDATA_INFO_REQ\n");
        CONSOLE_OUT ("Status - 0x%04X\n", event_result);
        /* Send Provisioning Data */
        CONSOLE_OUT ("Send Provisioning Data...\n");
        retval = MS_prov_data (phandle, &UI_prov_data);
        CONSOLE_OUT ("Retval - 0x%04X\n", retval);
        break;

    case PROV_EVT_PROVISIONING_COMPLETE:
        CONSOLE_OUT("Recvd PROV_EVT_PROVISIONING_COMPLETE\n");
        CONSOLE_OUT("Status - 0x%04X\n", event_result);

        if (API_SUCCESS == event_result)
        {
            if (PROV_ROLE_PROVISIONER == UI_prov_role)
            {
                /* Set provision started */
                UI_prov_started = MS_FALSE;

                if (0x0002 == UI_prov_data.uaddr)
                {
                    /* Holding a temporary structure for local prov data */
                    PROV_DATA_S temp_ps_prov_data;
                    EM_mem_copy
                    (
                        &temp_ps_prov_data,
                        &UI_prov_data,
                        sizeof(UI_prov_data)
                    );
                    /**
                        Assigning the Local Unicast Address of the Provisioner
                        to the Access Layer along with other related keys.
                    */
                    temp_ps_prov_data.uaddr = 0x0001;
                    /* Provide Provisioning Data to Access Layer */
                    MS_access_cm_set_prov_data
                    (
                        &temp_ps_prov_data
                    );
                    /**
                        NOTE:
                        Increment the appl_prov_data.uaddr for the next
                        set of device which is getting provisioned based on
                        the address and number of elements present in the last
                        provisioned device.
                    */
                }

                /* Set the Publish address for Config Client */
                UI_set_publish_address(UI_prov_data.uaddr, UI_config_client_model_handle);
                /* Get the Composition data */
                UI_config_client_get_composition_data(0x00);
            }
        }

        break;

    default:
        CONSOLE_OUT("Unknown Event - 0x%02X\n", event_type);
    }

    return API_SUCCESS;
}

void UI_register_prov(void)
{
    API_RESULT retval;
    CONSOLE_OUT("Registering with Provisioning layer...\n");
    retval = MS_prov_register(&UI_prov_capab, UI_prov_callback);
    CONSOLE_OUT("Retval - 0x%04X\n", retval);
}

void UI_setup_prov(UCHAR role, UCHAR brr)
{
    API_RESULT retval;

    if (PROV_ROLE_PROVISIONER == role)
    {
        CONSOLE_OUT("Setting up Provisioner for Provisioning ...\n");
        retval = MS_prov_setup
                 (
                     brr,
                     role,
                     NULL,
                     UI_PROV_SETUP_TIMEOUT_SECS
                 );
        UI_prov_role = PROV_ROLE_PROVISIONER;
    }

    CONSOLE_OUT("Retval - 0x%04X\n", retval);
}

void appl_mesh_sample (void)
{
    MS_ACCESS_NODE_ID node_id;
    MS_ACCESS_ELEMENT_DESC   element;
    MS_ACCESS_ELEMENT_HANDLE element_handle;
    API_RESULT retval;
    UCHAR role, brr;
    MS_CONFIG* config_ptr;
    #ifdef MS_HAVE_DYNAMIC_CONFIG
    MS_CONFIG  config;
    /* Initialize dynamic configuration */
    MS_INIT_CONFIG(config);
    config_ptr = &config;
    #else
    config_ptr = NULL;
    #endif /* MS_HAVE_DYNAMIC_CONFIG */
    /* Initialize OSAL */
    EM_os_init();
    /* Initialize Debug Module */
    EM_debug_init();
    /* Initialize Timer Module */
    EM_timer_init();
    timer_em_init();
    /* Initialize utilities */
    nvsto_init();
    /* Initialize Mesh Stack */
    MS_init(config_ptr);
    /* Register with underlying BLE stack */
    blebrr_register();
    /* Create Node */
    retval = MS_access_create_node(&node_id);
    /* Register Element */
    /**
        TBD: Define GATT Namespace Descriptions from
        https://www.bluetooth.com/specifications/assigned-numbers/gatt-namespace-descriptors

        Using 'main' (0x0106) as Location temporarily.
    */
    element.loc = 0x0106;
    retval = MS_access_register_element
             (
                 node_id,
                 &element,
                 &element_handle
             );

    if (API_SUCCESS == retval)
    {
        /* Register Configuration model client */
        retval = UI_register_config_model_client(element_handle);
    }

    if (API_SUCCESS == retval)
    {
        /* Register Generic OnOff model client */
        retval = UI_register_generic_onoff_model_client(element_handle);
    }

    /* Configure as provisioner */
    UI_register_prov();
    /**
        setup <role:[1 - Device, 2 - Provisioner]> <bearer:[1 - Adv, 2 - GATT]
    */
    role = PROV_ROLE_PROVISIONER;
    brr = PROV_BRR_ADV;
    UI_setup_prov(role, brr);
    return;
}

#endif /* (MESH_STANDALONE == 7) */

