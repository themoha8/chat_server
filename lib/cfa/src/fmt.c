#include "u.h"
#include "builtin.h"
#include "syscall.h"
#include "fmt.h"

enum { max_buf = 2048 };

void print_string(int stream, string s)
{
	sys_write(stream, (const char *) s.base, s.len, nil);
}

static int print_hex(slice s, uintptr num)
{
	uint64 rx = 0;
	int i = 0, n_digits = 0, bound = s.cap;
	char *buf = s.base;
	const char *hex_digits = "0123456789abcdef";

	if (bound == 0)
		return 0;


	buf[i++] = '0';

	if (bound - i == 0)
		return 1;

	buf[i++] = 'x';

	while (num > 0) {
		rx = (16 * rx) + (num % 16);
		num /= 16;
		n_digits++;
	}

	while (n_digits > 0 && bound - i != 0) {
		buf[i++] = hex_digits[(rx % 16)];
		rx /= 16;
		n_digits--;
	}

	return i;
}

int fmt_fprintf(int stream, const char *fmt, ...)
{
	va_list args;
	int num, j = 0;
	char buf[max_buf];
	char *s;
	uintptr p;

	va_start(args, fmt);

	while (*fmt != '\0' && max_buf > j) {
		if (*fmt == '%') {
			fmt++;
			switch (*fmt) {
			case 'c':
				buf[j++] = va_arg(args, int);
				break;
			case 's':
				s = va_arg(args, char *);
				j += c_strncpy(buf + j, s, max_buf - j);
				break;
			case 'd':
				num = va_arg(args, int);
				j += int_in_slice(unsafe_slice(buf + j, max_buf - j), num);
				break;
			case 'p':
				p = va_arg(args, uintptr);
				j += print_hex(unsafe_slice(buf + j, max_buf - j), p);
				break;
			case '%':
				buf[j++] = *fmt;
				break;
			}
		} else {
			buf[j++] = *fmt;
		}
		fmt++;
	}

	va_end(args);
	sys_write(stream, buf, j, nil);
	return j;
}
