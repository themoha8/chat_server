#ifndef ASSERT_H_SENTRY
#define ASSERT_H_SENTRY

#include "u.h"

void assert_fail(const char *expr, const char *file, uint64 line);

#ifndef NDEBUG
#define assert(expr) \
    do { \
        if (!(expr)) { \
            assert_fail(#expr, __FILE__, __LINE__); \
        } \
    } while (0)
#else
#define assert(expr) nil
#endif

#endif
