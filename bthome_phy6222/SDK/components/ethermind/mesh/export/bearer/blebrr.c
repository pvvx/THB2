/**************************************************************************************************
*******
**************************************************************************************************/


/**
    \file blebrr.c

    This File contains the BLE Bearer interface for the
    Mindtree Mesh stack.
*/

/*
    Copyright (C) 2016. Mindtree Ltd.
    All rights reserved.
*/

/* ------------------------------- Header File Inclusion */
#include "MS_brr_api.h"
#include "MS_prov_api.h"
#include "MS_access_api.h"
#include "blebrr.h"
#include "ll.h"
#include "MS_trn_api.h"
#include "pwrmgr.h"
#include "ll_sleep.h"
#include "led_light.h"



/* --------------------------------------------- External Global Variables */

/* --------------------------------------------- Global Definitions */
#define BLEBRR_MAX_ADV_FILTER_LIST_COUNT    100
#define BLEBRR_MAX_ADV_DATA_SIZE            31

#define BLEBRR_BCON_ELEMENTS                2
#define BLEBRR_ACTIVEADV_TIMEOUT            1 /* Second */
#define BLEBRR_ADV_SEC_TIMEOUT              3 /* Millisecond */
#define BLEBRR_ADV_TIMEOUT                  5 /* Millisecond */
#define BLEBRR_ADV_NON_TIMEOUT              3 /* Millisecond */
#define BLEBRR_SCAN_TIMEOUT                 (EM_TIMEOUT_MILLISEC | 30) /* Millisecond */
#define BLEBRR_NCON_ADVTYPE_OFFSET          2
#define BLEBRR_ADVREPEAT_PRO_COUNT          6
#define BLEBRR_ADVREPEAT_NET_COUNT          2
#define BLEBRR_SCAN_ADJ_STEP                30
#define BLEBRR_SCAN_ADJ_THD_MAX             45//90
#define BLEBRR_SCAN_ADJ_THD_MIN             15//90   
#define BLEBRR_TURNOFF_RELAY_THD            32
#define BLEBRR_SKIP_BEACON_QUEUE_DEPTH      0
#define BLEBRR_BCON_READY_TIME              10

/** Bearer Queue defines */
#define BLEBRR_QTYPE_DATA                   0x00
#define BLEBRR_QTYPE_BEACON                 0x01
#define BLEBRR_NUM_QUEUES                   0x02

/** Beacon type defines */
#define BLEBRR_UPROV_ADV_BCON               0x00
#define BLEBRR_UPROV_ADV_URI                0x01
#define BLEBRR_UPROV_GATT_BCON              0x02
#define BLEBRR_UPROV_GATT_URI               0x03
#define BLEBRR_SECNET_BCON                  0x04
#define BLEBRR_NUM_BCONS                    0x05

/** GATT Mode GAP Connectable Advertising Service data offset */
#define BLEBRR_GATT_ADV_SERV_DATA_OFFSET    11
#define BLEBRR_GATT_ADV_SERV_DATALEN_OFFSET 7

/** Advertising data maximum length */
#define BLEBRR_GAP_ADVDATA_LEN              31

/** Advertising data sets MAX */
#define BLEBRR_GAP_MAX_ADVDATA_SETS         2

#ifdef BLEBRR_LP_SUPPORT
    UCHAR blebrr_lp_flag = MS_FALSE;
    #define BLEBRR_LP_UNPROVISION_TIMEOUT             10*60   //unprovison timeout 10min
    #define BLEBRR_LP_PROVISIONED_TIMEOUT             1200   //
    #define BLEBRR_LP_PROVISIONED_WKP_TIMEOUT         60      //
    #define BLEBRR_LP_PROVISIONED_SLP_TIMEOUT         (BLEBRR_LP_PROVISIONED_TIMEOUT-BLEBRR_LP_PROVISIONED_WKP_TIMEOUT)
#endif




/* --------------------------------------------- Macros */
#define BLEBRR_MUTEX_INIT()                 MS_MUTEX_INIT(blebrr_mutex, BRR);
#define BLEBRR_MUTEX_INIT_VOID()            MS_MUTEX_INIT_VOID(blebrr_mutex, BRR);
#define BLEBRR_LOCK()                       MS_MUTEX_LOCK(blebrr_mutex, BRR);
#define BLEBRR_LOCK_VOID()                  MS_MUTEX_LOCK_VOID(blebrr_mutex, BRR);
#define BLEBRR_UNLOCK()                     MS_MUTEX_UNLOCK(blebrr_mutex, BRR);
#define BLEBRR_UNLOCK_VOID()                MS_MUTEX_UNLOCK_VOID(blebrr_mutex, BRR);

/* --------------------------------------------- Structures/Data Types */
/** BLEBRR Data Queue Element */
typedef struct _BLEBRR_Q_ELEMENT
{
    /* "Allocated" Data Pointer */
    UCHAR* pdata;

    /*
        Data Length. If data length is zero, the element is considered
        invalid.
    */
    UINT16 pdatalen;

    /* Type of data element */
    UCHAR type;

} BLEBRR_Q_ELEMENT;

/** BLEBRR Data Queue */
typedef struct _BLEBRR_Q
{
    /* List of Bearer Queue elements */
    BLEBRR_Q_ELEMENT element[BLEBRR_QUEUE_SIZE];

    /* Queue start index */
    UINT16 start;

    /* Queue end index */
    UINT16 end;

} BLEBRR_Q;

/** Advertising Data type */
typedef struct _BLEBRR_GAP_ADV_DATA
{
    /** Data */
    UCHAR data[BLEBRR_GAP_ADVDATA_LEN];

    /** Data Length */
    UCHAR datalen;

} BLEBRR_GAP_ADV_DATA;


/* --------------------------------------------- Global Variables */
#ifdef BLEBRR_LP_SUPPORT
    EM_timer_handle blebrr_lp_thandle;

#endif
#ifdef BLEBRR_FILTER_DUPLICATE_PACKETS
    DECL_STATIC UCHAR blebrr_adv_list[BLEBRR_MAX_ADV_FILTER_LIST_COUNT][BLEBRR_MAX_ADV_DATA_SIZE];
    DECL_STATIC UCHAR blebrr_adv_list_inser_index = 0;
#endif /* BLEBRR_FILTER_DUPLICATE_PACKETS */

BRR_BEARER_INFO blebrr_adv;  //HZF

