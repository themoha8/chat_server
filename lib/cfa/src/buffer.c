#include "u.h"					/* data types */
#include "builtin.h"			/* unsafe_slice */
#include "syscall.h"			/* mmap */
#include "buffer.h"

circular_buffer new_circular_buffer(uint64 size, const error ** err)
{
	const error *e;
	circular_buffer cb;
	char *buf;

	/*assert(size % 4096 == 0); */

	cb.buf = nil;
	cb.len = cb.head = cb.tail = 0;

	buf = sys_mmap((uintptr) nil, 2 * size, prot_read | prot_write, map_anonymous | map_private, -1, 0, &e);

	if (e != nil) {
		*err = e;
		return cb;
	}

	cb.buf = buf;
	cb.len = 2 * size;

	*err = nil;
	return cb;
}

void consume(circular_buffer * cb, uint64 n)
{
	cb->head += n;
	if (cb->head > cb->len / 2) {
		cb->head -= cb->len / 2;
		cb->tail -= cb->len / 2;
	}
}

void produce(circular_buffer * cb, uint64 n)
{
	cb->tail += n;
}

slice remaining_slice(circular_buffer * cb)
{
	return slice_left_right(unsafe_slice(cb->buf, cb->len), cb->tail, cb->head + cb->len / 2);
}

uint64 remaining_space(circular_buffer * cb)
{
	return cb->len / 2 - (cb->tail - cb->head);
}

void reset(circular_buffer * cb)
{
	cb->head = cb->tail = 0;
}

uint64 unconsumed_len(circular_buffer * cb)
{
	return cb->tail - cb->head;
}

slice unconsumed_slice(circular_buffer * cb)
{
	return unsafe_slice((const char *) &cb->buf[cb->head], unconsumed_len(cb));
}

string unconsumed_string(circular_buffer * cb)
{
	return unsafe_string((const byte *) &cb->buf[cb->head], unconsumed_len(cb));
}
