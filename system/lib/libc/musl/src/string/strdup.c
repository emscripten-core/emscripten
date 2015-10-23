#include <stdlib.h>
#include <string.h>
#include "libc.h"

char *__strdup(const char *s)
{
	size_t l = strlen(s);
	char *d = malloc(l+1);
	if (!d) return NULL;
	return memcpy(d, s, l+1);
}

weak_alias(__strdup, strdup);
