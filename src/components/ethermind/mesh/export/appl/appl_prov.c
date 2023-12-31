
/**
    \file appl_prov.c


*/

/*
    Copyright (C) 2013. Mindtree Limited.
    All rights reserved.
*/

/* --------------------------------------------- Header File Inclusion */
#include "MS_prov_api.h"
#include "prov_pl.h"
#include "appl_main.h"
#include "blebrr.h"
#include "cry.h"

static inline uint32 clock_time_rtc(void)
{
    return (*(volatile unsigned int*)0x4000f028) & 0xffffff;
}

/**
    Compilation switch to use selected Capabilities and Keys for ease of
    PTS testing.
    This Flag needs to be disabled by default to explore full range of
    provisioning capabilities and have specific peer provided OOB Pbulic keys etc.
*/
/* #define MESH_PTS_TEST */

/* --------------------------------------------- Global Definitions */
/** Size of beacon list to be maintained by Provisioner */
#define APPL_UNPROV_BEACON_LIST_SIZE            8

/** Beacon setup timeout in seconds */
#define APPL_PROV_SETUP_TIMEOUT_SECS            30

/** Attention timeout for device in seconds */
#define APPL_PROV_DEVICE_ATTENTION_TIMEOUT      30

/** Output OOB Actions supported */
#ifndef MESH_PTS_TEST
#define APPL_PROV_OUTPUT_OOB_ACTIONS \
    (PROV_MASK_OOOB_ACTION_BLINK | PROV_MASK_OOOB_ACTION_BEEP | \
     PROV_MASK_OOOB_ACTION_VIBRATE | PROV_MASK_OOOB_ACTION_NUMERIC | \
     PROV_MASK_OOOB_ACTION_ALPHANUMERIC)

/** Output OOB Maximum size supported */
#define APPL_PROV_OUTPUT_OOB_SIZE               0x08

/** Input OOB Actions supported */
#define APPL_PROV_INPUT_OOB_ACTIONS \
    (PROV_MASK_IOOB_ACTION_PUSH | PROV_MASK_IOOB_ACTION_TWIST | \
     PROV_MASK_IOOB_ACTION_NUMERIC | PROV_MASK_IOOB_ACTION_ALPHANUMERIC)

/** Input OOB Maximum size supported */
#define APPL_PROV_INPUT_OOB_SIZE                0x08
#else /* MESH_PTS_TEST */
#define APPL_PROV_OUTPUT_OOB_ACTIONS \
    PROV_MASK_OOOB_ACTION_NUMERIC

/** Output OOB Maximum size supported */
#define APPL_PROV_OUTPUT_OOB_SIZE               0x04

/** Input OOB Actions supported */
#define APPL_PROV_INPUT_OOB_ACTIONS \
    PROV_MASK_IOOB_ACTION_NUMERIC

/** Input OOB Maximum size supported */
#define APPL_PROV_INPUT_OOB_SIZE                0x04
#endif /* MESH_PTS_TEST */

/** Authentication values for OOB Display - To be made random */
#define APPL_DISPLAY_AUTH_DIGIT                 3
#define APPL_DISPLAY_AUTH_NUMERIC               35007
#define APPL_DISPLAY_AUTH_STRING                "f00l"

/** Base address as a provisioner */
#define APPL_UADDR_BASE                         0x0001
#define APPL_DEVICE_PRIMARY_ELEMENT_UCAST_ADDR  0x0100

/* --------------------------------------------- External Global Variables */

/* --------------------------------------------- Exported Global Variables */
void appl_prov_setup_provisioner(void);
void appl_prov_start_provisioning(void);
void appl_prov_register(void);

void appl_prov_input_auth_val (UCHAR mode, void* data, UINT16 datalen);
void appl_prov_set_dev_public_key(void);
void appl_prov_get_local_public_key(void);
void appl_prov_set_debug_keys(void);
void appl_prov_set_auth_action
(
    UCHAR mode,
    UCHAR* s_oob_val,
    UCHAR oob_act,
    UCHAR oob_sz
);

void main_prov_operations (void);
void appl_prov_setup_device(void);
API_RESULT appl_prov_callback
(
    PROV_HANDLE* phandle,
    UCHAR         event_type,
    API_RESULT    event_result,
    void*         event_data,
    UINT16        event_datalen
);

/* --------------------------------------------- Static Global Variables */
/** Registration state of provisioning */
DECL_STATIC UCHAR appl_prov_ready;

/** Current role of application - Provisioner/Device */
DECL_STATIC UCHAR appl_prov_role;

DECL_STATIC UCHAR appl_prov_device_num_elements;

/** Provisioning capabilities of local device */
#ifdef MESH_PTS_TEST
DECL_STATIC PROV_CAPABILITIES_S appl_prov_capab =
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
    { APPL_PROV_OUTPUT_OOB_ACTIONS, APPL_PROV_OUTPUT_OOB_SIZE },

    /** Input OOB information */
    { APPL_PROV_INPUT_OOB_ACTIONS, APPL_PROV_INPUT_OOB_SIZE },
};
#else /* MESH_PTS_TEST */
/**
    Choose this if application needs to use bare minimum capabilities
    to get provisioned against popular mobile phone Mesh Applications.
*/
DECL_STATIC PROV_CAPABILITIES_S appl_prov_capab =
{
    /** Number of Elements */
    0x01,

    /** Supported algorithms */
    PROV_MASK_ALGO_EC_FIPS_P256,

    /** Public key type */
    0x00,

    /** Static OOB type */
    0x00,

    /** Output OOB information */
    { 0x00, 0x00 },

    /** Input OOB information */
    { 0x00, 0x00 },
};
#endif /* MESH_PTS_TEST */

