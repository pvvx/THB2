
/**
    \file EM_timer.c

    This File contains source codes for the EtherMind
    Timer Library Implementation for FreeRTOS.
*/

/*
    Copyright (C) 2013. Mindtree Ltd.
    All rights reserved.
*/

/* ----------------------------------------------- Header File Inclusion */
#include "EM_timer_internal.h"
#include "OSAL_Clock.h"

/* ----------------------------------------------- Global Definitions */
/* Timer Elements */
TIMER_ENTITY timer_entity[EM_TIMER_MAX_ENTITIES];
TIMER_ENTITY* timer_q_start = NULL;
TIMER_ENTITY* timer_q_end   = NULL;

#if 0
    /* Timer Library Mutex */
    EM_thread_mutex_type timer_mutex;
#endif /* 0 */

#if 1
/* Timer */
#if defined ( OSAL_CBTIMER_NUM_TASKS )
    #include "osal_cbtimer.h"
#endif


/* ----------------------------------------------- Static Global Variables */
/*********************************************************************
    CONSTANTS
*/
// Number of callback timers supported per task (limited by the number of OSAL event timers)
#define NUM_CBTIMERS_PER_TASK          15

// Total number of callback timers
#define NUM_CBTIMERS                   ( OSAL_CBTIMER_NUM_TASKS * NUM_CBTIMERS_PER_TASK )


typedef struct
{
    pfnCbTimer_t pfnCbTimer; // callback function to be called when timer expires
    uint8* pData;            // data to be passed in to callback function
} cbTimer_t;

extern cbTimer_t cbTimers[];

extern uint16 baseTaskID;
#define EVENT_ID( timerId )            ( 0x0001 << ( ( timerId ) % NUM_CBTIMERS_PER_TASK ) )

// Find out task id using timer id
#define TASK_ID( timerId )             ( ( ( timerId ) / NUM_CBTIMERS_PER_TASK ) + baseTaskID )

// Find out bank task id using task id
#define BANK_TASK_ID( taskId )         ( ( baseTaskID - ( taskId ) ) * NUM_CBTIMERS )


//extern uint16 $Sub$$osal_CbTimerProcessEvent( uint8 taskId, uint16 events );
//uint16 $Sub$$osal_CbTimerProcessEvent( uint8 taskId, uint16 events )
//{
//    if ( events & SYS_EVENT_MSG )
//  {
//    // Process OSAL messages

//    // return unprocessed events
//    return ( events ^ SYS_EVENT_MSG );
//  }

//  if ( events )
//  {
//    uint8 i;
//    uint16 event;

//    // Process event timers
//    for ( i = 0; i < NUM_CBTIMERS_PER_TASK32; i++ )
//    {
//      if ( ( events >> i ) & 0x0001 )
//      {
//        cbTimer_t *pTimer = &cbTimers[BANK_TASK_ID32( taskId )+i];

//        // Found the first event
//        event =  0x0001 << i;

//        // Timer expired, call the registered callback function
//        if(pTimer->pfnCbTimer==NULL)
//        {
//        }
//        pTimer->pfnCbTimer( pTimer->pData );

//        // Mark entry as free
//        pTimer->pfnCbTimer = NULL;
//
//        // Null out data pointer
//        pTimer->pData = NULL;
//
//        // We only process one event at a time
//        break;
//      }
//    }

//    // return unprocessed events
//    return ( events ^ event );
//}
//}


#endif

#undef EM_RESTART_TIMER

/* ----------------------------------------------- Functions */

void EM_timer_init (void)
{
    UINT16 index;
    EM_TIMER_TRC(
        "Initializing EtherMind Timer Library Module ...\n");
    #if 0
    /* Initialize Timer Mutex */
    EM_thread_mutex_init(&timer_mutex, NULL);
    #endif /* 0 */

    /* Initialize Timer Elements */
    for (index = 0; index < EM_TIMER_MAX_ENTITIES; index ++)
    {
        timer_init_entity(&timer_entity[index]);
        timer_entity[index].timer_id = PHYOS_INVALID_TIMER_ID;
        timer_entity[index].handle = index;
    }

    return;
}


