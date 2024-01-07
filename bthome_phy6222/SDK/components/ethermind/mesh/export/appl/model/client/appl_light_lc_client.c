/**
    \file appl_light_lc_client.c

    \brief This file defines the Mesh Light Lc Model Application Interface
    - includes Data Structures and Methods for Client.
*/

/*
    Copyright (C) 2018. Mindtree Ltd.
    All rights reserved.
*/



/* --------------------------------------------- Header File Inclusion */
#include "appl_light_lc_client.h"


/* --------------------------------------------- Global Definitions */


/* --------------------------------------------- Static Global Variables */
static const char main_light_lc_client_options[] = "\n\
======== Light_Lc Client Menu ========\n\
     0. Exit. \n\
     1. Refresh. \n\
 \n\
    10. Send Light Lc Light Onoff Get. \n\
    11. Send Light Lc Light Onoff Set. \n\
    12. Send Light Lc Light Onoff Set Unacknowledged. \n\
    13. Send Light Lc Mode Get. \n\
    14. Send Light Lc Mode Set. \n\
    15. Send Light Lc Mode Set Unacknowledged. \n\
    16. Send Light Lc Om Get. \n\
    17. Send Light Lc Om Set. \n\
    18. Send Light Lc Om Set Unacknowledged. \n\
    19. Send Light Lc Property Get. \n\
    20. Send Light Lc Property Set. \n\
    21. Send Light Lc Property Set Unacknowledged. \n\
 \n\
    22. Get Model Handle. \n\
    23. Set Publish Address. \n\
 \n\
Your Option ? \0";


/* --------------------------------------------- External Global Variables */
static MS_ACCESS_MODEL_HANDLE   appl_light_lc_client_model_handle;


/* --------------------------------------------- Function */
/* light_lc client application entry point */
void main_light_lc_client_operations(void)
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
        retval = MS_light_lc_client_init
                 (
                     element_handle,
                     &appl_light_lc_client_model_handle,
                     appl_light_lc_client_cb
                 );

        if (API_SUCCESS == retval)
        {
            CONSOLE_OUT(
                "Light Lc Client Initialized. Model Handle: 0x%04X\n",
                appl_light_lc_client_model_handle);
        }
        else
        {
            CONSOLE_OUT(
                "[ERR] Light Lc Client Initialization Failed. Result: 0x%04X\n",
                retval);
        }

        model_initialized = 0x01;
    }

    MS_LOOP_FOREVER()
    {
        CONSOLE_OUT
        ("%s", main_light_lc_client_options);
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

        case 10: /* Send Light Lc Light Onoff Get */
            appl_send_light_lc_light_onoff_get();
            break;

        case 11: /* Send Light Lc Light Onoff Set */
            appl_send_light_lc_light_onoff_set();
            break;

        case 12: /* Send Light Lc Light Onoff Set Unacknowledged */
            appl_send_light_lc_light_onoff_set_unacknowledged();
            break;

        case 13: /* Send Light Lc Mode Get */
            appl_send_light_lc_mode_get();
            break;

        case 14: /* Send Light Lc Mode Set */
            appl_send_light_lc_mode_set();
            break;

        case 15: /* Send Light Lc Mode Set Unacknowledged */
            appl_send_light_lc_mode_set_unacknowledged();
            break;

        case 16: /* Send Light Lc Om Get */
            appl_send_light_lc_om_get();
            break;

        case 17: /* Send Light Lc Om Set */
            appl_send_light_lc_om_set();
            break;

        case 18: /* Send Light Lc Om Set Unacknowledged */
            appl_send_light_lc_om_set_unacknowledged();
            break;

        case 19: /* Send Light Lc Property Get */
            appl_send_light_lc_property_get();
            break;

        case 20: /* Send Light Lc Property Set */
            appl_send_light_lc_property_set();
            break;

        case 21: /* Send Light Lc Property Set Unacknowledged */
            appl_send_light_lc_property_set_unacknowledged();
            break;

        case 22: /* Get Model Handle */
            appl_light_lc_client_get_model_handle();
            break;

        case 23: /* Set Publish Address */
            appl_light_lc_client_set_publish_address();
            break;
        }
    }
}

