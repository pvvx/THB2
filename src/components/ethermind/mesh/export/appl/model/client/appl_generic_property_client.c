/**
    \file appl_generic_property_client.c

    \brief This file defines the Mesh Generic Property Model Application Interface
    - includes Data Structures and Methods for Client.
*/

/*
    Copyright (C) 2018. Mindtree Ltd.
    All rights reserved.
*/



/* --------------------------------------------- Header File Inclusion */
#include "appl_generic_property_client.h"


/* --------------------------------------------- Global Definitions */


/* --------------------------------------------- Static Global Variables */
static const char main_generic_property_client_options[] = "\n\
======== Generic_Property Client Menu ========\n\
     0. Exit. \n\
     1. Refresh. \n\
 \n\
    10. Send Generic Admin Properties Get. \n\
    11. Send Generic Admin Property Get. \n\
    12. Send Generic Admin Property Set. \n\
    13. Send Generic Admin Property Set Unacknowledged. \n\
    14. Send Generic Client Properties Get. \n\
    15. Send Generic Manufacturer Properties Get. \n\
    16. Send Generic Manufacturer Property Get. \n\
    17. Send Generic Manufacturer Property Set. \n\
    18. Send Generic Manufacturer Property Set Unacknowledged. \n\
    19. Send Generic User Properties Get. \n\
    20. Send Generic User Property Get. \n\
    21. Send Generic User Property Set. \n\
    22. Send Generic User Property Set Unacknowledged. \n\
 \n\
    23. Get Model Handle. \n\
    24. Set Publish Address. \n\
 \n\
Your Option ? \0";


/* --------------------------------------------- External Global Variables */
static MS_ACCESS_MODEL_HANDLE   appl_generic_property_client_model_handle;


/* --------------------------------------------- Function */
/* generic_property client application entry point */
void main_generic_property_client_operations(void)
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
        retval = MS_generic_property_client_init
                 (
                     element_handle,
                     &appl_generic_property_client_model_handle,
                     appl_generic_property_client_cb
                 );

        if (API_SUCCESS == retval)
        {
            CONSOLE_OUT(
                "Generic Property Client Initialized. Model Handle: 0x%04X\n",
                appl_generic_property_client_model_handle);
        }
        else
        {
            CONSOLE_OUT(
                "[ERR] Generic Property Client Initialization Failed. Result: 0x%04X\n",
                retval);
        }

        model_initialized = 0x01;
    }

    MS_LOOP_FOREVER()
    {
        CONSOLE_OUT
        ("%s", main_generic_property_client_options);
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

        case 10: /* Send Generic Admin Properties Get */
            appl_send_generic_admin_properties_get();
            break;

        case 11: /* Send Generic Admin Property Get */
            appl_send_generic_admin_property_get();
            break;

        case 12: /* Send Generic Admin Property Set */
            appl_send_generic_admin_property_set();
            break;

        case 13: /* Send Generic Admin Property Set Unacknowledged */
            appl_send_generic_admin_property_set_unacknowledged();
            break;

        case 14: /* Send Generic Client Properties Get */
            appl_send_generic_client_properties_get();
            break;

        case 15: /* Send Generic Manufacturer Properties Get */
            appl_send_generic_manufacturer_properties_get();
            break;

        case 16: /* Send Generic Manufacturer Property Get */
            appl_send_generic_manufacturer_property_get();
            break;

        case 17: /* Send Generic Manufacturer Property Set */
            appl_send_generic_manufacturer_property_set();
            break;

        case 18: /* Send Generic Manufacturer Property Set Unacknowledged */
            appl_send_generic_manufacturer_property_set_unacknowledged();
            break;

        case 19: /* Send Generic User Properties Get */
            appl_send_generic_user_properties_get();
            break;

        case 20: /* Send Generic User Property Get */
            appl_send_generic_user_property_get();
            break;

        case 21: /* Send Generic User Property Set */
            appl_send_generic_user_property_set();
            break;

        case 22: /* Send Generic User Property Set Unacknowledged */
            appl_send_generic_user_property_set_unacknowledged();
            break;

        case 23: /* Get Model Handle */
            appl_generic_property_client_get_model_handle();
            break;

        case 24: /* Set Publish Address */
            appl_generic_property_client_set_publish_address();
            break;
        }
    }
}

