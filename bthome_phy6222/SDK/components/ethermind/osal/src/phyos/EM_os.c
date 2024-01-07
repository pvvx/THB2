
/**
    \file EM_os.c

    This source file is part of EtherMind OS Abstraction module.
    In this file the platform dependent library calls are abstracted
    and mapped to calls used within the EtherMind Stack.
*/

/*
    Copyright (C) 2013. Mindtree Ltd.
    All rights reserved.
*/

/* -------------------------------------------- Header File Inclusion */
#include "EM_os.h"


/* -------------------------------------------- External Global Variables */
extern uint32_t osal_sys_tick;


/* -------------------------------------------- Exported Global Variables */

/* -------------------------------------------- Static Global Variables */

/* -------------------------------------------- Functions */

void EM_os_init (void)
{
}

/**
    \fn EM_thread_create

    \brief To create a task/thread

    \par Description:
    This function implements the OS Wrapper for the Thread Create call.
    It creates a new thread of control  that  executes concurrently  with
    the  calling  thread.

    \param [out] thread
           Caller allocated location to hold the Thread ID
           for the newly Created Thread
    \param [in] attr
           The Thread Attributes, pre-initialized using EM_thread_attr_init()
    \param [in] start_routine
           The Thread Start Routine for the newly created task/thread.
           Upon creation, the new task/thread calls this function,
           passing it 'thread_args' as the argument
    \param [in] thread_args
           This parameter points to a caller allocated "resident" memory
           location, containing the arguments to be passed to
           the newly created task's/thread's start routine

    \return INT32 : 0 on Success. -1 on Failure.
*/
INT32 EM_thread_create
(
    /* OUT */ EM_thread_type*          thread,
    /* IN */  EM_thread_attr_type*     attr,
    /* IN */  EM_THREAD_START_ROUTINE  start_routine,
    /* IN */  void*                    thread_args
)
{
    return 0;
}


/**
    \fn EM_thread_attr_init

    \brief To initialize a task's/thread's attributes to default settings

    \par Description:
    This function implements the OS Wrapper for the Thread attribute init
    call. It initializes the given thread attribute object with default
    values.

    \param [out] attr
           Thread attribute object to be initialized

    \return INT32 : 0 on Success. -1 on Failure.
*/
INT32 EM_thread_attr_init
(
    /* OUT */ EM_thread_attr_type*     attr
)
{
    (void) attr;
    return 0;
}


/**
    \fn EM_thread_mutex_init

    \brief To initialize a Mutex object

    \par Description:
    This function implements the OS Wrapper for the Thread Mutex Init Call.
    It creates or initializes the mutex object and fills it with default
    values for the attributes.

    \param [out] mutex
           The Mutex variable to be Initialized
    \param [in] mutex_attr
           Attribute of the mutex variable on initialization

    \return INT32 : 0 on Success. -1 on Failure.
*/
INT32 EM_thread_mutex_init
(
    /* OUT */ EM_thread_mutex_type*          mutex,
    /* IN */  EM_thread_mutex_attr_type*     mutex_attr
)

{
    (void) mutex_attr;
    return 0;
}


/**
    \fn EM_thread_mutex_lock

    \brief To lock a Mutex object

    \par Description:
    This function implements the OS Wrapper for the Thread Mutex Lock call.
    It locks the given mutex.
    If the given mutex is currently unlocked, it becomes locked and owned by
    the calling thread, and this routine returns immediately.
    If the mutex is already locked by another thread, this routine suspends
    the calling thread until the mutex is unlocked.

    \param [in,out] mutex
           The Mutex variable to be locked

    \return INT32 : 0 on Success. -1 on Failure.
*/
INT32 EM_thread_mutex_lock
(
    /* INOUT */ EM_thread_mutex_type*     mutex
)
{
    return 0;
}


/**
    \fn EM_thread_mutex_unlock

    \brief To unlock a Mutex object

    \par Description:
    This function implements the OS Wrapper for the Mutex Unlock call.
    It unlocks the given mutex. The mutex is assumed to be locked and
    owned by the calling thread.

    \param [in,out] mutex
           The Mutex variable to be unlocked

    \return INT32 : 0 on Success. -1 on Failure.
*/
INT32 EM_thread_mutex_unlock
(
    /* INOUT */ EM_thread_mutex_type*     mutex
)
{
    return 0;
}


/**
    \fn EM_thread_cond_init

    \brief To initialize a Conditional Variable object

    \par Description:
    This function implements the Conditional Variable Initialisation call.
    It initializes the given conditional variable.

    \param [out] cond
           The Conditional Variable
    \param [in] cond_attr
           Attributes of the conditional variable on initialization

    \return INT32 : 0 on Success. -1 on Failure.

    \note
    A conditional variable is a synchronization primitive that allows
    threads to suspend execution and relinquish the processors
    until some predicate on shared resources is satisfied.
*/
INT32 EM_thread_cond_init
(
    /* OUT */ EM_thread_cond_type*          cond,
    /* IN */  EM_thread_cond_attr_type*     cond_attr
)
{
    (void) cond_attr;
    return 0;
}


