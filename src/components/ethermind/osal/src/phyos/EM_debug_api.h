
/**
    \file EM_debug_api.h

    This Header File contains the APIs and the ADTs exported by the
    EtherMind Debug Library for Windows (User-mode).
*/

/*
    Copyright (C) 2013. Mindtree Ltd.
    All rights reserved.
*/

#ifndef _H_EM_DEBUG_API_
#define _H_EM_DEBUG_API_

/* ----------------------------------------------- Header File Inclusion */
#include "EM_os.h"

#define EM_DEBUG_DONT_LOG_FILE_PATH
#define EM_ENABLE_DISABLE_RUNTIME_DEBUG

/* ----------------------------------------------- Debug Macros */
/* Define Debug Message Types */
/* Message Type - ERR */
#define EM_DEBUG_MSG_ERR              1

/* Message Type - TRC */
#define EM_DEBUG_MSG_TRC              2

/* Message Type - INF */
#define EM_DEBUG_MSG_INF              3

/* Define Debug Levels */
/* No runtime error logging */
#define EM_DEBUG_LEVEL_NONE           0

/* Log only ERR messages */
#define EM_DEBUG_LEVEL_ERR            1

/* Log ERR and TRC messages */
#define EM_DEBUG_LEVEL_TRC            2

/* Log ERR, TRC and INF messages */
#define EM_DEBUG_LEVEL_INF            3
#define EM_DEBUG_LEVEL_ALL            3

/* Maximum number of module pages */
#define EM_MODULE_PAGE_BITS_COUNT     4
#define EM_MAX_MODULE_PAGE_COUNT      (1 << EM_MODULE_PAGE_BITS_COUNT)

/* Debug Enable/Disable flag */
#define EM_DEBUG_ENABLE               0x01
#define EM_DEBUG_DISABLE              0x00

/**
    Special wildcard define to represent all modules.
    Used to enable/disable all module at once.
*/
#define EM_MODULE_ALL                 0xFFFFFFFF

#define EM_GET_PAGE_IDX_MODULE_BIT_MASK(module_id, page_idx, m_bit_mask) \
    { \
        /* Extract the Page Number and Module Bit Mask */ \
        (page_idx) = ((module_id) >> (32 - EM_MODULE_PAGE_BITS_COUNT)); \
        (m_bit_mask) = (((page_idx) << (32 - EM_MODULE_PAGE_BITS_COUNT)) ^ (module_id)); \
    }

#define EM_GET_MODULE_ID(module_id, page_idx, m_bit_mask) \
    { \
        /* Create Module Id from the Page Number and Module Bit Mask */ \
        (module_id) = (((page_idx) << (32 - EM_MODULE_PAGE_BITS_COUNT)) | (m_bit_mask)); \
    }


#ifdef EM_ENABLE_DISABLE_RUNTIME_DEBUG
#define EM_enable_module_debug_flag(module_id) \
    EM_update_module_debug_flag((module_id), EM_DEBUG_ENABLE)

#define EM_disable_module_debug_flag(module_id) \
    EM_update_module_debug_flag((module_id), EM_DEBUG_DISABLE)
#endif /* EM_ENABLE_DISABLE_RUNTIME_DEBUG */

