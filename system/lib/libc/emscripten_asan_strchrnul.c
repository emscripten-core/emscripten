#include <string.h>

#define ALIGN (sizeof(size_t))
#define ONES ((size_t)-1/UCHAR_MAX)
#define HIGHS (ONES * (UCHAR_MAX/2+1))
#define HASZERO(x) ((x)-ONES & ~(x) & HIGHS)

char *__strchrnul(const char *s, int c)
{
	for (;; ++s)
		if (*s == (char) c || !*s)
			return (char *) s;
}

extern __typeof(__strchrnul) strchrnul __attribute__((weak, alias("__strchrnul")));
