#include <stdlib.h>
#include <string.h>
#include <errno.h>

extern char **__environ;
extern char **__env_map;

int unsetenv(const char *name)
{
	int i, j;
	size_t l = strlen(name);

	if (!*name || strchr(name, '=')) {
		errno = EINVAL;
		return -1;
	}
again:
	for (i=0; __environ[i] && (memcmp(name, __environ[i], l) || __environ[i][l] != '='); i++);
	if (__environ[i]) {
		if (__env_map) {
			for (j=0; __env_map[j] && __env_map[j] != __environ[i]; j++);
			free (__env_map[j]);
			for (; __env_map[j]; j++)
				__env_map[j] = __env_map[j+1];
		}
		for (; __environ[i]; i++)
			__environ[i] = __environ[i+1];
		goto again;
	}
	return 0;
}
