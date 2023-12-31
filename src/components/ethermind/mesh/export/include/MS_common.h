
/**
    \file MS_common.h

    This Header file describes common declarations for the
    EtherMind Mesh modules.
*/

/*
    Copyright (C) 2017. Mindtree Ltd.
    All rights reserved.
*/

#ifndef _H_MS_COMMON_
#define _H_MS_COMMON_

/**
    \defgroup ms_common_module MESH Common
    \{
*/

/* -------------------------------------------- Header File Inclusion */
/* The EtherMind OS Abstraction */
#include "EM_os.h"

/* The EtherMind Configuration Parameters */
#include "MS_features.h"

/* The EtherMind Tunable Constant */
#include "MS_limits.h"

/* The Bluetooth Assigned Numbers */
#include "MS_assigned_numbers.h"

/* The EtherMind Error Codes */
#include "MS_error.h"

/* The EtherMind Debug Library */
#include "EM_debug.h"

/* For EM_assert() macro */
#include "EM_assert.h"

/* For Status Flag APIs */
/* #include "MS_status.h" */

/* The EtherMind Timer Library */
#include "EM_timer.h"

/* EtherMind Platform Specific Initialization & Shutdown Handlers */
#include "MS_common_pl.h"

/* For Memory (leak, corruption) Testing */
#ifdef MEMWATCH
    #include "memwatch.h"
#endif /* MEMWATCH */

/* TODO: See what should be the order of inclusion */
#include "MS_model_states.h"

/* -------------------------------------------- Global Definitions */

/**
    \cond ignore_this
    \{
*/

/* MS_COMMON Debug Macros */
#ifndef MS_COMMON_NO_DEBUG
    #define MS_COMMON_ERR(...)          EM_debug_error(MS_MODULE_ID_COMMON, __VA_ARGS__)
#else   /* COMMON_NO_DEBUG */
    #define MS_COMMON_ERR                          EM_debug_null
#endif  /* MS_COMMON_NO_DEBUG */

#ifdef MS_COMMON_DEBUG

    #define MS_COMMON_TRC(...)          EM_debug_trace(MS_MODULE_ID_COMMON, __VA_ARGS__)
    #define MS_COMMON_INF(...)          EM_debug_info(MS_MODULE_ID_COMMON, __VA_ARGS__)

#else /* MS_COMMON_DEBUG */

    #define MS_COMMON_TRC                          EM_debug_null
    #define MS_COMMON_INF                          EM_debug_null

#endif /* MS_COMMON_DEBUG */

/** \endcond */

/**
    \defgroup ms_common_defines Defines
    \{
*/

/**
    \defgroup ms_common_constants Constants
    \{
*/

#define MS_STACK_INIT_UNDEFINED                0x00
#define MS_STACK_INIT_ETHERMIND_INIT           0x01

/* Definition for True/False */
#ifndef MS_FALSE
    #define MS_FALSE                                   0
#endif /* MS_FALSE */

#ifndef MS_TRUE
    #define MS_TRUE                                    1
#endif /* MS_TRUE */

/** \} */

/** \} */

/* -------------------------------------------- Macros */

/**
    \defgroup ms_common_utility_macros Utility Macros
    \{
*/

/**
    Packing Macros.

    Syntax: MS_PACK_<Endian-ness LE/BE>_<no_of_bytes>_BYTE

    Usage: Based on the endian-ness defined for each protocol/profile layer,
    appropriate packing macros to be used by each layer.

    Example: HCI is defined as little endian protocol,
    so if HCI defines HCI_PACK_2_BYTE for packing a parameter of size 2 byte,
    that shall be mapped to MS_PACK_LE_2_BYTE

    By default both the packing and unpaking macros uses pointer to
    a single or multi-octet variable which to be packed to or unpacked from
    a buffer (unsinged character array).

    For the packing macro, another variation is available,
    where the single or multi-octet variable itself is used (not its pointer).

    Syntax: MS_PACK_<Endian-ness LE/BE>_<no_of_bytes>_BYTE_VAL
*/

/* Little Endian Packing Macros */
#define MS_PACK_LE_1_BYTE(dst, src) \
    { \
        UCHAR val; \
        val = (UCHAR)(*(src)); \
        MS_PACK_LE_1_BYTE_VAL((dst), val); \
    }

#define MS_PACK_LE_1_BYTE_VAL(dst, src) \
    *((UCHAR *)(dst) + 0) = src;

#define MS_PACK_LE_2_BYTE(dst, src) \
    { \
        UINT16 val; \
        val = (UINT16)(*(src)); \
        MS_PACK_LE_2_BYTE_VAL((dst), val); \
    }

