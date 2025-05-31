#ifndef IOVEC_H
#define IOVEC_H

typedef struct iovec_t {
	void *iov_base;
	uint64 iov_len;
} iovec;

iovec iovec_from_byte_slice(slice s);
iovec iovec_from_c_string(const char *c_str);
iovec iovec_from_string(string s);

#endif
