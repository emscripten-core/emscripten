#include <time.h>

char *ctime(const time_t *t)
{
	return asctime(localtime(t));
}