/** Default Provisioning method to start */
DECL_STATIC PROV_METHOD_S appl_prov_method =
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

/** Unprovisioned device identifier */
DECL_STATIC PROV_DEVICE_S appl_lprov_device =
{
    /** UUID */
    /* {0x45, 0x74, 0x68, 0x65, 0x72, 0x4d, 0x69, 0x6e, 0x64, 0x4d, 0x65, 0x73, 0x68, 0x44, 0x65, 0x76}, */
    /* {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11}, */
    {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF},

    /** OOB Flag */
    0x00,

    /**
        Encoded URI Information
        For example, to give a web address, "https://www.abc.com"
        the URI encoded data would be -
        0x17 0x2F 0x2F 0x77 0x77 0x77 0x2E 0x61 0x62 0x63 0x2E 0x63 0x6F 0x6D
        where 0x17 is the URI encoding for https:
    */
    NULL
};

/** Data exchanged during Provisiong procedure */
DECL_STATIC PROV_DATA_S appl_prov_data =
{
    /** NetKey */
    {0x45, 0x74, 0x68, 0x65, 0x72, 0x4d, 0x69, 0x6e, 0x64, 0x4e, 0x65, 0x74, 0x4b, 0x65, 0x79, 0x00},

    /** Index of the NetKey */
    0x0000,

    /** Flags bitmask */
    0x00,

    /** Current value of the IV index */
    0x00000001,

    /** Unicast address of the primary element */
    APPL_DEVICE_PRIMARY_ELEMENT_UCAST_ADDR
};

/** Unprovisioned device beacons list */
DECL_STATIC PROV_DEVICE_S appl_rprov_device[APPL_UNPROV_BEACON_LIST_SIZE];

/** Unprovisioned device beacons count */
DECL_STATIC CHAR appl_num_devices;

/** Provisioning Handle */
DECL_STATIC PROV_HANDLE appl_prov_handle;

DECL_STATIC const char main_prov_options[] = "\n\
========= Provisioning Menu =============== \n\
    0.  Exit. \n\
    1.  Refresh \n\
 \n\
   10.  Register Provisioning. \n\
 \n\
   11.  Setup Provisioner. \n\
   12.  Setup Device. \n\
 \n\
   15.  Bind Device. \n\
   16.  Enter OOB Authval. \n\
 \n\
   30.  Set OOB Public Key for Device. \n\
   31.  Set OOB Authentication. \n\
   32.  Get Current Public Key. \n\
   33.  Set ECDH Debug Key Pairs. \n\
 \n\
   40.  Abort Procedure. \n\
 \n\
  100.  Provide Provisioning Data to Network and Transport Layer (for testing)\n\
 \n\
Your Option ? \0";

/* --------------------------------------------- Functions */
void appl_prov_set_uuid (UCHAR* uuid)
{
    EM_mem_copy (appl_lprov_device.uuid, uuid, MS_DEVICE_UUID_SIZE);
}

void appl_prov_update_uuid(UCHAR octet)
{
    appl_lprov_device.uuid[0] = octet;
}

void appl_prov_bind_device(UCHAR brr)
{
    API_RESULT retval;
    PROV_DEVICE_S* appl_tmp_dev_to_bind;

    if (PROV_ROLE_PROVISIONER == appl_prov_role)
    {
        appl_tmp_dev_to_bind = &appl_rprov_device[0x00];
    }
    else
    {
        appl_tmp_dev_to_bind = &appl_lprov_device;
    }

    printf("Binding device for provisioning...\n");
    retval = MS_prov_bind
             (
                 PROV_BRR_GATT,
                 appl_tmp_dev_to_bind,
                 APPL_PROV_DEVICE_ATTENTION_TIMEOUT,
                 &appl_prov_handle
             );
    printf("Retval - 0x%04X\n", retval);
}

