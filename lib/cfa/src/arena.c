#include "u.h"
#include "builtin.h"
#include "syscall.h"
#include "assert.h"
#include "fmt.h"
#include "arena.h"

enum { page_size = 4096, default_alignment = sizeof(void *) };

temp_arena temp_arena_create(arena * a)
{
	temp_arena t;

	t.arena = a;
	t.prev_off = a->prev_off;
	t.curr_off = a->curr_off;
	return t;
}

void temp_arena_recover(temp_arena * temp)
{
	temp->arena->prev_off = temp->prev_off;
	temp->arena->curr_off = temp->curr_off;
}

bool is_power_of_two(uintptr x)
{
	return (x & (x - 1)) == 0;
}

uintptr align_forward(uintptr ptr, uint64 align)
{
	uintptr p, a, modulo;

	assert(is_power_of_two(align));

	p = ptr;
	a = (uintptr) align;
	/* Same as (p % a) buf faster as 'a' is a power of two. */
	modulo = p & (a - 1);

	if (modulo != 0) {
		/* If 'p' address is not aligned, push address
		 * to the next value which is aligned.
		 */
		p += a - modulo;
	}
	return p;
}

void *arena_alloc_align(arena * a, uint64 size, uint64 align)
{
	/* Align 'curr_off' forward to the specified alignment. */
	uintptr curr_ptr = (uintptr) a->buf + (uintptr) a->curr_off;
	uintptr offset = align_forward(curr_ptr, align);
	offset -= (uintptr) a->buf;	/* Change to relative offset. */

	/* Check to see if the backing memory has space left. */
	if (offset + size <= a->buf_len) {
		a->prev_off = offset;
		a->curr_off = offset + size;
		return &a->buf[offset];
	}

	return nil;
}

void *arena_alloc(arena * a, uint64 size)
{
	return arena_alloc_align(a, size, default_alignment);
}

void *arena_resize_align(arena * a, void *old_mem, uint64 old_size, uint64 new_size, uint64 align)
{
	byte *old_m = (byte *) old_mem;

	assert(is_power_of_two(align));

	if (old_m == nil || old_size == 0)
		return arena_alloc_align(a, new_size, align);

	if (a->buf <= old_m && old_m < a->buf + a->buf_len) {
		if (a->buf + a->prev_off == old_m) {
			a->curr_off = a->prev_off + new_size;
			return old_mem;
		} else {
			void *new_mem = arena_alloc_align(a, new_size, align);
			uint64 copy_size = old_size < new_size ? old_size : new_size;
			memmove(new_mem, old_mem, copy_size);
			return new_mem;
		}
	}

	assert(0 && "memory is out of bounds of the buffer in this arena\n");
	return nil;
}

void *arena_resize(arena * a, void *old_mem, uint64 old_size, uint64 new_size)
{
	return arena_resize_align(a, old_mem, old_size, new_size, default_alignment);
}

void arena_init(arena * a, void *buf, uint64 buf_len)
{
	a->buf = (byte *) buf;
	a->buf_len = buf_len;
	a->curr_off = 0;
	a->prev_off = 0;
}

void arena_free_all(arena * a)
{
	a->curr_off = 0;
	a->prev_off = 0;
}

void arena_create(arena * a, uint64 size)
{
	const error *err;
	char *buf;

	/* Align to page size. */
	size = ((size - 1) / page_size + 1) * page_size;

	buf = sys_mmap((uintptr) nil, size, prot_read | prot_write, map_private | map_anonymous, -1, 0, &err);
	if (err != nil) {
		fmt_fprintf(stderr, "sys_mmap failed (arena_create): %s\n", err->msg);
		sys_exit(1);
	}

	arena_init(a, buf, size);
}
