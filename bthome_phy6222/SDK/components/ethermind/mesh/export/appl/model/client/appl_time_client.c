/**
    \file appl_time_client.c

    \brief This file defines the Mesh Time Model Application Interface
    - includes Data Structures and Methods for Client.
*/

/*
    Copyright (C) 2018. Mindtree Ltd.
    All rights reserved.
*/



/* --------------------------------------------- Header File Inclusion */
#include "appl_time_client.h"


/* --------------------------------------------- Global Definitions */


/* --------------------------------------------- Static Global Variables */
static const char main_time_client_options[] = "\n\
======== Time Client Menu ========\n\
     0. Exit. \n\
     1. Refresh. \n\
 \n\
    10. Send Tai Utc Delta Get. \n\
    11. Send Tai Utc Delta Set. \n\
    12. Send Time Get. \n\
    13. Send Time Role Get. \n\
    14. Send Time Role Set. \n\
    15. Send Time Set. \n\
    16. Send Time Zone Get. \n\
    17. Send Time Zone Set. \n\
 \n\
    18. Get Model Handle. \n\
    19. Set Publish Address. \n\
 \n\
Your Option ? \0";


/* --------------------------------------------- External Global Variables */
static MS_ACCESS_MODEL_HANDLE   appl_time_client_model_handle;


/* --------------------------------------------- Function */
/* time client application entry point */
void main_time_client_operations(void)
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
        retval = MS_time_client_init
                 (
                     element_handle,
                     &appl_time_client_model_handle,
                     appl_time_client_cb
                 );

        if (API_SUCCESS == retval)
        {
            CONSOLE_OUT(
                "Time Client Initialized. Model Handle: 0x%04X\n",
                appl_time_client_model_handle);
        }
        else
        {
            CONSOLE_OUT(
                "[ERR] Time Client Initialization Failed. Result: 0x%04X\n",
                retval);
        }

        model_initialized = 0x01;
    }

    MS_LOOP_FOREVER()
    {
        CONSOLE_OUT
        ("%s", main_time_client_options);
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

        case 10: /* Send Tai Utc Delta Get */
            appl_send_tai_utc_delta_get();
            break;

        case 11: /* Send Tai Utc Delta Set */
            appl_send_tai_utc_delta_set();
            break;

        case 12: /* Send Time Get */
            appl_send_time_get();
            break;

        case 13: /* Send Time Role Get */
            appl_send_time_role_get();
            break;

        case 14: /* Send Time Role Set */
            appl_send_time_role_set();
            break;

        case 15: /* Send Time Set */
            appl_send_time_set();
            break;

        case 16: /* Send Time Zone Get */
            appl_send_time_zone_get();
            break;

        case 17: /* Send Time Zone Set */
            appl_send_time_zone_set();
            break;

        case 18: /* Get Model Handle */
            appl_time_client_get_model_handle();
            break;

        case 19: /* Set Publish Address */
            appl_time_client_set_publish_address();
            break;
        }
    }
}

