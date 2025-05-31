#include "u.h"
#include "builtin.h"
#include "fmt.h"
#include "assert.h"
#include "pool.h"

static bool is_power_of_two(uint64 n)
{
	return (n & (n - 1)) == 0;
}

static uintptr align_forward(uintptr ptr, uint64 align)
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

static void pool_free_all(pool * p)
{
	uint64 chunkc = p->buf_len / p->chunk_size;
	uint64 i;

	for (i = 0; i < chunkc; i++) {
		void *ptr = &p->buf[i * p->chunk_size];
		free_node *n = (free_node *) ptr;

		n->next = p->head;
		p->head = n;
	}
}

void pool_init(pool * p, void *buf, uint64 buf_len, uint64 chunk_size, uint64 chunk_align)
{
	/* Align buffer to the specified chunk alignment. */
	uintptr ini_start = (uintptr) buf;
	uintptr start = align_forward((uintptr) buf, chunk_align);
	buf_len = buf_len - (start - ini_start);

	/* Align chunk size up to the required chunk_align. */
	chunk_size = align_forward((uintptr) chunk_size, chunk_align);

	assert(chunk_size >= sizeof(free_node) && "Chunk size is too small.");
	assert(buf_len >= chunk_size && "Buffer length is smaller than the chunk size.");

	/* Store the adjusted parameters. */
	p->buf = (byte *) buf;
	p->buf_len = buf_len;
	p->chunk_size = chunk_size;
	p->head = nil;

	/* Set up the free list for free chunks. */
	pool_free_all(p);
}

void *pool_get(pool * p)
{
	/* Get the latest free node. */
	free_node *n = p->head;

	if (n == nil) {
		fmt_fprintf(stderr, "Pool allocator has no free memory\n");
		return nil;
	}

	/* Pop free node. */
	p->head = p->head->next;

	/* return memset(n, 0, p->chunk_size); */
	return n;
}

void pool_put(pool * p, void *ptr)
{
	free_node *n;
	void *start = p->buf;
	void *end = &p->buf[p->buf_len];

	if (ptr == nil) {
		return;
	}

	if (!(start <= ptr || ptr < end))
		assert(0 && "Memory is out of bounds of the buffer in this pool");

	n = (free_node *) ptr;
	n->next = p->head;
	p->head = n;
}