#define MS_PACK_LE_2_BYTE_VAL(dst, src) \
    *((UCHAR *)(dst) + 0) = (UCHAR)(src); \
    *((UCHAR *)(dst) + 1) = (UCHAR)(src >> 8);

#define MS_PACK_LE_3_BYTE(dst, src) \
    { \
        UINT32 val; \
        val = (UINT32)(*(src)); \
        MS_PACK_LE_3_BYTE_VAL((dst), val); \
    }

#define MS_PACK_LE_3_BYTE_VAL(dst, src) \
    *((UCHAR *)(dst) + 0) = (UCHAR)(src);\
    *((UCHAR *)(dst) + 1) = (UCHAR)(src >> 8);\
    *((UCHAR *)(dst) + 2) = (UCHAR)(src >> 16);

#define MS_PACK_LE_4_BYTE(dst, src) \
    { \
        UINT32 val; \
        val = (UINT32)(*(src)); \
        MS_PACK_LE_4_BYTE_VAL((dst), val); \
    }

#define MS_PACK_LE_4_BYTE_VAL(dst, src) \
    *((UCHAR *)(dst) + 0) = (UCHAR)(src);\
    *((UCHAR *)(dst) + 1) = (UCHAR)(src >> 8);\
    *((UCHAR *)(dst) + 2) = (UCHAR)(src >> 16);\
    *((UCHAR *)(dst) + 3) = (UCHAR)(src >> 24);

/* TBD: Update based on 64 Bit, 128 Bit Data Types */
#define MS_PACK_LE_8_BYTE(dst,val)\
    EM_mem_copy ((dst), (val), 8)

#define MS_PACK_LE_16_BYTE(dst,val)\
    EM_mem_copy ((dst), (val), 16)

#define MS_PACK_LE_N_BYTE(dst,val,n)\
    EM_mem_copy ((dst), (val), (n))

/* Big Endian Packing Macros */
#define MS_PACK_BE_1_BYTE(dst, src) \
    { \
        UCHAR val; \
        val = (UCHAR)(*((UCHAR *)(src))); \
        MS_PACK_BE_1_BYTE_VAL((dst), val); \
    }

#define MS_PACK_BE_1_BYTE_VAL(dst, src) \
    *((UCHAR *)(dst) + 0) = src;

#define MS_PACK_BE_2_BYTE(dst, src) \
    { \
        UINT16 val; \
        val = (UINT16)(*((UINT16 *)(src))); \
        MS_PACK_BE_2_BYTE_VAL((dst), val); \
    }

#define MS_PACK_BE_2_BYTE_VAL(dst, src) \
    *((UCHAR *)(dst) + 1) = (UCHAR)(src); \
    *((UCHAR *)(dst) + 0) = (UCHAR)(src >> 8);

#define MS_PACK_BE_3_BYTE(dst, src) \
    { \
        UINT32 val; \
        val = (UINT32)(*((UINT32 *)(src))); \
        MS_PACK_BE_3_BYTE_VAL((dst), val); \
    }

#define MS_PACK_BE_3_BYTE_VAL(dst, src) \
    *((UCHAR *)(dst) + 2) = (UCHAR)(src);\
    *((UCHAR *)(dst) + 1) = (UCHAR)(src >> 8);\
    *((UCHAR *)(dst) + 0) = (UCHAR)(src >> 16);

#define MS_PACK_BE_4_BYTE(dst, src) \
    { \
        UINT32 val; \
        val = (UINT32)(*((UINT32 *)(src))); \
        MS_PACK_BE_4_BYTE_VAL((dst), val); \
    }

#define MS_PACK_BE_4_BYTE_VAL(dst, src) \
    *((UCHAR *)(dst) + 3) = (UCHAR)(src);\
    *((UCHAR *)(dst) + 2) = (UCHAR)(src >> 8);\
    *((UCHAR *)(dst) + 1) = (UCHAR)(src >> 16);\
    *((UCHAR *)(dst) + 0) = (UCHAR)(src >> 24);

/* TBD: Update based on 64 Bit, 128 Bit Data Types */
#define MS_PACK_BE_8_BYTE(dst,val)\
    EM_mem_copy ((dst), (val), 8)

#define MS_PACK_BE_16_BYTE(dst,val)\
    EM_mem_copy ((dst), (val), 16)

#define MS_PACK_BE_N_BYTE(dst,val,n)\
    EM_mem_copy ((dst), (val), (n))


/**
    Unpacking Macros.

    Syntax: MS_UNPACK_<Endian-ness LE/BE>_<no_of_bytes>_BYTE

    Usage: Based on the endian-ness defined for each protocol/profile layer,
    appropriate unpacking macros to be used by each layer.

    Example: HCI is defined as little endian protocol,
    so if HCI defines HCI_UNPACK_4_BYTE for unpacking a parameter of size 4 byte,
    that shall be mapped to MS_UNPACK_LE_4_BYTE
*/