/**
    \fn EM_thread_cond_wait

    \brief To wait on a Conditional Variable object

    \par Description:
    This function implements the OS Wrapper for the Thread Wait on
    Conditional Variable call.
    It waits for the conditional variable to be signaled.
    This routine releases the  mutex and waits for the condition variable
    to  be signaled. The execution of the calling thread is suspended
    until the conditional variable is signaled.
    The mutex must be locked by the calling thread before
    calling this function. Before returning to the calling thread,
    it will re-acquire the mutex variable.

    \param [in,out] cond
           The Conditional Variable
    \param [in,out] cond_mutex
           The Conditional Mutex Variable

    \return INT32 : 0 on Success. -1 on Failure.
*/
void EM_thread_cond_wait
(
    /* INOUT */ EM_thread_cond_type*      cond,
    /* INOUT */ EM_thread_mutex_type*     cond_mutex
)
{
    EM_thread_mutex_unlock (cond_mutex);
    EM_thread_mutex_lock (cond_mutex);
}


/**
    \fn EM_thread_cond_signal

    \brief To signal a Conditional Variable object

    \par Description:
    This function implements the OS Wrapper for the Thread Signalling on
    Conditional Variable Call.
    It will signal and restart one of the threads that are waiting on
    the given conditional variable.

    \param [in,out] cond
           The Conditional Variable to signal

    \return INT32 : 0 on Success. -1 on Failure.
*/
INT32 EM_thread_cond_signal
(
    /* INOUT */ EM_thread_cond_type*     cond
)
{
    return 0;
}

extern uint32  osal_memory_statics(void);
/**
    \fn EM_alloc_mem

    \brief To allocate memory dynamically

    \par Description:
    This function implements the OS Wrapper for the Memory Allocation call.
    It allocates memory of 'size' bytes and returns a pointer to
    the allocated memory.

    \param [in] size
           The number of bytes to allocate

    \return void * :  Pointer to the newly Allocated Memory segment.
                      NULL, in case of failure
*/
void* EM_alloc_mem ( /* IN */ UINT32 size )
{
    void* buf;
    buf = osal_mem_alloc (size);

    if (NULL == buf)
    {
        printf ("Memory Allocation Failed!, size = %d ", size);
//        osal_memory_statics();
    }

//  printf("<+%d> ", size);
    return buf;
}


/**
    \fn EM_free_mem

    \brief To free dynamically allocated memory

    \par Description:
    This function implements the OS Wrapper for the Memory Free call.
    It frees the memory space specified by the given pointer.

    \param [in] ptr
           Pointer to the Memory location to be freed

    \return None
*/
void EM_free_mem ( /* IN */ void* ptr )
{
    osal_mem_free(ptr);
    return;
}


/**
    \fn EM_sleep

    \brief To delay execution for a specified number of seconds

    \par Description:
    This function implements the OS Wrapper for a Thread or task to sleep
    for specified number of seconds.

    \param [in] tm
           Number of seconds the calling task or thread wants to sleep

    \return None
*/
void EM_sleep( /* IN */ UINT32 tm )
{
    /*
        1s = 1000 ms.
        1ms = portTICK_RATE_MS Ticks
    */
    /* The number of ticks for which the calling task should be held in the Blocked state. */
}


/**
    \fn EM_usleep

    \brief To delay execution for a specified number of microseconds

    \par Description:
    This function implements the OS Wrapper for a Thread or task to sleep
    for specified number of micro-seconds.

    \param [in] tm
           Number of micro-seconds the calling task or thread wants to sleep

    \return None
*/
void EM_usleep( /* IN */ UINT32 tm )
{
    /*
        1000 us = 1ms.
        1ms = portTICK_RATE_MS Ticks
    */
    /* The number of ticks for which the calling task should be held in the Blocked state. */
}


/**
    \fn EM_get_current_time

    \brief To get the current system time

    \par Description:
    This function implements the OS Wrapper to get current time
    from the system.

    \param [out] curtime
           Current Time

    \return None
*/
void EM_get_current_time (/* OUT */ UINT32* curtime)
{
    *curtime = osal_sys_tick;
    return;
}


/**
    \fn EM_get_local_time

    \brief To get the local time

    \par Description:
    This function implements the OS Wrapper to get local time
    from the system.

    \param [out] buf
           Local Time and date buffer
    \param [out] buflen
           Size of buffer provided

    \return None
*/
void EM_get_local_time( /* OUT */ UCHAR* buf, /* IN */ UINT16 buf_len)
{
    return;
}

