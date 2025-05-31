#include "u.h"
#include "builtin.h"
#include "syscall.h"
#include "fmt.h"
#include "arena.h"

void _start(void)
{
	int i;

	char backing_buffer[256];
	arena a;
	arena_init(&a, backing_buffer, 256);

	for (i = 0; i < 5; i++) {
		int *x, *f;
		char *str;

		/*arena_free_all(&a); */
		x = (int *) arena_alloc(&a, sizeof(int));
		f = (int *) arena_alloc(&a, sizeof(int));
		str = arena_alloc(&a, 10);

		*x = 123;
		*f = 987;
		memmove(str, "Hellope", 8);

		fmt_fprintf(stdout, "%p: %d\n", x, *x);
		fmt_fprintf(stdout, "%p: %d\n", f, *f);
		fmt_fprintf(stdout, "%p: %s\n", str, str);
		str = arena_resize(&a, str, 10, 16);
		memmove(str + 7, " world!", 8);
		fmt_fprintf(stdout, "%p: %s\n", str, str);
	}

	arena_free_all(&a);

	sys_exit(0);
}
