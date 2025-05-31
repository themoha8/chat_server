#include "u.h"
#include "arena.h"
#include "syscall.h"
#include "arena.h"
#include "builtin.h"

const char *poem =
	"\nIn the realm of code where logic reigns, \n"
	"A language born from simple chains, \n"
	"With curly braces and semicolons, \n"
	"The art of C, where thought is woven.\n"
	"Variables dance in memory's space, \n"
	"Integers, floats, each finds its place, \n"
	"Arrays align in a structured array, \n"
	"While pointers guide the data's way.\n"
	"Functions call with a rhythmic grace, \n"
	"Parameters passed in a swift embrace, \n"
	"Return values whisper, 'I’m here to stay,' \n"
	"In the heart of the program, they find their way.\n\nAuthor: anon";

void _start(void)
{
	int i;
	char *tmp, *p2, *p;
	char buf[2048];
	arena a;

	arena_init(&a, buf, 2048);

	p = (char *) arena_alloc(&a, 1024);

	tmp = p;
	for (i = 0; poem[i] != '\0'; i++) {
		*tmp = poem[i];
		tmp++;
	}
	p[i] = '\n';
	p[i + 1] = '\n';

	sys_write(1, p, i + 2, nil);

	p2 = (char *) arena_alloc(&a, 64);

	for (i = 0; i < 10; i++) {
		p2[i] = i + '0';
	}
	p2[i] = '\n';

	sys_write(1, p2, i + 1, nil);
	sys_exit(0);
}
