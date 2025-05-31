#ifndef POOL_H_SENTRY
#define POOL_H_SENTRY 1

#include "u.h"

typedef struct free_node_t {
	struct free_node_t *next;
} free_node;

typedef struct pool_t {
	byte *buf;
	uint64 buf_len;
	uint64 chunk_size;
	free_node *head;
} pool;

void pool_init(pool * p, void *buf, uint64 buf_len, uint64 chunk_size, uint64 chunk_align);
void *pool_get(pool * p);
void pool_put(pool * p, void *ptr);

#endif