API_RESULT appl_prov_callback
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
    static UCHAR  prov_flag;
    static UINT16 prov_keyid;

    /* printf("\n"); */

    switch (event_type)
    {
    case PROV_EVT_UNPROVISIONED_BEACON:
        if (APPL_UNPROV_BEACON_LIST_SIZE == appl_num_devices)
        {
            /* printf ("Device List Full. Dropping Beacon...\n"); */
            break;
        }

        /* Reference the beacon pointer */
        rdev = (PROV_DEVICE_S*)event_data;
        retval = API_SUCCESS;

        for (i = 0; i < APPL_UNPROV_BEACON_LIST_SIZE; i++)
        {
            if (0 == EM_mem_cmp(rdev->uuid, appl_rprov_device[i].uuid, 16))
            {
                /* Mark for filtering */
                retval = API_FAILURE;
                /**
                    Break when there is an exisiting instance for incoming
                    UUID of the unprovisioned device.
                */
                break;
            }
        }

        if (API_SUCCESS != retval)
        {
            retval = API_SUCCESS;
            break;
        }

        printf ("Recvd PROV_EVT_UNPROVISIONED_BEACON\n");
        printf ("Status - 0x%04X\n", event_result);
        printf("\nID: %d", appl_num_devices);
        printf("\nUUID : [");
        #if 0

        for (i = 0; i < MS_DEVICE_UUID_SIZE; i++)
        {
            printf("%02X", rdev->uuid[i]);
        }

        #else /* 0 */
        EM_mem_set(pdata, 0x0, sizeof(pdata));
        t_data = pdata;

        for (i = 0; i < (MS_DEVICE_UUID_SIZE); i++)
        {
            sprintf((char*)t_data,"%02X", rdev->uuid[i]);
            t_data += 2;
        }

        printf(" %s ", pdata);
        #endif /* 0 */
        printf("]");
        printf("\nOOB  : 0x%04X", rdev->oob);
        printf("\nURI  : 0x%08X", rdev->uri);
        printf("\n\n");
        /* Copy the beacon to the list */
        EM_mem_copy (&appl_rprov_device[appl_num_devices], rdev, sizeof (PROV_DEVICE_S));
        appl_num_devices++;
        break;

    case PROV_EVT_PROVISIONING_SETUP:
        printf ("Recvd PROV_EVT_PROVISIONING_SETUP\n");
        printf ("Status - 0x%04X\n", event_result);

        /* Decipher the data based on the role */
        if (PROV_ROLE_PROVISIONER == appl_prov_role)
        {
            /* Display the capabilities */
            rcap = (PROV_CAPABILITIES_S*)event_data;
            printf ("Remote Device Capabilities:\n");
            printf ("\tNum Elements     - %d\n", rcap->num_elements);
            printf ("\tSupp Algorithms  - 0x%04X\n", rcap->supp_algorithms);
            printf ("\tSupp PublicKey   - 0x%02X\n", rcap->supp_pubkey);
            printf ("\tSupp Static OOB  - 0x%02X\n", rcap->supp_soob);
            printf ("\tOutput OOB Size  - %d\n", rcap->ooob.size);
            printf ("\tOutput OOB Action- 0x%04X\n", rcap->ooob.action);
            printf ("\tInput OOB Size   - %d\n", rcap->ioob.size);
            printf ("\tInput OOB Action - 0x%04X\n", rcap->ioob.action);
            /* Store the current number of elements */
            appl_prov_device_num_elements = rcap->num_elements;
            /* Start with default method */
            printf ("Start Provisioning with default method...\n");
            retval = MS_prov_start (phandle, &appl_prov_method);
            printf ("Retval - 0x%04X\n", retval);
        }
        else
        {
            /* Display the attention timeout */
            printf ("Attention Timeout - %d\n", *((UCHAR*)event_data));
        }

        break;

    case PROV_EVT_OOB_DISPLAY:
        printf ("Recvd PROV_EVT_OOB_DISPLAY\n");
        printf ("Status - 0x%04X\n", event_result);
        /* Reference the Authenticatio Type information */
        oob_info = (PROV_OOB_TYPE_S*)event_data;
        printf("Authenticaion Action - 0x%02X\n", oob_info->action);
        printf("Authenticaion Size - 0x%02X\n", oob_info->size);

        /* If role is Device, the action is of Output OOB, else Input OOB */
        if (PROV_ROLE_DEVICE == appl_prov_role)
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
            EM_str_copy (authstr, APPL_DISPLAY_AUTH_STRING);
            printf("\n\n>>> AuthVal - %s <<<\n\n", authstr);
            pauth = authstr;
            authsize = EM_str_len(authstr);
        }
        else if (2 == authtype)
        {
            authnum = (UINT32)APPL_DISPLAY_AUTH_NUMERIC;
            printf("\n\n>>> AuthVal - %d <<<\n\n", authnum);
            pauth = (UCHAR*)&authnum;
            authsize = sizeof(UINT32);
        }
        else
        {
            authnum = (UINT32)APPL_DISPLAY_AUTH_DIGIT;
            printf("\n\n>>> AuthVal - %d <<<\n\n", authnum);
            pauth = (UCHAR*)&authnum;
            authsize = sizeof(UINT32);
        }

        /* Call to input the oob */
        printf("Setting the Authval...\n");
        retval = MS_prov_set_authval(&appl_prov_handle, pauth, authsize);
        printf("Retval - 0x%04X\n", retval);
        break;

    case PROV_EVT_OOB_ENTRY:
        printf ("Recvd PROV_EVT_OOB_ENTRY\n");
        printf ("Status - 0x%04X\n", event_result);
        /* Reference the Authenticatio Type information */
        oob_info = (PROV_OOB_TYPE_S*)event_data;
        printf("Authenticaion Action - 0x%02X\n", oob_info->action);
        printf("Authenticaion Size - 0x%02X\n", oob_info->size);
        break;

    case PROV_EVT_DEVINPUT_COMPLETE:
        printf ("Recvd PROV_EVT_DEVINPUT_COMPLETE\n");
        printf ("Status - 0x%04X\n", event_result);
        break;

    case PROV_EVT_PROVDATA_INFO_REQ:
        printf ("Recvd PROV_EVT_PROVDATA_INFO_REQ\n");
        printf ("Status - 0x%04X\n", event_result);
        /* Send Provisioning Data */
        printf ("Send Provisioning Data...\n");
        retval = MS_prov_data (phandle, &appl_prov_data);
        printf ("Retval - 0x%04X\n", retval);
        printf ("NetKey  : ");
        appl_dump_bytes(appl_prov_data.netkey, PROV_KEY_NETKEY_SIZE);
        printf ("Key ID  : 0x%04X\n", appl_prov_data.keyid);
        printf ("Flags   : 0x%02X\n", appl_prov_data.flags);
        printf ("IVIndex : 0x%08X\n", appl_prov_data.ivindex);
        printf ("UAddr   : 0x%04X\n", appl_prov_data.uaddr);
        break;

    case PROV_EVT_PROVDATA_INFO:
        printf ("Recvd PROV_EVT_PROVDATA_INFO\n");
        printf ("Status - 0x%04X\n", event_result);
        /* Reference the Provisioning Data */
        rdata = (PROV_DATA_S*)event_data;
        printf ("NetKey  : ");
        appl_dump_bytes(rdata->netkey, PROV_KEY_NETKEY_SIZE);
        printf ("Key ID  : 0x%04X\n", rdata->keyid);
        printf ("Flags   : 0x%02X\n", rdata->flags);
        printf ("IVIndex : 0x%08X\n", rdata->ivindex);
        printf ("UAddr   : 0x%04X\n", rdata->uaddr);
        // ========HZF
        UINT32  T1, T2;
        blebrr_scan_pl(FALSE);
        T1 = clock_time_rtc();//read_current_fine_time();
        /* Provide Provisioning Data to Access Layer */
        MS_access_cm_set_prov_data
        (
            rdata
        );
        T2 = clock_time_rtc();
        printf("consume time of function MS_access_cm_set_prov_data-1: %d RTC tick\r\n", (T2 - T1));
        /* Cache the Flags and Key Index to the Globals */
        prov_flag  = rdata->flags;
        prov_keyid = rdata->keyid;
        break;

    case PROV_EVT_PROVISIONING_COMPLETE:
        printf ("Recvd PROV_EVT_PROVISIONING_COMPLETE\n");
        printf ("Status - 0x%04X\n", event_result);
        printf ("appl_prov_role - 0x%02X\n", appl_prov_role);

        if (API_SUCCESS == event_result)
        {
            if (PROV_ROLE_PROVISIONER == appl_prov_role)
            {
                if (APPL_DEVICE_PRIMARY_ELEMENT_UCAST_ADDR == appl_prov_data.uaddr)
                {
                    /* Holding a temporary structure for local prov data */
                    PROV_DATA_S temp_ps_prov_data;
                    EM_mem_copy
                    (
                        &temp_ps_prov_data,
                        &appl_prov_data,
                        sizeof(appl_prov_data)
                    );
                    /**
                        Assigning the Local Unicast Address of the Provisioner
                        to the Access Layer along with other related keys.
                    */
                    temp_ps_prov_data.uaddr = 0x0001;
// ========HZF
                    UINT32  T1, T2;
                    blebrr_scan_pl(FALSE);
                    T1 = clock_time_rtc();//read_current_fine_time();
                    /* Provide Provisioning Data to Access Layer */
                    MS_access_cm_set_prov_data
                    (
                        &temp_ps_prov_data
                    );
                    T2 = clock_time_rtc();
                    printf("consume time of function MS_access_cm_set_prov_data-2: %d RTC tick\r\n", (T2 - T1));
                    /**
                        TODO:
                        Increment the appl_prov_data.uaddr for the next
                        set of device which is getting provisioned based on
                        the address and number of elements present in the last
                        provisioned device.
                    */
                }

                appl_prov_data.uaddr += appl_prov_device_num_elements;
            }
            else
            {
                /* Already Set while handling PROV_EVT_PROVDATA_INFO */

                /**
                    TODO/NOTE:
                    The sending of Secure Network Beacon from Provisioning
                    Complete might not be feasible over GATT Bearer.
                    This is because the Provisioner might disconnect the
                    link once Provisioning is completed!
                    To then send an "SNB", one can use the CLI commands or
                    make use of the Callback event "BLEBRR_GATT_IFACE_DOWN"
                    received through * the callback registered using the API
                    \ref blebrr_register_gatt_iface_event_pl(...)
                */
                if (0x00 != prov_flag)
                {
                    MS_SUBNET_HANDLE subnet;
                    /* Get associated Subnet Handle */
                    retval = MS_access_cm_find_subnet
                             (
                                 prov_keyid,
                                 &subnet
                             );

                    if (API_SUCCESS == retval)
                    {
                        /* Send the Secure Network Beacon */
                        MS_net_broadcast_secure_beacon(subnet);
                    }
                }
            }
        }

        #ifdef MESH_PTS_TEST

        /**
            This piece is added for Unprovisioned beacon Auto Start.
            This was needed for few invalid behaviour PTS testcases.
        */
        if (API_SUCCESS != event_result)
        {
            if (PROV_ROLE_DEVICE == appl_prov_role)
            {
                printf ("Setting up Device...\n");
                retval = MS_prov_setup
                         (
                             PROV_BRR_ADV,
                             PROV_ROLE_DEVICE,
                             &appl_lprov_device,
                             APPL_PROV_SETUP_TIMEOUT_SECS,
                             APPL_PROV_SETUP_TIMEOUT_SECS
                         );
                printf ("Retval - 0x%04X\n", retval);
                appl_prov_role = PROV_ROLE_DEVICE;
                retval = MS_prov_bind(PROV_BRR_ADV, &appl_rprov_device[0x00], APPL_PROV_DEVICE_ATTENTION_TIMEOUT, &appl_prov_handle);
                printf("Retval - 0x%04X\n", retval);
            }
        }

        #endif /* MESH_PTS_TEST */
        break;

    default:
        printf ("Unknown Event - 0x%02X\n", event_type);
    }

    return API_SUCCESS;
}