void timer_em_init ( void )
{
    TIMER_ENTITY* timer;
    UINT16 index;
    /* Lock Timer */
    timer_lock();
    EM_TIMER_TRC(
        "Stack ON Initialization for Timer Library ...\n");

    /* Initialize Timer Entities */
    for (index = 0; index < EM_TIMER_MAX_ENTITIES; index ++)
    {
        timer = &timer_entity[index];
        timer->timer_id = PHYOS_INVALID_TIMER_ID;
    }

    /* Initialize Timer Q */
    timer_q_start = timer_q_end = NULL;
    timer_unlock();
    return;
}


void timer_em_shutdown ( void )
{
    UINT16 index;
    TIMER_ENTITY* timer;
    /* Lock Timer */
    timer_lock();
    /* Initialize Timer Q */
    timer_q_start = timer_q_end = NULL;

    /* Initialize Timer Entities */
    for (index = 0; index < EM_TIMER_MAX_ENTITIES; index++)
    {
        timer = &timer_entity[index];

        if (TIMER_ENTITY_IN_USE == timer->in_use)
        {
            /* Stop Timer */
            osal_CbTimerStop(timer->timer_id);

            if (timer->data_length > EM_TIMER_STATIC_DATA_SIZE)
            {
                timer_free(timer->allocated_data);
            }

            timer_init_entity(timer);
            timer->timer_id = PHYOS_INVALID_TIMER_ID;
        }
    }

    EM_TIMER_TRC(
        "Stack OFF on Timer Library Module ...\n");
    timer_unlock();
    return;
}

#if 0
Status_t osal_CbTimerStart32( pfnCbTimer_t pfnCbTimer, uint8* pData,
                              uint32 timeout, uint8* pTimerId )
{
    uint8 i;

    // Validate input parameters
    if ( pfnCbTimer == NULL )
    {
        return ( INVALIDPARAMETER );
    }

    // Look for an unused timer first
    for ( i = 0; i < NUM_CBTIMERS32; i++ )
    {
        if ( cbTimers[i].pfnCbTimer == NULL )
        {
            // Start the OSAL event timer first
            if ( osal_start_timerEx( TASK_ID32( i ), EVENT_ID32( i ), timeout ) == SUCCESS )
            {
                // Set up the callback timer
                cbTimers[i].pfnCbTimer = pfnCbTimer;
                cbTimers[i].pData = pData;

                if ( pTimerId != NULL )
                {
                    // Caller is intreseted in the timer id
                    *pTimerId = i;
                }

                return ( SUCCESS );
            }
        }
    }

    // No timer available
    return ( NO_TIMER_AVAIL );
}
#endif


