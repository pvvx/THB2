
/**
 *  \file cliface.h
 *
 *  This file contains definitions for command line interface (CLI) framework.
 */

/*
 *  Copyright (C) 2017. Mindtree Ltd.
 *  All rights reserved.
 */

#ifndef _H_CLIFACE_
#define _H_CLIFACE_

/* --------------------------------------------- Header File Inclusion */
#include "mcu.h"
#include <string.h>
/* --------------------------------------------- Global Definitions */


/* --------------------------------------------- Macros */

/* Debug Macros */
/* TBD: Mapped with debug sub-system */
#define CLI_ERR(...)       /* printf(__VA_ARGS__) */
#define CLI_TRC(...)       /* printf(__VA_ARGS__) */
#define CLI_INF(...)       /* printf(__VA_ARGS__) */

#define CLI_STR_COMPARE(s0, s1)    strcmp((const char *)(s0), (const char *)(s1))

#define CLI_NULL_CHECK(ptr) \
        if (NULL == (ptr)) \
        {\
            CLI_ERR( \
            "[CLI] NULL Pointer\n"); \
            \
            return 0xffff; \
        }

#define CLI_IS_WHITE_SPACE(ch)   ((' ' == (ch)) || ('\t' == (ch)))
#define CLI_IS_CMD_SEPARATOR(ch) ((' ' == (ch)) || ('\t' == (ch)) || ('\r' == (ch)) || ('\n' == (ch)))

/** TBD: Move to limits/configuration header file */
#define CLI_MAX_ARGS    16

#define CLI_strlen(s)   strlen((const char*)s)

/* --------------------------------------------- Data Types/ Structures */
/**
 * CLI command handler.
 *
 * CLI will call the handler for the received command.
 *
 * \param [in] argc    Number of arguments.
 * \param [in] argv    List of arguments.
 */
typedef uint16_t (* CLI_CMD_HANDLER)
        (
            uint32_t        argc,
            uint8_t       * argv[]
        ) ;

/** This data structure represents a CLI command */
typedef struct _cli_command
{
    /** Command name */
    const uint8_t           * cmd;

    /* Command description */
    const uint8_t           * desc;

    /** Command handler */
    const CLI_CMD_HANDLER   cmd_hdlr;

} CLI_COMMAND;


/* --------------------------------------------- Functions */
uint16_t CLI_init
           (
                void
           );

uint16_t CLI_process_line
           (
               /* IN */ uint8_t       * buffer,
               /* IN */ uint32_t        buffer_len,
               /* IN */ CLI_COMMAND * cmd_list,
               /* IN */ uint32_t        cmd_count
           );

int32_t CLI_strtoi
      (
          /* IN */ uint8_t *data,
          /* IN */ uint16_t data_length,
          /* IN */ uint8_t base
      );

uint16_t CLI_strtoarray
          (
              /* IN */  uint8_t  * data,
              /* IN */  uint16_t   data_length,
              /* OUT */ uint8_t  * output_array,
              /* IN */  uint16_t   output_array_len
          );

uint16_t CLI_strtoarray_le
          (
              /* IN */  uint8_t  * data,
              /* IN */  uint16_t   data_length,
              /* OUT */ uint8_t  * output_array,
              /* IN */  uint16_t   output_array_len
          );

#endif /* _H_CLIFACE_ */


