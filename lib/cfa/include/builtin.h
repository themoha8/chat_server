#ifndef BUILTIN_H_SENTRY
#define BUILTIN_H_SENTRY

typedef struct string_t {
	char *base;
	uint64 len;
} string;

typedef struct slice_t {
	void *base;
	uint64 len;
	uint64 cap;
} slice;

/*#define append_str(vs, v) \
	do { \
		uint64 new_len = (vs).len + sizeof(v); \
		if (new_len > (vs).cap) { \
			(vs) = grow_slice((vs), new_len, sizeof(char)); \
		} \
		memcpy((char *)(vs).base + (vs).len*sizeof(v), &(v), sizeof(v)); \
		(vs).len += sizeof(v); \
	} while(0)*/

/*
#define append_str(vs, v) \
	do { \
		int i, num = 1; \
		if ((vs).len + num > (vs).cap) { \
			(vs) = grow_slice((vs), num, sizeof(v)); \
		} \
		for (i = 0; i < num; ++i) { \
			memcpy((char *)(vs).base + (vs).len*sizeof(v), &(v), sizeof(v)); \
			(vs).len += 1; \
		} \
	} while(0)
*/
#define	va_start(ap, last) __builtin_va_start((ap), (last))
#define va_arg(v, l) __builtin_va_arg(v, l)
#define va_end(v) __builtin_va_end(v)

typedef __builtin_va_list __va_list;
typedef __va_list va_list;

// #define new(type) arena_alloc(sizeof(type))
#define make(type, len, cap) make_slice(sizeof(type), len, cap)

uint64 copy(slice dst, slice src);
slice make_slice(uint64 type_size, uint64 len, uint64 cap);
slice grow_slice(slice old_s, uint64 new_len, uint64 type_size);
// string sl_to_str_new_base(slice s);

slice get_slice(string s);
slice unsafe_slice(const void *buf, uint64 len);
slice slice_left(slice s, uint64 l_bytes);
slice slice_right(slice s, uint64 r_bytes);
slice slice_left_right(slice s, uint64 l_bytes, uint64 r_bytes);

string get_string(slice s);
string unsafe_string(const byte * buf, uint64 len);
string unsafe_c_string(const char *c_str);
string string_left(string s, uint64 l_bytes);
string string_right(string s, uint64 r_bytes);
string string_left_right(string s, uint64 l_bytes, uint64 r_bytes);

uint64 c_strlen(const char *s);
int c_strncpy(char *dest, const char *src, int64 len);

uint64 c_string_in_slice(slice s, const char *c_str);
uint64 c_nstring_in_slice(slice s, const char *c_str, uint64 len);
uint64 string_in_slice(slice sl, string s);
uint64 int_in_slice(slice s, int64 x);

void *memcpy(void *dst, const void *src, uint64 length);
void *memmove(void *dst, const void *src, uint64 length);
int memequal(const void *dst, const void *src, uint64 length);

void panic(const char *msg);
#endif
