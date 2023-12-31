/**
    \file appl_scheduler_client.c

    \brief This file defines the Mesh Scheduler Model Application Interface
    - includes Data Structures and Methods for Client.
*/

/*
    Copyright (C) 2018. Mindtree Ltd.
    All rights reserved.
*/



/* --------------------------------------------- Header File Inclusion */
#include "appl_scheduler_client.h"


/* --------------------------------------------- Global Definitions */


/* --------------------------------------------- Static Global Variables */
static const char main_scheduler_client_options[] = "\n\
======== Scheduler Client Menu ========\n\
     0. Exit. \n\
     1. Refresh. \n\
 \n\
    10. Send Scheduler Action Get. \n\
    11. Send Scheduler Action Set. \n\
    12. Send Scheduler Action Set Unacknowledged. \n\
    13. Send Scheduler Get. \n\
 \n\
    14. Get Model Handle. \n\
    15. Set Publish Address. \n\
 \n\
Your Option ? \0";


/* --------------------------------------------- External Global Variables */
static MS_ACCESS_MODEL_HANDLE   appl_scheduler_client_model_handle;


/* --------------------------------------------- Function */
/* scheduler client application entry point */
void main_scheduler_client_operations(void)
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
        retval = MS_scheduler_client_init
                 (
                     element_handle,
                     &appl_scheduler_client_model_handle,
                     appl_scheduler_client_cb
                 );

        if (API_SUCCESS == retval)
        {
            CONSOLE_OUT(
                "Scheduler Client Initialized. Model Handle: 0x%04X\n",
                appl_scheduler_client_model_handle);
        }
        else
        {
            CONSOLE_OUT(
                "[ERR] Scheduler Client Initialization Failed. Result: 0x%04X\n",
                retval);
        }

        model_initialized = 0x01;
    }

    MS_LOOP_FOREVER()
    {
        CONSOLE_OUT
        ("%s", main_scheduler_client_options);
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

        case 10: /* Send Scheduler Action Get */
            appl_send_scheduler_action_get();
            break;

        case 11: /* Send Scheduler Action Set */
            appl_send_scheduler_action_set();
            break;

        case 12: /* Send Scheduler Action Set Unacknowledged */
            appl_send_scheduler_action_set_unacknowledged();
            break;

        case 13: /* Send Scheduler Get */
            appl_send_scheduler_get();
            break;

        case 14: /* Get Model Handle */
            appl_scheduler_client_get_model_handle();
            break;

        case 15: /* Set Publish Address */
            appl_scheduler_client_set_publish_address();
            break;
        }
    }
}

/* Send Scheduler Action Get */
void appl_send_scheduler_action_get(void)
{
    API_RESULT retval;
    int  choice;
    MS_SCHEDULER_ACTION_GET_STRUCT  param;
    CONSOLE_OUT
    (">> Send Scheduler Action Get\n");
    CONSOLE_OUT
    ("Enter Index (8-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.index = (UCHAR)choice;
    retval = MS_scheduler_action_get(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Scheduler Action Set */
void appl_send_scheduler_action_set(void)
{
    API_RESULT retval;
    int  choice;
    MS_SCHEDULER_ACTION_SET_STRUCT  param;
    CONSOLE_OUT
    (">> Send Scheduler Action Set\n");
    CONSOLE_OUT
    ("Enter Index (4-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.index = (UCHAR)choice;
    CONSOLE_OUT
    ("Enter Schedule Register\n");
    appl_input_schedule_register(&param);
    retval = MS_scheduler_action_set(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Scheduler Action Set Unacknowledged */
void appl_send_scheduler_action_set_unacknowledged(void)
{
    API_RESULT retval;
    int  choice;
    MS_SCHEDULER_ACTION_SET_STRUCT  param;
    CONSOLE_OUT
    (">> Send Scheduler Action Set Unacknowledged\n");
    CONSOLE_OUT
    ("Enter Index (4-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.index = (UCHAR)choice;
    CONSOLE_OUT
    ("Enter Schedule Register\n");
    appl_input_schedule_register(&param);
    retval = MS_scheduler_action_set_unacknowledged(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* User input for Schedule Register fields */
void appl_input_schedule_register( /* OUT */ MS_SCHEDULER_ACTION_SET_STRUCT*   schedule_register)
{
    int  choice;
    CONSOLE_OUT
    ("Enter Scheduled year for the action (7-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    schedule_register->year = (UINT8)choice;
    CONSOLE_OUT
    ("Enter Scheduled month for the action (12-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    schedule_register->month = (UINT16)choice;
    CONSOLE_OUT
    ("Enter Scheduled day of the month for the action (5-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    schedule_register->day = (UINT8)choice;
    CONSOLE_OUT
    ("Enter Scheduled hour for the action (5-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    schedule_register->hour = (UINT8)choice;
    CONSOLE_OUT
    ("Enter Scheduled minute for the action (6-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    schedule_register->minute = (UINT8)choice;
    CONSOLE_OUT
    ("Enter Scheduled second for the action (6-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    schedule_register->second = (UINT8)choice;
    CONSOLE_OUT
    ("Enter Scheduled days of the week for the action (7-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    schedule_register->day_of_week = (UINT8)choice;
    CONSOLE_OUT
    ("Enter Action to be performed at the scheduled time (4-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    schedule_register->action = (UINT8)choice;
    CONSOLE_OUT
    ("Enter Transition time for this action (8-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    schedule_register->transition_time = (UINT8)choice;
    CONSOLE_OUT
    ("Enter Scene number to be used for some actions (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    schedule_register->scene_number = (UINT16)choice;
}

/* Send Scheduler Get */
void appl_send_scheduler_get(void)
{
    API_RESULT retval;
    CONSOLE_OUT
    (">> Send Scheduler Get\n");
    retval = MS_scheduler_get();
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}


/* Get Model Handle */
void appl_scheduler_client_get_model_handle(void)
{
    API_RESULT retval;
    MS_ACCESS_MODEL_HANDLE model_handle;
    retval = MS_scheduler_client_get_model_handle(&model_handle);

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
void appl_scheduler_client_set_publish_address(void)
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
    ("Enter Scheduler client Server Address (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    publish_info.addr.addr = (UINT16)choice;
    CONSOLE_OUT
    ("Enter AppKey Index\n");
    CONSOLE_IN
    ("%x", &choice);
    publish_info.appkey_index = choice;
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
    Scheduler client calls the registered callback to indicate events occurred to the application.

    \param [in] handle        Model Handle.
    \param [in] opcode        Opcode.
    \param [in] data_param    Data associated with the event if any or NULL.
    \param [in] data_len      Size of the event data. 0 if event data is NULL.
*/
API_RESULT appl_scheduler_client_cb
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
        "[SCHEDULER_CLIENT] Callback. Opcode 0x%04X\n", opcode);
    appl_dump_bytes(data_param, data_len);

    switch(opcode)
    {
    case MS_ACCESS_SCHEDULER_ACTION_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_SCHEDULER_ACTION_STATUS_OPCODE\n");
    }
    break;

    case MS_ACCESS_SCHEDULER_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_SCHEDULER_STATUS_OPCODE\n");
    }
    break;
    }

    return retval;
}

