/**
    \file appl_health_client.c

    \brief This file defines the Mesh Health Model Application Interface
    - includes Data Structures and Methods for Client.
*/

/*
    Copyright (C) 2018. Mindtree Ltd.
    All rights reserved.
*/



/* --------------------------------------------- Header File Inclusion */
#include "appl_health_client.h"


/* --------------------------------------------- Global Definitions */


/* --------------------------------------------- Static Global Variables */
static const char main_health_client_options[] = "\n\
======== Health Client Menu ========\n\
     0. Exit. \n\
     1. Refresh. \n\
 \n\
    10. Send Health Attention Get. \n\
    11. Send Health Attention Set. \n\
    12. Send Health Attention Set Unacknowledged. \n\
    13. Send Health Fault Clear. \n\
    14. Send Health Fault Clear Unacknowledged. \n\
    15. Send Health Fault Get. \n\
    16. Send Health Fault Test. \n\
    17. Send Health Fault Test Unacknowledged. \n\
    18. Send Health Period Get. \n\
    19. Send Health Period Set. \n\
    20. Send Health Period Set Unacknowledged. \n\
 \n\
    21. Get Model Handle. \n\
    22. Set Publish Address. \n\
 \n\
Your Option ? \0";


/* --------------------------------------------- External Global Variables */
static MS_ACCESS_MODEL_HANDLE   appl_health_client_model_handle;


/* --------------------------------------------- Function */
/* health client application entry point */
void main_health_client_operations(void)
{
    int choice;
    MS_ACCESS_ELEMENT_HANDLE element_handle;
    static UCHAR model_initialized = 0x00;

    /**
        Register with Access Layer.
    */
    if (0x00 == model_initialized)
    {
        API_RESULT retval;
        /* Use Default Element Handle. Index 0 */
        element_handle = MS_ACCESS_DEFAULT_ELEMENT_HANDLE;
        retval = MS_health_client_init
                 (
                     element_handle,
                     &appl_health_client_model_handle,
                     appl_health_client_cb
                 );

        if (API_SUCCESS == retval)
        {
            CONSOLE_OUT(
                "Health Client Initialized. Model Handle: 0x%04X\n",
                appl_health_client_model_handle);
        }
        else
        {
            CONSOLE_OUT(
                "[ERR] Health Client Initialization Failed. Result: 0x%04X\n",
                retval);
        }

        model_initialized = 0x01;
    }

    MS_LOOP_FOREVER()
    {
        CONSOLE_OUT
        ("%s", main_health_client_options);
        CONSOLE_IN
        ("%d", &choice);

        if (choice < 0)
        {
            CONSOLE_OUT
            ("*** Invalid Choice. Try Again.\n");
            continue;
        }

        switch (choice)
        {
        case 0:
            return;

        case 1:
            break;

        case 10: /* Send Health Attention Get */
            appl_send_health_attention_get();
            break;

        case 11: /* Send Health Attention Set */
            appl_send_health_attention_set();
            break;

        case 12: /* Send Health Attention Set Unacknowledged */
            appl_send_health_attention_set_unacknowledged();
            break;

        case 13: /* Send Health Fault Clear */
            appl_send_health_fault_clear();
            break;

        case 14: /* Send Health Fault Clear Unacknowledged */
            appl_send_health_fault_clear_unacknowledged();
            break;

        case 15: /* Send Health Fault Get */
            appl_send_health_fault_get();
            break;

        case 16: /* Send Health Fault Test */
            appl_send_health_fault_test();
            break;

        case 17: /* Send Health Fault Test Unacknowledged */
            appl_send_health_fault_test_unacknowledged();
            break;

        case 18: /* Send Health Period Get */
            appl_send_health_period_get();
            break;

        case 19: /* Send Health Period Set */
            appl_send_health_period_set();
            break;

        case 20: /* Send Health Period Set Unacknowledged */
            appl_send_health_period_set_unacknowledged();
            break;

        case 21: /* Get Model Handle */
            appl_health_client_get_model_handle();
            break;

        case 22: /* Set Publish Address */
            appl_health_client_set_publish_address();
            break;
        }
    }
}

