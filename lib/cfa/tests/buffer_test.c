#include "u.h"
#include "builtin.h"
#include "syscall.h"
#include "fmt.h"
#include "buffer.h"

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
	"In the heart of the program, they find their way.\n\nAuthor: anon\n";

void _start(void)
{
	slice s;
	uint64 len, len_poem;
	circular_buffer cb;
	const error *err;

	cb = new_circular_buffer(4096, &err);

	if (err != nil) {
		fmt_fprintf(stderr, "new_circular_buffer: %s\n", err->msg);
		sys_exit(1);
	}

	len_poem = c_strlen(poem);

	memcpy(cb.buf, poem, len_poem);
	produce(&cb, len_poem);

	sys_write(stdout, cb.buf, len_poem, nil);
	consume(&cb, len_poem);

	s = remaining_slice(&cb);
	len = c_string_in_slice(s, "Bla bla bla\n");
	len += c_string_in_slice(slice_left(s, len), "Py py py...\n");
	len += int_in_slice(slice_left(s, len), 158);
	len += c_string_in_slice(slice_left(s, len), "\n");
	produce(&cb, len);

	print_string(stdout, get_string(slice_right(s, len)));
	consume(&cb, len);
	sys_exit(0);
}