EM_RESULT EM_start_timer
(
    EM_timer_handle* handle,
    UINT32 timeout,
    void (* callback) (void*, UINT16),
    void* data,
    UINT16 data_length
)
{
    UCHAR* data_ptr = NULL;
    EM_RESULT retval;
    TIMER_ENTITY current_timer;
    // HZF
    osalTimeUpdate();

    if (NULL == handle)
    {
        EM_TIMER_ERR(
            "NULL Argument Unacceptable for Timer Handles.\n");
        return EM_TIMER_HANDLE_IS_NULL;
    }

    EM_TIMER_TRC(
        "Preparing to Add New Timer Entity. Timeout = %d, Data Size = %d.\n",
        timeout, data_length);

    /* Timer Library expects to have a valid callback */
    if (NULL == callback)
    {
        EM_TIMER_ERR(
            "FAILED to Add New Timer Element. NULL Callback.\n");
        return EM_TIMER_CALLBACK_IS_NULL;
    }

    if (0 != data_length)
    {
        if (data_length > EM_TIMER_STATIC_DATA_SIZE)
        {
            data_ptr = (UCHAR*) timer_malloc (data_length);

            if (NULL == data_ptr)
            {
                EM_TIMER_ERR(
                    "FAILED to allocate Memory for Timer Handler Argument.\n");
                return EM_TIMER_MEMORY_ALLOCATION_FAILED;
            }

            current_timer.allocated_data = data_ptr;
        }
        else
        {
            data_ptr = current_timer.static_data;
        }

        /* Copy the Data to be passed to the Timer Handler */
        EM_mem_copy(data_ptr, data, data_length);
    }

    /* Store Timer Data Length, Callback & Timeout */
    current_timer.callback = callback;
    current_timer.data_length = data_length;
    current_timer.timeout = timeout;
    /* Lock Timer */
    timer_lock();
    /* Insert this Timer Entity into the List */
    retval = timer_add_entity(&current_timer);

    if (EM_SUCCESS != retval)
    {
        EM_TIMER_ERR(
            "FAILED to Add New Timer to Timer Queue. Error Code = 0x%04X\n",
            retval);

        if (current_timer.data_length > EM_TIMER_STATIC_DATA_SIZE)
        {
            timer_free (current_timer.allocated_data);
        }

        timer_unlock();
        return retval;
    }

    /* Store the Handle */
    *handle = current_timer.handle;
    EM_TIMER_TRC(
        "Successfully Added New Timer to Timer Queue. Handle = 0x%02X\n",
        *handle);
    timer_unlock();
    return EM_SUCCESS;
}


/* Callback registered with timer module */
void timer_timeout_handler (UINT8* handle)
{
    EM_RESULT retval;
    TIMER_ENTITY* timer;
    EM_TIMER_TRC (
        "In Timer handler (Timer Handle: 0x%02X)\n", *handle);
    /* Lock Timer */
    timer_lock();
    /* Get the appropriate timer entity */
    retval = timer_search_entity_timer_id (&timer, *handle);

    if (EM_SUCCESS != retval)
    {
        EM_TIMER_ERR(
            "*** UNKNOWN Spurious Timeout Callback?!?!\n");
        /* Unlock Timer */
        timer_unlock ();
        return;
    }

    /* Unlock Timer */
    timer_unlock ();

    if (timer->data_length > EM_TIMER_STATIC_DATA_SIZE)
    {
        /* Call the registered timeout handler */
        timer->callback (timer->allocated_data, timer->data_length);
    }
    else
    {
        /* Use Static Data */
        timer->callback (timer->static_data, timer->data_length);
    }

    /* Lock Timer */
    timer_lock ();
    #if 0
    /* Stop Timer */
    xTimerStop  (timer->timer_id, 0);
    #endif /* 0 */
    /* Free the Timer */
    timer_del_entity (timer, 1);
    /* Unlock Timer */
    timer_unlock ();
    return;
}


EM_RESULT EM_stop_timer
(
    EM_timer_handle* handle
)
{
    TIMER_ENTITY* timer;
    EM_RESULT retval;
    UINT8  timer_id;
    UINT32 new_timeout;
    Status_t status;

    if (EM_TIMER_MAX_ENTITIES <= *handle)
    {
        EM_TIMER_ERR(
            "NULL Argument Unacceptable for Timer Handles.\r\n");
        /* TODO: Use appropriate error value */
        *handle = EM_TIMER_HANDLE_INIT_VAL;
        return EM_TIMER_HANDLE_IS_NULL;
    }

    osalTimeUpdate();
    retval = EM_FAILURE;
    /* Lock Timer */
    timer_lock();
    timer = &timer_entity[*handle];
    /* Store the timer id before deleting entity */
    timer_id = timer->timer_id;
    new_timeout = 0x10;

    if(timer_search_entity(timer) == EM_SUCCESS)
    {
        if((osal_CbTimerUpdate(timer_id,new_timeout) == EM_SUCCESS)
                || (osalFindTimer(TASK_ID( timer->timer_id ),EVENT_ID( timer->timer_id )) == NULL))
        {
            retval = timer_del_entity(timer, 0x01);

            if (EM_SUCCESS != retval)
            {
                EM_TIMER_ERR(
                    "FAILED to find Timer Element. Handle = 0x%02X. Error Code = 0x%04X\n",
                    *handle, retval);
            }
            else
            {
                EM_TIMER_TRC(
                    "Successfully Deleted Timer Element for Handle 0x%02X.\n",
                    *handle);
                /* Stop Timer */
                status = osal_CbTimerStop(timer_id);
                osal_clear_event(TASK_ID( timer->timer_id ),EVENT_ID( timer->timer_id ));

                if (SUCCESS != status)
                {
                    retval = EM_FAILURE;
                }
                else
                {
                    retval = EM_SUCCESS;
                    EM_TIMER_TRC("*** Stopped Timer [ID: %04X]\n",
                                 timer_id);
                }
            }
        }
    }

    *handle = EM_TIMER_HANDLE_INIT_VAL;
    /* Unlock Timer */
    timer_unlock();
    return retval;
}


