
/**
    \file cliface.h

    This file contains definitions for command line interface (CLI) framework.
*/

/*
    Copyright (C) 2017. Mindtree Ltd.
    All rights reserved.
*/

#ifndef _H_CLIFACE_
#define _H_CLIFACE_

/* --------------------------------------------- Header File Inclusion */
#include "EM_os.h"


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
        return EM_FAILURE; \
    }

#define CLI_IS_WHITE_SPACE(ch)   ((' ' == (ch)) || ('\t' == (ch)))
#define CLI_IS_CMD_SEPARATOR(ch) ((' ' == (ch)) || ('\t' == (ch)) || ('\r' == (ch)) || ('\n' == (ch)))

/** TBD: Move to limits/configuration header file */
#define CLI_MAX_ARGS    16

#define CLI_strlen(s)   EM_str_len(s)

/* --------------------------------------------- Data Types/ Structures */
/**
    CLI command handler.

    CLI will call the handler for the received command.

    \param [in] argc    Number of arguments.
    \param [in] argv    List of arguments.
*/
typedef EM_RESULT (* CLI_CMD_HANDLER)
(
    UINT32        argc,
    UCHAR*        argv[]
) DECL_REENTRANT;

/** This data structure represents a CLI command */
typedef struct _cli_command
{
    /** Command name */
    DECL_CONST UCHAR*            cmd;

    /* Command description */
    DECL_CONST UCHAR*            desc;

    /** Command handler */
    DECL_CONST CLI_CMD_HANDLER   cmd_hdlr;

} CLI_COMMAND;

/* --------------------------------------------- Global Definitions */
extern CLI_COMMAND* g_cli_cmd_list;
extern UINT8 g_cli_cmd_len;



/* --------------------------------------------- Functions */
uint16_t CLI_init
(
    CLI_COMMAND* list,
    UINT8   len
);


EM_RESULT CLI_process_line
(
    /* IN */ UCHAR*        buffer,
    /* IN */ UINT32        buffer_len
//    /* IN */ CLI_COMMAND* cmd_list,
//    /* IN */ UINT32        cmd_count
);

INT32 CLI_strtoi
(
    /* IN */ UCHAR* data,
    /* IN */ UINT16 data_length,
    /* IN */ UINT8 base
);

EM_RESULT CLI_strtoarray
(
    /* IN */  UCHAR*   data,
    /* IN */  UINT16   data_length,
    /* OUT */ UINT8*   output_array,
    /* IN */  UINT16   output_array_len
);

EM_RESULT CLI_strtoarray_le
(
    /* IN */  UCHAR*   data,
    /* IN */  UINT16   data_length,
    /* OUT */ UINT8*   output_array,
    /* IN */  UINT16   output_array_len
);

#endif /* _H_CLIFACE_ */