DECL_STATIC BRR_HANDLE blebrr_advhandle;

DECL_STATIC UCHAR blebrr_bconidx;
#ifdef  BLEBRR_LP_SUPPORT
    DECL_STATIC UCHAR blebrr_beacon;
#endif
DECL_STATIC UCHAR blebrr_update_advcount = BLEBRR_BCON_READY_TIME;
DECL_STATIC BLEBRR_Q_ELEMENT blebrr_bcon[BRR_BCON_COUNT];
DECL_STATIC BLEBRR_Q blebrr_queue;

MS_DEFINE_MUTEX_TYPE(static, blebrr_mutex)
DECL_STATIC EM_timer_handle blebrr_timer_handle = EM_TIMER_HANDLE_INIT_VAL;
UCHAR blebrr_state;    // HZF
//DECL_STATIC UCHAR blebrr_state;

/* Set provision started */
UCHAR blebrr_prov_started;

UCHAR blebrr_adv_restart;


UINT32 blebrr_scanTimeOut = 100;

extern UCHAR blebrr_advtype;


DECL_STATIC UCHAR blebrr_datacount;

/* DECL_STATIC UCHAR blebrr_scan_type; */
DECL_STATIC UCHAR blebrr_advrepeat_count;
DECL_STATIC UCHAR blebrr_scan_interleave;

BLEBRR_GAP_ADV_DATA blebrr_gap_adv_data[BLEBRR_GAP_MAX_ADVDATA_SETS] =
{
    /* Index 0x00: Mesh Provisioning Service ADV Data */
    {
        {
            /**
                Flags:
                    0x01: LE Limited Discoverable Mode
                    0x02: LE General Discoverable Mode
                    0x04: BR/EDR Not Supported
                    0x08: Simultaneous LE and BR/EDR to Same Device
                          Capable (Controller)
                    0x10: Simultaneous LE and BR/EDR to Same Device
                          Capable (Host)
            */
            0x02, 0x01, 0x06,

            /**
                Service UUID List:
                    Mesh Provisioning Service (0x1827)
            */
            0x03, 0x03, 0x27, 0x18,

            /**
                Service Data List:
                    Mesh Provisioning Service (0x1827)
                    Mesh UUID (16 Bytes)
                    Mesh OOB Info (2 Bytes)
            */
            0x15, 0x16,
            0x27, 0x18,
            0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
            0x00, 0x00
        },

        /** Advertising Data length */
        29
    },
    /* Index 0x01: Mesh Proxy Service ADV Data */
    {
        {
            /**
                Flags:
                    0x01: LE Limited Discoverable Mode
                    0x02: LE General Discoverable Mode
                    0x04: BR/EDR Not Supported
                    0x08: Simultaneous LE and BR/EDR to Same Device
                          Capable (Controller)
                    0x10: Simultaneous LE and BR/EDR to Same Device
                          Capable (Host)
            */
            0x02, 0x01, 0x06,

            /**
                Service UUID List:
                    Mesh Proxy Service (0x1828)
            */
            0x03, 0x03, 0x28, 0x18,

            /**
                Service Data List:
                    Mesh Provisioning Service (0x1828)
                    Type (1 Byte) "0x00 - Network ID; 0x01 - Node Identity"
                    NetWork ID (8 Bytes)
            */
            0x0C, 0x16,
            0x28, 0x18,
            0x00,
            0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88
        },

        /** Advertising Data length */
        20
    }
};

DECL_STATIC UCHAR pl_advdata_offset;
UCHAR blebrr_sleep;


// ------------ add by HZF
uint32 blebrr_advscan_timeout_count = 0;
extern uint32_t osal_sys_tick;


/* ------------------------------- Functions */
void blebrr_handle_evt_adv_complete (UINT8 enable);
DECL_STATIC void blebrr_timer_start (UINT32 timeout);

API_RESULT blebrr_queue_depth_check(void);

/**
    \brief

    \par Description


    \return void
*/
void blebrr_scan_enable(void)
{
    BLEBRR_LOCK_VOID();

    if ((BLEBRR_STATE_IDLE == BLEBRR_GET_STATE()) &&
            (MS_TRUE != blebrr_sleep))
    {
        blebrr_scan_pl(MS_TRUE);
        /* Update state */
//        BLEBRR_SET_STATE(BLEBRR_STATE_IN_SCAN_ENABLE);
    }

    BLEBRR_UNLOCK_VOID();
}

/**
    \brief

    \par Description


    \param type
    \param bcon

    \return void
*/
DECL_STATIC UCHAR blebrr_get_beacon_type (UCHAR type, UCHAR bcon)
{
    return (BRR_BCON_PASSIVE == type)?
           ((BRR_BCON_TYPE_UNPROV_DEVICE == bcon)? BLEBRR_UPROV_ADV_BCON: BLEBRR_SECNET_BCON):
           ((BRR_BCON_TYPE_UNPROV_DEVICE == bcon)? BLEBRR_UPROV_GATT_BCON: BLEBRR_NUM_BCONS);
}

/**
    \brief

    \par Description


    \param type

    \return void
*/
DECL_STATIC BLEBRR_Q_ELEMENT* blebrr_enqueue_alloc (void)
{
    BLEBRR_Q_ELEMENT* elt;
    UINT16 ei;
    /* Get reference to the requested Queue block members */
    elt = blebrr_queue.element;
    ei = blebrr_queue.end;

    /* Check if queue end element is free */
    if (0 != (elt + ei)->pdatalen)
    {
        /* Not free */
        elt = NULL;
    }
    else
    {
        /* Set the element to be returned */
        elt = (elt + ei);
        /* Update the data availability */
//        blebrr_datacount++;

        /* EM_debug_trace (0, "[BLEBRR] Enqueue at Q Index: %d\n", ei); */

        /* Update queue end */
        if(ei == BLEBRR_QUEUE_SIZE - 1)
            ei = 0;
        else
            ei++;

//        ei++;
//        ei &= (BLEBRR_QUEUE_SIZE - 1);
        blebrr_queue.end = ei;
    }

    return elt;
}

DECL_STATIC void blebrr_dequeue_manual (void)
{
    UINT16 ei;
    ei = blebrr_queue.end;

    /* Update the data availability */
//    blebrr_datacount--;

    /* EM_debug_trace (0, "[BLEBRR] Enqueue at Q Index: %d\n", ei); */
//    printf ("[BLEBRR] Dequeue at Q Index: __%d\n", ei);

    /* Update queue end */
    if(ei == 0)
        ei = BLEBRR_QUEUE_SIZE - 1;
    else
        ei--;

    blebrr_queue.end = ei;
}


