#include <stdlib.h>
#include <string.h>
#include "libc.h"

char *getenv(const char *name)
{
	int i;
	size_t l = strlen(name);
	if (!__environ || !*name || strchr(name, '=')) return NULL;
	for (i=0; __environ[i] && (strncmp(name, __environ[i], l)
		|| __environ[i][l] != '='); i++);
	if (__environ[i]) return __environ[i] + l+1;
	return NULL;
}
