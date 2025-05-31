#include "u.h"					/* data types */
#include "syscall.h"			/* sys_write */
#include "assert.h"
#include "arena.h"
#include "fmt.h"
#include "builtin.h"

static arena global_arena;

typedef long word;

static const uint64 max_alloc = 1 << 31;

enum { page_size = 4096, wsize = sizeof(word), wmask = wsize - 1 };

void *memcpy(void *dst0, const void *src0, uint64 length)
{
	char *dst = dst0;
	const char *src = src0;
	uint64 t;

	if (length == 0 || dst == src)	/* Nothing to do. */
		return dst;

	if ((uintptr) dst < (uintptr) src) {
		/* Copy forward. */
		t = (uintptr) src;		/* Only need low bits. */
		if ((t | (uintptr) dst) & wmask) {
			/* Try to align operands. This cannot be done
			 * unless the low bits match.
			 */
			if ((t ^ (uintptr) dst) & wmask || length < wsize)
				t = length;
			else
				t = wsize - (t & wmask);

			length -= t;
			do {
				*dst = *src;
				dst++;
				src++;
				t--;
			} while (t);
		}
		/* Copy whole words, then mop up any trailing bytes. */
		t = length / wsize;
		if (t) {
			do {
				*(word *) dst = *(word *) src;
				src += wsize;
				dst += wsize;
				t--;
			} while (t);
		}
		t = length & wmask;
		if (t) {
			do {
				*dst = *src;
				dst++;
				src++;
				t--;
			} while (t);
		}
	} else {
		/*
		 * Copy backwards.  Otherwise essentially the same. 
		 * Alignment works as before, except that it takes
		 * (t&wmask) bytes to align, not wsize-(t&wmask).
		 */
		src += length;
		dst += length;
		t = (uintptr) src;
		if ((t | (uintptr) dst) & wmask) {
			if ((t ^ (uintptr) dst) & wmask || length <= wsize) {
				t = length;
			} else {
				t &= wmask;
			}
			length -= t;
			do {
				dst--;
				src--;
				*dst = *src;
				t--;
			} while (t);
		}
		t = length / wsize;
		if (t) {
			do {
				src -= wsize;
				dst -= wsize;
				*(word *) dst = *(word *) src;
				t--;
			} while (t);
		}
		t = length & wmask;
		if (t) {
			do {
				dst--;
				src--;
				*dst = *src;
				t--;
			} while (t);
		}
	}
	return dst;
}

uint64 c_strlen(const char *s)
{
	int i = 0;

	if (s == 0) {
		return i;
	}

	while (s[i] != '\0')
		i++;

	return i;
}

slice get_slice(string s)
{
	slice ret;

	ret.base = s.base;
	ret.len = ret.cap = s.len;

	return ret;
}

slice unsafe_slice(const void *buf, uint64 len)
{
	slice s;

	s.base = (void *) buf;
	s.len = s.cap = len;

	return s;
}

slice slice_left(slice s, uint64 lbytes)
{
	slice ret;

	ret.base = (byte *) s.base + lbytes;
	ret.len = s.len - lbytes;
	ret.cap = s.cap - lbytes;

	return ret;
}

slice slice_right(slice s, uint64 rbytes)
{
	slice ret;

	assert(rbytes <= s.cap);

	ret.base = s.base;
	ret.len = rbytes;
	ret.cap = s.cap;

	return ret;
}

slice slice_left_right(slice s, uint64 lbytes, uint64 rbytes)
{
	slice ret;

	assert(rbytes <= s.cap);

	ret.base = (char *) s.base + lbytes;
	ret.len = rbytes - lbytes;
	ret.cap = s.cap - lbytes;

	return ret;
}

string get_string(slice s)
{
	string ret;

	ret.base = s.base;
	ret.len = s.len;

	return ret;
}

string unsafe_string(const uchar * buf, uint64 len)
{
	string s;

	s.base = (char *) buf;
	s.len = len;

	return s;
}

string unsafe_c_string(const char *c_str)
{
	return unsafe_string((uchar *) c_str, c_strlen(c_str));
}

string string_left(string s, uint64 lbytes)
{
	string ret;

	ret.base = s.base + lbytes;
	ret.len = s.len - lbytes;

	return ret;
}

string string_right(string s, uint64 rbytes)
{
	string ret;

	ret.base = s.base;
	ret.len = rbytes;

	return ret;
}

string string_left_right(string s, uint64 lbytes, uint64 rbytes)
{
	string ret;

	ret.base = s.base + lbytes;
	ret.len = rbytes - lbytes;

	return ret;
}

uint64 copy(slice dst, slice src)
{
	uint64 l;

	l = dst.len > src.len ? src.len : dst.len;

	memcpy(dst.base, src.base, l);
	return l;
}

