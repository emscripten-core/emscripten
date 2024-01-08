#include <stdlib.h>
#include <langinfo.h>
#include <limits.h>
#include <time.h>
#include <ctype.h>
#include <stddef.h>
#include <string.h>
#include <strings.h>
#include <emscripten/console.h>
#include "time_impl.h"

enum {
	TM_YEAR_BASE = 1900,
	TM_WDAY_BASE = 1 /* monday */,
	EPOCH_YEAR = 1970,
	EPOCH_WDAY = 4 /* thursday */,
	DAYSPERWEEK = 7,
	MONSPERYEAR = 12,
	DAYSPERNYEAR = 365,
};

int __is_leap_year(int y)
{
	/* Avoid overflow */
	if (y>INT_MAX-1900) y -= 2000;
	y += 1900;
	return !(y%4) && ((y%100) || !(y%400));
}

static int
leaps_thru_end_of(const int y)
{
	return (y >= 0) ? (y / 4 - y / 100 + y / 400) :
		-(leaps_thru_end_of(-(y + 1)) + 1);
}

static const int mon_lengths[2][MONSPERYEAR] = {
	{ 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
	{ 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }
};

char *strptime(const char *restrict s, const char *restrict f, struct tm *restrict tm)
{
	int i, w, neg, adj, min, range, *dest, dummy;
	const char *ex;
	size_t len;
	int want_century = 0, century = 0, relyear = 0, have_yday = 0, have_wday = 0, have_mday = 0, have_mon = 0;
	while (*f) {
		if (*f != '%') {
			if (isspace(*f)) for (; *s && isspace(*s); s++);
			else if (*s != *f) return 0;
			else s++;
			f++;
			continue;
		}
		f++;
		if (*f == '+') f++;
		if (isdigit(*f)) {
			char *new_f;
			w=strtoul(f, &new_f, 10);
			f = new_f;
		} else {
			w=-1;
		}
		adj=0;
		switch (*f++) {
		case 'a': case 'A':
			dest = &tm->tm_wday;
			have_wday = 1;
			min = ABDAY_1;
			range = 7;
			goto symbolic_range;
		case 'b': case 'B': case 'h':
			dest = &tm->tm_mon;
			have_mon = 1;
			min = ABMON_1;
			range = 12;
			goto symbolic_range;
		case 'c':
			s = strptime(s, nl_langinfo(D_T_FMT), tm);
			if (!s) return 0;
			break;
		case 'C':
			dest = &century;
			if (w<0) w=2;
			want_century |= 2;
			goto numeric_digits;
		case 'd': case 'e':
			dest = &tm->tm_mday;
			have_mday = 1;
			min = 1;
			range = 31;
			goto numeric_range;
		case 'D':
			s = strptime(s, "%m/%d/%y", tm);
			if (!s) return 0;
			break;
		case 'H':
			dest = &tm->tm_hour;
			min = 0;
			range = 24;
			goto numeric_range;
		case 'I':
			dest = &tm->tm_hour;
			min = 1;
			range = 12;
			goto numeric_range;
		case 'j':
			dest = &tm->tm_yday;
			have_yday = 1;
			min = 1;
			range = 366;
			adj = 1;
			goto numeric_range;
		case 'm':
			dest = &tm->tm_mon;
			have_mon = 1;
			min = 1;
			range = 12;
			adj = 1;
			goto numeric_range;
		case 'M':
			dest = &tm->tm_min;
			min = 0;
			range = 60;
			goto numeric_range;
		case 'n': case 't':
			for (; *s && isspace(*s); s++);
			break;
		case 'p':
			ex = nl_langinfo(AM_STR);
			len = strlen(ex);
			if (!strncasecmp(s, ex, len)) {
				tm->tm_hour %= 12;
				s += len;
				break;
			}
			ex = nl_langinfo(PM_STR);
			len = strlen(ex);
			if (!strncasecmp(s, ex, len)) {
				tm->tm_hour %= 12;
				tm->tm_hour += 12;
				s += len;
				break;
			}
			return 0;
		case 'r':
			s = strptime(s, nl_langinfo(T_FMT_AMPM), tm);
			if (!s) return 0;
			break;
		case 'R':
			s = strptime(s, "%H:%M", tm);
			if (!s) return 0;
			break;
		case 'S':
			dest = &tm->tm_sec;
			min = 0;
			range = 61;
			goto numeric_range;
		case 'T':
			s = strptime(s, "%H:%M:%S", tm);
			if (!s) return 0;
			break;
		case 'U':
		case 'W':
			/* Throw away result, for now. (FIXME?) */
			dest = &dummy;
			min = 0;
			range = 54;
			goto numeric_range;
		case 'w':
			dest = &tm->tm_wday;
			have_wday = 1;
			min = 0;
			range = 7;
			goto numeric_range;
		case 'x':
			s = strptime(s, nl_langinfo(D_FMT), tm);
			if (!s) return 0;
			break;
		case 'X':
			s = strptime(s, nl_langinfo(T_FMT), tm);
			if (!s) return 0;
			break;
		case 'y':
			dest = &relyear;
			w = 2;
			want_century |= 1;
			goto numeric_digits;
		case 'Y':
			dest = &tm->tm_year;
			if (w<0) w=4;
			adj = 1900;
			want_century = 0;
			goto numeric_digits;
		case '%':
			if (*s++ != '%') return 0;
			break;
		default:
			return 0;
		numeric_range:
			if (!isdigit(*s)) return 0;
			*dest = 0;
			for (i=1; i<=min+range && isdigit(*s); i*=10)
				*dest = *dest * 10 + *s++ - '0';
			if (*dest - min >= (unsigned)range) return 0;
			*dest -= adj;
			switch((char *)dest - (char *)tm) {
			case offsetof(struct tm, tm_yday):
				;
			}
			goto update;
		numeric_digits:
			neg = 0;
			if (*s == '+') s++;
			else if (*s == '-') neg=1, s++;
			if (!isdigit(*s)) return 0;
			for (*dest=i=0; i<w && isdigit(*s); i++)
				*dest = *dest * 10 + *s++ - '0';
			if (neg) *dest = -*dest;
			*dest -= adj;
			goto update;
		symbolic_range:
			for (i=2*range-1; i>=0; i--) {
				ex = nl_langinfo(min+i);
				len = strlen(ex);
				if (strncasecmp(s, ex, len)) continue;
				s += len;
				*dest = i % range;
				break;
			}
			if (i<0) return 0;
			goto update;
		update:
			//FIXME
			;
		}
	}
	if (want_century) {
		tm->tm_year = relyear;
		if (want_century & 2) tm->tm_year += century * 100 - 1900;
		else if (tm->tm_year <= 68) tm->tm_year += 100;
	}

	/* Compute some missing values when possible. */
	const int *mon_lens = mon_lengths[__is_leap_year(tm->tm_year)];
	if (!have_yday && have_mon && have_mday) {
      //emscripten_errf("setting yday: tm_mday=%d tm_mon=%d", tm->tm_mday, tm->tm_mon);
		tm->tm_yday = tm->tm_mday - 1;
		for (i = 0; i < tm->tm_mon; i++)
			tm->tm_yday += mon_lens[i];
		have_yday = 1;
	}
	if (have_yday) {
		int days = tm->tm_yday;
		if (!have_wday) {
      const int year = tm->tm_year + TM_YEAR_BASE;
      //emscripten_errf("setting wday: tm_year=%d, tm->tm_yday=%d %d", tm->tm_year, tm->tm_yday, year);
			tm->tm_wday = EPOCH_WDAY +
					((year - EPOCH_YEAR) % DAYSPERWEEK) *
					(DAYSPERNYEAR % DAYSPERWEEK) +
					leaps_thru_end_of(year - 1) -
					leaps_thru_end_of(EPOCH_YEAR - 1) +
					tm->tm_yday;
			tm->tm_wday %= DAYSPERWEEK;
      //emscripten_errf("tm_wday -> %d", tm->tm_wday);
			if (tm->tm_wday < 0) {
        //emscripten_errf("adding wday");
				tm->tm_wday += DAYSPERWEEK;
      }
		}
		if (!have_mon) {
			tm->tm_mon = 0;
			while (tm->tm_mon < MONSPERYEAR && days >= mon_lens[tm->tm_mon])
				days -= mon_lens[tm->tm_mon++];
		}
		if (!have_mday)
			tm->tm_mday = days + 1;
	}

	return (char *)s;
}