/**
    \brief

    \par Description


    \param type

    \return void
*/
DECL_STATIC BLEBRR_Q_ELEMENT* blebrr_dequeue (void)
{
    BLEBRR_Q_ELEMENT* elt;
    UINT16 si;
    /* Get reference to the requested Queue block members */
    elt = blebrr_queue.element;
    si = blebrr_queue.start;

    /* Check if queue start element is valid */
    if (0 == (elt + si)->pdatalen)
    {
        /* Not valid */
        elt = NULL;
    }
    else
    {
        /* Set the element to be returned */
        elt = (elt + si);
        /* EM_debug_trace (0, "[BLEBRR] Dequeue at Q Index: %d\n", si); */

        /* Is Adv data type in element? */
        if (BRR_BCON_COUNT == elt->type)
        {
            /* Update the data availability */
            blebrr_datacount--;
        }

        /* Update the data availability */
//        blebrr_datacount--;

        /* Update queue start */
        if(si == BLEBRR_QUEUE_SIZE - 1)
            si = 0;
        else
            si++;

//        si++;
//        si &= (BLEBRR_QUEUE_SIZE - 1);
        blebrr_queue.start = si;
    }

    return elt;
}

/**
    \brief

    \par Description


    \param bcon

    \return void
*/
DECL_STATIC void blebrr_clear_bcon (UCHAR bconidx)
{
    BLEBRR_Q_ELEMENT* elt;
    /* Get reference to the beacon queue element */
    elt = &blebrr_bcon[bconidx];

    /* Clear the element and the next one for the given type of beacon */
    if (NULL != elt->pdata)
    {
        EM_free_mem (elt->pdata);
        elt->pdata = NULL;
        elt->pdatalen = 0;
        elt->type = BRR_BCON_COUNT;

        if ((BRR_BCON_TYPE_UNPROV_DEVICE == bconidx) &&
                (NULL != (elt + 1)->pdata) &&
                (0 != (elt + 1)->pdatalen))
        {
            EM_free_mem((elt + 1)->pdata);
            (elt + 1)->pdata = NULL;
            (elt + 1)->pdatalen = 0;
            (elt + 1)->type = BRR_BCON_COUNT;
        }

        blebrr_datacount--;
    }
}

UCHAR blebrr_get_queue_depth(void)
{
    UCHAR depth;
    BLEBRR_Q_ELEMENT* elt;
    UINT16 ei;
    /* Get reference to the requested Queue block members */
    elt = blebrr_queue.element;
    ei = blebrr_queue.end;
    depth = 0;

    if((blebrr_queue.end == blebrr_queue.start) && (0 != (elt + ei)->pdatalen))
    {
        depth = BLEBRR_QUEUE_SIZE;
    }
    else if(blebrr_queue.end > blebrr_queue.start)
    {
        depth = blebrr_queue.end-blebrr_queue.start;
    }
    else if(blebrr_queue.end < blebrr_queue.start)
    {
        depth = BLEBRR_QUEUE_SIZE-(blebrr_queue.start-blebrr_queue.end);
    }

    return depth;
}

API_RESULT blebrr_queue_depth_check(void)
{
    API_RESULT retval =API_SUCCESS;
    uint8_t randData;
    UCHAR depth =blebrr_get_queue_depth();

    if(depth>BLEBRR_TURNOFF_RELAY_THD)
    {
        LL_Rand(&randData, 1);
        randData=randData>>1;

        if( depth > randData)
        {
            retval= API_FAILURE;
        }

        BLEBRR_LOG("[Queue DATA CNT] = %d %d %4X\n", depth,randData,retval);
    }

    return retval;
}

extern uint8             llState, llSecondaryState;
/**
    \brief

    \par Description

 * *
 * *  \param void

    \return void
*/
DECL_STATIC API_RESULT blebrr_update_advdata(void)
{
    BLEBRR_Q_ELEMENT* elt;
    UCHAR type;
    elt = NULL;
    UCHAR   is_proxy_beacon;
    is_proxy_beacon = 1;

    //ZQY skip bcon adv when queue is not empty
//    printf("blebrr_get_queue_depth:%d\n",blebrr_get_queue_depth());

    if(blebrr_update_advcount < BLEBRR_BCON_READY_TIME)
    {
        is_proxy_beacon = 0;
        blebrr_update_advcount++;
    }
    else
    {
        blebrr_update_advcount = 0;
    }

    if(blebrr_get_queue_depth()>BLEBRR_SKIP_BEACON_QUEUE_DEPTH)
    {
        is_proxy_beacon = 0;
    }

    if (is_proxy_beacon)
    {
        UCHAR bconidx;
        bconidx = blebrr_bconidx;

        do
        {
            if (0 != blebrr_bcon[blebrr_bconidx].pdatalen)
            {
                elt = &blebrr_bcon[blebrr_bconidx];
                is_proxy_beacon = 0;
            }

            if (BRR_BCON_COUNT == ++blebrr_bconidx)
            {
                blebrr_bconidx = 0;
            }
        }
        while ((blebrr_bconidx != bconidx) && (NULL == elt));
    }

    if (!is_proxy_beacon && NULL == elt)
    {
        elt = blebrr_dequeue();
        is_proxy_beacon = 1;
    }

    if (NULL == elt)
    {
        return API_FAILURE;
    }

    /* Set the type */
    type = (BRR_BCON_COUNT == elt->type) ? BRR_BCON_PASSIVE : elt->type;
    /* Set the advertising data */
    blebrr_advrepeat_count = 1;
    blebrr_advertise_data_pl(type, elt->pdata, elt->pdatalen);

    /* Is Adv data type in element? */
    if (BRR_BCON_COUNT == elt->type)
    {
        #ifdef  BLEBRR_LP_SUPPORT
        blebrr_beacon = 0;
        #endif
        /* Yes, Free the element */
        EM_free_mem(elt->pdata);
        elt->pdatalen = 0;
    }

    #ifdef  BLEBRR_LP_SUPPORT
    else
    {
        blebrr_beacon = 1;
    }

    #endif
    return API_SUCCESS;
}

#if 0
    DECL_STATIC void blebrr_timer_restart (UINT32 timeout);
#endif


