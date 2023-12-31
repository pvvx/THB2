/**
    \file appl_scene_client.c

    \brief This file defines the Mesh Scene Model Application Interface
    - includes Data Structures and Methods for Client.
*/

/*
    Copyright (C) 2018. Mindtree Ltd.
    All rights reserved.
*/



/* --------------------------------------------- Header File Inclusion */
#include "appl_scene_client.h"


/* --------------------------------------------- Global Definitions */


/* --------------------------------------------- Static Global Variables */
static const char main_scene_client_options[] = "\n\
======== Scene Client Menu ========\n\
     0. Exit. \n\
     1. Refresh. \n\
 \n\
    10. Send Scene Delete. \n\
    11. Send Scene Delete Unacknowledged. \n\
    12. Send Scene Get. \n\
    13. Send Scene Recall. \n\
    14. Send Scene Recall Unacknowledged. \n\
    15. Send Scene Register Get. \n\
    16. Send Scene Store. \n\
    17. Send Scene Store Unacknowledged. \n\
 \n\
    18. Get Model Handle. \n\
    19. Set Publish Address. \n\
 \n\
Your Option ? \0";


/* --------------------------------------------- External Global Variables */
static MS_ACCESS_MODEL_HANDLE   appl_scene_client_model_handle;


/* --------------------------------------------- Function */
/* scene client application entry point */
void main_scene_client_operations(void)
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
        retval = MS_scene_client_init
                 (
                     element_handle,
                     &appl_scene_client_model_handle,
                     appl_scene_client_cb
                 );

        if (API_SUCCESS == retval)
        {
            CONSOLE_OUT(
                "Scene Client Initialized. Model Handle: 0x%04X\n",
                appl_scene_client_model_handle);
        }
        else
        {
            CONSOLE_OUT(
                "[ERR] Scene Client Initialization Failed. Result: 0x%04X\n",
                retval);
        }

        model_initialized = 0x01;
    }

    MS_LOOP_FOREVER()
    {
        CONSOLE_OUT
        ("%s", main_scene_client_options);
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

        case 10: /* Send Scene Delete */
            appl_send_scene_delete();
            break;

        case 11: /* Send Scene Delete Unacknowledged */
            appl_send_scene_delete_unacknowledged();
            break;

        case 12: /* Send Scene Get */
            appl_send_scene_get();
            break;

        case 13: /* Send Scene Recall */
            appl_send_scene_recall();
            break;

        case 14: /* Send Scene Recall Unacknowledged */
            appl_send_scene_recall_unacknowledged();
            break;

        case 15: /* Send Scene Register Get */
            appl_send_scene_register_get();
            break;

        case 16: /* Send Scene Store */
            appl_send_scene_store();
            break;

        case 17: /* Send Scene Store Unacknowledged */
            appl_send_scene_store_unacknowledged();
            break;

        case 18: /* Get Model Handle */
            appl_scene_client_get_model_handle();
            break;

        case 19: /* Set Publish Address */
            appl_scene_client_set_publish_address();
            break;
        }
    }
}

/* Send Scene Delete */
void appl_send_scene_delete(void)
{
    API_RESULT retval;
    int  choice;
    MS_SCENE_STRUCT  param;
    CONSOLE_OUT
    (">> Send Scene Delete\n");
    CONSOLE_OUT
    ("Enter Scene Number (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.scene_number = (UINT16)choice;
    retval = MS_scene_delete(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Scene Delete Unacknowledged */
void appl_send_scene_delete_unacknowledged(void)
{
    API_RESULT retval;
    int  choice;
    MS_SCENE_STRUCT  param;
    CONSOLE_OUT
    (">> Send Scene Delete Unacknowledged\n");
    CONSOLE_OUT
    ("Enter Scene Number (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.scene_number = (UINT16)choice;
    retval = MS_scene_delete_unacknowledged(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Scene Get */
void appl_send_scene_get(void)
{
    API_RESULT retval;
    CONSOLE_OUT
    (">> Send Scene Get\n");
    retval = MS_scene_get();
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Scene Recall */
void appl_send_scene_recall(void)
{
    API_RESULT retval;
    int  choice;
    MS_SCENE_RECALL_STRUCT  param;
    CONSOLE_OUT
    (">> Send Scene Recall\n");
    CONSOLE_OUT
    ("Enter Scene Number (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.scene_number = (UINT16)choice;
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

    retval = MS_scene_recall(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Scene Recall Unacknowledged */
void appl_send_scene_recall_unacknowledged(void)
{
    API_RESULT retval;
    int  choice;
    MS_SCENE_RECALL_STRUCT  param;
    CONSOLE_OUT
    (">> Send Scene Recall Unacknowledged\n");
    CONSOLE_OUT
    ("Enter Scene Number (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.scene_number = (UINT16)choice;
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

    retval = MS_scene_recall_unacknowledged(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Scene Register Get */
void appl_send_scene_register_get(void)
{
    API_RESULT retval;
    CONSOLE_OUT
    (">> Send Scene Register Get\n");
    retval = MS_scene_register_get();
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Scene Store */
void appl_send_scene_store(void)
{
    API_RESULT retval;
    int  choice;
    MS_SCENE_STRUCT  param;
    CONSOLE_OUT
    (">> Send Scene Store\n");
    CONSOLE_OUT
    ("Enter Scene Number (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.scene_number = (UINT16)choice;
    retval = MS_scene_store(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Scene Store Unacknowledged */
void appl_send_scene_store_unacknowledged(void)
{
    API_RESULT retval;
    int  choice;
    MS_SCENE_STRUCT  param;
    CONSOLE_OUT
    (">> Send Scene Store Unacknowledged\n");
    CONSOLE_OUT
    ("Enter Scene Number (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.scene_number = (UINT16)choice;
    retval = MS_scene_store_unacknowledged(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}


/* Get Model Handle */
void appl_scene_client_get_model_handle(void)
{
    API_RESULT retval;
    MS_ACCESS_MODEL_HANDLE model_handle;
    retval = MS_scene_client_get_model_handle(&model_handle);

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
void appl_scene_client_set_publish_address(void)
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
    ("Enter Scene client Server Address (16-bit in HEX)\n");
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
    Scene client calls the registered callback to indicate events occurred to the application.

    \param [in] handle        Model Handle.
    \param [in] opcode        Opcode.
    \param [in] data_param    Data associated with the event if any or NULL.
    \param [in] data_len      Size of the event data. 0 if event data is NULL.
*/
API_RESULT appl_scene_client_cb
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
        "[SCENE_CLIENT] Callback. Opcode 0x%04X\n", opcode);
    appl_dump_bytes(data_param, data_len);

    switch(opcode)
    {
    case MS_ACCESS_SCENE_REGISTER_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_SCENE_REGISTER_STATUS_OPCODE\n");
    }
    break;

    case MS_ACCESS_SCENE_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_SCENE_STATUS_OPCODE\n");
    }
    break;
    }

    return retval;
}

