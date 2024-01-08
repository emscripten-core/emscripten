/*
 * This file has been modified from upstream musl version and now includes
 * some code derived from both Bionic libc and FreeBSD libc.
 *
 * The FreeBSD portions are coverted by the following copyright and license:
 *
 * https://github.com/freebsd/freebsd-src/blob/main/lib/libc/stdtime/strptime.c
 *
 * Copyright (c) 1997, 1998, 2005, 2008 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code was contributed to The NetBSD Foundation by Klaus Klein.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * The Bionic portions are coverted by the following copyright and license:
 *
 * https://github.com/aosp-mirror/platform_bionic/blob/main/libc/tzcode/strptime.c
 *
 * Copyright (c) 1997, 1998, 2005, 2008 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code was contributed to The NetBSD Foundation by Klaus Klein.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <stdlib.h>
#include <langinfo.h>
#include <time.h>
#include <ctype.h>
#include <stddef.h>
#include <string.h>
#include <strings.h>

#ifdef __EMSCRIPTEN__
#include <emscripten/console.h>
#include <limits.h>
#include "time_impl.h"

enum {
	TM_SUNDAY = 0,
	TM_MONDAY = 1,
	TM_YEAR_BASE = 1900,
	TM_WDAY_BASE = 1 /* monday */,
	EPOCH_YEAR = 1970,
	EPOCH_WDAY = 4 /* thursday */,
	DAYSPERWEEK = 7,
	MONSPERYEAR = 12,
	DAYSPERNYEAR = 365,
	SECSPERMIN = 60,
	MINSPERHOUR = 60,
};

int is_leap(int y)
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

/* RFC-822/RFC-2822 */
static const char * const nast[5] = {
       "EST",    "CST",    "MST",    "PST",    "\0\0\0"
};
static const char * const nadt[5] = {
       "EDT",    "CDT",    "MDT",    "PDT",    "\0\0\0"
};