/**
    \brief

    \par Description


    \param type
    \param pdata
    \param datalen
    \param elt

    \return void
*/
DECL_STATIC API_RESULT blebrr_send
(
    UCHAR type,
    void* pdata,
    UINT16 datalen,
    BLEBRR_Q_ELEMENT* elt
)
{
    API_RESULT retval;
    UCHAR* data;
    UINT16 packet_len;
    UCHAR offset;
    data = (UCHAR*)pdata;
    /*  BLEBRR_LOG("[ADV-Tx >]: ");
        BLEBRR_dump_bytes(data, datalen); */
    /* Get the offset based on the type */
    offset = (0 != type)? BLEBRR_NCON_ADVTYPE_OFFSET: 0;
    /* Calculate the total length, including Adv Data Type headers */
    packet_len = datalen + offset;
    /* Allocate and save the data */
    elt->pdata = EM_alloc_mem(packet_len);

    if (NULL == elt->pdata)
    {
//        BLEBRR_LOG("Failed to allocate memory!\n");
        return API_FAILURE;
    }

    if (offset >= 1)
    {
        /* Add the Length and Adv type headers */
        elt->pdata[0] = (UCHAR)(datalen + (offset - 1));

        if (offset - 1)
        {
            elt->pdata[1] = type;
        }
    }

    /* Update the data and datalen */
    EM_mem_copy((elt->pdata + offset), data, datalen);
    elt->pdatalen = packet_len;

    /* Is the Adv/Scan timer running? */
    if (EM_TIMER_HANDLE_INIT_VAL != blebrr_timer_handle)
    {
        /* Yes. Do nothing */
        if (BLEBRR_STATE_SCAN_ENABLED == BLEBRR_GET_STATE())
        {
            retval = EM_stop_timer(&blebrr_timer_handle);

            if(retval == EM_SUCCESS)
                blebrr_scan_pl (MS_FALSE);
        }
    }
    else
    {
        /*
            No. Scan must be enabled. Disable it to trigger alternating
            Adv/Scan Procedure
        */
        if (BLEBRR_STATE_SCAN_ENABLED == BLEBRR_GET_STATE()
                || BLEBRR_STATE_IN_SCAN_ENABLE == BLEBRR_GET_STATE())
        {
            blebrr_scan_pl (MS_FALSE);
        }

        #if 1
        else if (BLEBRR_STATE_ADV_ENABLED == BLEBRR_GET_STATE())
        {
            /* Disable Advertising */
            blebrr_advertise_pl(MS_FALSE);
        }

        #endif
        else if (BLEBRR_STATE_IDLE == BLEBRR_GET_STATE())
        {
            /* No, Enable Advertising with Data */
            retval = blebrr_update_advdata();

            if (API_SUCCESS != retval)
            {
                /* Enable Scan */
                blebrr_scan_pl(MS_TRUE);
                /* Update state */
//                BLEBRR_SET_STATE(BLEBRR_STATE_IN_SCAN_ENABLE);
            }
            else
            {
                /* Update state */
//                BLEBRR_SET_STATE(BLEBRR_STATE_IN_ADV_ENABLE);
            }
        }
    }

    return API_SUCCESS;
}


/**
    \brief

    \par Description


    \param handle
    \param pdata
    \param datalen

    \return void
*/
DECL_STATIC API_RESULT blebrr_bcon_send(BRR_HANDLE* handle, void* pdata, UINT16 datalen)
{
    BRR_BEACON_INFO* info;
    BLEBRR_Q_ELEMENT* elt;
    UCHAR op, action, type, bcon, bcontype;
    UCHAR bconidx;
    /* Get the beacon information */
    info = (BRR_BEACON_INFO*)pdata;
    /* Get the Operation and Action */
    op = (info->action & 0x0F);
    action = ((info->action & 0xF0) >> 4);
    /* Get the Broadcast/Observe type and Beacon type */
    type = (info->type & 0x0F);
    bcon = ((info->type & 0xF0) >> 4);
    /* Lock */
    BLEBRR_LOCK();

    /* Check the operations */
    switch (op)
    {
    case BRR_OBSERVE:
        /* blebrr_scan_type = type; */
        break;

    case BRR_BROADCAST:
        /* Get the Beacon mapping at the BLEBRR */
        bcontype = blebrr_get_beacon_type (type, bcon);
        /* Set the bcon index */
        bconidx = bcon;

        if (BRR_ENABLE == action)
        {
            /* Update the connectable beacon packet */
            if ((BRR_BCON_ACTIVE == type) && ((NULL != info->bcon_data)))
            {
                /* Active Beacon (advdata) Source Index */
                UCHAR abs_index;
                abs_index = blebrr_gatt_mode_get();

                if (BLEBRR_GATT_PROV_MODE == abs_index)
                {
                    /* Copy the incoming UUID and OOB info to global connectable ADV data for PB GATT */
                    /* TODO have a state to decide about provisioned and unprovisioned state */
                    EM_mem_copy
                    (
                        blebrr_gap_adv_data[abs_index].data + BLEBRR_GATT_ADV_SERV_DATA_OFFSET,
                        info->bcon_data + 1,
                        16 + 2
                    );
                    /**
                        NOTE: It is not need to calculate assign the Service Data Length as
                             Service Data length is Fixed for Connectable Provisioning ADV.
                        This data length is : 1 + 2 + 16 + 2 = 0x15 Bytes, already updated
                        in the global data strucutre blebrr_gap_adv_data[0].
                    */
                    /* Disable Interleaving */
                    blebrr_scan_interleave = MS_FALSE;
                }
                /* Assuming that this Active Beacon is for GATT Proxy*/
                else
                {
                    /* Copy the incoming UUID and OOB info to global connectable ADV data for PB GATT */
                    /* TODO have a state to decide about provisioned and unprovisioned state */
                    abs_index = BLEBRR_GATT_PROXY_MODE;
                    /* Copy the incoming Proxy ADV data */
                    EM_mem_copy
                    (
                        blebrr_gap_adv_data[abs_index].data + BLEBRR_GATT_ADV_SERV_DATA_OFFSET,
                        info->bcon_data,
                        info->bcon_datalen
                    );
                    /* Copy the incoming Proxy ADV datalen + the BLEBRR_GATT_ADV_SERV_DATA_OFFSET */
                    blebrr_gap_adv_data[abs_index].datalen = BLEBRR_GATT_ADV_SERV_DATA_OFFSET + info->bcon_datalen;
                    /**
                        Assign the service data length correctly for Proxy ADVs
                        Total incoming data + 1 Byte of AD Flags + 2 Bytes of Service UUID
                    */
                    blebrr_gap_adv_data[abs_index].data[BLEBRR_GATT_ADV_SERV_DATALEN_OFFSET] =
                        info->bcon_datalen + 1 + 2;
                }

                /* Re-assign updated ADV data to Info Structure */
                info->bcon_data    = blebrr_gap_adv_data[abs_index].data + pl_advdata_offset;
                info->bcon_datalen = blebrr_gap_adv_data[abs_index].datalen - pl_advdata_offset;
            }

            /* Check if beacon element is free */
            if (0 != blebrr_bcon[bconidx].pdatalen)
            {
                /* Unlock */
                BLEBRR_UNLOCK();
                BLEBRR_LOG("Beacon Not Free!\n");
                return API_FAILURE;
            }

            elt = &blebrr_bcon[bconidx];
            blebrr_datacount++;
            /* Update element type */
            elt->type = type;
            /* Schedule to send */
            blebrr_send
            (
                ((BRR_BCON_TYPE_UNPROV_DEVICE == bcon) &&
                 (BRR_BCON_ACTIVE != type))? MESH_AD_TYPE_BCON : 0,
                info->bcon_data,
                info->bcon_datalen,
                elt
            );

            /* Check if URI data is present for Unprovisioned device */
            if ((BRR_BCON_TYPE_UNPROV_DEVICE == bconidx) &&
                    (NULL != info->uri) &&
                    (NULL != info->uri->payload) &&
                    (0 != info->uri->length))
            {
                elt = &blebrr_bcon[bconidx + 1];
                /* Update element type */
                elt->type = bcontype + 1;
                /* Schedule to send */
                blebrr_send
                (
                    0,
                    info->uri->payload,
                    info->uri->length,
                    elt
                );
            }
        }
        else
        {
            /* Remove the beacon with type from the queue */
            blebrr_clear_bcon (bconidx);
        }

        break;

    default:
        break;
    }

    /* Unlock */
    BLEBRR_UNLOCK();
    return API_SUCCESS;
}


