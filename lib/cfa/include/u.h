#ifndef U_H_SENTRY
#define U_H_SENTRY

#define nil (void *)0

enum { true = 0 == 0, false = 0 != 0 };

typedef char int8;
typedef unsigned char uint8;
typedef unsigned char byte;
typedef unsigned char bool;

typedef short int16;
typedef unsigned short uint16;

typedef int int32;
typedef unsigned int uint32;

typedef long int64;
typedef unsigned long uint64;

typedef float float32;
typedef double float64;

typedef unsigned long uintptr;
typedef int32 rune;				/* Code-point values in Unicode are 21 bits wide. */
typedef unsigned char uchar;

#endif
