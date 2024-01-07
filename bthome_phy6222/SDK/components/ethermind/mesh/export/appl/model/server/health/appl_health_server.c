/**
    \file appl_health_server.c
*/

/*
    Copyright (C) 2017. Mindtree Ltd.
    All rights reserved.
*/



/* --------------------------------------------- Header File Inclusion */
#include "appl_health_server.h"
#include "MS_health_server_api.h"


/* --------------------------------------------- Global Definitions */


/* --------------------------------------------- Static Global Variables */
static const char main_health_server_options[] = "\n\
======== Health Server Menu ========\n\
     0. Exit. \n\
     1. Refresh. \n\
 \n\
     5. Publish No Fault in Master Credential. \n\
     6. Publish No Fault in Friend Credential. \n\
 \n\
Your Option ? \0";


/* --------------------------------------------- External Global Variables */
static MS_ACCESS_MODEL_HANDLE   appl_health_server_model_handle;


static void appl_health_self_test_00(UINT8 test_id, UINT16 company_id)
{
    UINT8 fault_code;
    /* Dummy Function. Reports fault status by default */
    /* Dummy fault code */
    fault_code = (UINT8)(test_id + 1);
    MS_health_server_report_fault
    (
        &appl_health_server_model_handle,
        test_id,
        company_id,
        fault_code
    );
}

static void appl_health_self_test_02(UINT8 test_id, UINT16 company_id)
{
    UINT8 fault_code;
    /* Dummy Function. Reports fault status by default */
    /* Dummy fault code */
    fault_code = (UINT8)(test_id + 1);
    MS_health_server_report_fault
    (
        &appl_health_server_model_handle,
        test_id,
        company_id,
        fault_code
    );
}

static void appl_health_self_test_FF(UINT8 test_id, UINT16 company_id)
{
    UINT8 fault_code;
    /* Dummy Function. Reports fault status by default */
    /* Dummy fault code */
    fault_code = (UINT8)(test_id + 1);
    MS_health_server_report_fault
    (
        &appl_health_server_model_handle,
        test_id,
        company_id,
        fault_code
    );
}

/* List of Self Tests */
static MS_HEALTH_SERVER_SELF_TEST appl_health_server_self_tests[] =
{
    {
        0x00, /* Test ID: 0x00 */
        appl_health_self_test_00
    },
    {
        0x02, /* Test ID: 0x02 */
        appl_health_self_test_02
    },
    {
        0xFF, /* Test ID: 0xFF */
        appl_health_self_test_FF
    }
};

/* --------------------------------------------- Function */
void appl_health_server_send_status(void)
{
    UCHAR current_status[4] = { 0x00, 0x6A, 0x00, 0x00 };
    MS_health_server_publish_current_status(current_status, sizeof(current_status));
}

/* health server application entry point */
void main_health_server_operations(/* IN */ UINT8 have_menu)
{
    int choice;
    MS_ACCESS_ELEMENT_HANDLE element_handle;
    static UCHAR model_initialized = 0x00;
    UINT16                       company_id;
    MS_HEALTH_SERVER_SELF_TEST* self_tests;
    UINT32                       num_self_tests;

    /**
        Register with Access Layer.
    */
    if (0x00 == model_initialized)
    {
        API_RESULT retval;
        company_id = MS_DEFAULT_COMPANY_ID;
        self_tests = &appl_health_server_self_tests[0];
        num_self_tests = sizeof(appl_health_server_self_tests)/sizeof(MS_HEALTH_SERVER_SELF_TEST);
        /* Use Default Element Handle. Index 0 */
        element_handle = MS_ACCESS_DEFAULT_ELEMENT_HANDLE;
        retval = MS_health_server_init
                 (
                     element_handle,
                     &appl_health_server_model_handle,
                     company_id,
                     self_tests,
                     num_self_tests,
                     appl_health_server_cb
                 );

        if (API_SUCCESS == retval)
        {
            CONSOLE_OUT(
                "Health Server Initialized. Model Handle: 0x%04X\n",
                appl_health_server_model_handle);
        }
        else
        {
            CONSOLE_OUT(
                "[ERR] Sensor Server Initialization Failed. Result: 0x%04X\n",
                retval);
        }

        model_initialized = 0x01;
    }

    if (MS_TRUE != have_menu)
    {
        CONSOLE_OUT("Not to use menu options\n");
        return;
    }

    MS_LOOP_FOREVER()
    {
        CONSOLE_OUT
        ("%s", main_health_server_options);
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

        case 5:
            appl_health_server_send_status();
            break;
        }
    }
}

API_RESULT main_health_server_log_fault
(
    /* IN */ UINT8 test_id,
    /* IN */ UINT8 fault
)
{
    API_RESULT retval;
    retval = MS_health_server_report_fault
             (
                 &appl_health_server_model_handle,
                 test_id,
                 MS_DEFAULT_COMPANY_ID,
                 fault
             );
    return retval;
}

/**
    \brief Health Server application Asynchronous Notification Callback.

    \par Description
    Health Server calls the registered callback to indicate events occurred to the
    application.

    \param handle        Model Handle.
    \param event_type    Health Server Event type.
    \param event_param   Parameter associated with the event if any or NULL.
    \param param_len     Size of the event parameter data. 0 if event param is NULL.
*/
API_RESULT appl_health_server_cb
(
    MS_ACCESS_MODEL_HANDLE* handle,
    UINT8                    event_type,
    UINT8*                   event_param,
    UINT16                   param_len
)
{
    CONSOLE_OUT(
        "[Health Server Callback] Event Type: 0x%02X\n", event_type);

    switch (event_type)
    {
    case MS_HEALTH_SERVER_ATTENTION_START:
    {
        CONSOLE_OUT(
            "[Health Server Callback] Attention Start. Attention Timeout: 0x%02X\n",
            (*event_param));
    }
    break;

    case MS_HEALTH_SERVER_ATTENTION_RESTART:
    {
        CONSOLE_OUT(
            "[Health Server Callback] Attention Restart. Attention Timeout: 0x%02X\n",
            (*event_param));
    }
    break;

    case MS_HEALTH_SERVER_ATTENTION_STOP:
    {
        CONSOLE_OUT(
            "[Health Server Callback] Attention Stop\n");
    }
    break;

    default:
        CONSOLE_OUT(
            "Unhandled Event Type: 0x%02X\n", event_type);
    }

    return API_SUCCESS;
}

