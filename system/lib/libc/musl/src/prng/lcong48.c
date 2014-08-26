#include <stdlib.h>
#include <string.h>

extern unsigned short __seed48[7];

void lcong48(unsigned short p[7])
{
	memcpy(__seed48, p, sizeof __seed48);
}
