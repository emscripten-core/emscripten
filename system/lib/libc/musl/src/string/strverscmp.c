#define _GNU_SOURCE
#include <ctype.h>
#include <string.h>

int strverscmp(const char *l, const char *r)
{
	int haszero=1;
	while (*l==*r) {
		if (!*l) return 0;

		if (*l=='0') {
			if (haszero==1) {
				haszero=0;
			}
		} else if (isdigit(*l)) {
			if (haszero==1) {
				haszero=2;
			}
		} else {
			haszero=1;
		}
		l++; r++;
	}
	if (haszero==1 && (*l=='0' || *r=='0')) {
		haszero=0;
	}
	if ((isdigit(*l) && isdigit(*r) ) && haszero) {
		size_t lenl=0, lenr=0;
		while (isdigit(l[lenl]) ) lenl++;
		while (isdigit(r[lenr]) ) lenr++;
		if (lenl==lenr) {
			return (*l -  *r);
		} else if (lenl>lenr) {
			return 1;
		} else {
			return -1;
		}
	} else {
		return (*l -  *r);
	}
}