/**
    \brief

    \par Description


    \param handle
    \param pdata
    \param datalen

    \return void
*/
extern uint8 llState;
extern uint8 llSecondaryState;

DECL_STATIC API_RESULT blebrr_adv_send(BRR_HANDLE* handle, UCHAR type, void* pdata, UINT16 datalen)
{
    API_RESULT retval;
    BLEBRR_Q_ELEMENT* elt;

    /* Validate handle */
    if (*handle != blebrr_advhandle)
    {
        return API_FAILURE;
    }

    if ((NULL == pdata) ||
            (0 == datalen))
    {
        return API_FAILURE;
    }

    /* Enable interleaving */
    blebrr_scan_interleave = MS_TRUE;
    blebrr_update_advcount = BLEBRR_BCON_READY_TIME;    //send beacon immediately

    /* If beacon type, pass to the handler */
    if (MESH_AD_TYPE_BCON == type)
    {
        BRR_BEACON_INFO* info;
        /* Get reference to the beacon info */
        info = (BRR_BEACON_INFO*)pdata;

        if (BRR_BCON_TYPE_SECURE_NET != (info->type >> 4))
        {
            return blebrr_bcon_send(handle, pdata, datalen);
        }
        else
        {
            /* Update the data and length reference */
            pdata = info->bcon_data;
            datalen = info->bcon_datalen;
        }
    }

    /* Lock */
    BLEBRR_LOCK();
    /* Allocate the next free element in the data queue */
    elt = blebrr_enqueue_alloc();

    /* Is any element free? */
    if (NULL == elt)
    {
        /* Unlock */
        BLEBRR_UNLOCK();
        BLEBRR_LOG("Queue Full! blebrr_advscan_timeout_count = %d, ble state = %d,depth %d llstate %02x llsec %02x\r\n", blebrr_advscan_timeout_count, BLEBRR_GET_STATE(),blebrr_get_queue_depth(),llState,llSecondaryState);
        blebrr_scan_pl(FALSE);    // HZF
        return API_FAILURE;
    }

    /* Update element type */
    elt->type = BRR_BCON_COUNT;
    /* Schedule to send */
    retval = blebrr_send
             (
                 type,
                 pdata,
                 datalen,
                 elt
             );

    if(retval == API_FAILURE)
        blebrr_dequeue_manual();
    else
        blebrr_datacount++;

    /* Unlock */
    BLEBRR_UNLOCK();
    return API_SUCCESS;
}

#ifdef BLEBRR_LP_SUPPORT
DECL_STATIC void blebrr_adv_sleep(BRR_HANDLE* handle)
{
    BLEBRR_LOCK_VOID();
    /* Set bearer sleep state */
    blebrr_sleep = MS_TRUE;
    MS_prov_stop_interleave_timer();
    MS_proxy_server_stop_timer();

    if (BLEBRR_STATE_SCAN_ENABLED == BLEBRR_GET_STATE() ||
            BLEBRR_STATE_IDLE == BLEBRR_GET_STATE())
    {
        /* Disable Scan */
        blebrr_scan_pl(MS_FALSE);
        /* Update state */
        BLEBRR_SET_STATE(BLEBRR_STATE_IDLE);
    }
    else if(BLEBRR_STATE_ADV_ENABLED == BLEBRR_GET_STATE())
    {
        /* Disable Advertising */
        blebrr_advertise_pl(MS_FALSE);
    }

    /* Enter platform sleep */
    EM_enter_sleep_pl();
    BLEBRR_UNLOCK_VOID();
}

DECL_STATIC void blebrr_adv_wakeup(BRR_HANDLE* handle, UINT8 mode)
{
    BLEBRR_LOCK_VOID();
    /* Exit platform sleep */
    EM_exit_sleep_pl();
    /* Reset bearer sleep state */
    blebrr_sleep = MS_FALSE;

    if (BRR_TX & mode)
    {
        if (BLEBRR_STATE_IDLE == BLEBRR_GET_STATE())
        {
            blebrr_update_advcount = BLEBRR_BCON_READY_TIME; //send beacon immediately
            /* Enable Advertise */
            blebrr_lp_flag = MS_TRUE;
            blebrr_update_advdata();
            /* Update state */
//            BLEBRR_SET_STATE(BLEBRR_STATE_IN_ADV_ENABLE);
        }
    }
    else if (BRR_RX & mode)
    {
        if (BLEBRR_STATE_IDLE == BLEBRR_GET_STATE())
        {
            /* Enable Scan */
            blebrr_scan_pl(MS_TRUE);
            /* Update state */
//            BLEBRR_SET_STATE(BLEBRR_STATE_IN_SCAN_ENABLE);
        }
    }

    BLEBRR_UNLOCK_VOID();
}