uint64 c_string_in_slice(slice s, const char *c_str)
{
	return string_in_slice(s, unsafe_c_string(c_str));
}

uint64 c_nstring_in_slice(slice s, const char *c_str, uint64 len)
{
	return string_in_slice(s, unsafe_string((uchar *) c_str, len));
}

uint64 string_in_slice(slice sl, string s)
{
	uint64 l;

	l = sl.len > s.len ? s.len : sl.len;

	memcpy(sl.base, s.base, l);
	return l;
}

uint64 int_in_slice(slice s, int64 x)
{
	uint64 n_digits = 0, rx = 0, i = 0;
	char *buf = s.base;
	uint64 bound = s.cap;

	if (bound == 0)
		return 0;

	if (x == 0) {
		buf[0] = '0';
		return 1;
	}

	/* sign */
	if (x < 0) {
		x = -x;
		buf[i++] = '-';
	}

	while (x > 0) {
		rx = (10 * rx) + (x % 10);
		x /= 10;
		n_digits++;
	}

	while (n_digits > 0 && bound - i != 0) {
		buf[i++] = (rx % 10) + '0';
		rx /= 10;
		n_digits--;
	}

	return i;
}

void *memmove(void *dst, const void *src, uint64 length)
{
	return memcpy(dst, src, length);
}

int memequal(const void *dst, const void *src, uint64 length)
{
	const char *d = dst, *s = src;

	while (length) {
		if (*d != *s)
			return 0;
		length--;
		d++;
		s++;
	}
	return 1;
}

void panic(const char *msg)
{
	sys_write(stderr, msg, c_strlen(msg), nil);
	/* breakpoint for gdb */
	__asm__ __volatile__("int3");
}

slice make_slice(uint64 type_size, uint64 len, uint64 cap)
{
	uint64 mem = type_size * cap;
	slice ret;

	if (mem > max_alloc) {
		if (type_size * len > max_alloc) {
			panic("builtin.c (make_slice): len out of range\n");
		}
		panic("builtin.c (make_slice): cap out of range\n");
	}

	if (global_arena.buf == nil) {
		arena_create(&global_arena, mem * 2);
	}

	ret.base = arena_alloc(&global_arena, mem);
	if (ret.base == nil) {
		arena new_arena;
		const error *err;

		arena_create(&new_arena, global_arena.buf_len * 2);
		memcpy(new_arena.buf, global_arena.buf, global_arena.buf_len);

		err = sys_munmap((uintptr) global_arena.buf, global_arena.buf_len);
		if (err != nil)
			fmt_fprintf(stderr, "sys_munmap failed (builtin.c): %s\n", err->msg);

		global_arena.buf = new_arena.buf;
		global_arena.buf_len = new_arena.buf_len;
		arena_free_all(&global_arena);
		ret.base = arena_alloc(&global_arena, mem);
	}

	ret.len = len;
	ret.cap = cap;

	return ret;
}

static uint64 new_slice_cap(uint64 new_len, uint64 old_cap)
{
	uint64 new_cap = old_cap;
	uint64 threshold = 256;
	uint64 double_cap = new_cap + new_cap;

	if (new_len > double_cap)
		return new_len;

	if (old_cap < threshold)
		return double_cap;

	while (1) {
		uint64 old_new_cap = new_cap;

		/* Transition from growing 2x for small slices
		 * to growing 1.25x for large slices. This formula
		 * gives a smooth-ish transition between the two.
		 */
		new_cap += (new_cap + 3 * threshold) >> 2;

		/* check overflow */
		if (new_cap < old_new_cap)
			return new_len;

		if (new_cap >= new_len)
			return new_cap;
	}
}

slice grow_slice(slice old_s, uint64 new_len, uint64 type_size)
{
	uint64 new_cap = new_slice_cap(new_len, old_s.cap);
	uintptr new_cap_mem;
	void *p;
	slice ret;

	new_cap_mem = new_cap * type_size;

	if (new_cap_mem > max_alloc)
		panic("builtin.c (grow_slice): len out of range\n");

	p = arena_resize(&global_arena, old_s.base, old_s.cap, new_cap_mem);
	memmove(p, old_s.base, old_s.len * type_size);

	ret.base = p;
	ret.len = new_len;
	ret.cap = new_cap;

	return ret;
}

string sl_to_str_new_base(slice s)
{
	byte *mem;
	string ret;

	mem = arena_alloc(&global_arena, s.len);
	assert(mem != nil);

	ret.base = (char *) mem;
	ret.len = s.len;

	copy(get_slice(ret), s);
	return ret;
}

int c_strncpy(char *dest, const char *src, int64 len)
{
	int i;

	if (!dest || !src || len <= 0)
		return 0;

	for (i = 0; i < len - 1 && src[i] != '\0'; i++)
		dest[i] = src[i];

	dest[i] = '\0';
	return i;
}
