#include <math.h>

extern int signgam;
float __lgammaf_r(float, int *);

float lgammaf(float x)
{
	return __lgammaf_r(x, &signgam);
}