void main_prov_operations (void)
{
    UCHAR pubkey[PROV_PUBKEY_SIZE_PL];
    UCHAR auth[PROV_AUTHVAL_SIZE_PL];
    API_RESULT retval;
    int choice;
    UCHAR brr, i;
    UCHAR authstr[PROV_AUTHVAL_SIZE_PL << 1];
    UINT32 authnum;
    UCHAR* pauth;
    UINT16 authsize;

    while (1)
    {
        printf("%s", main_prov_options);
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
            appl_prov_register();
            break;

        case 11:
            appl_prov_setup_provisioner();
            break;

        case 12:
            printf ("Select the Bearer (0: PB-ADV, 1: PB-GATT): ");
            fflush (stdout);
            scanf ("%d", &choice);
            /* Set the Bearer */
            brr = (0 != choice)? PROV_BRR_GATT: PROV_BRR_ADV;

            if (PROV_BRR_GATT == brr)
            {
                blebrr_gatt_mode_set(BLEBRR_GATT_PROV_MODE);
            }

            printf ("Setting up Device...\n");
            retval = MS_prov_setup
                     (
                         brr,
                         PROV_ROLE_DEVICE,
                         &appl_lprov_device,
                         APPL_PROV_SETUP_TIMEOUT_SECS,
                         APPL_PROV_SETUP_TIMEOUT_SECS
                     );
            printf ("Retval - 0x%04X\n", retval);
            appl_prov_role = PROV_ROLE_DEVICE;
            break;

        case 15:
            appl_prov_start_provisioning();
            break;

        case 16:
            printf("Select OOB type (0 - Number, 1- String): ");
            scanf("%d", &choice);

            if (0 != choice)
            {
                printf("Enter the Authentication String: ");
                scanf("%s", authstr);
                pauth = authstr;
                authsize = EM_str_len(authstr);
            }
            else
            {
                printf("Enter the Authentication Value: ");
                scanf("%d", &choice);
                authnum = (UINT32)choice;
                pauth = (UCHAR*)&authnum;
                authsize = sizeof (UINT32);
            }

            /* Call to input the oob */
            printf("Input the Authval...\n");
            retval = MS_prov_input_authval(&appl_prov_handle, pauth, authsize);
            printf("Retval - 0x%04X\n", retval);
            break;

        case 30:
            #ifndef MESH_PTS_TEST
            printf("Device supports OOB Public Key? (1/0): ");
            scanf("%d", &choice);
            /* Update the Public Key OOB in method */
            appl_prov_method.pubkey = (choice)? PROV_PUBKEY_OOB: PROV_PUBKEY_OOB;

            if (0 != choice)
            {
                /* Set the OOB Public Key */
                printf("Enter OOB Public Key of device (64 octets):\n");

                for (i = 0; i < sizeof(pubkey); i++)
                {
                    scanf("%x", &choice);
                    pubkey[i] = (UCHAR)choice;
                }

                prov_set_device_oob_pubkey_pl(pubkey, sizeof(pubkey));
            }

            #else /* MESH_PTS_TEST */
            appl_prov_set_dev_public_key();
            #endif /* MESH_PTS_TEST */
            break;

        case 31:
            printf("Select OOB Authentication type?\n 0 - None\n 1 - Static\n 2 - Output\n 3 - Input\n: ");
            scanf("%d", &choice);
            /* Update the Public Key OOB in method */
            appl_prov_method.auth = choice;

            if (PROV_AUTH_OOB_STATIC == choice)
            {
                /* Set the Static OOB Auth Key */
                printf("Enter OOB Authrntication value (16 octets):\n");

                for (i = 0; i < sizeof(auth); i++)
                {
                    #ifndef MESH_PTS_TEST
                    scanf("%x", &choice);
                    auth[i] = (UCHAR)choice;
                    #else /* MESH_PTS_TEST */
                    /**
                        Set the STATIC OOB value in PTS PIXIT table:
                        OOB: 0x11111111111111111111111111111111[16 Octets 0f 0x11]
                    */
                    auth[i] = 0x11;
                    #endif /* MESH_PTS_TEST */
                }

                prov_set_static_oob_auth_pl(auth, sizeof(auth));
            }
            else if (PROV_AUTH_OOB_OUTPUT == choice)
            {
                /* Enter the OOB action to be used */
                printf("Enter OOB Action?\n 0 - Blink\n 1 - Beep\n 2 - Vibrate\n 3 - Numeric\n 4 - Alphanumeric\n: ");
                scanf("%d", &choice);
                appl_prov_method.oob.action = (UCHAR)choice;
                /* Enter the OOB size to be used */
                printf("Enter OOB Size (1 - 8): ");
                scanf("%d", &choice);
                appl_prov_method.oob.size = (UCHAR)choice;
            }
            else if (PROV_AUTH_OOB_INPUT == choice)
            {
                /* Enter the OOB action to be used */
                printf("Enter OOB Action?\n 0 - Push\n 1 - Twist\n 2 - Numeric\n 3 - Alphanumeric\n: ");
                scanf("%d", &choice);
                appl_prov_method.oob.action = (UCHAR)choice;
                /* Enter the OOB size to be used */
                printf("Enter OOB Size (1 - 8): ");
                scanf("%d", &choice);
                appl_prov_method.oob.size = (UCHAR)choice;
            }

            break;

        case 32:
            /* Print the Local ECDH Public Key */
            appl_prov_get_local_public_key();
            break;

        case 33:
            #ifdef MESH_PTS_TEST
            /* PTS defines the below Public/PrivateKey as its Default in PIXIT */
            appl_prov_set_debug_keys();
            #endif /* MESH_PTS_TEST */
            break;

        case 40:
            printf("Enter the reason to Abort (in Hex): ");
            scanf("%x", &choice);
            printf("Aborting...\n");
            retval = MS_prov_abort(&appl_prov_handle, (UCHAR)choice);
            printf("Retval - 0x%04X\n", retval);
            break;

        case 100:
            break;

        default:
            break;
        }
    }
}

