#include "u.h"					/* data types */
#include "syscall.h"			/* stderr */
#include "fmt.h"				/* fmt_fprintf */

void assert_fail(const char *expr, const char *file, uint64 line)
{
	fmt_fprintf(stderr, "%s:%ld: assertion '%s' is failed\n", file, line, expr);
	/* breakpoint for gdb */
	__asm__ __volatile__("int3");
}
