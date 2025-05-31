#ifndef ARENA_H_SENTRY
#define ARENA_H_SENTRY

typedef struct arena_t {
	byte *buf;
	uint64 buf_len;
	uint64 prev_off;
	uint64 curr_off;
} arena;

typedef struct temp_arena_t {
	arena *arena;
	uint64 prev_off;
	uint64 curr_off;
} temp_arena;

temp_arena temp_arena_create(arena * a);
void temp_arena_recover(temp_arena * temp);
uintptr align_forward(uintptr ptr, uint64 align);
void *arena_alloc_align(arena * a, uint64 size, uint64 align);
void *arena_alloc(arena * a, uint64 size);
void *arena_resize_align(arena * a, void *old_mem, uint64 old_size, uint64 new_size, uint64 align);
void *arena_resize(arena * a, void *old_mem, uint64 old_size, uint64 new_size);
void arena_init(arena * a, void *buf, uint64 buf_len);
void arena_free_all(arena * a);
void arena_create(arena * a, uint64 size);

#endif
