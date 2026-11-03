#include <stdlib.h>
#include <langinfo.h>
#include <time.h>
#include <ctype.h>
#include <stddef.h>
#include <string.h>
#include <strings.h>

char *strptime(const char *restrict s, const char *restrict f, struct tm *restrict tm)
{
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
			min = ABDAY_1;
			range = 7;
			goto symbolic_range;
		case 'b': case 'B': case 'h':
			dest = &tm->tm_mon;
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
			min = 1;
			range = 31;
			goto numeric_range;
		case 'D':
			s = strptime(s, "%m/%d/%y", tm);
			if (!s) return 0;
			break;
		case 'F':
			/* Use temp buffer to implement the odd requirement
			 * that entire field be width-limited but the year
			 * subfield not itself be limited. */
			i = 0;
			char tmp[20];
			if (*s == '-' || *s == '+') tmp[i++] = *s++;
			while (*s=='0' && isdigit(s[1])) s++;
			for (; *s && i<(size_t)w && i+1<sizeof tmp; i++) {
				tmp[i] = *s++;
			}
			tmp[i] = 0;
			char *p = strptime(tmp, "%12Y-%m-%d", tm);
			if (!p) return 0;
			s -= tmp+i-p;
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
			min = 1;
			range = 366;
			adj = 1;
			goto numeric_range;
		case 'm':
			dest = &tm->tm_mon;
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
		case 's':
			/* Parse only. Effect on tm is unspecified
			 * and presently no effect is implemented.. */
			if (*s == '-') s++;
			if (!isdigit(*s)) return 0;
			while (isdigit(*s)) s++;
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
			/* Throw away result of %U, %V, %W, %g, and %G. Effect
			 * is unspecified and there is no clear right choice. */
			dest = &dummy;
			min = 0;
			range = 54;
			goto numeric_range;
		case 'V':
			dest = &dummy;
			min = 1;
			range = 53;
			goto numeric_range;
		case 'g':
			dest = &dummy;
			w = 2;
			goto numeric_digits;
		case 'G':
			dest = &dummy;
			if (w<0) w=4;
			goto numeric_digits;
		case 'u':
			dest = &tm->tm_wday;
			min = 1;
			range = 7;
			goto numeric_range;
		case 'w':
			dest = &tm->tm_wday;
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
		case 'z':
			if (*s == '+') neg = 0;
			else if (*s == '-') neg = 1;
			else return 0;
			for (i=0; i<4; i++) if (!isdigit(s[1+i])) return 0;
			tm->__tm_gmtoff = (s[1]-'0')*36000+(s[2]-'0')*3600
				+ (s[3]-'0')*600 + (s[4]-'0')*60;
			if (neg) tm->__tm_gmtoff = -tm->__tm_gmtoff;
			s += 5;
			break;
		case 'Z':
			if (!strncmp(s, tzname[0], len = strlen(tzname[0]))) {
				tm->tm_isdst = 0;
				s += len;
			} else if (!strncmp(s, tzname[1], len=strlen(tzname[1]))) {
				tm->tm_isdst = 1;
				s += len;
			} else {
				/* FIXME: is this supposed to be an error? */
				while ((*s|32)-'a' <= 'z'-'a') s++;
			}
			break;
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
	return (char *)s;
}
