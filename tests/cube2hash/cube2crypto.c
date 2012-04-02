#include <stdlib.h>
#include "util.h"
#include "tiger.h"
#include "cube2crypto.h"

char *cube2crypto_hashstring(char *string)
{
	char *result = (char *)malloc(49);
	union hashval hv;
	
	tiger_hash((uchar *)string, strlen(string), &hv);
	
	int i;
	for(i = 0; i < sizeof(hv.bytes); i++)
	{
		uchar c = hv.bytes[i];
		*(result+(i*2))   = "0123456789ABCDEF"[c&0xF];
		*(result+(i*2)+1) = "0123456789ABCDEF"[c>>4];
	}
	*(result+(i*2)+2) = '\0';
	
	return result;
}
