/**
    \file appl_model_state_handler.c
*/

/*
    Copyright (C) 2017. Mindtree Ltd.
    All rights reserved.
*/



/* --------------------------------------------- Header File Inclusion */
#include "appl_model_state_handler.h"
#include "MS_common.h"


/* --------------------------------------------- Global Definitions */


/* --------------------------------------------- Static Global Variables */


/* --------------------------------------------- Data Types/Structures */

/* ------- Transition Time Data Structures */

/* ------- Scene Data Structures */
#define MS_MAX_SCENE_REGS     16

/* --------------------------------------------- Function */
void appl_model_states_initialization(void)
{
}

/** Scene Server Handlers */

void* appl_scene_save_current_state(/* IN */ UINT32 scene_index)
{
    /* Free previous context - if any */
    /* Empty right now - dummy returning the same scene index itself */
    return (void*)scene_index;
}

void* appl_scene_delete_saved_state(/* IN */ UINT32 scene_index, /* IN */ void* context)
{
    /* Free context - if any */
    /* Empty right now - dummy returning NULL */
    return (void*)NULL;
}

void* appl_scene_recall_saved_state(/* IN */ UINT32 scene_index, /* IN */ void* context)
{
    /* Recall context - if any */
    /* Empty right now - dummy returning NULL */
    return (void*)NULL;
}
