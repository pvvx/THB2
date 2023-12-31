
/**
    \file EM_debug.c

    This file contains source codes for the EtherMind Debug Library
    Implementation for Windows (User-mode).
*/

/*
    Copyright (C) 2013. Mindtree Ltd.
    All rights reserved.
*/

/* --------------------------------------------------- Header File Inclusion */
#include "EM_debug_internal.h"

/* --------------------------------------------------- Global Definitions */

/* ------------------------------------------------- Static Global Variables */
#ifdef EM_ENABLE_DISABLE_RUNTIME_DEBUG
    /* Runtime debug level */
    DECL_STATIC UCHAR em_runtime_debug_level;

    /* Module Specific debug enabled/disabled flag */
    DECL_STATIC UINT32 em_runtime_debug_flag[EM_MAX_MODULE_PAGE_COUNT];
#endif /* EM_ENABLE_DISABLE_RUNTIME_DEBUG */

/* Hack. TBD */
#define EM_MODULE_ID_DEBUG                  0
/* ------------------------------------------------- Functions */

void EM_debug_init (void)
{
    #ifdef EM_ENABLE_DISABLE_RUNTIME_DEBUG
    UINT32 enable_all_bit_mask, page_index;
    #endif /* EM_ENABLE_DISABLE_RUNTIME_DEBUG */
    #ifdef EM_ENABLE_DISABLE_RUNTIME_DEBUG
    /* Initialize runtime debug level */
    em_runtime_debug_level = EM_DEBUG_LEVEL_ALL;
    #ifndef EM_DISABLE_DEBUG_LOG_ON_STARTUP
    /* Enable all module debug log - by default */
    enable_all_bit_mask = 0xFFFFFFFF;
    #else
    enable_all_bit_mask = 0x00000000;
    #endif /* EM_DISABLE_DEBUG_LOG_ON_STARTUP */

    for (page_index = 0; page_index < EM_MAX_MODULE_PAGE_COUNT; page_index++)
    {
        em_runtime_debug_flag[page_index] = enable_all_bit_mask;
    }

    #endif /* EM_ENABLE_DISABLE_RUNTIME_DEBUG */
    return;
}

void EM_debug_shutdown (void)
{
    return;
}

#ifndef EM_DISABLE_ALL_DEBUG
INT32 EM_debug_printf(UCHAR msg_type, UINT32 module_id, const CHAR* fmt, ...)
{
    #ifdef EM_ENABLE_DISABLE_RUNTIME_DEBUG
    UINT32 page_index;
    UINT32 module_bit_mask;
    #endif /* EM_ENABLE_DISABLE_RUNTIME_DEBUG */
    va_list parg;
//    return 0;
    #ifdef EM_ENABLE_DISABLE_RUNTIME_DEBUG

    /* Check Message Level */
    if (msg_type > em_runtime_debug_level)
    {
        return -1;
    }

    /* Extract Page Index and Module Bit Mask */
    EM_GET_PAGE_IDX_MODULE_BIT_MASK(module_id, page_index, module_bit_mask);

    /* Check if the module debug log is enabled */
    if ((em_runtime_debug_flag[page_index] & module_bit_mask) != module_bit_mask)
    {
        return -1;
    }

    #endif /* EM_ENABLE_DISABLE_RUNTIME_DEBUG */
    va_start (parg, fmt);
    vprintf (fmt, parg);
    va_end (parg);
    return 0;
}