/* Little Endian Unpacking Macros */
#define MS_UNPACK_LE_1_BYTE(dst,src)\
    *((UCHAR *)(dst)) = (UCHAR)(*((UCHAR *)(src)));

#define MS_UNPACK_LE_2_BYTE(dst,src)\
    *((UINT16 *)(dst))  = *((src) + 1); \
    *((UINT16 *)(dst))  = *((UINT16 *)(dst)) << 8; \
    *((UINT16 *)(dst)) |= *((src) + 0);

#define MS_UNPACK_LE_3_BYTE(dst,src)\
    *((UINT32 *)(dst))  = *((src) + 2);\
    *((UINT32 *)(dst))  = (*((UINT32 *)(dst))) << 8;\
    *((UINT32 *)(dst)) |= *((src) + 1);\
    *((UINT32 *)(dst))  = (*((UINT32 *)(dst))) << 8;\
    *((UINT32 *)(dst)) |= *((src) + 0);

#define MS_UNPACK_LE_4_BYTE(dst,src)\
    *((UINT32 *)(dst))  = *((src) + 3);\
    *((UINT32 *)(dst))  = (*((UINT32 *)(dst))) << 8;\
    *((UINT32 *)(dst)) |= *((src) + 2);\
    *((UINT32 *)(dst))  = (*((UINT32 *)(dst))) << 8;\
    *((UINT32 *)(dst)) |= *((src) + 1);\
    *((UINT32 *)(dst))  = (*((UINT32 *)(dst))) << 8;\
    *((UINT32 *)(dst)) |= *((src) + 0);

/* TBD: Update based on 64 Bit, 128 Bit Data Types */
#define MS_UNPACK_LE_8_BYTE(dst,src)\
    EM_mem_copy ((dst), (src), 8)

#define MS_UNPACK_LE_16_BYTE(dst,src)\
    EM_mem_copy ((dst), (src), 16)

#define MS_UNPACK_LE_N_BYTE(dst,src,n)\
    EM_mem_copy ((dst), (src), (n))

/* Big Endian Unpacking Macros */
#define MS_UNPACK_BE_1_BYTE(dst,src)\
    *((UCHAR *)(dst)) = (UCHAR)(*((UCHAR *)(src)));

#define MS_UNPACK_BE_2_BYTE(dst,src)\
    *((UINT16 *)(dst))  = *((src) + 0); \
    *((UINT16 *)(dst))  = *((UINT16 *)(dst)) << 8; \
    *((UINT16 *)(dst)) |= *((src) + 1);

#define MS_UNPACK_BE_3_BYTE(dst,src)\
    *((UINT32 *)(dst))  = *((src) + 0);\
    *((UINT32 *)(dst))  = (*((UINT32 *)(dst))) << 8;\
    *((UINT32 *)(dst)) |= *((src) + 1);\
    *((UINT32 *)(dst))  = (*((UINT32 *)(dst))) << 8;\
    *((UINT32 *)(dst)) |= *((src) + 2);

#define MS_UNPACK_BE_4_BYTE(dst,src)\
    *((UINT32 *)(dst))  = *((src) + 0);\
    *((UINT32 *)(dst))  = (*((UINT32 *)(dst))) << 8;\
    *((UINT32 *)(dst)) |= *((src) + 1);\
    *((UINT32 *)(dst))  = (*((UINT32 *)(dst))) << 8;\
    *((UINT32 *)(dst)) |= *((src) + 2);\
    *((UINT32 *)(dst))  = (*((UINT32 *)(dst))) << 8;\
    *((UINT32 *)(dst)) |= *((src) + 3);

/* TBD: Update based on 64 Bit, 128 Bit Data Types */
#define MS_UNPACK_BE_8_BYTE(dst,src)\
    EM_mem_copy ((dst), (src), 8)

#define MS_UNPACK_BE_16_BYTE(dst,src)\
    EM_mem_copy ((dst), (src), 16)

#define MS_UNPACK_BE_N_BYTE(dst,src,n)\
    EM_mem_copy ((dst), (src), (n))

#ifndef MS_DISABLE_MUTEX

/* Macro to define a Mutex Variable */
#define MS_DEFINE_MUTEX(mutex) EM_thread_mutex_type mutex;

/* Macro to define a Mutex Variable with a type qualifier */
#define MS_DEFINE_MUTEX_TYPE(type, mutex) type EM_thread_mutex_type mutex;

/* Macro to define a Conditional Variable */
#define MS_DEFINE_COND(cond) EM_thread_cond_type cond;

/* Macro to define a Conditional Variable with a type qualifier */
#define MS_DEFINE_COND_TYPE(type, cond) type EM_thread_cond_type cond;