API_RESULT blebrr_sleep_handler(void)
{
    API_RESULT retval;
    retval=MS_brr_sleep();
    return retval;
}

API_RESULT blebrr_wakeup_handler(void)
{
    API_RESULT retval;
    UCHAR state;
    /* Fetch PROXY feature state */
    MS_access_cm_get_features_field(&state, MS_FEATURE_PROXY);

    if(state == MS_TRUE)
    {
        retval = MS_brr_wakeup(BRR_TX|BRR_RX);
    }
    else
    {
        retval = MS_brr_wakeup(BRR_RX);
    }

    return retval;
}

static void enter_lp_sleep_mode (void)
{
    light_timeout_handle();
    hal_pwrmgr_unlock(MOD_USR1);
    blebrr_sleep_handler();
    printf("sleep mode:%d\n", isSleepAllow());
}


void blebrr_lp_mode (void* args, UINT16 size)
{
    UCHAR       mode;
    UCHAR       glp_mode;
    blebrr_lp_thandle = EM_TIMER_HANDLE_INIT_VAL;
    UCHAR state;
    /* Fetch PROXY feature state */
    MS_access_cm_get_features_field(&state, MS_FEATURE_PROXY);
    MS_IGNORE_UNUSED_PARAM(size);
    mode = (*((UCHAR*)args));

    if(mode == BLEBRR_LP_OFF)
    {
        pwroff_cfg_t pwr_wkp_cfg[]= {{P14,NEGEDGE}};
        hal_pwrmgr_poweroff( pwr_wkp_cfg, sizeof(pwr_wkp_cfg)/sizeof(pwr_wkp_cfg[0]) );
    }
    else if(mode == BLEBRR_LP_SLEEP)
    {
        blebrr_wakeup_handler();

        if(state != MS_TRUE)
        {
            glp_mode = BLEBRR_LP_WKP;
            EM_start_timer
            (
                &blebrr_lp_thandle,
                EM_TIMEOUT_MILLISEC | BLEBRR_LP_PROVISIONED_WKP_TIMEOUT,
                blebrr_lp_mode,
                (void*)&glp_mode,
                sizeof(glp_mode)
            );
        }
    }
    else
    {
        glp_mode = BLEBRR_LP_SLEEP;
        EM_start_timer
        (
            &blebrr_lp_thandle,
            EM_TIMEOUT_MILLISEC | BLEBRR_LP_PROVISIONED_SLP_TIMEOUT,
            blebrr_lp_mode,
            (void*)&glp_mode,
            sizeof(glp_mode)
        );
        enter_lp_sleep_mode();
    }
}


API_RESULT blebrr_lp_start(UCHAR mode)
{
    API_RESULT retval;
    UCHAR state;
    UINT32 timeout;
    /* Fetch PROXY feature state */
    MS_access_cm_get_features_field(&state, MS_FEATURE_PROXY);

    switch(mode)
    {
    case BLEBRR_LP_OFF:
    {
        timeout = BLEBRR_LP_UNPROVISION_TIMEOUT;
    }
    break;

    case BLEBRR_LP_SLEEP:
    {
        timeout = EM_TIMEOUT_MILLISEC | BLEBRR_LP_PROVISIONED_SLP_TIMEOUT;
    }
    break;

    case BLEBRR_LP_WKP:
    {
        timeout = EM_TIMEOUT_MILLISEC | BLEBRR_LP_PROVISIONED_WKP_TIMEOUT;
    }
    break;

    default:
        break;
    }

    retval = EM_start_timer
             (
                 &blebrr_lp_thandle,
                 timeout,
                 blebrr_lp_mode,
                 (void*)&mode,
                 sizeof(mode)
             );

    if(mode == BLEBRR_LP_SLEEP)
        enter_lp_sleep_mode();

    return retval;
}

void blebrr_lp_stop(void)
{
    EM_stop_timer (&blebrr_lp_thandle);
}


#endif /* BLEBRR_LP_SUPPORT */

/**
    \brief

    \par Description


    \param args
    \param size

    \return void
*/
DECL_STATIC void blebrr_advscan_timeout_handler (void* args, UINT16 size)
{
    MS_IGNORE_UNUSED_PARAM(args);
    MS_IGNORE_UNUSED_PARAM(size);
    BLEBRR_LOCK_VOID();
    blebrr_advscan_timeout_count ++;
    /* Reset Timer Handler */
    blebrr_timer_handle = EM_TIMER_HANDLE_INIT_VAL;

    /* Check the state of AdvScan procedure */
    switch (BLEBRR_GET_STATE())
    {
    case BLEBRR_STATE_ADV_ENABLED:

        /* Disable Adv */
        if (!blebrr_advertise_pl (MS_FALSE))        // HZF
            /* Update state */
//            BLEBRR_SET_STATE(BLEBRR_STATE_IN_ADV_DISABLE);
            break;

    case BLEBRR_STATE_SCAN_ENABLED:
        /* Disable Scan */
        blebrr_scan_pl (MS_FALSE);
        /* Update state */
//        BLEBRR_SET_STATE(BLEBRR_STATE_IN_SCAN_DISABLE);
        break;

    default:
        /* Should not reach here */
        BLEBRR_LOG("=======blebrr_advscan_timeout_handler error: state = %2X, state will not change\r\n", BLEBRR_GET_STATE());
        break;
    }

    BLEBRR_UNLOCK_VOID();
}


/**
    \brief

    \par Description


    \param timeout

    \return void
*/
DECL_STATIC void blebrr_timer_start (UINT32 timeout)
{
    EM_RESULT retval;

    if(blebrr_timer_handle == EM_TIMER_HANDLE_INIT_VAL)
    {
        retval = EM_start_timer
                 (
                     &blebrr_timer_handle,
                     timeout,
                     blebrr_advscan_timeout_handler,
                     NULL,
                     0
                 );

        if (EM_SUCCESS != retval)
        {
            /* TODO: Log */
        }
    }
}

