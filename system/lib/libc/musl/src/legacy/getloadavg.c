#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int getloadavg(double *a, int n)
{
	int i;
	double b[3];
	FILE *f = fopen("/proc/loadavg", "rbe");
	if (!f) return -1;
	i = fscanf(f, "%lf %lf %lf", b, b+1, b+2);
	fclose(f);
	if (n > i) n = i;
	if (n < 0) return -1;
	memcpy(a, b, n * sizeof *a);
	return n;
}