API_RESULT appl_prov_update_remote_uuid
(
    UCHAR*   uuid,
    UINT16 uuid_len,
    UINT16 oob_info,
    UCHAR*   uri
)
{
    API_RESULT retval;
    UINT32 i;
    retval = API_SUCCESS;

    for (i = 0; i < APPL_UNPROV_BEACON_LIST_SIZE; i++)
    {
        if (0 == EM_mem_cmp(uuid, appl_rprov_device[i].uuid, 16))
        {
            /* Mark for filtering */
            retval = API_FAILURE;
            /**
                Break when there is an exisiting instance for incoming
                UUID of the unprovisioned device.
            */
            break;
        }
    }

    if (API_SUCCESS != retval)
    {
        return retval;
    }

    printf ("Recvd PROV_EVT_UNPROVISIONED_BEACON");
    printf ("Status - 0x%04X", API_SUCCESS);
    /* printf("\nID: %d", appl_num_devices); */
    printf("UUID  : [%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X]",
           uuid[0], uuid[1], uuid[2], uuid[3], uuid[4], uuid[5],
           uuid[6], uuid[7], uuid[8], uuid[9], uuid[10], uuid[11],
           uuid[2], uuid[13], uuid[14], uuid[15]);
    printf("OOB  : 0x%04X", oob_info);

    if (NULL != uri)
    {
        printf("\nURI  : 0x%08X", uri);
    }

    /* Copy the beacon to the list */
    EM_mem_copy (&appl_rprov_device[appl_num_devices].uuid, uuid, MS_DEVICE_UUID_SIZE);
    appl_rprov_device[appl_num_devices].oob = oob_info;
    /* appl_rprov_device[appl_num_devices].uri = uri; */
    /* Increment the list */
    appl_num_devices++;
    return retval;
}