/* Send Generic Admin Properties Get */
void appl_send_generic_admin_properties_get(void)
{
    API_RESULT retval;
    CONSOLE_OUT
    (">> Send Generic Admin Properties Get\n");
    retval = MS_generic_admin_properties_get();
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Generic Admin Property Get */
void appl_send_generic_admin_property_get(void)
{
    API_RESULT retval;
    int  choice;
    MS_GENERIC_ADMIN_PROPERTY_GET_STRUCT  param;
    CONSOLE_OUT
    (">> Send Generic Admin Property Get\n");
    CONSOLE_OUT
    ("Enter Admin Property ID (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.admin_property_id = (UINT16)choice;
    retval = MS_generic_admin_property_get(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Generic Admin Property Set */
void appl_send_generic_admin_property_set(void)
{
    API_RESULT retval;
    int  choice;
    MS_GENERIC_ADMIN_PROPERTY_SET_STRUCT  param;
    CONSOLE_OUT
    (">> Send Generic Admin Property Set\n");
    CONSOLE_OUT
    ("Enter Admin Property ID (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.admin_property_id = (UINT16)choice;
    CONSOLE_OUT
    ("Enter Admin User Access (8-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.admin_user_access = (UCHAR)choice;
    CONSOLE_OUT
    ("Enter Admin Property Value Length (in decimal)\n");
    CONSOLE_IN
    ("%d", &choice);
    CONSOLE_OUT
    ("Enter Admin Property Value (%d-octets in HEX)\n", (UINT16)choice);
    {
        UINT16 index, input_len;
        input_len = (UINT16)choice;
        param.admin_property_value = EM_alloc_mem(input_len);

        if(NULL == param.admin_property_value)
        {
            CONSOLE_OUT
            ("Memory allocation failed for Admin Property Value. Returning\n");
            return;
        }

        param.admin_property_value_len = (UINT16) input_len;

        for(index = 0; index < input_len; index++)
        {
            CONSOLE_IN
            ("%x", &choice);
            param.admin_property_value[index] = (UCHAR)choice;
        }
    }
    retval = MS_generic_admin_property_set(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);

    if(NULL != param.admin_property_value)
    {
        EM_free_mem(param.admin_property_value);
    }
}

/* Send Generic Admin Property Set Unacknowledged */
void appl_send_generic_admin_property_set_unacknowledged(void)
{
    API_RESULT retval;
    int  choice;
    MS_GENERIC_ADMIN_PROPERTY_SET_STRUCT  param;
    CONSOLE_OUT
    (">> Send Generic Admin Property Set Unacknowledged\n");
    CONSOLE_OUT
    ("Enter Admin Property ID (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.admin_property_id = (UINT16)choice;
    CONSOLE_OUT
    ("Enter Admin User Access (8-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.admin_user_access = (UCHAR)choice;
    CONSOLE_OUT
    ("Enter Admin Property Value Length (in decimal)\n");
    CONSOLE_IN
    ("%d", &choice);
    CONSOLE_OUT
    ("Enter Admin Property Value (%d-octets in HEX)\n", (UINT16)choice);
    {
        UINT16 index, input_len;
        input_len = (UINT16)choice;
        param.admin_property_value = EM_alloc_mem(input_len);

        if(NULL == param.admin_property_value)
        {
            CONSOLE_OUT
            ("Memory allocation failed for Admin Property Value. Returning\n");
            return;
        }

        param.admin_property_value_len = (UINT16) input_len;

        for(index = 0; index < input_len; index++)
        {
            CONSOLE_IN
            ("%x", &choice);
            param.admin_property_value[index] = (UCHAR)choice;
        }
    }
    retval = MS_generic_admin_property_set_unacknowledged(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);

    if(NULL != param.admin_property_value)
    {
        EM_free_mem(param.admin_property_value);
    }
}

/* Send Generic Client Properties Get */
void appl_send_generic_client_properties_get(void)
{
    API_RESULT retval;
    int  choice;
    MS_GENERIC_CLIENT_PROPERTIES_GET_STRUCT  param;
    CONSOLE_OUT
    (">> Send Generic Client Properties Get\n");
    CONSOLE_OUT
    ("Enter Client Property ID (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.client_property_id = (UINT16)choice;
    retval = MS_generic_client_properties_get(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Generic Manufacturer Properties Get */
void appl_send_generic_manufacturer_properties_get(void)
{
    API_RESULT retval;
    CONSOLE_OUT
    (">> Send Generic Manufacturer Properties Get\n");
    retval = MS_generic_manufacturer_properties_get();
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Generic Manufacturer Property Get */
void appl_send_generic_manufacturer_property_get(void)
{
    API_RESULT retval;
    int  choice;
    MS_GENERIC_MANUFACTURER_PROPERTY_GET_STRUCT  param;
    CONSOLE_OUT
    (">> Send Generic Manufacturer Property Get\n");
    CONSOLE_OUT
    ("Enter Manufacturer Property ID (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.manufacturer_property_id = (UINT16)choice;
    retval = MS_generic_manufacturer_property_get(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Generic Manufacturer Property Set */
void appl_send_generic_manufacturer_property_set(void)
{
    API_RESULT retval;
    int  choice;
    MS_GENERIC_MANUFACTURER_PROPERTY_SET_STRUCT  param;
    CONSOLE_OUT
    (">> Send Generic Manufacturer Property Set\n");
    CONSOLE_OUT
    ("Enter Manufacturer Property ID (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.manufacturer_property_id = (UINT16)choice;
    CONSOLE_OUT
    ("Enter Manufacturer User Access (8-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.manufacturer_user_access = (UCHAR)choice;
    retval = MS_generic_manufacturer_property_set(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Generic Manufacturer Property Set Unacknowledged */
void appl_send_generic_manufacturer_property_set_unacknowledged(void)
{
    API_RESULT retval;
    int  choice;
    MS_GENERIC_MANUFACTURER_PROPERTY_SET_STRUCT  param;
    CONSOLE_OUT
    (">> Send Generic Manufacturer Property Set Unacknowledged\n");
    CONSOLE_OUT
    ("Enter Manufacturer Property ID (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.manufacturer_property_id = (UINT16)choice;
    CONSOLE_OUT
    ("Enter Manufacturer User Access (8-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.manufacturer_user_access = (UCHAR)choice;
    retval = MS_generic_manufacturer_property_set_unacknowledged(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Generic User Properties Get */
void appl_send_generic_user_properties_get(void)
{
    API_RESULT retval;
    CONSOLE_OUT
    (">> Send Generic User Properties Get\n");
    retval = MS_generic_user_properties_get();
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Generic User Property Get */
void appl_send_generic_user_property_get(void)
{
    API_RESULT retval;
    int  choice;
    MS_GENERIC_USER_PROPERTY_GET_STRUCT  param;
    CONSOLE_OUT
    (">> Send Generic User Property Get\n");
    CONSOLE_OUT
    ("Enter User Property ID (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.user_property_id = (UINT16)choice;
    retval = MS_generic_user_property_get(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Generic User Property Set */
void appl_send_generic_user_property_set(void)
{
    API_RESULT retval;
    int  choice;
    MS_GENERIC_USER_PROPERTY_SET_STRUCT  param;
    CONSOLE_OUT
    (">> Send Generic User Property Set\n");
    CONSOLE_OUT
    ("Enter User Property ID (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.user_property_id = (UINT16)choice;
    CONSOLE_OUT
    ("Enter User Property Value Length (in decimal)\n");
    CONSOLE_IN
    ("%d", &choice);
    CONSOLE_OUT
    ("Enter User Property Value (%d-octets in HEX)\n", (UINT16)choice);
    {
        UINT16 index, input_len;
        input_len = (UINT16)choice;
        param.user_property_value = EM_alloc_mem(input_len);

        if(NULL == param.user_property_value)
        {
            CONSOLE_OUT
            ("Memory allocation failed for User Property Value. Returning\n");
            return;
        }

        param.user_property_value_len = (UINT16) input_len;

        for(index = 0; index < input_len; index++)
        {
            CONSOLE_IN
            ("%x", &choice);
            param.user_property_value[index] = (UCHAR)choice;
        }
    }
    retval = MS_generic_user_property_set(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);

    if(NULL != param.user_property_value)
    {
        EM_free_mem(param.user_property_value);
    }
}

/* Send Generic User Property Set Unacknowledged */
void appl_send_generic_user_property_set_unacknowledged(void)
{
    API_RESULT retval;
    int  choice;
    MS_GENERIC_USER_PROPERTY_SET_STRUCT  param;
    CONSOLE_OUT
    (">> Send Generic User Property Set Unacknowledged\n");
    CONSOLE_OUT
    ("Enter User Property ID (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.user_property_id = (UINT16)choice;
    CONSOLE_OUT
    ("Enter User Property Value Length (in decimal)\n");
    CONSOLE_IN
    ("%d", &choice);
    CONSOLE_OUT
    ("Enter User Property Value (%d-octets in HEX)\n", (UINT16)choice);
    {
        UINT16 index, input_len;
        input_len = (UINT16)choice;
        param.user_property_value = EM_alloc_mem(input_len);

        if(NULL == param.user_property_value)
        {
            CONSOLE_OUT
            ("Memory allocation failed for User Property Value. Returning\n");
            return;
        }

        param.user_property_value_len = (UINT16) input_len;

        for(index = 0; index < input_len; index++)
        {
            CONSOLE_IN
            ("%x", &choice);
            param.user_property_value[index] = (UCHAR)choice;
        }
    }
    retval = MS_generic_user_property_set_unacknowledged(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);

    if(NULL != param.user_property_value)
    {
        EM_free_mem(param.user_property_value);
    }
}


/* Get Model Handle */
void appl_generic_property_client_get_model_handle(void)
{
    API_RESULT retval;
    MS_ACCESS_MODEL_HANDLE model_handle;
    retval = MS_generic_property_client_get_model_handle(&model_handle);

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
void appl_generic_property_client_set_publish_address(void)
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
    ("Enter Generic_Property client Server Address (16-bit in HEX)\n");
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
    Generic_Property client calls the registered callback to indicate events occurred to the application.

    \param [in] handle        Model Handle.
    \param [in] opcode        Opcode.
    \param [in] data_param    Data associated with the event if any or NULL.
    \param [in] data_len      Size of the event data. 0 if event data is NULL.
*/
API_RESULT appl_generic_property_client_cb
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
        "[GENERIC_PROPERTY_CLIENT] Callback. Opcode 0x%04X\n", opcode);
    appl_dump_bytes(data_param, data_len);

    switch(opcode)
    {
    case MS_ACCESS_GENERIC_ADMIN_PROPERTIES_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_GENERIC_ADMIN_PROPERTIES_STATUS_OPCODE\n");
    }
    break;

    case MS_ACCESS_GENERIC_ADMIN_PROPERTY_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_GENERIC_ADMIN_PROPERTY_STATUS_OPCODE\n");
    }
    break;

    case MS_ACCESS_GENERIC_CLIENT_PROPERTIES_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_GENERIC_CLIENT_PROPERTIES_STATUS_OPCODE\n");
    }
    break;

    case MS_ACCESS_GENERIC_MANUFACTURER_PROPERTIES_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_GENERIC_MANUFACTURER_PROPERTIES_STATUS_OPCODE\n");
    }
    break;

    case MS_ACCESS_GENERIC_MANUFACTURER_PROPERTY_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_GENERIC_MANUFACTURER_PROPERTY_STATUS_OPCODE\n");
    }
    break;

    case MS_ACCESS_GENERIC_USER_PROPERTIES_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_GENERIC_USER_PROPERTIES_STATUS_OPCODE\n");
    }
    break;

    case MS_ACCESS_GENERIC_USER_PROPERTY_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_GENERIC_USER_PROPERTY_STATUS_OPCODE\n");
    }
    break;
    }

    return retval;
}