INT32 EM_debug_dump_bytes(UINT32 module_id, UCHAR* buffer, UINT32 length)
{
    char hex_stream[49];
    char char_stream[17];
    UINT32 i;
    UINT16 offset, count;
    UCHAR c;
    EM_debug_dump(module_id, "-- Dumping %d Bytes --\n",
                  (int)length);
    EM_debug_dump(module_id,
                  "-------------------------------------------------------------------\n");
    count = 0;
    offset = 0;

    for(i = 0; i < length; i ++)
    {
        c =  buffer[i];
        sprintf(hex_stream + offset, "%02X ", c);

        if ( (c >= 0x20) && (c <= 0x7E) )
        {
            char_stream[count] = c;
        }
        else
        {
            char_stream[count] = '.';
        }

        count ++;
        offset += 3;

        if(16 == count)
        {
            char_stream[count] = '\0';
            count = 0;
            offset = 0;
            EM_debug_dump(module_id, "%s   %s\n",
                          hex_stream, char_stream);
            EM_mem_set(hex_stream, 0, 49);
            EM_mem_set(char_stream, 0, 17);
        }
    }

    if(0 != offset)
    {
        char_stream[count] = '\0';
        /* Maintain the alignment */
        EM_debug_dump(module_id, "%-48s   %s\n",
                      hex_stream, char_stream);
    }

    EM_debug_dump(module_id,
                  "-------------------------------------------------------------------\n");
    return 0;
}


INT32 EM_debug_dump_decimal(UINT32 module_id, UCHAR* buffer, UINT32 length)
{
    char stream[100];
    UINT32 i;
    UINT16 offset, count;
    EM_debug_dump(module_id, "Dumping %d Bytes (In Decimal): ------>\n",
                  (int)length);
    count = 0;
    offset = 0;

    for(i = 0; i < length; i ++)
    {
        sprintf(stream + offset, "%3d ", (unsigned int)buffer[i]);
        count ++;
        offset += 4;

        if(16 == count)
        {
            count = 0;
            offset = 0;
            EM_debug_dump(module_id, "%s\n", stream);
            EM_mem_set(stream, 0, 100);
        }
    }

    if(0 != offset)
    {
        EM_debug_dump(module_id, "%s\n", stream);
    }

    EM_debug_dump(module_id, "<------------------------------------>\n");
    return 0;
}
#endif /* EM_DISABLE_ALL_DEBUG */

#ifdef EM_LOG_TIMESTAMP
UCHAR* EM_debug_get_current_timestamp (void)
{
    #ifndef FREERTOS
    static UCHAR time_str[20];
    struct tm* dt;
    time_t now = time (0);
    dt = localtime (&now);
    /* Use gmtime for UTC */
    /* dt = gmtime (&now); */
    strftime (time_str, sizeof(time_str), "%H:%M:%S", dt);
    /* Use following piece of code to print date along with time */
    /* strftime (time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", dt); */
    return time_str;
    #else
    return NULL;
    #endif /* FREERTOS */
}
#endif /* BT_LOG_TIMESTAMP */


#ifdef EM_ENABLE_DISABLE_RUNTIME_DEBUG
void EM_set_debug_level(UCHAR level)
{
    /* Parameter validation */
    if (EM_DEBUG_LEVEL_ALL < level)
    {
        /* Failure */
        return;
    }

    /* Set runtime debug level */
    em_runtime_debug_level = level;
    return;
}

void EM_update_module_debug_flag(UINT32 module_id, UCHAR flag)
{
    UINT32 page_index;
    UINT32 module_bit_mask;

    /* Parameter Validation */
    if (EM_DEBUG_ENABLE < flag)
    {
        return;
    }

    /* Check if the request is for enable/disable all */
    if (EM_MODULE_ALL == module_id)
    {
        if (EM_DEBUG_ENABLE == flag)
        {
            module_bit_mask = 0xFFFFFFFF;
        }
        else
        {
            module_bit_mask = 0x00000000;
        }

        for (page_index = 0; page_index < EM_MAX_MODULE_PAGE_COUNT; page_index++)
        {
            em_runtime_debug_flag[page_index] = module_bit_mask;
        }

        return;
    }

    /* Extract Page Index and Module Bit Mask */
    EM_GET_PAGE_IDX_MODULE_BIT_MASK(module_id, page_index, module_bit_mask);

    /* Enable/disable based on the flag */
    if (EM_DEBUG_ENABLE == flag)
    {
        em_runtime_debug_flag[page_index] |= (module_bit_mask);
    }
    else
    {
        em_runtime_debug_flag[page_index] &= ~(module_bit_mask);
    }
}
#endif /* EM_ENABLE_DISABLE_RUNTIME_DEBUG */


