#include <string.h>

char *strchr(const char *s, int c) {
	for (;;) {
		if (*s == (char) c)
			return (char *) s;
		if (*s == 0)
			return NULL;
		s++;
	}
}
