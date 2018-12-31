#define _GNU_SOURCE
#include <string.h>
#include <dirent.h>
#include "libc.h"

int versionsort(const struct dirent **a, const struct dirent **b)
{
	return strverscmp((*a)->d_name, (*b)->d_name);
}

#undef versionsort64
LFS64(versionsort);