/* Send Health Attention Get */
void appl_send_health_attention_get(void)
{
    API_RESULT retval;
    CONSOLE_OUT
    (">> Send Health Attention Get\n");
    retval = MS_health_attention_get();
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Health Attention Set */
void appl_send_health_attention_set(void)
{
    API_RESULT retval;
    int  choice;
    MS_HEALTH_ATTENTION_STRUCT  param;
    CONSOLE_OUT
    (">> Send Health Attention Set\n");
    CONSOLE_OUT
    ("Enter Attention (8-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.attention = (UCHAR)choice;
    retval = MS_health_attention_set(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Health Attention Set Unacknowledged */
void appl_send_health_attention_set_unacknowledged(void)
{
    API_RESULT retval;
    int  choice;
    MS_HEALTH_ATTENTION_STRUCT  param;
    CONSOLE_OUT
    (">> Send Health Attention Set Unacknowledged\n");
    CONSOLE_OUT
    ("Enter Attention (8-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.attention = (UCHAR)choice;
    retval = MS_health_attention_set_unacknowledged(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Health Fault Clear */
void appl_send_health_fault_clear(void)
{
    API_RESULT retval;
    int  choice;
    MS_HEALTH_FAULT_GET_CLEAR_STRUCT  param;
    CONSOLE_OUT
    (">> Send Health Fault Clear\n");
    CONSOLE_OUT
    ("Enter Company ID (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.company_id = (UINT16)choice;
    retval = MS_health_fault_clear(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Health Fault Clear Unacknowledged */
void appl_send_health_fault_clear_unacknowledged(void)
{
    API_RESULT retval;
    int  choice;
    MS_HEALTH_FAULT_GET_CLEAR_STRUCT  param;
    CONSOLE_OUT
    (">> Send Health Fault Clear Unacknowledged\n");
    CONSOLE_OUT
    ("Enter Company ID (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.company_id = (UINT16)choice;
    retval = MS_health_fault_clear_unacknowledged(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Health Fault Get */
void appl_send_health_fault_get(void)
{
    API_RESULT retval;
    int  choice;
    MS_HEALTH_FAULT_GET_CLEAR_STRUCT  param;
    CONSOLE_OUT
    (">> Send Health Fault Get\n");
    CONSOLE_OUT
    ("Enter Company ID (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.company_id = (UINT16)choice;
    retval = MS_health_fault_get(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Health Fault Test */
void appl_send_health_fault_test(void)
{
    API_RESULT retval;
    int  choice;
    MS_HEALTH_FAULT_TEST_STRUCT  param;
    CONSOLE_OUT
    (">> Send Health Fault Test\n");
    CONSOLE_OUT
    ("Enter Test ID (8-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.test_id = (UCHAR)choice;
    CONSOLE_OUT
    ("Enter Company ID (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.company_id = (UINT16)choice;
    retval = MS_health_fault_test(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Health Fault Test Unacknowledged */
void appl_send_health_fault_test_unacknowledged(void)
{
    API_RESULT retval;
    int  choice;
    MS_HEALTH_FAULT_TEST_STRUCT  param;
    CONSOLE_OUT
    (">> Send Health Fault Test Unacknowledged\n");
    CONSOLE_OUT
    ("Enter Test ID (8-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.test_id = (UCHAR)choice;
    CONSOLE_OUT
    ("Enter Company ID (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.company_id = (UINT16)choice;
    retval = MS_health_fault_test_unacknowledged(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Health Period Get */
void appl_send_health_period_get(void)
{
    API_RESULT retval;
    CONSOLE_OUT
    (">> Send Health Period Get\n");
    retval = MS_health_period_get();
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Health Period Set */
void appl_send_health_period_set(void)
{
    API_RESULT retval;
    int  choice;
    MS_HEALTH_PERIOD_STRUCT  param;
    CONSOLE_OUT
    (">> Send Health Period Set\n");
    CONSOLE_OUT
    ("Enter FastPeriodDivisor (8-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.fastperioddivisor = (UCHAR)choice;
    retval = MS_health_period_set(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Health Period Set Unacknowledged */
void appl_send_health_period_set_unacknowledged(void)
{
    API_RESULT retval;
    int  choice;
    MS_HEALTH_PERIOD_STRUCT  param;
    CONSOLE_OUT
    (">> Send Health Period Set Unacknowledged\n");
    CONSOLE_OUT
    ("Enter FastPeriodDivisor (8-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.fastperioddivisor = (UCHAR)choice;
    retval = MS_health_period_set_unacknowledged(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}


/* Get Model Handle */
void appl_health_client_get_model_handle(void)
{
    API_RESULT retval;
    MS_ACCESS_MODEL_HANDLE model_handle;
    retval = MS_health_client_get_model_handle(&model_handle);

    if (API_SUCCESS == retval)
    {
        CONSOLE_OUT
        (">> Model Handle 0x%04X\n", model_handle);
    }
    else
    {
        CONSOLE_OUT
        (">> Get Model Handle Failed. Status 0x%04X\n", retval);
    }

    return;
}


/* Set Publish Address */
void appl_health_client_set_publish_address(void)
{
    int  choice;
    API_RESULT retval;
    MS_ACCESS_MODEL_HANDLE model_handle;
    MS_ACCESS_PUBLISH_INFO  publish_info;
    /* Set Publish Information */
    EM_mem_set(&publish_info, 0, sizeof(publish_info));
    publish_info.addr.use_label = MS_FALSE;
    CONSOLE_OUT
    ("Enter Model Handle (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    model_handle = (UINT16)choice;
    CONSOLE_OUT
    ("Enter Health client Server Address (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    publish_info.addr.addr = (UINT16)choice;
    /**
        Foundation Models use Device Key for transport layer encryption.

        Taking network index as input.
    */
    CONSOLE_OUT
    ("Enter Network Index (0 if part of a single network)\n");
    CONSOLE_IN
    ("%d", &choice);
    publish_info.appkey_index = (UINT16)(MS_CONFIG_LIMITS(MS_MAX_APPS) + choice);
    publish_info.remote = MS_FALSE;
    retval = MS_access_cm_set_model_publication
             (
                 model_handle,
                 &publish_info
             );

    if (API_SUCCESS == retval)
    {
        CONSOLE_OUT
        (">> Publish Address is set Successfully.\n");
    }
    else
    {
        CONSOLE_OUT
        (">> Failed to set publish address. Status 0x%04X\n", retval);
    }

    return;
}

/**
    \brief Client Application Asynchronous Notification Callback.

    \par Description
    Health client calls the registered callback to indicate events occurred to the application.

    \param [in] handle        Model Handle.
    \param [in] opcode        Opcode.
    \param [in] data_param    Data associated with the event if any or NULL.
    \param [in] data_len      Size of the event data. 0 if event data is NULL.
*/
API_RESULT appl_health_client_cb
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
        "[HEALTH_CLIENT] Callback. Opcode 0x%04X\n", opcode);
    appl_dump_bytes(data_param, data_len);

    switch(opcode)
    {
    case MS_ACCESS_HEALTH_ATTENTION_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_HEALTH_ATTENTION_STATUS_OPCODE\n");
    }
    break;

    case MS_ACCESS_HEALTH_CURRENT_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_HEALTH_CURRENT_STATUS_OPCODE\n");
    }
    break;

    case MS_ACCESS_HEALTH_FAULT_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_HEALTH_FAULT_STATUS_OPCODE\n");
    }
    break;

    case MS_ACCESS_HEALTH_PERIOD_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_HEALTH_PERIOD_STATUS_OPCODE\n");
    }
    break;
    }

    return retval;
}

