#include "u.h"					/* data types */
#include "builtin.h"			/* c_string_in_slice */
#include "time.h"

static const int days_since_jan_1st[][13] = {
	{0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365},	/* non-leap year. */
	{0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366},	/* leap year. */
};

/* t argc represents the number of seconds that have passed since 1970. */
struct tm time_to_tm(int64 t)
{
	int64 quadricentennials, centennials, quadrennials, annuals;
	int64 yday, mday, wday;
	int64 year, month, leap;
	int64 hour, min, sec;
	struct tm tm_time;

	t += 3 * 60 * 60;			/* MSK timezone. */

	/* Convert era from 1970 (unix) to 1601 (windows)
	 * 1601 + 369 = 1970 (369 = 3 * 100 + 17 * 4 + 1 year)
	 * (incl. 89 leap days (1600-1700 - 24, 1700-1800 - 24, 1800-1900 - 24,
	 * 1900-1970 - 17 lead days)) = (228 * 365 (non-leap years) + 72 * 366 (leap years)) +
	 * + 53 * 365 (non-leap years) + 17 * 366 (leap years) = 135 139 days
	 * 135 139 days * 60 secs * 60 mins * 24 hours
	 */
	sec = t + 11644473600;

	/* Since January 1, 1601 is Monday, then to get the correct result
	 * we need to take into account that we are counting not from Sunday,
	 * but from Monday.
	 */
	wday = (sec / 86400 + 1) % 7;	/* day of week. */

	/* Remove multiples of 400 years (incl. 97 leap days).
	 * (365 * 303 + 366 * 97) * 60 * 60 * 24 = 146097 days * 60 secs * 60 mins * 24 hours
	 */
	quadricentennials = sec / 12622780800;
	sec %= 12622780800;

	/* Remove multiples of 100 years (incl. 24 leap days), can't be more than 3
	 * (because multiples of 4*100=400 years (incl. leap days) have been removed).
	 *  (365 * 76 + 366 * 24) * 60 * 60 * 24 = 36524 days * 60 secs * 60 mins * 24 hours
	 */
	centennials = sec / 3155673600;
	if (centennials > 3) {
		centennials = 3;
	}
	sec -= centennials * 3155673600;

	/* Remove multiples of 4 years (incl. 1 leap day), can't be more than 24 
	 * (because multiples of 25*4=100 years (incl. leap days) have been removed).
	 * (365 * 3 + 366) * 60 * 60 * 24 = 1461 days * 60 secs * 60 mins * 24 hours
	 */
	quadrennials = sec / 126230400;
	if (quadrennials > 24) {
		quadrennials = 24;
	}
	sec -= quadrennials * 126230400;

	/* Remove multiples of years (incl. 0 leap days), can't be more than 3
	 * (because multiples of 4 years (incl. leap days) have been removed).
	 * 365 days * 60 secs * 60 mins * 24 hours
	 */
	annuals = sec / 31536000;
	if (annuals > 3) {
		annuals = 3;
	}
	sec -= annuals * 31536000;

	/* Calculate the year and find out if it's leap. */
	year = 1601 + quadricentennials * 400 + centennials * 100 + quadrennials * 4 + annuals;
	leap = (year % 4 == 0) && ((year % 100 != 0) || (year % 400 == 0));

	/* Calculate the day of the year and the time. */
	yday = sec / 86400;
	sec %= 86400;
	hour = sec / 3600;
	sec %= 3600;
	min = sec / 60;
	sec %= 60;

	/* Calculate the month. */
	for (month = 1, mday = 1; month <= 12; month++) {
		if (yday < days_since_jan_1st[leap][month]) {
			mday += yday - days_since_jan_1st[leap][month - 1];
			break;
		}
	}

	tm_time.tm_sec = sec;		/*  [0,59]. */
	tm_time.tm_min = min;		/*  [0,59]. */
	tm_time.tm_hour = hour;		/*  [0,23]. */
	tm_time.tm_mday = mday;		/*  [1,31]  (day of month). */
	tm_time.tm_mon = month - 1;	/*  [0,11]  (month). */
	tm_time.tm_year = year - 1900;	/*  70+     (year since 1900). */
	tm_time.tm_wday = wday;		/*  [0,6]   (day since Sunday AKA day of week). */
	tm_time.tm_yday = yday;		/*  [0,365] (day since January 1st). */
	tm_time.tm_isdst = -1;		/*  daylight saving time flag. 
								 * -1 - unknown, 0 - not set 1 - set
								 */
	tm_time.tm_gmtoff = 3 * 60 * 60;	/* MSK timezone. */
	tm_time.tm_zone = nil;

	return tm_time;
}

