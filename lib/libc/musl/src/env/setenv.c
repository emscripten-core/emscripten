#include <stdlib.h>
#include <string.h>
#include <errno.h>

int __putenv(char *s, int a);

int setenv(const char *var, const char *value, int overwrite)
{
	char *s;
	int l1, l2;

	if (!var || !*var || strchr(var, '=')) {
		errno = EINVAL;
		return -1;
	}
	if (!overwrite && getenv(var)) return 0;

	l1 = strlen(var);
	l2 = strlen(value);
	s = malloc(l1+l2+2);
	if (s) {
		memcpy(s, var, l1);
		s[l1] = '=';
		memcpy(s+l1+1, value, l2);
		s[l1+l2+1] = 0;
		if (!__putenv(s, 1)) return 0;
	}
	free(s);
	return -1;
}