void appl_prov_register(void)
{
    API_RESULT retval;
    MS_PROV_DEV_ENTRY prov_dev_list[MS_MAX_DEV_KEYS];
    UINT16            num_entries;
    UINT16            pointer_entries;

    if (MS_TRUE == appl_prov_ready)
    {
        return;
    }

    appl_prov_ready = MS_TRUE;
    /* Fetch the number of elements in local node */
    MS_access_cm_get_element_count(&appl_prov_capab.num_elements);
    num_entries = MS_MAX_DEV_KEYS;
    /* Update the next device address if provisioned devices are present in database */
    retval = MS_access_cm_get_prov_devices_list
             (
                 &prov_dev_list[0],
                 &num_entries,
                 &pointer_entries
             );

    if ((API_SUCCESS == retval) &&
            (0 != num_entries))
    {
        appl_prov_data.uaddr = prov_dev_list[num_entries - 1].uaddr +
                               prov_dev_list[num_entries - 1].num_elements;
        printf("Updating Provisioning Start Addr to 0x%04X\n", appl_prov_data.uaddr);
    }

    printf("Registering with Provisioning layer...\n");
    retval = MS_prov_register(&appl_prov_capab, appl_prov_callback);
    printf("Retval - 0x%04X\n", retval);
}

void appl_prov_setup(UCHAR role, UCHAR brr)
{
    API_RESULT retval;

    if (PROV_BRR_GATT == brr)
    {
        blebrr_gatt_mode_set(BLEBRR_GATT_PROV_MODE);
    }

    if (PROV_ROLE_PROVISIONER != role)
    {
        printf("Setting up Device for Provisioning ...\n");
        retval = MS_prov_setup
                 (
                     brr,
                     role,
                     &appl_lprov_device,
                     APPL_PROV_SETUP_TIMEOUT_SECS,
                     APPL_PROV_SETUP_TIMEOUT_SECS
                 );
        appl_prov_role = PROV_ROLE_DEVICE;
    }
    else
    {
        printf("Setting up Provisioner for Provisioning ...\n");
        retval = MS_prov_setup
                 (
                     brr,
                     role,
                     NULL,
                     APPL_PROV_SETUP_TIMEOUT_SECS,
                     APPL_PROV_SETUP_TIMEOUT_SECS
                 );
        appl_prov_role = PROV_ROLE_PROVISIONER;
        appl_num_devices = 0;
        /* Clear the list of Unprovisioned Device Here */
        EM_mem_set(appl_rprov_device, 0x0, sizeof(appl_rprov_device));
        /* Enable Scan */
        blebrr_scan_enable();
    }

    printf("Retval - 0x%04X\n", retval);
}

void appl_prov_bind(UCHAR brr, UCHAR index)
{
    API_RESULT retval;
    /* Call to bind with the selected device */
    printf("Binding with the selected device...\n");

    if (PROV_ROLE_PROVISIONER == appl_prov_role)
    {
        retval = MS_prov_bind(brr, &appl_rprov_device[index], APPL_PROV_DEVICE_ATTENTION_TIMEOUT, &appl_prov_handle);
    }
    else
    {
        retval = MS_prov_bind(brr, &appl_lprov_device, APPL_PROV_DEVICE_ATTENTION_TIMEOUT, &appl_prov_handle);
    }

    printf("Retval - 0x%04X\n", retval);
}

void appl_prov_setup_provisioner(void)
{
//    API_RESULT retval;
    UCHAR      brr;
    int choice;
    printf ("Select the Bearer (0: PB-ADV, 1: PB-GATT): ");
    fflush (stdout);
    scanf ("%d", &choice);
    /* Set the Bearer */
    brr = (0 != choice)? PROV_BRR_GATT: PROV_BRR_ADV;
    appl_prov_setup(PROV_ROLE_PROVISIONER, brr);
}

void appl_prov_setup_device(void)
{
//    API_RESULT retval;
    UCHAR      brr;
    int choice;
    printf("Select the Bearer (0: PB-ADV, 1: PB-GATT): ");
    fflush(stdout);
    scanf("%d", &choice);
    /* Set the Bearer */
    brr = (0 != choice) ? PROV_BRR_GATT : PROV_BRR_ADV;
    appl_prov_setup(PROV_ROLE_DEVICE, brr);

    if (PROV_BRR_ADV == brr)
    {
        appl_prov_bind(brr, 0);
    }
}