void blebrr_timer_stop (void)
{
    if(blebrr_timer_handle != EM_TIMER_HANDLE_INIT_VAL)
    {
        if(EM_stop_timer(&blebrr_timer_handle) != API_SUCCESS)
            return;
    }

    BLEBRR_SET_STATE(BLEBRR_STATE_IDLE);
}


#if 0
/**
    \brief

    \par Description


    \param timeout

    \return void
*/
DECL_STATIC void blebrr_timer_restart (UINT32 timeout)
{
    EM_RESULT retval;
//    printf("before blebrr_timer_handle:%d\n",blebrr_timer_handle);
    retval = EM_restart_timer
             (
                 blebrr_timer_handle,
                 timeout
             );
//    printf("after blebrr_timer_handle:%d\n",blebrr_timer_handle);

    if (EM_SUCCESS != retval)
    {
        /* TODO: Log */
    }
}
#endif



/**
    \brief

    \par Description


    \param enable

    \return void
*/
void blebrr_pl_scan_setup (UCHAR enable)
{
    API_RESULT retval;
    BLEBRR_LOCK_VOID();
    #ifdef BLEBRR_ENABLE_SCAN_TRACE
    BLEBRR_LOG ("Scan Setup - %d", enable);
    #endif /* BLEBRR_ENABLE_SCAN_TRACE */

    /* Is scan enabled? */
    if (MS_TRUE == enable)
    {
        /* Yes, Update state */
        BLEBRR_SET_STATE(BLEBRR_STATE_SCAN_ENABLED);

        /* Is data available in queue to be sent? */
        if (0 != blebrr_datacount)
        {
            UCHAR depth=blebrr_get_queue_depth();

            if(depth>0)
                blebrr_scanTimeOut = BLEBRR_SCAN_ADJ_THD_MIN;
            else
                blebrr_scanTimeOut = BLEBRR_SCAN_ADJ_THD_MAX;

            /* Yes, Start bearer timer for Scan Timeout */
            blebrr_timer_start ((EM_TIMEOUT_MILLISEC | blebrr_scanTimeOut));
        }
    }
    else
    {
        /* No, Enable Advertising with Data */
        retval = blebrr_update_advdata();

        if (API_SUCCESS != retval)
        {
            if (MS_TRUE != blebrr_sleep)
            {
                /* Enale Scan */
                blebrr_scan_pl(MS_TRUE);
                /* Update state */
//                BLEBRR_SET_STATE(BLEBRR_STATE_IN_SCAN_ENABLE);
            }
            else
            {
                /* Update state */
                BLEBRR_SET_STATE(BLEBRR_STATE_IDLE);
            }
        }
        else
        {
            /* Update state */
//            BLEBRR_SET_STATE(BLEBRR_STATE_IN_ADV_ENABLE);
        }
    }

    BLEBRR_UNLOCK_VOID();
}

/**
    \brief

    \par Description


    \param type
    \param enable

    \return void
*/
void blebrr_pl_advertise_setup (UCHAR enable)
{
    BLEBRR_LOCK_VOID();
    UCHAR adv_repeat_count;
//    API_RESULT retval;
    #ifdef BLEBRR_ENABLE_ADV_TRACE
    BLEBRR_LOG ("Adv Setup - %d", enable);
    #endif /* BLEBRR_ENABLE_ADV_TRACE */

    /* Is advertise enabled? */
    if (MS_TRUE == enable)
    {
        /* Yes, Update state */
        BLEBRR_SET_STATE(BLEBRR_STATE_ADV_ENABLED);

        /* Start bearer timer for Adv Timeout */
        if (blebrr_scan_interleave == MS_TRUE)
        {
            UCHAR timeout,proxy_state;
            MS_proxy_fetch_state(&proxy_state);

            if(proxy_state == MS_PROXY_CONNECTED)
                timeout = BLEBRR_ADV_SEC_TIMEOUT;
            else
                timeout= (blebrr_advtype == BRR_BCON_PASSIVE) ? BLEBRR_ADV_NON_TIMEOUT : BLEBRR_ADV_TIMEOUT;

            blebrr_timer_start (EM_TIMEOUT_MILLISEC | timeout);
        }
    }
    else
    {
        if (MS_TRUE == blebrr_sleep)
        {
            /* Update state */
            blebrr_adv_restart = MS_FALSE;
            BLEBRR_SET_STATE(BLEBRR_STATE_IDLE);
            return;
        }

//        if(blebrr_adv_restart == MS_TRUE)
        {
//            blebrr_adv_restart = MS_FALSE;
//            blebrr_update_advcount = BLEBRR_BCON_READY_TIME;
//            retval = blebrr_update_advdata();
//            if (API_SUCCESS != retval)
//            {
//                /* Enale Scan */
//                blebrr_scan_pl(MS_TRUE);
//                /* Update state */
//                BLEBRR_SET_STATE(BLEBRR_STATE_IN_SCAN_ENABLE);
//            }
//            else
//            {
//                /* Update state */
//                BLEBRR_SET_STATE(BLEBRR_STATE_IN_ADV_ENABLE);
//            }
        }
//        else
        {
            adv_repeat_count = (blebrr_prov_started == MS_TRUE)?BLEBRR_ADVREPEAT_PRO_COUNT:BLEBRR_ADVREPEAT_NET_COUNT;

            if (/*blebrr_beacon && */(adv_repeat_count > blebrr_advrepeat_count))
            {
                blebrr_advrepeat_count++;
                blebrr_advertise_pl(MS_TRUE);
                /* Update state */
//                BLEBRR_SET_STATE(BLEBRR_STATE_IN_ADV_ENABLE);
            }
            else
            {
                /* No, Enable Scanning */
                blebrr_scan_pl(MS_TRUE);
                #ifdef  BLEBRR_LP_SUPPORT
                UCHAR glp_mode;

                if((blebrr_lp_flag == MS_TRUE) &&(blebrr_beacon == 1))
                {
                    blebrr_beacon =0;
                    blebrr_lp_flag = MS_FALSE;
                    glp_mode = BLEBRR_LP_WKP;
                    blebrr_lp_start(glp_mode);
                }

                #endif
                /* Update state */
//                BLEBRR_SET_STATE(BLEBRR_STATE_IN_SCAN_ENABLE);
            }
        }
    }

    BLEBRR_UNLOCK_VOID();
}

