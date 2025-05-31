#include "u.h"					/* data types */
#include "builtin.h"			/* unsafe_c_string */
#include "iovec.h"

iovec iovec_from_byte_slice(slice s)
{
	iovec iov;

	iov.iov_base = s.base;
	iov.iov_len = s.len;

	return iov;
}

iovec iovec_from_c_string(const char *c_str)
{
	return iovec_from_string(unsafe_c_string(c_str));
}

iovec iovec_from_string(string s)
{
	iovec iov;

	iov.iov_base = s.base;
	iov.iov_len = s.len;

	return iov;
}
