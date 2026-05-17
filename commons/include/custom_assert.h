#ifndef CUSTOM_ASSERT_H__
#define CUSTOM_ASSERT_H__

#include <stdlib.h>
#include <stdio.h>

//————————————————————————————————————————————————//

/* Acts like an usual assert, but does exit(EXIT_FAILURE) instead of abort()
   Provided for saving info in currently writing files when programm stops. */
#define ASSERT(condition)                                                 \
if (!(condition))                                                         \
{                                                                         \
    fprintf(stderr, "%s:%d: %s: Assertion `%s' failed.\n",                \
                    __FILE__, __LINE__, __PRETTY_FUNCTION__, #condition); \
    exit(EXIT_FAILURE);                                                   \
}                                                                         \

/* Provided for reducing code size of checks in functions */
#define VERIFY(condition, action)                                         \
if (condition)                                                            \
{                                                                         \
    fprintf(stderr, "%-17s:%d:%-50s: Soft assertion `%s' failed.\n",      \
                    __FILE__, __LINE__, __PRETTY_FUNCTION__, #action);    \
    action;                                                               \
}                                                                         \

//————————————————————————————————————————————————//

#endif
