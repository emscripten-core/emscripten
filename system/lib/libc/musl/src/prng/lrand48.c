#include <stdlib.h>
#include <inttypes.h>

uint64_t __rand48_step(unsigned short *xi, unsigned short *lc);
extern unsigned short __seed48[7];

long nrand48(unsigned short s[3])
{
	return __rand48_step(s, __seed48+3) >> 17;
}

long lrand48(void)
{
	return nrand48(__seed48);
}
