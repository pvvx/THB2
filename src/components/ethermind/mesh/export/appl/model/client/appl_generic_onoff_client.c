/**
    \file appl_generic_onoff_client.c

    \brief This file defines the Mesh Generic Onoff Model Application Interface
    - includes Data Structures and Methods for Client.
*/

/*
    Copyright (C) 2018. Mindtree Ltd.
    All rights reserved.
*/



/* --------------------------------------------- Header File Inclusion */
#include "appl_generic_onoff_client.h"


/* --------------------------------------------- Global Definitions */


/* --------------------------------------------- Static Global Variables */
static const char main_generic_onoff_client_options[] = "\n\
======== Generic_Onoff Client Menu ========\n\
     0. Exit. \n\
     1. Refresh. \n\
 \n\
    10. Send Generic Onoff Get. \n\
    11. Send Generic Onoff Set. \n\
    12. Send Generic Onoff Set Unacknowledged. \n\
 \n\
    13. Get Model Handle. \n\
    14. Set Publish Address. \n\
 \n\
Your Option ? \0";


/* --------------------------------------------- External Global Variables */
static MS_ACCESS_MODEL_HANDLE   appl_generic_onoff_client_model_handle;


/* --------------------------------------------- Function */
/* generic_onoff client application entry point */
void main_generic_onoff_client_operations(/* IN */ UINT8 have_menu)
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
        retval = MS_generic_onoff_client_init
                 (
                     element_handle,
                     &appl_generic_onoff_client_model_handle,
                     appl_generic_onoff_client_cb
                 );

        if (API_SUCCESS == retval)
        {
            CONSOLE_OUT(
                "Generic Onoff Client Initialized. Model Handle: 0x%04X\n",
                appl_generic_onoff_client_model_handle);
        }
        else
        {
            CONSOLE_OUT(
                "[ERR] Generic Onoff Client Initialization Failed. Result: 0x%04X\n",
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
        ("%s", main_generic_onoff_client_options);
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

        case 10: /* Send Generic Onoff Get */
            appl_send_generic_onoff_get();
            break;

        case 11: /* Send Generic Onoff Set */
            appl_send_generic_onoff_set();
            break;

        case 12: /* Send Generic Onoff Set Unacknowledged */
            appl_send_generic_onoff_set_unacknowledged();
            break;

        case 13: /* Get Model Handle */
            appl_generic_onoff_client_get_model_handle();
            break;

        case 14: /* Set Publish Address */
            appl_generic_onoff_client_set_publish_address();
            break;
        }
    }
}

/* Send Generic Onoff Get */
void appl_send_generic_onoff_get(void)
{
    API_RESULT retval;
    CONSOLE_OUT
    (">> Send Generic Onoff Get\n");
    retval = MS_generic_onoff_get();
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Generic Onoff Set */
void appl_send_generic_onoff_set(void)
{
    API_RESULT retval;
    int  choice;
    MS_GENERIC_ONOFF_SET_STRUCT  param;
    CONSOLE_OUT
    (">> Send Generic Onoff Set\n");
    CONSOLE_OUT
    ("Enter OnOff (8-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.onoff = (UCHAR)choice;
    CONSOLE_OUT
    ("Enter TID (8-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.tid = (UCHAR)choice;
    CONSOLE_OUT
    ("Enter if optional fields to be send (1 - Yes, 0 - No)\n");
    CONSOLE_IN
    ("%d", &choice);

    if(0 == choice)
    {
        param.optional_fields_present = 0x00;
    }
    else
    {
        param.optional_fields_present = 0x01;
        CONSOLE_OUT
        ("Enter Transition Time (8-bit in HEX)\n");
        CONSOLE_IN
        ("%x", &choice);
        param.transition_time = (UCHAR)choice;
        CONSOLE_OUT
        ("Enter Delay (8-bit in HEX)\n");
        CONSOLE_IN
        ("%x", &choice);
        param.delay = (UCHAR)choice;
    }

    retval = MS_generic_onoff_set(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Generic Onoff Set Unacknowledged */
void appl_send_generic_onoff_set_unacknowledged(void)
{
    API_RESULT retval;
    int  choice;
    MS_GENERIC_ONOFF_SET_STRUCT  param;
    CONSOLE_OUT
    (">> Send Generic Onoff Set Unacknowledged\n");
    CONSOLE_OUT
    ("Enter OnOff (8-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.onoff = (UCHAR)choice;
    CONSOLE_OUT
    ("Enter TID (8-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.tid = (UCHAR)choice;
    CONSOLE_OUT
    ("Enter if optional fields to be send (1 - Yes, 0 - No)\n");
    CONSOLE_IN
    ("%d", &choice);

    if(0 == choice)
    {
        param.optional_fields_present = 0x00;
    }
    else
    {
        param.optional_fields_present = 0x01;
        CONSOLE_OUT
        ("Enter Transition Time (8-bit in HEX)\n");
        CONSOLE_IN
        ("%x", &choice);
        param.transition_time = (UCHAR)choice;
        CONSOLE_OUT
        ("Enter Delay (8-bit in HEX)\n");
        CONSOLE_IN
        ("%x", &choice);
        param.delay = (UCHAR)choice;
    }

    retval = MS_generic_onoff_set_unacknowledged(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}


/* Get Model Handle */
void appl_generic_onoff_client_get_model_handle(void)
{
    API_RESULT retval;
    MS_ACCESS_MODEL_HANDLE model_handle;
    retval = MS_generic_onoff_client_get_model_handle(&model_handle);

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
void appl_generic_onoff_client_set_publish_address(void)
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
    ("Enter Generic_Onoff client Server Address (16-bit in HEX)\n");
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
    Generic_Onoff client calls the registered callback to indicate events occurred to the application.

    \param [in] handle        Model Handle.
    \param [in] opcode        Opcode.
    \param [in] data_param    Data associated with the event if any or NULL.
    \param [in] data_len      Size of the event data. 0 if event data is NULL.
*/
API_RESULT appl_generic_onoff_client_cb
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