static const int mon_lengths[2][MONSPERYEAR] = {
	{ 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
	{ 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }
};

static const char *
_find_string(const char *bp, int *tgt, const char * const *n1,
		const char * const *n2, int c)
{
	int i;
	unsigned int len;

	/* check full name - then abbreviated ones */
	for (; n1 != NULL; n1 = n2, n2 = NULL) {
		for (i = 0; i < c; i++, n1++) {
			len = strlen(*n1);
			if (strncasecmp(*n1, (const char *)bp, len) == 0) {
				*tgt = i;
				return bp + len;
			}
		}
	}

	/* Nothing matched */
	return NULL;
}

/*
 * Calculate the week day of the first day of a year. Valid for
 * the Gregorian calendar, which began Sept 14, 1752 in the UK
 * and its colonies. Ref:
 * http://en.wikipedia.org/wiki/Determination_of_the_day_of_the_week
 */

static int
first_wday_of(int year)
{
	return (((2 * (3 - (year / 100) % 4)) + (year % 100) +
		((year % 100) / 4) + (is_leap(year) ? 6 : 0) + 1) % 7);
}
#endif

char *strptime(const char *restrict s, const char *restrict f, struct tm *restrict tm)
{
#ifdef __EMSCRIPTEN__
	int day_offset = -1, week_offset, offs;
	int have_year = 0, have_yday = 0, have_wday = 0, have_mday = 0, have_mon = 0;
#endif
	int i, w, neg, adj, min, range, *dest, dummy;
	const char *ex;
	size_t len;
	int want_century = 0, century = 0, relyear = 0;
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
#if __EMSCRIPTEN__
		case 'F': // GNU extension
			s = strptime(s, "%Y-%m-%d", tm);
			if (!s) return 0;
			break;
#endif
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
#ifdef __EMSCRIPTEN__
			/*
			 * 'U' and 'W' handling code modified from FreeBSD
			 */
			if (!isdigit(*s))
				return 0;

			len = 2;
			for (i = 0; len && *s != 0 && isdigit(*s); s++) {
				i *= 10;
				i += *s - '0';
				len--;
			}
			if (i > 53)
				return (NULL);

			if (f[-1] == 'U')
				day_offset = TM_SUNDAY;
			else
				day_offset = TM_MONDAY;

			week_offset = i;
			break;
#else
			/* Throw away result, for now. (FIXME?) */
			dest = &dummy;
			min = 0;
			range = 54;
			goto numeric_range;
#endif
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
			have_year = 1;
			if (w<0) w=4;
			adj = 1900;
			want_century = 0;
			goto numeric_digits;
#ifdef __EMSCRIPTEN__
		/*
		 * Timezone parsing code based on code from Bionic libc.
		 */
#define SECSPERHOUR (SECSPERMIN * MINSPERHOUR)
		case 'z':
			/*
			 * We recognize all ISO 8601 formats:
			 * Z	= Zulu time/UTC
			 * [+-]hhmm
			 * [+-]hh:mm
			 * [+-]hh
			 * We recognize all RFC-822/RFC-2822 formats:
			 * UT|GMT
			 *          North American : UTC offsets
			 * E[DS]T = Eastern : -4 | -5
			 * C[DS]T = Central : -5 | -6
			 * M[DS]T = Mountain: -6 | -7
			 * P[DS]T = Pacific : -7 | -8
			 */
			while (isspace(*s))
				s++;

			switch (*s++) {
			case 'G':
				if (*s++ != 'M')
					return NULL;
				/*FALLTHROUGH*/
			case 'U':
				if (*s++ != 'T')
					return NULL;
				/*FALLTHROUGH*/
			case 'Z':
				tm->tm_isdst = 0;
				tm->__tm_gmtoff = 0;
				tm->__tm_zone = "UTC";
				continue;
			case '+':
				neg = 0;
				break;
			case '-':
				neg = 1;
				break;
			default:
				--s;
				ex = _find_string(s, &i, nast, NULL, 4);
				if (ex != NULL) {
					tm->__tm_gmtoff = (-5 - i) * SECSPERHOUR;
					tm->__tm_zone = (char *)nast[i];
					s = ex;
					continue;
				}
				ex = _find_string(s, &i, nadt, NULL, 4);
				if (ex != NULL) {
					tm->tm_isdst = 1;
					tm->__tm_gmtoff = (-4 - i) * SECSPERHOUR;
					tm->__tm_zone = (char *)nadt[i];
					s = ex;
					continue;
				}
				return NULL;
			}
			if (!isdigit(s[0]) || !isdigit(s[1]))
				return NULL;
			offs = ((s[0]-'0') * 10 + (s[1]-'0')) * SECSPERHOUR;
			s += 2;
			if (*s == ':')
				s++;
			if (isdigit(*s)) {
				offs += (*s++ - '0') * 10 * SECSPERMIN;
				if (!isdigit(*s))
					return NULL;
				offs += (*s++ - '0') * SECSPERMIN;
			}
			if (neg)
				offs = -offs;
			tm->tm_isdst = 0;	/* XXX */
			tm->__tm_gmtoff = offs;
			tm->__tm_zone = NULL;	/* XXX */
			break;
#endif
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
		have_year = 1;
		if (want_century & 2) tm->tm_year += century * 100 - 1900;
		else if (tm->tm_year <= 68) tm->tm_year += 100;
	}

#ifdef __EMSCRIPTEN__
		/*
		 * Compute secondary fields based on day_offset/week_offset set
		 * in 'U'/'W' handlers
		 * Based on code in FreeBSD libc.
		 */
		if (!have_yday && day_offset != -1) {
			int tmpwday, tmpyday, fwo;

			fwo = first_wday_of(tm->tm_year + TM_YEAR_BASE);
			/* No incomplete week (week 0). */
			if (week_offset == 0 && fwo == day_offset)
				return (NULL);

			/* Set the date to the first Sunday (or Monday)
			 * of the specified week of the year.
			 */
			tmpwday = have_wday ? tm->tm_wday :
			    day_offset;
			tmpyday = (7 - fwo + day_offset) % 7 +
			    (week_offset - 1) * 7 +
			    (tmpwday - day_offset + 7) % 7;
			/* Impossible yday for incomplete week (week 0). */
			if (tmpyday < 0) {
				if (have_wday)
					return 0;
				tmpyday = 0;
			}
			tm->tm_yday = tmpyday;
			have_yday = 1;
		}

		if (have_yday && have_year) {
			static int start_of_month[2][13] = {
				{0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365},
				{0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366}
			};
			if (!have_mon) {
				i = 0;
				while (tm->tm_yday >=
						start_of_month[is_leap(tm->tm_year +
						TM_YEAR_BASE)][i])
					i++;
				if (i > 12) {
					i = 1;
					tm->tm_yday -=
							start_of_month[is_leap(tm->tm_year +
							TM_YEAR_BASE)][12];
					tm->tm_year++;
				}
				tm->tm_mon = i - 1;
				have_mon = 1;
			}
			if (!have_mday) {
				tm->tm_mday = tm->tm_yday -
						start_of_month[is_leap(tm->tm_year + TM_YEAR_BASE)]
						[tm->tm_mon] + 1;
				have_mday = 1;
			}
			if (!have_wday) {
				i = 0;
				int wday_offset = first_wday_of(tm->tm_year);
				while (i++ <= tm->tm_yday) {
					if (wday_offset++ >= 6)
						wday_offset = 0;
				}
				tm->tm_wday = wday_offset;
				have_wday = 1;
			}
		}

		/*
		 * Computed field code based on code from Bionic libc.
		 */

		if (!have_mday && tm->tm_mday < 31) have_mday = 1;
		if (!have_mon && tm->tm_mon < 12) have_mon = 1;
		const int *mon_lens = mon_lengths[is_leap(tm->tm_year)];
		if (!have_yday && have_mon && have_mday) {
			tm->tm_yday = tm->tm_mday - 1;
			for (i = 0; i < tm->tm_mon; i++)
				tm->tm_yday += mon_lens[i];
			have_yday = 1;
		}
		if (have_yday) {
			int days = tm->tm_yday;
			if (!have_wday) {
				const int year = tm->tm_year + TM_YEAR_BASE;
				tm->tm_wday = EPOCH_WDAY +
						((year - EPOCH_YEAR) % DAYSPERWEEK) *
						(DAYSPERNYEAR % DAYSPERWEEK) +
						leaps_thru_end_of(year - 1) -
						leaps_thru_end_of(EPOCH_YEAR - 1) +
						tm->tm_yday;
				tm->tm_wday %= DAYSPERWEEK;
				if (tm->tm_wday < 0) {
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
#endif

	return (char *)s;
}