UINT32 EM_get_remain_timer
(
    EM_timer_handle handle
)
{
    TIMER_ENTITY* timer;
    UINT32 remain_timeout;

    if (EM_TIMER_MAX_ENTITIES <= handle)
    {
        EM_TIMER_ERR(
            "NULL Argument Unacceptable for Timer Handles.\r\n");
        /* TODO: Use appropriate error value */
        return EM_TIMER_HANDLE_IS_NULL;
    }

    /* Lock Timer */
    timer_lock();
    timer = &timer_entity[handle];
    remain_timeout = osal_get_timeoutEx(TASK_ID( timer->timer_id ),EVENT_ID( timer->timer_id ));
    /* Unlock Timer */
    timer_unlock();
    return remain_timeout;
}



EM_RESULT EM_restart_timerEx
(
    EM_timer_handle handle,
    UINT32 new_timeout
)
{
    TIMER_ENTITY* timer;
    EM_RESULT retval;

    if (EM_TIMER_MAX_ENTITIES <= handle)
    {
        EM_TIMER_ERR(
            "NULL Argument Unacceptable for Timer Handles.\n");
        /* TODO: Use appropriate error value */
        return EM_TIMER_HANDLE_IS_NULL;
    }

    timer_lock();
    timer = &timer_entity[handle];
    retval = timer_search_entity(timer);

    if (EM_SUCCESS != retval)
    {
        EM_TIMER_ERR(
            "FAILED to Find Timer ELement for Handle 0x%02X. Error Code = 0x%04X\n",
            handle, retval);
    }
    else
    {
        if(osal_CbTimerUpdate(timer->timer_id,new_timeout) == EM_SUCCESS)
        {
            return ( SUCCESS );
        }
    }

    // No timer available
    return ( NO_TIMER_AVAIL );
}

#ifdef EM_RESTART_TIMER
EM_RESULT EM_restart_timer
(
    EM_timer_handle handle,
    UINT32 new_timeout
)
{
    TIMER_ENTITY* timer;
    Status_t   ret;
    EM_RESULT retval;
    // HZF
    osalTimeUpdate();

    if (EM_TIMER_MAX_ENTITIES <= handle)
    {
        EM_TIMER_ERR(
            "NULL Argument Unacceptable for Timer Handles.\n");
        /* TODO: Use appropriate error value */
        return EM_TIMER_HANDLE_IS_NULL;
    }

    /* Lock Timer */
    timer_lock();
    timer = &timer_entity[handle];
    retval = timer_search_entity(timer);

    if (EM_SUCCESS != retval)
    {
        EM_TIMER_ERR(
            "FAILED to Find Timer ELement for Handle 0x%02X. Error Code = 0x%04X\n",
            handle, retval);
    }
    else
    {
        timer->timeout = new_timeout;
        /* Stop the existing timer */
        osal_CbTimerStop(timer->timer_id);
        /* Start Timer. */
        ret = osal_CbTimerStart
              (
                  timer_timeout_handler,
                  &timer->handle,
                  ((EM_TIMEOUT_MILLISEC & timer->timeout) ?
                   (timer->timeout & (UINT32)~(EM_TIMEOUT_MILLISEC)):
                   (timer->timeout * 1000)),
                  &timer->timer_id
              );

        if (SUCCESS != ret)
        {
            EM_TIMER_ERR("*** FAILED to Restart timer\n");
            timer_del_entity (timer, 0x01);
            return EM_TIMER_FAILED_SET_TIME_EVENT;
        }

        EM_TIMER_TRC(
            "Successfully restarted Timer [ID: %02X]. Handle: 0x%02X\n", timer->timer_id, timer->handle);
    }

    timer_unlock();
    return retval;
}
#endif

