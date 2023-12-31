
/**
    \file EM_timer.h

    This Header File contains the APIs and the ADTs exported by the
    EtherMind Timer Library for Windows (User-mode).
*/

/*
    Copyright (C) 2013. Mindtree Ltd.
    All rights reserved.
*/

#ifndef _H_EXT_CBTIMER_
#define _H_EXT_CBTIMER_

/* --------------------------------------------------- Header File Inclusion */
#include "EM_os.h"
#include "cbtimer.h"

/* Enable support to get remaining time to expire of a timer entity */
#define EXT_CBTIMER_SUPPORT_REMAINING_TIME

/* --------------------------------------------------- Global Definitions */
/* Maximum number of timer entities */
#define EXT_CBTIMER_MAX_ENTITIES                        10

/* Mask to indicate millisecond timeout */
#define EXT_CBTIMEOUT_MILLISEC                          0x80000000

/* Timer Handles must be initialized to this value */
#define EXT_CBTIMER_HANDLE_INIT_VAL                     0xFF

#define EXT_CBTIMER_STATIC_DATA_SIZE                    32

/* Flag: Timer Entity State */
#define EXT_CBTIMER_ENTITY_FREE                         0x00
#define EXT_CBTIMER_ENTITY_IN_USE                       0x01
#define EXT_CBTIMER_ENTITY_IN_FREE                      0x02

/* Flag: Timer Entity Data to be freed or not */
#define EXT_CBTIMER_ENTITY_HOLD_ALLOC_DATA              0x00
#define EXT_CBTIMER_ENTITY_FREE_ALLOC_DATA              0x01

/* Timer module ID and Error codes */
#define EXT_CBTIMER_ERR_ID                              0xC000

#define EXT_CBTIMER_MUTEX_INIT_FAILED                   (0x0001 | EXT_CBTIMER_ERR_ID)
#define EXT_CBTIMER_COND_INIT_FAILED                    (0x0002 | EXT_CBTIMER_ERR_ID)
#define EXT_CBTIMER_MUTEX_LOCK_FAILED                   (0x0003 | EXT_CBTIMER_ERR_ID)
#define EXT_CBTIMER_MUTEX_UNLOCK_FAILED                 (0x0004 | EXT_CBTIMER_ERR_ID)
#define EXT_CBTIMER_MEMORY_ALLOCATION_FAILED            (0x0005 | EXT_CBTIMER_ERR_ID)

#define EXT_CBTIMER_HANDLE_IS_NULL                      (0x0011 | EXT_CBTIMER_ERR_ID)
#define EXT_CBTIMER_CALLBACK_IS_NULL                    (0x0012 | EXT_CBTIMER_ERR_ID)
#define EXT_CBTIMER_QUEUE_EMPTY                         (0x0013 | EXT_CBTIMER_ERR_ID)
#define EXT_CBTIMER_QUEUE_FULL                          (0x0014 | EXT_CBTIMER_ERR_ID)
#define EXT_CBTIMER_ENTITY_SEARCH_FAILED                (0x0015 | EXT_CBTIMER_ERR_ID)
#define EXT_CBTIMER_NULL_PARAMETER_NOT_ALLOWED          (0x0016 | EXT_CBTIMER_ERR_ID)
#define EXT_CBTIMER_TIMEOUT_ZERO_NOT_ALLOWED            (0x0017 | EXT_CBTIMER_ERR_ID)
#define EXT_CBTIMER_FAILED_SET_TIME_EVENT               (0x0018 | EXT_CBTIMER_ERR_ID)

/**
    Invalid Timer ID for PhyOS.
    NUM_CBTIMERS is not exposed from osal_cbtimer.c
*/
#define EXT_PHYOS_INVALID_TIMER_ID                      0xFF

/* ----------------------------------------------- Structures/Data Types */

/* Timer Entity */
typedef struct ext_cbtimer_entity_struct
{
    /* The Timer Handle - Index of the timer entity */
    UINT8 handle;

    /* Callback to call when Timer expires */
    void (* callback) (void*, UINT16);

    /**
        Timer Callback Parameter if
        data_length > EM_TIMER_STATIC_DATA_SIZE
    */
    UCHAR*  allocated_data;

    /* Next Element in the Timer Q */
    struct ext_cbtimer_entity_struct* next;

    /**
        Timer Callback Parameter if
        data_length <= EM_TIMER_STATIC_DATA_SIZE
    */
    UCHAR  static_data[EXT_CBTIMER_STATIC_DATA_SIZE];

    /* Timeout Value asked by the User */
    UINT32 timeout;

    #ifdef EXT_CBTIMER_SUPPORT_REMAINING_TIME
    /* Start Time Stamp - used to calculate remaining time */
    UINT64 start_timestamp;
    #endif /* EM_TIMER_SUPPORT_REMAINING_TIME */

    /* Length of the data */
    UINT16 data_length;

    /* Is this Entity Allocated ? */
    UCHAR in_use;

    /* PhyOS Timer ID */
    UINT8 timer_id;

} EXT_CBTIMER_ENTITY;

typedef UINT8  EXT_cbtimer_handle;



/* --------------------------------------------------- Function Declarations */

#ifdef __cplusplus
extern "C" {
#endif

void EXT_cbtimer_init ( void );
void ext_cbtimer_em_init ( void );
void ext_cbtimer_em_shutdown ( void );

EM_RESULT EXT_cbtimer_start_timer
(
    EXT_cbtimer_handle* handle,
    UINT32 timeout,
    void (* callback) (void*, UINT16),
    void* args, UINT16 size_args
);

EM_RESULT EXT_cbtimer_restart_timer
(
    EXT_cbtimer_handle handle,
    UINT32 new_timeout
);

EM_RESULT EXT_cbtimer_stop_timer ( EXT_cbtimer_handle handle );

UINT32 EXT_cbtimer_get_remain_timer
(
    EXT_cbtimer_handle handle
);


EM_RESULT EXT_cbtimer_get_remaining_time
(
    EXT_cbtimer_handle   handle,
    UINT32*           remaining_time_ms
);

EM_RESULT EXT_cbtimer_is_active_timer ( EXT_cbtimer_handle handle );

/* Debug Routine - Internal Use Only */
EM_RESULT EXT_cbtimer_list_timer ( void );

#ifdef __cplusplus
};
#endif

#endif /* _H_EM_TIMER_ */