/*
struct tm time_to_tm2(int64 t)
{
	int64 quadricentennials, centennials, quadrennials, annuals;
	int64 yday, mday, wday;
	int64 year, month, leap;
	int64 hour, min, sec;
	struct tm tm_time;

	t += 3 * 60 * 60;

	sec = t;

	wday = (sec / 86400 + 4) % 7;

	quadricentennials = sec / 12622780800;
	sec %= 12622780800;

	centennials = sec / 3155673600;
	if (centennials > 3) {
		centennials = 3;
	}
	sec -= centennials * 3155673600;

	quadrennials = sec / 126230400;
	if (quadrennials > 24) {
		quadrennials = 24;
	}
	sec -= quadrennials * 126230400;

	annuals = sec / 31536000;
	if (annuals > 3) {
		annuals = 3;
	}
	sec -= annuals * 31536000;

	year = 1970 + quadricentennials * 400 + centennials * 100 + quadrennials * 4 + annuals;
	leap = (year % 4 == 0) && ((year % 100 != 0) || (year % 400 == 0));

	yday = sec / 86400;
	if (sec % 86400 != 0)
		yday -= 1;

	sec %= 86400;
	hour = sec / 3600;
	sec %= 3600;
	min = sec / 60;
	sec %= 60;

	for (month = 1, mday = 1; month <= 12; month++) {
		if (yday < days_since_jan_1st[leap][month]) {
			mday += yday - days_since_jan_1st[leap][month - 1];
			break;
		}
	}

	tm_time.tm_sec = sec;
	tm_time.tm_min = min;
	tm_time.tm_hour = hour;
	tm_time.tm_mday = mday;
	tm_time.tm_mon = month - 1;
	tm_time.tm_year = year - 1900;
	tm_time.tm_wday = wday;
	tm_time.tm_yday = yday;
	tm_time.tm_isdst = -1;
	tm_time.tm_gmtoff = 3 * 60 * 60;
	tm_time.tm_zone = nil;

	return tm_time;
}
*/

static const char *wdays[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };

static const char *months[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct",
	"Nov", "Dec"
};

/* Sat, 04 Nov 2023 17:47:03 +0300 */
uint64 tm_rfc822_in_slice(slice s, const struct tm *tm)
{
	char *buf = s.base;
	int n = 0;

	n += c_string_in_slice(s, wdays[tm->tm_wday]);
	buf[n++] = ',';
	buf[n++] = ' ';

	if (tm->tm_mday < 10)
		buf[n++] = '0';

	n += int_in_slice(slice_left(s, n), tm->tm_mday);
	buf[n++] = ' ';

	n += c_string_in_slice(slice_left(s, n), months[tm->tm_mon]);
	buf[n++] = ' ';

	n += int_in_slice(slice_left(s, n), tm->tm_year + 1900);
	buf[n++] = ' ';

	if (tm->tm_hour < 10)
		buf[n++] = '0';

	n += int_in_slice(slice_left(s, n), tm->tm_hour);
	buf[n++] = ':';

	if (tm->tm_min < 10)
		buf[n++] = '0';

	n += int_in_slice(slice_left(s, n), tm->tm_min);
	buf[n++] = ':';

	if (tm->tm_sec < 10)
		buf[n++] = '0';

	n += int_in_slice(slice_left(s, n), tm->tm_sec);
	buf[n++] = ' ';

	buf[n++] = '+';
	buf[n++] = '0';
	buf[n++] = '3';
	buf[n++] = '0';
	buf[n++] = '0';

	return n;
}

/* 08.10.2023 15:13:54 MSK */
uint64 tm_in_slice(slice s, struct tm * tm)
{
	char *buf = s.base;
	int n = 0;

	if (tm->tm_mday < 10)
		buf[n++] = '0';

	n += int_in_slice(slice_left(s, n), tm->tm_mday);
	buf[n++] = '.';

	if (tm->tm_mon + 1 < 10)
		buf[n++] = '0';

	n += int_in_slice(slice_left(s, n), tm->tm_mon + 1);
	buf[n++] = '.';

	n += int_in_slice(slice_left(s, n), tm->tm_year + 1900);
	buf[n++] = ' ';

	if (tm->tm_hour < 10)
		buf[n++] = '0';

	n += int_in_slice(slice_left(s, n), tm->tm_hour);
	buf[n++] = ':';

	if (tm->tm_min < 10)
		buf[n++] = '0';

	n += int_in_slice(slice_left(s, n), tm->tm_min);
	buf[n++] = ':';

	if (tm->tm_sec < 10)
		buf[n++] = '0';

	n += int_in_slice(slice_left(s, n), tm->tm_sec);
	buf[n++] = ' ';

	buf[n++] = 'M';
	buf[n++] = 'S';
	buf[n++] = 'K';

	return n;
}
