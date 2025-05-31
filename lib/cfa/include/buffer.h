#ifndef BUFFER_H_SENTRY
#define BUFFER_H_SENTRY

typedef struct circular_buffer {
	char *buf;
	uint64 len;
	uint64 head;
	uint64 tail;
} circular_buffer;

circular_buffer new_circular_buffer(uint64 size, const error ** err);
void consume(circular_buffer * cb, uint64 n);
void produce(circular_buffer * cb, uint64 n);
slice remaining_slice(circular_buffer * cb);
uint64 remaining_space(circular_buffer * cb);
void reset(circular_buffer * cb);
uint64 unconsumed_len(circular_buffer * cb);
slice unconsumed_slice(circular_buffer * cb);
string unconsumed_string(circular_buffer * cb);

#endif
