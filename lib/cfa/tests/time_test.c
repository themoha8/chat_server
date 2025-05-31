#include "u.h"
#include "builtin.h"
#include "time.h"
#include "fmt.h"
#include "syscall.h"

void _start(void)
{
	struct timespec tp;
	struct tm tm_time;
	slice s;
	uint64 len;
	const error *err;

	err = sys_clock_gettime(clock_realtime, &tp);
	if (err != nil) {
		fmt_fprintf(stderr, "sys_clock_gettime: %s\n", err->msg);
		sys_exit(1);
	}

	tm_time = time_to_tm(tp.tv_sec);
	s = make(char, 512, 512);

	len = c_string_in_slice(s, "Unix time: ");
	len += int_in_slice(slice_left(s, len), tp.tv_sec);

	len += c_string_in_slice(slice_left(s, len), "\n\nStruct tm: \n");
	len += c_string_in_slice(slice_left(s, len), "tm_sec: ");
	len += int_in_slice(slice_left(s, len), tm_time.tm_sec);

	len += c_string_in_slice(slice_left(s, len), "\ntm_min: ");
	len += int_in_slice(slice_left(s, len), tm_time.tm_min);

	len += c_string_in_slice(slice_left(s, len), "\ntm_hour: ");
	len += int_in_slice(slice_left(s, len), tm_time.tm_hour);

	len += c_string_in_slice(slice_left(s, len), "\ntm_mday: ");
	len += int_in_slice(slice_left(s, len), tm_time.tm_mday);

	len += c_string_in_slice(slice_left(s, len), "\ntm_mon: ");
	len += int_in_slice(slice_left(s, len), tm_time.tm_mon);

	len += c_string_in_slice(slice_left(s, len), "\ntm_year: ");
	len += int_in_slice(slice_left(s, len), tm_time.tm_year);

	len += c_string_in_slice(slice_left(s, len), "\ntm_wday: ");
	len += int_in_slice(slice_left(s, len), tm_time.tm_wday);

	len += c_string_in_slice(slice_left(s, len), "\ntm_yday: ");
	len += int_in_slice(slice_left(s, len), tm_time.tm_yday);

	len += c_string_in_slice(slice_left(s, len), "\ntm_isdst: ");
	len += int_in_slice(slice_left(s, len), tm_time.tm_isdst);

	len += c_string_in_slice(slice_left(s, len), "\ntm_gmtoff: ");
	len += int_in_slice(slice_left(s, len), tm_time.tm_gmtoff);

	len += c_string_in_slice(slice_left(s, len), "\ntm_zone: ");
	len += c_string_in_slice(slice_left(s, len), tm_time.tm_zone);

	len += c_string_in_slice(slice_left(s, len), "\n\n");

	len += c_string_in_slice(slice_left(s, len), "Time in RF822 format: ");
	len += tm_rfc822_in_slice(slice_left(s, len), &tm_time);
	len += c_string_in_slice(slice_left(s, len), "\n\nTime in tm2 format: ");
	len += tm_in_slice(slice_left(s, len), &tm_time);
	len += c_string_in_slice(slice_left(s, len), "\n");
	print_string(stdout, get_string(slice_right(s, len)));
	sys_exit(0);
}
