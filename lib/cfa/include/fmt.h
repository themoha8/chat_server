#ifndef FMT_H_SENRY
#define FMT_H_SENRY

#include "builtin.h"
#include "syscall.h"

void print_string(int stream, string s);
int fmt_fprintf(int stream, const char *fmt, ...);

#endif