EM_RESULT EM_is_active_timer
(
    EM_timer_handle handle
)
{
    TIMER_ENTITY* timer;
    EM_RESULT retval;

    if (EM_TIMER_MAX_ENTITIES <= handle)
    {
        EM_TIMER_ERR(
            "NULL Argument Unacceptable for Timer Handles.\n");
        /* TODO: Use appropriate error value */
        return EM_TIMER_HANDLE_IS_NULL;
    }

    /* Lock Timer */
    timer_lock();
    timer = &timer_entity[handle];
    retval = timer_search_entity(timer);

    if (EM_SUCCESS != retval)
    {
        EM_TIMER_ERR(
            "FAILED to Find the Timer Entity for Handle 0x%02X. Error Code = 0x%04X\n",
            handle, retval);
    }

    timer_unlock();
    return retval;
}


EM_RESULT timer_search_entity ( TIMER_ENTITY* timer )
{
    TIMER_ENTITY* current_timer;

    /* Is Queue Empty */
    if (NULL == timer_q_start)
    {
        return EM_TIMER_QUEUE_EMPTY;
    }

    /* Handle the first Element */
    if (timer == timer_q_start)
    {
        return EM_SUCCESS;
    }

    current_timer = timer_q_start->next;

    while (NULL != current_timer)
    {
        if (timer == current_timer)
        {
            return EM_SUCCESS;
        }

        current_timer = current_timer->next;
    }

    return EM_TIMER_ENTITY_SEARCH_FAILED;
}

/* Get the timer based on obtained timer id */
EM_RESULT timer_search_entity_timer_id
(
    TIMER_ENTITY** timer,
    UINT8          handle
)
{
    TIMER_ENTITY* current_timer;

    /* Is Queue Empty */
    if (NULL == timer_q_start)
    {
        return EM_TIMER_QUEUE_EMPTY;
    }

    /* Handle the first Element */
    if (handle == timer_q_start->handle)
    {
        /* Note the timer entity */
        *timer = timer_q_start;
        return EM_SUCCESS;
    }

    current_timer = timer_q_start->next;

    while (NULL != current_timer)
    {
        if (handle == current_timer->handle)
        {
            /* Note the timer entity */
            *timer = current_timer;
            return EM_SUCCESS;
        }

        current_timer = current_timer->next;
    }

    return EM_TIMER_ENTITY_SEARCH_FAILED;
}

