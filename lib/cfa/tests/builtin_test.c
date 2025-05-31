#include "u.h"
#include "builtin.h"
#include "arena.h"
#include "fmt.h"
#include "syscall.h"

void _start(void)
{
	slice s1, s2;
	uint64 len;
	int *buf_s2;

	s1 = make(char, 16, 16);
	s2 = make(int, 8, 8);

	buf_s2 = (int *) s2.base;

	buf_s2[0] = 5;
	buf_s2[1] = 6;

	s2 = grow_slice(s2, 16, 4);

	len = c_string_in_slice(s1, "Hello world!\n");

	s1 = grow_slice(slice_right(s1, len), 64, 1);
	len += c_string_in_slice(slice_left(s1, len), "Hello everyone!\n");

	buf_s2 = (int *) s2.base;
	len += int_in_slice(slice_left(s1, len), buf_s2[0]);
	len += c_string_in_slice(slice_left(s1, len), "\n");

	print_string(stdout, get_string(slice_right(s1, len)));
	sys_exit(0);
}
