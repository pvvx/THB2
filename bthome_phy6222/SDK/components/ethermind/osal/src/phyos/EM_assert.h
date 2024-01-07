
/**
    \file EM_assert.h

    This file contains the implementation of EM_assert() macro,
    used to evaluate an expression and take action based on whether
    the expression is true or false.

    Version: Windows User Mode
*/

/*
    Copyright (C) 2013. Mindtree Ltd.
    All rights reserved.
*/

#ifndef _H_EM_ASSERT_
#define _H_EM_ASSERT_

/* -------------------------------------------- Header File Inclusion */
/* The EtherMind Debug Library */
#include "EM_debug.h"


/* -------------------------------------------- Macros */
/*
    Macro to assert the validity (truth-value) of an expression.
    This is similar to the standard C macro assert().

    The default behaviour of this macro is to dump the following
    information to the debug system (which could be a file or
    a serial port or some other interface, as abstracted by the
    Debug Library):
        1. The expression under assertion
        2. The source file (using the macro __FILE__)
        3. The source line number (using the macro __LINE__)

    This macro is diagnostic in nature and its default implementation
    does not abort the system as opposed to the standard assert() macro,
    since in an embedded system or even when running in the kernel/driver
    mode, if the system aborts it is very difficult and sometime even
    impossible to find out what caused this catastrophic effect.

    This macro can be modified to suit the target environment and
    also according to the various stages of life-cycle of the system.
*/
#define EM_assert(exp) \
    if (!(exp)) \
    { \
        EM_debug_error(0, \
                       "[ASSERT] %s:%d `" #exp "`\n", __FILE__, __LINE__); \
    }

#endif /* _H_EM_ASSERT_ */

