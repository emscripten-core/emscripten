#include <math.h>

extern int signgam;
double __lgamma_r(double, int *);

double lgamma(double x)
{
	return __lgamma_r(x, &signgam);
}
