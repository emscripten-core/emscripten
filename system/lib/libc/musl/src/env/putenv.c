#include <stdlib.h>
#include <string.h>

extern char **__environ;
char **__env_map;

int __putenv(char *s, int a)
{
	int i=0, j=0;
	char *z = strchr(s, '=');
	char **newenv = 0;
	char **newmap = 0;
	static char **oldenv;

	if (!z) return unsetenv(s);
	if (z==s) return -1;
	for (; __environ[i] && memcmp(s, __environ[i], z-s+1); i++);
	if (a) {
		if (!__env_map) {
			__env_map = calloc(2, sizeof(char *));
			if (__env_map) __env_map[0] = s;
		} else {
			for (; __env_map[j] && __env_map[j] != __environ[i]; j++);
			if (!__env_map[j]) {
				newmap = realloc(__env_map, sizeof(char *)*(j+2));
				if (newmap) {
					__env_map = newmap;
					__env_map[j] = s;
					__env_map[j+1] = NULL;
				}
			} else {
				free(__env_map[j]);
				__env_map[j] = s;
			}
		}
	}
	if (!__environ[i]) {
		newenv = malloc(sizeof(char *)*(i+2));
		if (!newenv) {
			if (a && __env_map) __env_map[j] = 0;
			return -1;
		}
		memcpy(newenv, __environ, sizeof(char *)*i);
		newenv[i] = s;
		newenv[i+1] = 0;
		__environ = newenv;
		free(oldenv);
		oldenv = __environ;
	}

	__environ[i] = s;
	return 0;
}

int putenv(char *s)
{
	return __putenv(s, 0);
}
