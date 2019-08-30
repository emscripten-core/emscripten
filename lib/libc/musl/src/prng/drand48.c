#include <stdlib.h>
#include <inttypes.h>

uint64_t __rand48_step(unsigned short *xi, unsigned short *lc);
extern unsigned short __seed48[7];

double erand48(unsigned short s[3])
{
	union {
		uint64_t u;
		double f;
	} x = { 0x3ff0000000000000ULL | __rand48_step(s, __seed48+3)<<4 };
	return x.f - 1.0;
}

double drand48(void)
{
	return erand48(__seed48);
}