/* Send Light Lc Light Onoff Get */
void appl_send_light_lc_light_onoff_get(void)
{
    API_RESULT retval;
    CONSOLE_OUT
    (">> Send Light Lc Light Onoff Get\n");
    retval = MS_light_lc_light_onoff_get();
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Light Lc Light Onoff Set */
void appl_send_light_lc_light_onoff_set(void)
{
    API_RESULT retval;
    int  choice;
    MS_LIGHT_LC_LIGHT_ONOFF_SET_STRUCT  param;
    CONSOLE_OUT
    (">> Send Light Lc Light Onoff Set\n");
    CONSOLE_OUT
    ("Enter Light OnOff (8-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.light_onoff = (UCHAR)choice;
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

    retval = MS_light_lc_light_onoff_set(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Light Lc Light Onoff Set Unacknowledged */
void appl_send_light_lc_light_onoff_set_unacknowledged(void)
{
    API_RESULT retval;
    int  choice;
    MS_LIGHT_LC_LIGHT_ONOFF_SET_STRUCT  param;
    CONSOLE_OUT
    (">> Send Light Lc Light Onoff Set Unacknowledged\n");
    CONSOLE_OUT
    ("Enter Light OnOff (8-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.light_onoff = (UCHAR)choice;
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

    retval = MS_light_lc_light_onoff_set_unacknowledged(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Light Lc Mode Get */
void appl_send_light_lc_mode_get(void)
{
    API_RESULT retval;
    CONSOLE_OUT
    (">> Send Light Lc Mode Get\n");
    retval = MS_light_lc_mode_get();
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Light Lc Mode Set */
void appl_send_light_lc_mode_set(void)
{
    API_RESULT retval;
    int  choice;
    MS_LIGHT_LC_MODE_STRUCT  param;
    CONSOLE_OUT
    (">> Send Light Lc Mode Set\n");
    CONSOLE_OUT
    ("Enter Mode (8-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.mode = (UCHAR)choice;
    retval = MS_light_lc_mode_set(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Light Lc Mode Set Unacknowledged */
void appl_send_light_lc_mode_set_unacknowledged(void)
{
    API_RESULT retval;
    int  choice;
    MS_LIGHT_LC_MODE_STRUCT  param;
    CONSOLE_OUT
    (">> Send Light Lc Mode Set Unacknowledged\n");
    CONSOLE_OUT
    ("Enter Mode (8-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.mode = (UCHAR)choice;
    retval = MS_light_lc_mode_set_unacknowledged(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Light Lc Om Get */
void appl_send_light_lc_om_get(void)
{
    API_RESULT retval;
    CONSOLE_OUT
    (">> Send Light Lc Om Get\n");
    retval = MS_light_lc_om_get();
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Light Lc Om Set */
void appl_send_light_lc_om_set(void)
{
    API_RESULT retval;
    int  choice;
    MS_LIGHT_LC_OM_STRUCT  param;
    CONSOLE_OUT
    (">> Send Light Lc Om Set\n");
    CONSOLE_OUT
    ("Enter Mode (8-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.mode = (UCHAR)choice;
    retval = MS_light_lc_om_set(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Light Lc Om Set Unacknowledged */
void appl_send_light_lc_om_set_unacknowledged(void)
{
    API_RESULT retval;
    int  choice;
    MS_LIGHT_LC_OM_STRUCT  param;
    CONSOLE_OUT
    (">> Send Light Lc Om Set Unacknowledged\n");
    CONSOLE_OUT
    ("Enter Mode (8-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.mode = (UCHAR)choice;
    retval = MS_light_lc_om_set_unacknowledged(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Light Lc Property Get */
void appl_send_light_lc_property_get(void)
{
    API_RESULT retval;
    int  choice;
    MS_LIGHT_LC_PROPERTY_GET_STRUCT  param;
    CONSOLE_OUT
    (">> Send Light Lc Property Get\n");
    CONSOLE_OUT
    ("Enter Light LC Property ID (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.light_lc_property_id = (UINT16)choice;
    retval = MS_light_lc_property_get(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Light Lc Property Set */
void appl_send_light_lc_property_set(void)
{
    API_RESULT retval;
    int  choice;
    MS_LIGHT_LC_PROPERTY_SET_STRUCT  param;
    CONSOLE_OUT
    (">> Send Light Lc Property Set\n");
    CONSOLE_OUT
    ("Enter Light LC Property ID (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.light_lc_property_id = (UINT16)choice;
    CONSOLE_OUT
    ("Enter Light LC Property Value Length (in decimal)\n");
    CONSOLE_IN
    ("%d", &choice);
    CONSOLE_OUT
    ("Enter Light LC Property Value (%d-octets in HEX)\n", (UINT16)choice);
    {
        UINT16 index, input_len;
        input_len = (UINT16)choice;
        param.light_lc_property_value = EM_alloc_mem(input_len);

        if(NULL == param.light_lc_property_value)
        {
            CONSOLE_OUT
            ("Memory allocation failed for Light LC Property Value. Returning\n");
            return;
        }

        param.light_lc_property_value_len = (UINT16) input_len;

        for(index = 0; index < input_len; index++)
        {
            CONSOLE_IN
            ("%x", &choice);
            param.light_lc_property_value[index] = (UCHAR)choice;
        }
    }
    retval = MS_light_lc_property_set(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);

    if(NULL != param.light_lc_property_value)
    {
        EM_free_mem(param.light_lc_property_value);
    }
}

/* Send Light Lc Property Set Unacknowledged */
void appl_send_light_lc_property_set_unacknowledged(void)
{
    API_RESULT retval;
    int  choice;
    MS_LIGHT_LC_PROPERTY_SET_STRUCT  param;
    CONSOLE_OUT
    (">> Send Light Lc Property Set Unacknowledged\n");
    CONSOLE_OUT
    ("Enter Light LC Property ID (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.light_lc_property_id = (UINT16)choice;
    CONSOLE_OUT
    ("Enter Light LC Property Value Length (in decimal)\n");
    CONSOLE_IN
    ("%d", &choice);
    CONSOLE_OUT
    ("Enter Light LC Property Value (%d-octets in HEX)\n", (UINT16)choice);
    {
        UINT16 index, input_len;
        input_len = (UINT16)choice;
        param.light_lc_property_value = EM_alloc_mem(input_len);

        if(NULL == param.light_lc_property_value)
        {
            CONSOLE_OUT
            ("Memory allocation failed for Light LC Property Value. Returning\n");
            return;
        }

        param.light_lc_property_value_len = (UINT16) input_len;

        for(index = 0; index < input_len; index++)
        {
            CONSOLE_IN
            ("%x", &choice);
            param.light_lc_property_value[index] = (UCHAR)choice;
        }
    }
    retval = MS_light_lc_property_set_unacknowledged(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);

    if(NULL != param.light_lc_property_value)
    {
        EM_free_mem(param.light_lc_property_value);
    }
}


/* Get Model Handle */
void appl_light_lc_client_get_model_handle(void)
{
    API_RESULT retval;
    MS_ACCESS_MODEL_HANDLE model_handle;
    retval = MS_light_lc_client_get_model_handle(&model_handle);

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
void appl_light_lc_client_set_publish_address(void)
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
    ("Enter Light_Lc client Server Address (16-bit in HEX)\n");
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
    Light_Lc client calls the registered callback to indicate events occurred to the application.

    \param [in] handle        Model Handle.
    \param [in] opcode        Opcode.
    \param [in] data_param    Data associated with the event if any or NULL.
    \param [in] data_len      Size of the event data. 0 if event data is NULL.
*/
API_RESULT appl_light_lc_client_cb
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
        "[LIGHT_LC_CLIENT] Callback. Opcode 0x%04X\n", opcode);
    appl_dump_bytes(data_param, data_len);

    switch(opcode)
    {
    case MS_ACCESS_LIGHT_LC_LIGHT_ONOFF_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_LIGHT_LC_LIGHT_ONOFF_STATUS_OPCODE\n");
    }
    break;

    case MS_ACCESS_LIGHT_LC_MODE_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_LIGHT_LC_MODE_STATUS_OPCODE\n");
    }
    break;

    case MS_ACCESS_LIGHT_LC_OM_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_LIGHT_LC_OM_STATUS_OPCODE\n");
    }
    break;

    case MS_ACCESS_LIGHT_LC_PROPERTY_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_LIGHT_LC_PROPERTY_STATUS_OPCODE\n");
    }
    break;
    }

    return retval;
}