/**
    \brief

    \par Description


    \param None

    \return void
*/
void blebrr_pl_advertise_end (void)
{
    BLEBRR_LOCK_VOID();
    blebrr_advrepeat_count = 0;
    BLEBRR_UNLOCK_VOID();
}

#ifdef BLEBRR_FILTER_DUPLICATE_PACKETS
/**
    \brief

    \par Description


    \param p_adv_data_with_bd_addr

    \return void
*/
DECL_STATIC API_RESULT blebrr_adv_duplicate_filter(/* IN */ UCHAR* p_adv_data_with_bd_addr)
{
    UCHAR length, index;
    /* Get the ADV data packet length */
    length = p_adv_data_with_bd_addr[1 + BT_BD_ADDR_SIZE];

    for (index = 0; index < BLEBRR_MAX_ADV_FILTER_LIST_COUNT; index++)
    {
        /* First Match BD Addr */
        if (0 == EM_mem_cmp
                (
                    &blebrr_adv_list[index][0],
                    p_adv_data_with_bd_addr,
                    1 + BT_BD_ADDR_SIZE
                ))
        {
            /* Check Data Length */
            if (blebrr_adv_list[index][1 + BT_BD_ADDR_SIZE] == p_adv_data_with_bd_addr[1 + BT_BD_ADDR_SIZE])
            {
                if (0 == EM_mem_cmp
                        (
                            &blebrr_adv_list[index][1 + BT_BD_ADDR_SIZE + 1],
                            &p_adv_data_with_bd_addr[1 + BT_BD_ADDR_SIZE + 1],
                            length
                        ))
                {
                    return API_SUCCESS;
                }
            }

            /* Update Adv data */
            EM_mem_copy
            (
                &blebrr_adv_list[index][1 + BT_BD_ADDR_SIZE],
                &p_adv_data_with_bd_addr[1 + BT_BD_ADDR_SIZE],
                length + 1
            );
            return API_FAILURE;
        }
    }

    /* Find out the suitable location to save the most recent ADV packet */
    /* New peer device. Add */
    EM_mem_copy
    (
        &blebrr_adv_list[blebrr_adv_list_inser_index][0],
        p_adv_data_with_bd_addr,
        length + 1 + BT_BD_ADDR_SIZE + 1
    );
    /* Increment and Wrap (if required) */
    blebrr_adv_list_inser_index++;

    if (BLEBRR_MAX_ADV_FILTER_LIST_COUNT <= blebrr_adv_list_inser_index)
    {
        blebrr_adv_list_inser_index = 0;
    }

    return API_FAILURE;
}
#endif /* BLEBRR_FILTER_DUPLICATE_PACKETS */


extern uint8  osal_memory_audit(void* ptr);
/**
    \brief

    \par Description


    \param type
    \param pdata
    \param pdatalen
    \param rssi

    \return void
*/
void blebrr_pl_recv_advpacket (UCHAR type, UCHAR* pdata, UINT16 pdatalen, UCHAR rssi)
{
    MS_BUFFER info;
    #ifdef BLEBRR_FILTER_DUPLICATE_PACKETS
    /* Duplicate Filtering */
    retval = blebrr_adv_duplicate_filter(p_adv_data_with_bd_addr);

    /* If found the ADV packet as duplicate, drop the ADV packet */
    if (API_SUCCESS == retval)
    {
        return API_FAILURE;
    }

    #endif /* BLEBRR_FILTER_DUPLICATE_PACKETS */

    /* Handle only if Non-Connectable (Passive) Advertising */
    if (BRR_BCON_PASSIVE != type)
    {
        return;
    }

    /* Pack the RSSI as metadata */
    info.payload = &rssi;
    info.length = sizeof(UCHAR);

    /* Deliver the packet to the bearer */
    if (NULL != blebrr_adv.bearer_recv)
    {
        /*  BLEBRR_LOG("[ADV-Rx <]: ");
            BLEBRR_dump_bytes(pdata, pdatalen); */
        blebrr_adv.bearer_recv(&blebrr_advhandle, pdata, pdatalen, &info);
    }
    else
    {
        BLEBRR_LOG("BEARER RECV Callback Currently NULL !!\n");
    }
}

/**
    \brief

    \par Description

 * *
 * *  \param void

    \return void
*/
void blebrr_register(void)
{
    /* Initialize locals */
    BLEBRR_MUTEX_INIT_VOID();
    /* Initialize Timer Handler */
    blebrr_timer_handle = EM_TIMER_HANDLE_INIT_VAL;
    /* Initialize the transport */
    blebrr_init_pl();
    /* Get the platform Advdata initial offset if any */
    pl_advdata_offset = blebrr_get_advdata_offset_pl ();
    /* Reset the bearer sleep */
    blebrr_sleep = MS_FALSE;
    /* Add the Adv Bearer */
    blebrr_adv.bearer_send = blebrr_adv_send;
    #ifdef BLEBRR_LP_SUPPORT
    blebrr_adv.bearer_sleep = blebrr_adv_sleep;
    blebrr_adv.bearer_wakeup = blebrr_adv_wakeup;
    blebrr_lp_thandle = EM_TIMER_HANDLE_INIT_VAL;
    #endif /* BLEBRR_LP_SUPPORT */
    MS_brr_add_bearer(BRR_TYPE_ADV, &blebrr_adv, &blebrr_advhandle);
    /* Enable all Mesh related Module Debugging */
    EM_enable_module_debug_flag
    (
        MS_MODULE_ID_APP    |
        MS_MODULE_ID_ACCESS |
        MS_MODULE_ID_TRN    |
        MS_MODULE_ID_LTRN   |
        MS_MODULE_ID_NET    |
        MS_MODULE_ID_COMMON |
        MS_MODULE_ID_BRR    |
        MS_MODULE_ID_STBX   |
        MS_MODULE_ID_PROV
    );
    /* Set Debug Level Trace as default. Enable Information only if required */
    EM_set_debug_level(EM_DEBUG_LEVEL_TRC);
    /* Allow the tasks to start and be ready */
    EM_sleep (1);
    #if 0
    /* Start Observing */
    BLEBRR_LOG ("Start Observing...\n");
    blebrr_scan_pl (MS_TRUE);
    /* Update state */
//    BLEBRR_SET_STATE(BLEBRR_STATE_IN_SCAN_ENABLE);
    #else /* 0 */
    BLEBRR_SET_STATE(BLEBRR_STATE_IDLE);
    #endif /* 0 */
}

