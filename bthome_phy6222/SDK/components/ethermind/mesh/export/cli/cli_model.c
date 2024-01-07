
/**
    \file cli_main.c

    This File contains the "model" handlers for the CLI application,
    to exercise various functionalities of the Mindtree Mesh stack.
*/

/*
    Copyright (C) 2017. Mindtree Ltd.
    All rights reserved.
*/

/* ------------------------------- Header File Inclusion */
#include "cli_main.h"

/* ------------------------------- Global Variables */
/* Level - Model */
DECL_CONST CLI_COMMAND cli_model_cmd_list[] =
{
    /* Help */
    { "help", "Help", cli_help },

    /* Model Server */
    { "models", "Model Server Options", cli_model_server },

    /* Model Client */
    { "modelc", "Model Client Options", cli_model_client },

    /* Back */
    { "back", "One Level Up", cli_back },

    /* Root */
    { "root", "Back to Root", cli_root }

};

/* ------------------------------- Functions */
/* Model */
API_RESULT cli_model(UINT32 argc, UCHAR* argv[])
{
    CONSOLE_OUT("In Model\n");
    cli_cmd_stack_push((CLI_COMMAND*)cli_model_cmd_list, sizeof(cli_model_cmd_list) / sizeof(CLI_COMMAND));
    cli_help(argc, argv);
    return API_SUCCESS;
}