EM_RESULT timer_add_entity ( TIMER_ENTITY* timer )
{
    UINT16 index;
    Status_t   ret;
    TIMER_ENTITY* new_timer;
    new_timer = NULL;

    for (index = 0; index < EM_TIMER_MAX_ENTITIES; index++)
    {
        new_timer = &timer_entity[index];

        if (TIMER_ENTITY_FREE == new_timer->in_use)
        {
            new_timer->in_use = TIMER_ENTITY_IN_USE;
            break;
        }
        else
        {
            new_timer = NULL;
        }
    }

    if (NULL == new_timer)
    {
        EM_TIMER_ERR(
            "FAILED to Allocate New Timer Entity. Timer List FULL !\n");
//        printf(
//        "FAILED to Allocate New Timer Entity. Timer List FULL !\n");
        #ifdef EM_STATUS
        /* Timer List Full: Update EtherMind Status Flag */
        EM_status_set_bit (STATUS_BIT_TIMER_ENTITY_FULL, STATUS_BIT_SET);
        #endif /* EM_STATUS */
        return EM_TIMER_QUEUE_FULL;
    }

    new_timer->next = NULL;
    new_timer->timeout = timer->timeout;
    new_timer->callback = timer->callback;
    new_timer->data_length = timer->data_length;

    if (new_timer->data_length > EM_TIMER_STATIC_DATA_SIZE)
    {
        new_timer->allocated_data = timer->allocated_data;
    }
    else
    {
        EM_mem_copy
        (
            new_timer->static_data,
            timer->static_data,
            new_timer->data_length
        );
    }

    /* Start the timer */
    #ifdef EM_TIMER_SUPPORT_REMAINING_TIME
    new_timer->start_timestamp = em_timer_get_ms_timestamp();
    #endif /* EM_TIMER_SUPPORT_REMAINING_TIME */
    /* Start timer. Set Timeout. This will also start the timer. */
    ret = osal_CbTimerStart
          (
              timer_timeout_handler,
              &new_timer->handle,
              ((EM_TIMEOUT_MILLISEC & new_timer->timeout) ?
               (new_timer->timeout & (UINT32)~(EM_TIMEOUT_MILLISEC)):
               (new_timer->timeout * 1000)),
              &new_timer->timer_id
          );

    if (SUCCESS != ret)
    {
        EM_TIMER_ERR("*** FAILED to Start timer\n");
//      printf("*** FAILED to Start timer, ret = %d\r\n", ret);
        return EM_TIMER_FAILED_SET_TIME_EVENT;
    }

    EM_TIMER_TRC("Successfully started Timer [ID: %02X]. Handle: 0x%02X\n",
                 new_timer->timer_id, timer->handle);
    timer->handle = new_timer->handle;
    timer->timer_id = new_timer->timer_id;

    /* If the Timer Q Empty */
    if (NULL == timer_q_start)
    {
        timer_q_start = timer_q_end = new_timer;
        return EM_SUCCESS;
    }

    timer_q_end->next = new_timer;
    timer_q_end = new_timer;
    return EM_SUCCESS;
}


EM_RESULT timer_del_entity
(
    TIMER_ENTITY* timer,
    UCHAR free
)
{
    TIMER_ENTITY* current_timer, *previous_timer;

    /* Either None or One Element */
    if (timer_q_start == timer_q_end)
    {
        if (NULL == timer_q_start)
        {
            /* Queue is Empty */
            return EM_TIMER_QUEUE_EMPTY;
        }
        else
        {
            if (timer == timer_q_start)
            {
                /* Queue has One Element */
                timer_q_start = timer_q_end = NULL;
            }
            else
            {
                /* Match NOT found in the Only element in Timer Queue */
                return EM_TIMER_ENTITY_SEARCH_FAILED;
            }
        }
    }
    else
    {
        /* Queue has more than One Element */
        if (timer == timer_q_start)
        {
            /* Match in the First Element */
            timer_q_start = timer_q_start->next;
        }
        else
        {
            previous_timer = timer_q_start;
            current_timer = timer_q_start->next;

            while (NULL != current_timer)
            {
                if (timer == current_timer)
                {
                    previous_timer->next = current_timer->next;

                    if (current_timer == timer_q_end)
                    {
                        timer_q_end = previous_timer;
                    }

                    break;
                }

                previous_timer = current_timer;
                current_timer  = current_timer->next;
            }

            if (NULL == current_timer)
            {
                return EM_TIMER_ENTITY_SEARCH_FAILED;
            }
        }
    }

    /* Free Allocated Data */
    if ((0x01 == free) &&
            (timer->data_length > EM_TIMER_STATIC_DATA_SIZE))
    {
        timer_free (timer->allocated_data);
    }

    timer_init_entity(timer);
    return EM_SUCCESS;
}


