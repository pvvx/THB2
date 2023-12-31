
/**
    \file fsm_engine.h

    This file defines interface offered by the FSM module.
*/

/*
    Copyright (C) 2013. Mindtree Ltd.
    All rights reserved.
*/

#ifndef _H_FSM_ENGINE_
#define _H_FSM_ENGINE_

/* --------------------------------------------- Header File Inclusion */
#include "MS_common.h"
#include "fsm_defines.h"


#ifndef FSM_NO_DEBUG
    #ifdef VAR_ARG_IN_MACRO_NOT_SUPPORTED
        #define FSM_ERR
    #else
        #define FSM_ERR(...)       EM_debug_error(MS_MODULE_ID_FSM,__VA_ARGS__)
    #endif  /*  VAR_ARG_IN_MACRO_NOT_SUPPORTED */
#else  /* FSM_NO_DEBUG */
    #define FSM_ERR            EM_debug_null
#endif /* FSM_NO_DEBUG */

#ifdef FSM_DEBUG

    #define FSM_TRC(...)       EM_debug_trace(BT_MODULE_ID_FSM,__VA_ARGS__)
    #define FSM_INF(...)       EM_debug_info(BT_MODULE_ID_FSM,__VA_ARGS__)

#else /* FSM_DEBUG */
    #ifdef VAR_ARG_IN_MACRO_NOT_SUPPORTED
        #define FSM_TRC
        #define FSM_INF
    #else
        #define FSM_TRC            EM_debug_null
        #define FSM_INF            EM_debug_null
    #endif  /*  VAR_ARG_IN_MACRO_NOT_SUPPORTED */

#endif /* FSM_DEBUG */

/* --------------------------------------------- Functions */
#ifdef __cplusplus
extern "C" {
#endif

void ms_fsm_init (void);

API_RESULT ms_fsm_register_module
(
    /* IN */  DECL_CONST FSM_MODULE_TABLE_T* module_fsm,
    /* OUT */ UCHAR* fsm_id
);

API_RESULT ms_fsm_post_event
(
    UCHAR     fsm_id,
    EVENT_T   fsm_event,
    void*       param
);

#ifdef __cplusplus
};
#endif

#endif /* _H_FSM_ENGINE_ */