static void appl_list_beacons(void)
{
    UINT8 i, j;

    if (0 == appl_num_devices)
    {
        printf("No Unprovisioned beacon in list...\n");
        return;
    }

    for (i = 0; i < appl_num_devices; i++)
    {
        printf("%d. [", i);

        for (j = 0; j < MS_DEVICE_UUID_SIZE; j++)
        {
            printf("%02X", appl_rprov_device[i].uuid[j]);
        }

        printf("]\n");
    }
}

void appl_prov_start_provisioning(void)
{
//    API_RESULT retval;
    UCHAR brr, i;
    int choice;

    if (PROV_ROLE_PROVISIONER == appl_prov_role)
    {
        if (0 == appl_num_devices)
        {
            printf("No Unprovisioned beacon in list...\n");
            return;
        }

        printf("Select the device to provision:\n");
        appl_list_beacons();
        scanf("%d", &choice);
        i = choice;
    }
    else
    {
        i = 0;
    }

    printf("Select the Bearer (0: PB-ADV, 1: PB-GATT): ");
    fflush(stdout);
    scanf("%d", &choice);
    /* Set the Bearer */
    brr = (0 != choice) ? PROV_BRR_GATT : PROV_BRR_ADV;
    appl_prov_bind(brr, i);
}

void appl_prov_input_auth_val (UCHAR mode, void* data, UINT16 datalen)
{
    UCHAR*       pauth;
    UINT16     authsize;
    UINT32     authnum;
    API_RESULT retval;

    /**
        MODE = 0 -> Numeric
        MODE = 1 -> Alphanumeric
    */
    if (0 != mode)
    {
        pauth    = (UCHAR*) data;
        authsize = datalen;
    }
    else
    {
        authnum  = *((UINT32*)data);
        pauth    = (UCHAR*)&authnum;
        authsize = sizeof (UINT32);
    }

    /* Call to input the oob */
    printf("Input the Authval...\n");
    retval = MS_prov_input_authval(&appl_prov_handle, pauth, authsize);
    printf("Retval - 0x%04X\n", retval);
}

void appl_prov_set_dev_public_key(void)
{
    /* PTS defines the below Publickey as its Default in PIXIT */
    UCHAR appl_t_pub_key[PROV_PUBKEY_SIZE_PL] =
    {
        0xF4, 0x65, 0xE4, 0x3F, 0xF2, 0x3D, 0x3F, 0x1B,
        0x9D, 0xC7, 0xDF, 0xC0, 0x4D, 0xA8, 0x75, 0x81,
        0x84, 0xDB, 0xC9, 0x66, 0x20, 0x47, 0x96, 0xEC,
        0xCF, 0x0D, 0x6C, 0xF5, 0xE1, 0x65, 0x00, 0xCC,
        0x02, 0x01, 0xD0, 0x48, 0xBC, 0xBB, 0xD8, 0x99,
        0xEE, 0xEF, 0xC4, 0x24, 0x16, 0x4E, 0x33, 0xC2,
        0x01, 0xC2, 0xB0, 0x10, 0xCA, 0x6B, 0x4D, 0x43,
        0xA8, 0xA1, 0x55, 0xCA, 0xD8, 0xEC, 0xB2, 0x79
    };
    /* Update the Public Key OOB in method */
    appl_prov_method.pubkey = PROV_PUBKEY_OOB;
    prov_set_device_oob_pubkey_pl(appl_t_pub_key, sizeof(appl_t_pub_key));
    printf ("Setting Device Public Key  : \n");
    appl_dump_bytes(appl_t_pub_key, sizeof(appl_t_pub_key));
}

void appl_prov_get_local_public_key(void)
{
    UCHAR  appl_t_pub_key[PROV_PUBKEY_SIZE_PL];
    UCHAR  pdata[(PROV_PUBKEY_SIZE_PL * 2) + 1];
    UCHAR*   t_data;
//    INT32  ret;
    UINT32 i;
    EM_mem_set(appl_t_pub_key, 0x0, PROV_PUBKEY_SIZE_PL);
    EM_mem_set(pdata, 0x0, sizeof(pdata));
    t_data = pdata;
    MS_prov_get_local_public_key(appl_t_pub_key);
    printf("\n Local ECDH P256 Public Key: [Dump]\n");
    appl_dump_bytes(appl_t_pub_key, PROV_PUBKEY_SIZE_PL);
    printf("\n Local ECDH P256 Public Key: [MSB-LSB]:\n");
    #if 0

    for (i = 0; i < PROV_PUBKEY_SIZE_PL; i++)
    {
        printf("%02X", appl_t_pub_key[i]);
    }

    printf("\n");
    #else

    for (i = 0; i < (PROV_PUBKEY_SIZE_PL); i++)
    {
        sprintf((char*)t_data,"%02X", appl_t_pub_key[i]);
        t_data += 2;
    }

    printf("%s\n", pdata);
    #endif
}

