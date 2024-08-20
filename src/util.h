#pragma once

#include <stdio.h>
#include <stdlib.h>

/**
 * Macro to mark locations that require an implementation, ends the execution.
 */
#define NOT_IMPLEMENTED                                                        \
    {                                                                          \
        fprintf(stderr, "Not yet implemented: %s in %s (line %d)\n", __func__, \
                __FILE__, __LINE__);                                           \
        abort();                                                               \
    }

/**
 * Macro to suppress compiler warnings for unused variables, ends the execution.
 */
#define UNUSED(x)                                                      \
    {                                                                  \
        (void)x;                                                       \
        fprintf(stderr, "Executed UNUSED in %s (line %d)\n", __FILE__, \
                __LINE__);                                             \
        abort();                                                       \
    }
