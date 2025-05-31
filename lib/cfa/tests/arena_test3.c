#include "u.h"
#include "builtin.h"
#include "syscall.h"
#include "fmt.h"

void _start(void)
{
	slice s;

	s = make(char, 50, 50);
	c_string_in_slice(s, "Hello, world\n");
	print_string(stdout, get_string(s));
	s = grow_slice(s, 100, sizeof(char));
	s = make(char, 4096, 4096);
	sys_exit(0);
}