#ifdef EM_DEBUG_DONT_LOG_FILE_PATH
#define EM_FILE_NAME     \
    (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#else
#define EM_FILE_NAME     __FILE__
#endif /* EM_DEBUG_DONT_LOG_FILE_PATH */

/* Debug print macros, based on the debug message types */
#define EM_debug_printf_err(module_id, ...) EM_debug_printf(EM_DEBUG_MSG_ERR, (module_id), __VA_ARGS__)
#define EM_debug_printf_trc(module_id, ...) EM_debug_printf(EM_DEBUG_MSG_TRC, (module_id), __VA_ARGS__)
#define EM_debug_printf_inf(module_id, ...) EM_debug_printf(EM_DEBUG_MSG_INF, (module_id), __VA_ARGS__)

#ifdef EM_LOG_TIMESTAMP
#define EM_debug_error(module_id, ...) \
    EM_debug_printf_err((module_id), "\n[** ERR **]:[%s]:[%s]:[%d]: [<%s>] ", \
                        EM_debug_get_current_timestamp(), EM_FILE_NAME, __LINE__); \
    EM_debug_printf_err((module_id), __VA_ARGS__)

#define EM_debug_trace(module_id, ...) \
    EM_debug_printf_trc((module_id), "\n[-- TRC --]:[%s]:[%s]:[%d]: [<%s>] ", \
                        EM_debug_get_current_timestamp(), EM_FILE_NAME, __LINE__); \
    EM_debug_printf_trc((module_id), __VA_ARGS__)

#define EM_debug_info(module_id, ...) \
    EM_debug_printf_inf((module_id), "\n[== INF ==]:[%s]:[%s]:[%d]: [<%s>] ", \
                        EM_debug_get_current_timestamp(), EM_FILE_NAME, __LINE__); \
    EM_debug_printf_inf((module_id), __VA_ARGS__)

#define EM_debug_dump(module_id, ...) \
    EM_debug_printf_inf((module_id), "\n[++ HEX ++]:[%s]:[%s]:[%d]: [<%s>] ", \
                        EM_debug_get_current_timestamp(), EM_FILE_NAME, __LINE__); \
    EM_debug_printf_inf((module_id), __VA_ARGS__)

#else /* EM_LOG_TIMESTAMP */
#define EM_debug_error(module_id, ...) \
    EM_debug_printf_err((module_id), "\n[** ERR **]:[%s]:[%d]: ", \
                        EM_FILE_NAME, __LINE__); \
    EM_debug_printf_err((module_id), __VA_ARGS__)

#define EM_debug_trace(module_id, ...) \
    EM_debug_printf_trc((module_id), "\n[-- TRC --]:[%s]:[%d]: ", \
                        EM_FILE_NAME, __LINE__); \
    EM_debug_printf_trc((module_id), __VA_ARGS__)

#define EM_debug_info(module_id, ...) \
    EM_debug_printf_inf((module_id), "\n[== INF ==]:[%s]:[%d]: ", \
                        EM_FILE_NAME, __LINE__); \
    EM_debug_printf_inf((module_id), __VA_ARGS__)

#define EM_debug_dump(module_id, ...) \
    EM_debug_printf_inf((module_id), "\n[++ HEX ++]:[%s]:[%d]: ", \
                        EM_FILE_NAME, __LINE__); \
    EM_debug_printf_inf((module_id), __VA_ARGS__)

#endif  /* EM_LOG_TIMESTAMP */

/* TBD: Check where is this being used */
#define EM_debug_direct(module_id, ...) \
    EM_debug_printf_inf((module_id), "\n[~~ LOG ~~]: "); \
    EM_debug_printf_inf((module_id), __VA_ARGS__)


/* ----------------------------------------------- Global Definitions */
#define EM_debug_null(...)

/* ----------------------------------------------- Structures/Data Types */


/* ----------------------------------------------- Function Declarations */
#ifdef __cplusplus
extern "C" {
#endif

/* Debug Library Init & Shutdown Routines */
void EM_debug_init ( void );
void EM_debug_shutdown ( void );

#ifndef EM_DISABLE_ALL_DEBUG
INT32 EM_debug_printf(UCHAR msg_type, UINT32 module_id, const CHAR* fmt, ...);
INT32 EM_debug_dump_bytes(UINT32 module_id, UCHAR* buffer, UINT32 length);
INT32 EM_debug_dump_decimal(UINT32 module_id, UCHAR* buffer, UINT32 length);
#else
#define EM_debug_printf(...)
#define EM_debug_dump_bytes(module_id, buffer, length)
#define EM_debug_dump_decimal(module_id, buffer, length )
#endif /* EM_DISABLE_ALL_DEBUG */

#ifdef EM_LOG_TIMESTAMP
UCHAR* EM_debug_get_current_timestamp (void);
#endif /* EM_LOG_TIMESTAMP */

#ifdef EM_ENABLE_DISABLE_RUNTIME_DEBUG
void EM_set_debug_level(UCHAR level);

void EM_update_module_debug_flag(UINT32 module_id, UCHAR flag);
#endif /* EM_ENABLE_DISABLE_RUNTIME_DEBUG */

#ifdef __cplusplus
};
#endif

#endif /* _H_EM_DEBUG_API_ */

