#define _GNU_SOURCE
#include <stdlib.h>

extern char **__environ;

int clearenv()
{
	__environ[0] = 0;
	return 0;
}
