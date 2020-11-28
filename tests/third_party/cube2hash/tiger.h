#ifndef _TIGER_H
#define _TIGER_H

union hashval
{
	unsigned char bytes[3*8];
	unsigned long long int chunks[3];
};

void tiger_hash(const unsigned char *str, int length, union hashval *val);

#endif