EM_RESULT timer_init_entity (TIMER_ENTITY* timer)
{
    timer->in_use = TIMER_ENTITY_FREE;
    timer->timeout = 0;
    timer->callback = NULL;
    timer->allocated_data = NULL;
    timer->data_length = 0;
    timer->next = NULL;
    #ifdef EM_TIMER_SUPPORT_REMAINING_TIME
    timer->start_timestamp = 0;
    #endif /* EM_TIMER_SUPPORT_REMAINING_TIME */
    return EM_SUCCESS;
}


#ifdef EM_TIMER_SUPPORT_REMAINING_TIME

/*
    This function returns elapsed time in microsecond since system power on.
*/
UINT64 em_timer_get_ms_timestamp(void)
{
    return osal_GetSystemClock();
}

EM_RESULT EM_timer_get_remaining_time
(
    EM_timer_handle   handle,
    UINT32*           remaining_time_ms
)
{
    TIMER_ENTITY* timer;
    EM_RESULT     retval;
    UINT64        current_timestamp;
    UINT32        time_ms;

    if (NULL == handle)
    {
        EM_TIMER_ERR(
            "NULL Argument Unacceptable for Timer Handles.\n");
        return EM_TIMER_HANDLE_IS_NULL;
    }

    /* Lock Timer */
    timer_lock();
    timer = (TIMER_ENTITY*)handle;
    retval = timer_search_entity(timer);

    if (EM_SUCCESS != retval)
    {
        EM_TIMER_ERR(
            "FAILED to Find Timer ELement for Handle %p. Error Code = 0x%04X\n",
            (void*)handle, retval);
    }
    else
    {
        time_ms = ((EM_TIMEOUT_MILLISEC & timer->timeout) ?
                   (timer->timeout & (UINT32)~(EM_TIMEOUT_MILLISEC)) :
                   (timer->timeout * 1000));
        /* Get Current Time */
        /* Remaining Time = (Timeout in ms) - (Current Time - Timer Start Time) */
        current_timestamp = em_timer_get_ms_timestamp();

        if ((current_timestamp < timer->start_timestamp) ||
                (time_ms < (current_timestamp - timer->start_timestamp))
           )
        {
            EM_TIMER_ERR(
                "FAILED to Find Remaining Time.TO:%d < (CurT:%lld - StartT:%lld\n",
                time_ms, current_timestamp, timer->start_timestamp);
        }
        else
        {
            time_ms -= (UINT32)(current_timestamp - timer->start_timestamp);
            *remaining_time_ms = time_ms;
            EM_TIMER_TRC(
                "[EM_TIMER] Remaining Time (ms): %d\n", time_ms);
            retval = EM_SUCCESS;
        }
    }

    timer_unlock();
    return retval;
}
#endif /* EM_TIMER_SUPPORT_REMAINING_TIME */

EM_RESULT EM_list_timer ( void )
{
    #ifdef EM_TIMERL_DEBUG
    UINT16 index, free;
    TIMER_ENTITY* timer;
    timer_lock();
    EM_TIMERL_TRC("\n");
    EM_TIMERL_TRC("========================================= \n");
    EM_TIMERL_TRC("Start Q = %p\n", timer_q_start);
    timer = timer_q_start;

    while(NULL != timer)
    {
        EM_TIMERL_TRC("    Handle = 0x%02X",
                      timer->handle);
        timer = timer->next;
    }

    EM_TIMERL_TRC("End   Q = %p\n", timer_q_end);
    free = 0;

    for (index = 0; index < EM_TIMER_MAX_ENTITIES; index ++)
    {
        if (TIMER_ENTITY_FREE == timer_entity[index].in_use)
        {
            free ++;
        }
    }

    EM_TIMERL_TRC("Max Q Entity = %d, Free = %d\n",
                  EM_TIMER_MAX_ENTITIES, free);
    EM_TIMERL_TRC("========================================= \n");
    EM_TIMERL_TRC("\n");
    timer_unlock();
    #endif /* EM_TIMERL_DEBUG */
    return EM_SUCCESS;
}

