/**
    \file appl_scene_server.c
*/

/*
    Copyright (C) 2017. Mindtree Ltd.
    All rights reserved.
*/



/* --------------------------------------------- Header File Inclusion */
#include "appl_scene_server.h"
#include "appl_model_state_handler.h"


/* --------------------------------------------- Global Definitions */


/* --------------------------------------------- Static Global Variables */
static const char main_scene_server_options[] = "\n\
======== Scene Server Menu ========\n\
     0. Exit. \n\
     1. Refresh. \n\
 \n\
 \n\
Your Option ? \0";


/* --------------------------------------------- External Global Variables */
static MS_ACCESS_MODEL_HANDLE   appl_scene_server_model_handle;
static MS_ACCESS_MODEL_HANDLE   appl_scene_setup_server_model_handle;

/* --------------------------------------------- Function */
/* scene server application entry point */
void main_scene_server_operations(/* IN */ UINT8 have_menu)
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
        retval = MS_scene_server_init
                 (
                     element_handle,
                     &appl_scene_server_model_handle,
                     &appl_scene_setup_server_model_handle,
                     appl_scene_server_cb
                 );

        if (API_SUCCESS == retval)
        {
            CONSOLE_OUT(
                "Scene Server Initialized. Model Handle: 0x%04X\n",
                appl_scene_server_model_handle);
            CONSOLE_OUT(
                "Scene Setup Server Initialized. Model Handle: 0x%04X\n",
                appl_scene_setup_server_model_handle);
            appl_model_states_initialization();
        }
        else
        {
            CONSOLE_OUT(
                "[ERR] Scene Server Initialization Failed. Result: 0x%04X\n",
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
        ("%s", main_scene_server_options);
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
        }
    }
}

#if 0
/**
    \brief Server Application Asynchronous Notification Callback.

    \par Description
    Scene server calls the registered callback to indicate events occurred to the application.

    \param [in] ctx           Context of message received for a specific model instance.
    \param [in] msg_raw       Uninterpreted/raw received message.
    \param [in] req_type      Requested message type.
    \param [in] state_params  Model specific state parameters.
    \param [in] ext_params    Additional parameters.

    TODO: Update
*/
void* appl_scene_server_cb
(
    /* IN */ MS_ACCESS_MODEL_HANDLE*              handle,
    /* IN */ UINT8                                event_type,
    /* IN */ void*                                event_param,
    /* IN */ UINT16                               event_length,
    /* IN */ void*                                context
)
{
    void* param_p;
    param_p = NULL;

    switch(event_type)
    {
    case MS_SCENE_EVENT_STORE:
    {
        param_p = appl_scene_save_current_state(*(UINT32*)event_param);
    }
    break;

    case MS_SCENE_EVENT_DELETE:
    {
        param_p = appl_scene_delete_saved_state(*(UINT32*)event_param, context);
    }
    break;

    case MS_SCENE_EVENT_RECALL_START:
    {
    }
    break;

    case MS_SCENE_EVENT_RECALL_COMPLETE:
    {
        param_p = appl_scene_recall_saved_state(*(UINT32*)event_param, context);
    }
    break;

    case MS_SCENE_EVENT_RECALL_IMMEDIATE:
    {
        param_p = appl_scene_recall_saved_state(*(UINT32*)event_param, context);
    }
    break;
    }

    return param_p;
}

#endif /* 0 */