/* Send Tai Utc Delta Get */
void appl_send_tai_utc_delta_get(void)
{
    API_RESULT retval;
    CONSOLE_OUT
    (">> Send Tai Utc Delta Get\n");
    retval = MS_tai_utc_delta_get();
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Tai Utc Delta Set */
void appl_send_tai_utc_delta_set(void)
{
    API_RESULT retval;
    int  choice;
    MS_TAI_UTC_DELTA_SET_STRUCT  param;
    CONSOLE_OUT
    (">> Send Tai Utc Delta Set\n");
    CONSOLE_OUT
    ("Enter TAI UTC Delta New (15-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.tai_utc_delta_new = (UINT16)choice;
    CONSOLE_OUT
    ("Enter Padding (1-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.padding = (UCHAR)choice;
    CONSOLE_OUT
    ("Enter TAI of Delta Change (5-octets in HEX)\n");
    {
        UINT16 index;

        for(index = 0; index < 5; index++)
        {
            CONSOLE_IN
            ("%x", &choice);
            param.tai_of_delta_change[index] = (UCHAR)choice;
        }
    }
    retval = MS_tai_utc_delta_set(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Time Get */
void appl_send_time_get(void)
{
    API_RESULT retval;
    CONSOLE_OUT
    (">> Send Time Get\n");
    retval = MS_time_get();
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Time Role Get */
void appl_send_time_role_get(void)
{
    API_RESULT retval;
    CONSOLE_OUT
    (">> Send Time Role Get\n");
    retval = MS_time_role_get();
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Time Role Set */
void appl_send_time_role_set(void)
{
    API_RESULT retval;
    int  choice;
    MS_TIME_ROLE_STRUCT  param;
    CONSOLE_OUT
    (">> Send Time Role Set\n");
    CONSOLE_OUT
    ("Enter Time Role (8-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.time_role = (UCHAR)choice;
    retval = MS_time_role_set(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Time Set */
void appl_send_time_set(void)
{
    API_RESULT retval;
    int  choice;
    MS_TIME_STRUCT  param;
    CONSOLE_OUT
    (">> Send Time Set\n");
    CONSOLE_OUT
    ("Enter TAI Seconds (5-octets in HEX)\n");
    {
        UINT16 index;

        for(index = 0; index < 5; index++)
        {
            CONSOLE_IN
            ("%x", &choice);
            param.tai_seconds[index] = (UCHAR)choice;
        }
    }
    CONSOLE_OUT
    ("Enter Subsecond (8-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.subsecond = (UCHAR)choice;
    CONSOLE_OUT
    ("Enter Uncertainty (8-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.uncertainty = (UCHAR)choice;
    CONSOLE_OUT
    ("Enter Time Authority (1-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.time_authority = (UCHAR)choice;
    CONSOLE_OUT
    ("Enter TAI UTC Delta (15-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.tai_utc_delta = (UINT16)choice;
    CONSOLE_OUT
    ("Enter Time Zone Offset (8-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.time_zone_offset = (UCHAR)choice;
    retval = MS_time_set(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Time Zone Get */
void appl_send_time_zone_get(void)
{
    API_RESULT retval;
    CONSOLE_OUT
    (">> Send Time Zone Get\n");
    retval = MS_time_zone_get();
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Time Zone Set */
void appl_send_time_zone_set(void)
{
    API_RESULT retval;
    int  choice;
    MS_TIME_ZONE_SET_STRUCT  param;
    CONSOLE_OUT
    (">> Send Time Zone Set\n");
    CONSOLE_OUT
    ("Enter Time Zone Offset New (8-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.time_zone_offset_new = (UCHAR)choice;
    CONSOLE_OUT
    ("Enter TAI of Zone Change (5-octets in HEX)\n");
    {
        UINT16 index;

        for(index = 0; index < 5; index++)
        {
            CONSOLE_IN
            ("%x", &choice);
            param.tai_of_zone_change[index] = (UCHAR)choice;
        }
    }
    retval = MS_time_zone_set(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}


/* Get Model Handle */
void appl_time_client_get_model_handle(void)
{
    API_RESULT retval;
    MS_ACCESS_MODEL_HANDLE model_handle;
    retval = MS_time_client_get_model_handle(&model_handle);

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
void appl_time_client_set_publish_address(void)
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
    ("Enter Time client Server Address (16-bit in HEX)\n");
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
    Time client calls the registered callback to indicate events occurred to the application.

    \param [in] handle        Model Handle.
    \param [in] opcode        Opcode.
    \param [in] data_param    Data associated with the event if any or NULL.
    \param [in] data_len      Size of the event data. 0 if event data is NULL.
*/
API_RESULT appl_time_client_cb
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
        "[TIME_CLIENT] Callback. Opcode 0x%04X\n", opcode);
    appl_dump_bytes(data_param, data_len);

    switch(opcode)
    {
    case MS_ACCESS_TAI_UTC_DELTA_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_TAI_UTC_DELTA_STATUS_OPCODE\n");
    }
    break;

    case MS_ACCESS_TIME_ROLE_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_TIME_ROLE_STATUS_OPCODE\n");
    }
    break;

    case MS_ACCESS_TIME_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_TIME_STATUS_OPCODE\n");
    }
    break;

    case MS_ACCESS_TIME_ZONE_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_TIME_ZONE_STATUS_OPCODE\n");
    }
    break;
    }

    return retval;
}