/*
    Macro to Initialize Mutex.
    To be used in void function as it returns no error.
*/
#define MS_MUTEX_INIT_VOID(mutex, MODULE)                                \
    if (EM_thread_mutex_init(&(mutex), NULL) < 0)                        \
    {                                                                    \
        EM_debug_error(                                                      \
                                                                             MS_MODULE_ID_##MODULE,                                           \
                                                                             "FAILED to Initialize Mutex in " #MODULE ".\n");                 \
        return;                                                          \
    }

/*
    Macro to Initialize Mutex.
    It returns an error if mutex initialization fails.
*/
#define MS_MUTEX_INIT(mutex, MODULE)                                     \
    if (EM_thread_mutex_init(&(mutex), NULL) < 0)                        \
    {                                                                    \
        EM_debug_error(                                                      \
                                                                             MS_MODULE_ID_##MODULE,                                           \
                                                                             "FAILED to Initialize Mutex in " #MODULE ".\n");                 \
        return MODULE##_MUTEX_INIT_FAILED;                               \
    }

/*
    Macro to Initialize Conditional Variable.
    To be used in void function as it returns no error.
*/
#define MS_COND_INIT_VOID(cond, MODULE)                                  \
    if (EM_thread_cond_init(&(cond), NULL) < 0)                          \
    {                                                                    \
        EM_debug_error(                                                      \
                                                                             MS_MODULE_ID_##MODULE,                                           \
                                                                             "FAILED to Initialize Conditional Variable in " #MODULE ".\n");  \
        return;                                                          \
    }

/*
    Macro to Initialize Conditional Variable.
    It returns an error if conditional variable initialization fails.
*/
#define MS_COND_INIT(cond, MODULE)                                       \
    if (EM_thread_cond_init(&(cond), NULL) < 0)                          \
    {                                                                    \
        EM_debug_error(                                                      \
                                                                             MS_MODULE_ID_##MODULE,                                           \
                                                                             "FAILED to Initialize Conditional Variable in " #MODULE ".\n");  \
        return MODULE##_COND_INIT_FAILED;                                \
    }

/*
    Locks the Module Specific Mutex which prevents any global variable being
    overwritten by any function. It returns an error if mutex lock fails.
*/
#define MS_MUTEX_LOCK(mutex, MODULE)                                 \
    if (EM_thread_mutex_lock(&(mutex)) < 0)                          \
    {                                                                \
        EM_debug_error(                                                  \
                                                                         MS_MODULE_ID_##MODULE,                                       \
                                                                         "FAILED to Lock Mutex in " #MODULE ".\n");                   \
        return MODULE##_MUTEX_LOCK_FAILED;                           \
    }

/*
    Locks the Module Specific Mutex which prevents any global variable being
    overwritten by any function. To be used in void function as it
    returns no error.
*/
#define MS_MUTEX_LOCK_VOID(mutex, MODULE)                            \
    if (EM_thread_mutex_lock(&(mutex)) < 0)                          \
    {                                                                \
        EM_debug_error(                                                  \
                                                                         MS_MODULE_ID_##MODULE,                                       \
                                                                         "FAILED to Lock Mutex in " #MODULE ".\n");                   \
        return;                                                      \
    }

/*
    Locks the Module Specific Mutex which prevents any global variable being
    overwritten by any function.
    It returns the error 'value' if mutex lock failes.
*/
#define MS_MUTEX_LOCK_RETURN_ON_FAILURE(mutex, MODULE, value)        \
    if (EM_thread_mutex_lock(&(mutex)) < 0)                          \
    {                                                                \
        EM_debug_error(                                                  \
                                                                         MS_MODULE_ID_##MODULE,                                       \
                                                                         "FAILED to Lock Mutex in " #MODULE ".\n");                   \
        return (value);                                              \
    }

/*
    Locks the Module Specific Mutex which prevents any global variable being
    overwritten by any function. On failure, only an Error is logged.
    It can be used from both void and non-void functions.
*/
#define MS_MUTEX_LOCK_DONOT_RETURN_ON_FAILURE(mutex, MODULE)         \
    if (EM_thread_mutex_lock(&(mutex)) < 0)                          \
    {                                                                \
        EM_debug_error(                                                  \
                                                                         MS_MODULE_ID_##MODULE,                                       \
                                                                         "FAILED to Lock Mutex in " #MODULE ".\n");                   \
    }

/*
    Unlocks the Module Specific Mutex which realeses the global variables
    to be written into. It returns an error if mutex unlock fails.
*/
#define MS_MUTEX_UNLOCK(mutex, MODULE)                               \
    if (EM_thread_mutex_unlock(&(mutex)) < 0)                        \
    {                                                                \
        EM_debug_error(                                                  \
                                                                         MS_MODULE_ID_##MODULE,                                       \
                                                                         "FAILED to Unlock Mutex in " #MODULE ".\n");                 \
        return MODULE##_MUTEX_UNLOCK_FAILED;                         \
    }

/*
    Unlocks the Module Specific Mutex which realeses the global variables
    to be written into. To be used in void functions as it returns
    no error.
*/
#define MS_MUTEX_UNLOCK_VOID(mutex, MODULE)                          \
    if (EM_thread_mutex_unlock(&(mutex)) < 0)                        \
    {                                                                \
        EM_debug_error(                                                  \
                                                                         MS_MODULE_ID_##MODULE,                                       \
                                                                         "FAILED to Unlock Mutex in " #MODULE ".\n");                 \
        return;                                                      \
    }

/*
    Unlocks the Module Specific Mutex which realeses the global variables
    to be written into.
    It returns the error 'value' if mutex unlock failes.
*/
#define MS_MUTEX_UNLOCK_RETURN_ON_FAILURE(mutex, MODULE, value)      \
    if (EM_thread_mutex_unlock(&(mutex)) < 0)                        \
    {                                                                \
        EM_debug_error(                                                  \
                                                                         MS_MODULE_ID_##MODULE,                                       \
                                                                         "FAILED to Unlock Mutex in " #MODULE ".\n");                 \
        return (value);                                              \
    }

/*
    Unlocks the Module Specific Mutex which realeses the global variables
    to be written into. On failure, only Error is logged.
    It can be used from both void and non-void functions.
*/
#define MS_MUTEX_UNLOCK_DONOT_RETURN_ON_FAILURE(mutex, MODULE)       \
    if (EM_thread_mutex_unlock(&(mutex)) < 0)                        \
    {                                                                \
        EM_debug_error(                                                  \
                                                                         MS_MODULE_ID_##MODULE,                                       \
                                                                         "FAILED to Unlock Mutex in " #MODULE ".\n");                 \
    }

#else  /* MS_DISABLE_MUTEX */

/* Macro to define a Mutex Variable */
#define MS_DEFINE_MUTEX(mutex)

/* Macro to define a Mutex Variable with a type qualifier */
#define MS_DEFINE_MUTEX_TYPE(type, mutex)

/* Macro to define a Conditional Variable */
#define MS_DEFINE_COND(cond)

/* Macro to define a Conditional Variable with a type qualifier */
#define MS_DEFINE_COND_TYPE(type, cond)

/*
    Macro to Initialize Mutex.
    To be used in void function as it returns no error.
*/
#define MS_MUTEX_INIT_VOID(mutex, MODULE)

/*
    Macro to Initialize Mutex.
    It returns an error if mutex initialization fails.
*/
#define MS_MUTEX_INIT(mutex, MODULE)

/*
    Macro to Initialize Conditional Variable.
    To be used in void function as it returns no error.
*/
#define MS_COND_INIT_VOID(cond, MODULE)

/*
    Macro to Initialize Conditional Variable.
    It returns an error if conditional variable initialization fails.
*/
#define MS_COND_INIT(cond, MODULE)

/*
    Locks the Module Specific Mutex which prevents any global variable being
    overwritten by any function. It returns an error if mutex lock fails.
*/
#define MS_MUTEX_LOCK(mutex, MODULE)

/*
    Locks the Module Specific Mutex which prevents any global variable being
    overwritten by any function. To be used in void function as it
    returns no error.
*/
#define MS_MUTEX_LOCK_VOID(mutex, MODULE)

/*
    Locks the Module Specific Mutex which prevents any global variable being
    overwritten by any function.
    It returns the error 'value' if mutex lock failes.
*/
#define MS_MUTEX_LOCK_RETURN_ON_FAILURE(mutex, MODULE, value)

/*
    Locks the Module Specific Mutex which prevents any global variable being
    overwritten by any function. On failure, only an Error is logged.
    It can be used from both void and non-void functions.
*/
#define MS_MUTEX_LOCK_DONOT_RETURN_ON_FAILURE(mutex, MODULE)

/*
    Unlocks the Module Specific Mutex which realeses the global variables
    to be written into. It returns an error if mutex unlock fails.
*/
#define MS_MUTEX_UNLOCK(mutex, MODULE)

/*
    Unlocks the Module Specific Mutex which realeses the global variables
    to be written into. To be used in void functions as it returns
    no error.
*/
#define MS_MUTEX_UNLOCK_VOID(mutex, MODULE)

/*
    Unlocks the Module Specific Mutex which realeses the global variables
    to be written into.
    It returns the error 'value' if mutex unlock failes.
*/
#define MS_MUTEX_UNLOCK_RETURN_ON_FAILURE(mutex, MODULE, value)

/*
    Unlocks the Module Specific Mutex which realeses the global variables
    to be written into. On failure, only Error is logged.
    It can be used from both void and non-void functions.
*/
#define MS_MUTEX_UNLOCK_DONOT_RETURN_ON_FAILURE(mutex, MODULE)

#endif /* MS_DISABLE_MUTEX */

/* Abstractions for bit-wise operation */
#define MS_EXTRACT_BITNUM(val, bitnum)        (((val) >> (bitnum)) & 1)
#define MS_SET_BITNUM(val, bitnum)            ((val) |= (1 << (bitnum)))
#define MS_CLR_BITNUM(val, bitnum)            ((val) &= (~(1 << (bitnum))))

/* Macro to find Minimum and Maximum value */
#define MS_GET_MIN(a, b) \
    (((a) > (b)) ? (b) : (a))

#define MS_GET_MAX(a, b) \
    (((a) > (b)) ? (a) : (b))

/* Unreferenced variable macro to avoid compilation warnings */
#define MS_IGNORE_UNUSED_PARAM(v) (void)(v)

/* Loop for ever */
#define MS_LOOP_FOREVER() for(;;)

#ifdef MS_HAVE_DYNAMIC_CONFIG
#define MS_INIT_CONFIG(config) \
    (config).config_MS_REPLAY_CACHE_SIZE = MS_REPLAY_CACHE_SIZE; \
    (config).config_MS_DEFAULT_COMPANY_ID = MS_DEFAULT_COMPANY_ID; \
    (config).config_MS_DEFAULT_PID = MS_DEFAULT_PID; \
    (config).config_MS_DEFAULT_VID = MS_DEFAULT_VID
#else
#define MS_INIT_CONFIG(config)
#endif /* MS_HAVE_DYNAMIC_CONFIG */

#define MS_CONFIG_LIMITS(x) (x)

#define MS_DEFINE_GLOBAL_ARRAY(type, var, s) \
    type var[(s)]

#define MS_DECLARE_GLOBAL_ARRAY(type, var, s) \
    extern type var[(s)]

#define MS_INIT_GLOBAL_ARRAY(type, var, s, i) \
    EM_mem_set(var, (i), ((s) * sizeof(type)))


#ifdef MS_HAVE_MODEL_OPCODE_EMPTY_HANDLERS
/* Macro to define an empty model Opcode Handler */
#define MODEL_OPCODE_HANDLER_EMPTY_DEF(x) \
    static API_RESULT (x) \
    ( \
      MS_ACCESS_MODEL_HANDLE * handle, \
      MS_NET_ADDR              saddr, \
      MS_NET_ADDR              daddr, \
      MS_SUBNET_HANDLE         subnet_handle, \
      MS_APPKEY_HANDLE         appkey_handle, \
      UINT32                   opcode, \
      UCHAR                  * data_param, \
      UINT16                   data_len \
    ) \
    { \
        API_RESULT retval; \
        \
        MS_IGNORE_UNUSED_PARAM(handle); \
        MS_IGNORE_UNUSED_PARAM(saddr); \
        MS_IGNORE_UNUSED_PARAM(daddr); \
        MS_IGNORE_UNUSED_PARAM(subnet_handle); \
        MS_IGNORE_UNUSED_PARAM(appkey_handle); \
        MS_IGNORE_UNUSED_PARAM(opcode); \
        MS_IGNORE_UNUSED_PARAM(data_param); \
        MS_IGNORE_UNUSED_PARAM(data_len); \
        \
        retval = API_SUCCESS; \
        \
        return retval; \
    }

/* Callback Handler */
#define MODEL_OPCODE_HANDLER_CALL(handler) \
    (handler) (handle, saddr, daddr, subnet_handle, appkey_handle, opcode, data_param, data_len)
#else
/* Macro to define an empty model Opcode Handler */
#define MODEL_OPCODE_HANDLER_EMPTY_DEF(x)

/* Callback Handler */
#define MODEL_OPCODE_HANDLER_CALL(handler)
#endif /* MS_HAVE_MODEL_OPCODE_EMPTY_HANDLERS */

#define MS_STREAM_REV_ENDIANNESS(s, d, n) \
    { \
        UCHAR i; \
        for (i = 0; i < (n); i++) \
        { \
            (d)[(n - 1) - i] = (s)[i]; \
        } \
    }

/** \} */

/**
    \addtogroup ms_common_constants Constants
    \{
*/

/*
    Module Identifier definitions.
    Currently used for runtime debug enable/disable scenario.
    In future, this can be used for other purposes as well,
    hence these defines are placed under common header file.
*/
/* Page 4 - Bluetooth Protocol Modules */
#define MS_MODULE_PAGE_4                      0x40000000

/* Module - Bit Mask */
#define MS_MODULE_BIT_MASK_COMMON             0x00000001
#define MS_MODULE_BIT_MASK_BRR                0x00000002
#define MS_MODULE_BIT_MASK_NET                0x00000004
#define MS_MODULE_BIT_MASK_LTRN               0x00000008
#define MS_MODULE_BIT_MASK_TRN                0x00000010
#define MS_MODULE_BIT_MASK_ACCESS             0x00000020
#define MS_MODULE_BIT_MASK_APP                0x00000040
#define MS_MODULE_BIT_MASK_STBX               0x00000080
#define MS_MODULE_BIT_MASK_CONFIG             0x00000100
#define MS_MODULE_BIT_MASK_FSM                0x00000200
#define MS_MODULE_BIT_MASK_PROV               0x00000400
#define MS_MODULE_BIT_MASK_MESH_MODEL         0x00000800

/* Module ID */
#define MS_MODULE_ID_COMMON                   (MS_MODULE_PAGE_4 | MS_MODULE_BIT_MASK_COMMON)
#define MS_MODULE_ID_BRR                      (MS_MODULE_PAGE_4 | MS_MODULE_BIT_MASK_BRR)
#define MS_MODULE_ID_NET                      (MS_MODULE_PAGE_4 | MS_MODULE_BIT_MASK_NET)
#define MS_MODULE_ID_LTRN                     (MS_MODULE_PAGE_4 | MS_MODULE_BIT_MASK_LTRN)
#define MS_MODULE_ID_TRN                      (MS_MODULE_PAGE_4 | MS_MODULE_BIT_MASK_TRN)
#define MS_MODULE_ID_ACCESS                   (MS_MODULE_PAGE_4 | MS_MODULE_BIT_MASK_ACCESS)
#define MS_MODULE_ID_APP                      (MS_MODULE_PAGE_4 | MS_MODULE_BIT_MASK_APP)
#define MS_MODULE_ID_STBX                     (MS_MODULE_PAGE_4 | MS_MODULE_BIT_MASK_STBX)
#define MS_MODULE_ID_CONFIG                   (MS_MODULE_PAGE_4 | MS_MODULE_BIT_MASK_CONFIG)
#define MS_MODULE_ID_FSM                      (MS_MODULE_PAGE_4 | MS_MODULE_BIT_MASK_FSM)
#define MS_MODULE_ID_PROV                     (MS_MODULE_PAGE_4 | MS_MODULE_BIT_MASK_PROV)
#define MS_MODULE_ID_MESH_MODEL               (MS_MODULE_PAGE_4 | MS_MODULE_BIT_MASK_MESH_MODEL)

/** Device UUID Size */
#define MS_DEVICE_UUID_SIZE    16

/** Beacon Type - Size */
#define MS_BCON_TYPE_SIZE      1

/** Beacon OOB Indicator Size */
#define MS_BCON_OOB_IND_SIZE   2

/** Beacon URI Hash Size */
#define MS_BCON_URI_HASH_SIZE   4

/** Beacon Types */
/** Unprovisioned Device Beacon Type */
#define MS_BCON_TYPE_UNPRVSNG_DEV    0x00

/* Secure Network Beacon Type */
#define MS_BCON_TYPE_SECURE          0x01

/** Friend Role */
/* Invalid */
#define MS_FRND_ROLE_INVALID   0x00

/* Friend */
#define MS_FRND_ROLE_FRIEND    0x01

/* LPN */
#define MS_FRND_ROLE_LPN       0x02

/** Relay Feature */
#define MS_FEATURE_RELAY    0x00
/** Proxy Feature */
#define MS_FEATURE_PROXY    0x01
/** Friend Feature */
#define MS_FEATURE_FRIEND   0x02
/** Low Power Feature */
#define MS_FEATURE_LPN      0x03

/** Secure Nework Beacon */
#define MS_FEATURE_SEC_NET_BEACON 0x04

/** Operation: Enable */
#define MS_ENABLE           0x01
/** Operation: Disable */
#define MS_DISABLE          0x00
/**
    Feature not supported.
    Used as stutus for Get/Set Friend/Proxy etc.,
    when the feature is not supported.
*/
#define MS_NOT_SUPPORTED    0x02

/** Network Tx State */
#define MS_NETWORK_TX_STATE            0x00
/** Relay Tx State */
#define MS_RELAY_TX_STATE              0x01

/** Label UUID Length. Associated with Virtual Address */
#define MS_LABEL_UUID_LENGTH           16

/** \} */

/* -------------------------------------------- Structures/Data Types */

/**
    \addtogroup ms_common_defines Defines
    \{
*/

/**
    \defgroup ms_common_structures Structures
    \{
*/
/** Payload type */
typedef struct _MS_BUFFER
{
    /* Payload Pointer */
    UCHAR* payload;

    /* Payload Length */
    UINT16 length;

} MS_BUFFER;

/**
    Dynamic configuration of Mesh Datastructure.
    Used only if 'MS_HAVE_DYNAMIC_GLOBAL_ARRAY' is defined.
*/
typedef struct _MS_CONFIG
{
    /** The size of the Replay Protection cache. */
    UINT16 config_MS_REPLAY_CACHE_SIZE;

    /** Company ID */
    UINT16 config_MS_DEFAULT_COMPANY_ID;

    /** Product ID */
    UINT16 config_MS_DEFAULT_PID;

    /** Vendor ID */
    UINT16 config_MS_DEFAULT_VID;

} MS_CONFIG;

/** \} */

/** \} */

/* -------------------------------------------- Function/API Declarations */
#ifdef __cplusplus
extern "C" {
#endif

#ifdef MS_HAVE_DYNAMIC_CONFIG
/* Global Configuration for Mesh Stack */
extern MS_CONFIG ms_global_config;
#endif /* MS_HAVE_DYNAMIC_CONFIG */

/**
    \defgroup ms_common_api API Definitions
    \{
*/

/**
    API to initialize Mesh Stack. This is the first API that the
    application should call before any other API. This function
    initializes all the internal stack modules and creates necessary tasks.

    \note
*/

/**
    \brief To initialize Mesh Stack.

    \par Description
    API to initialize Mesh Stack. This is the first API that the
    application should call before any other API. This function
    initializes all the internal stack modules and data structures.

    \param [in] blob
           If 'MS_HAVE_DYNAMIC_CONFIG' defined,
               application shall provide the desired dynamic configuration
               using a pointer to MS_CONFIG data structure instance.
           else,
               this parameter shall be NULL and ignored by the API.

    \return API_SUCCESS or an error code indicating reason for failure
*/
void MS_init
(
    /* IN */ void* blob
);

/**
    API to turn off Bluetooth Hardware. This API should be called after
    \ref MS_init.

    \return
        \ref API_RESULT on successful Bluetooth OFF
*/
API_RESULT MS_shutdown
(
    void
);

/**
    \brief To start transition timer.

    \par Description
    API to start a transition timer.

    \param [in] transition
           State Transition data structure, which includes the timeout,
           transition start and complete callback etc.

    \param [out] transition_time_handle
           Transition Time Handle, which can be used to stop the transition
           timer if required.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_common_start_transition_timer
(
    /* IN */  MS_ACCESS_STATE_TRANSITION_TYPE*    transition,
    /* OUT */ UINT16*                             transition_time_handle
);

/**
    \brief To stop transition timer.

    \par Description
    API to stop a transition timer.

    \param [in] transition_time_handle
           Transition Time Handle, returned by the Start Transition Timer
           interface.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_common_stop_transition_timer
(
    /* IN */ UINT16  transition_time_handle
);

/**
    \brief To get remaining Transition Time.

    \par Description
    API to get remaining Transition Time.

    \param [in] transition_time_handle
           Transition Time Handle, returned by the Start Transition Timer
           interface.

    \param [out] remaining_transition_time
           Remaining Transition Time.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_common_get_remaining_transition_time
(
    /* IN */  UINT16   transition_time_handle,
    /* OUT */ UINT8*   remaining_transition_time
);

/**
    \brief To get remaining Transition Time, with offset.

    \par Description
    API to get remaining Transition Time with offset in ms.

    \param [in] transition_time_handle
           Transition Time Handle, returned by the Start Transition Timer
           interface.

    \param [in] offset_in_ms
           Offset in ms.

    \param [out] remaining_transition_time
           Remaining Transition Time.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_common_get_remaining_transition_time_with_offset
(
    /* IN */  UINT16   transition_time_handle,
    /* IN */  UINT32   offset_in_ms,
    /* OUT */ UINT8*   remaining_transition_time
);

/**
    \brief To convert transition time from milisecond.

    \par Description
    API to convert transition timer in milisecond to Generic Default
    Transition Time state format.

    \param [in] transition_time_in_ms
           Transition Time in milisecond.

    \param [out] transition_time
           Converted value in Generic Default Transition Time state format.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_common_get_transition_time_from_ms
(
    /* IN */  UINT32 transition_time_in_ms,
    /* OUT */ UINT8* transition_time
);

/**
    \cond ignore_this Ignore this function while generating doxygen document
*/

/* Internal Function */
API_RESULT ms_common_init_transition_timer(void);


/* Internal Function */
API_RESULT ms_internal_verificaiton_check(void);
/**
    \endcond
*/

#ifdef __cplusplus
};
#endif

/** \} */

/** \} */

#endif /* _H_MS_COMMON_ */