void appl_prov_set_debug_keys(void)
{
    /* PTS defines the below Public/PrivateKey as its Default in PIXIT */
    UCHAR appl_t_pvt_key[(PROV_PUBKEY_SIZE_PL/2)] =
    {
        0x52, 0x9A, 0xA0, 0x67, 0x0D, 0x72, 0xCD, 0x64,
        0x97, 0x50, 0x2E, 0xD4, 0x73, 0x50, 0x2B, 0x03,
        0x7E, 0x88, 0x03, 0xB5, 0xC6, 0x08, 0x29, 0xA5,
        0xA3, 0xCA, 0xA2, 0x19, 0x50, 0x55, 0x30, 0xBA
    };
    UCHAR appl_t_pub_key[PROV_PUBKEY_SIZE_PL] =
    {
        0xF4, 0x65, 0xE4, 0x3F, 0xF2, 0x3D, 0x3F, 0x1B,
        0x9D, 0xC7, 0xDF, 0xC0, 0x4D, 0xA8, 0x75, 0x81,
        0x84, 0xDB, 0xC9, 0x66, 0x20, 0x47, 0x96, 0xEC,
        0xCF, 0x0D, 0x6C, 0xF5, 0xE1, 0x65, 0x00, 0xCC,
        0x02, 0x01, 0xD0, 0x48, 0xBC, 0xBB, 0xD8, 0x99,
        0xEE, 0xEF, 0xC4, 0x24, 0x16, 0x4E, 0x33, 0xC2,
        0x01, 0xC2, 0xB0, 0x10, 0xCA, 0x6B, 0x4D, 0x43,
        0xA8, 0xA1, 0x55, 0xCA, 0xD8, 0xEC, 0xB2, 0x79
    };
    cry_set_ecdh_debug_keypair(appl_t_pvt_key, appl_t_pub_key);
    printf ("Setting Debug Public Key  : \n");
    appl_dump_bytes(appl_t_pub_key, sizeof(appl_t_pub_key));
    printf ("Setting Debug Private Key  : \n");
    appl_dump_bytes(appl_t_pvt_key, sizeof(appl_t_pvt_key));
}

void appl_prov_set_auth_action
(
    UCHAR mode,
    UCHAR* s_oob_val,
    UCHAR oob_act,
    UCHAR oob_sz
)
{
    UCHAR auth[PROV_AUTHVAL_SIZE_PL];
    /**
        Valid Values of Mode for OOB usage are:
        0x00 - None
        0x01 - Static
        0x02 - Output
        0x03 - Input
    */
    /* Update the Authentication method based on "Mode" in "method" */
    appl_prov_method.auth = mode;

    if (PROV_AUTH_OOB_STATIC == mode)
    {
        /**
            TODO: Have a User provided value from CLI through "s_oob_val"
        */
        /**
            Set the STATIC OOB value in PTS PIXIT table:
            OOB: 0x11111111111111111111111111111111[16 Octets 0f 0x11]
        */
        EM_mem_set(auth, 0x11, sizeof(auth));
        prov_set_static_oob_auth_pl(auth, sizeof(auth));
        printf("\n Setting PROV_AUTH_OOB_STATIC mode");
        printf("\n Static OOB used is:\n");
        appl_dump_bytes(auth, sizeof(auth));
    }
    else if (PROV_AUTH_OOB_OUTPUT == mode)
    {
        /**
            Output OOB Action valid values
            0x00 - Blink
            0x01 - Beep
            0x02 - Vibrate
            0x03 - Numeric
            0x04 - Alphanumeric
        */
        appl_prov_method.oob.action = oob_act;
        appl_prov_method.oob.size = oob_sz;
        printf("\n Setting PROV_AUTH_OOB_OUTPUT mode\n");
        printf("\n OOB of size: %02d", oob_sz);
        printf("\n OOB action : %s\n",
               (0x00 == oob_act) ? "Blink" :
               (0x01 == oob_act) ? "Beep":
               (0x02 == oob_act) ? "Vibrate":
               (0x03 == oob_act) ? "Numeric":
               (0x04 == oob_act) ? "Alphanumeric": "Unknown");
    }
    else if (PROV_AUTH_OOB_INPUT == mode)
    {
        /**
            Input OOB Action valid values
            0x00 - Push
            0x01 - Twist
            0x02 - Numeric
            0x03 - Alphanumeric
        */
        appl_prov_method.oob.action = oob_act;
        appl_prov_method.oob.size = oob_sz;
        printf("\n Setting PROV_AUTH_OOB_INPUT mode");
        printf("\n OOB of size: %02d", oob_sz);
        printf("\n OOB action : %s\n",
               (0x00 == oob_act) ? "Push" :
               (0x01 == oob_act) ? "Twist":
               (0x02 == oob_act) ? "Numeric":
               (0x03 == oob_act) ? "Alphanumeric": "Unknown");
    }
    else
    {
        /* Assign default OOB mode */
        appl_prov_method.auth = PROV_AUTH_OOB_NONE;
        printf("\n Setting PROV_AUTH_OOB_NONE mode\n");
    }
}

API_RESULT appl_is_configured (void)
{
    MS_APPKEY_HANDLE  handle;
    UINT8*              key;
    UINT8             aid;
    DECL_CONST UINT8  t_key[16] = {0};
    MS_NET_ADDR addr;
    API_RESULT retval;
    CONSOLE_OUT("Check for Primary Unicast Address\n");
    retval = MS_access_cm_get_primary_unicast_address(&addr);

    if ((API_SUCCESS != retval) ||
            (MS_NET_ADDR_UNASSIGNED == addr))
    {
        return API_FAILURE;
    }

    CONSOLE_OUT("Fetching App Key for Handle 0x0000\n");
    handle = 0x0000;
    retval = MS_access_cm_get_app_key
             (
                 handle,
                 &key,
                 &aid
             );

    /* Check Retval. Print App Key */
    if (API_SUCCESS == retval)
    {
        CONSOLE_OUT("App Key[0x%02X]: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\r\n",
                    handle, key[0], key[1], key[2], key[3], key[4], key[5], key[6], key[7],
                    key[8], key[9], key[10], key[11], key[12], key[13], key[14], key[15]);

        if (0 == EM_mem_cmp(key, t_key, 16))
        {
            /* NO AppKey Bound */
            retval = API_FAILURE;
        }
        else
        {
            /* Found a Valid App Key */
            /* Keeping the retval as API_SUCCESS */
        }
    }

    return retval;
}

