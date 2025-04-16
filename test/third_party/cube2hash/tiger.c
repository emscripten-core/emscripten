///////////////////////// cryptography /////////////////////////////////

/* Based off the reference implementation of Tiger, a cryptographically
 * secure 192 bit hash function by Ross Anderson and Eli Biham. More info at:
 * http://www.cs.technion.ac.il/~biham/Reports/Tiger/
 */

#define TIGER_PASSES 3

#include "tiger.h"
#include "util.h"

chunk sboxes[4*256];

#define sb1 (sboxes)
#define sb2 (sboxes+256)
#define sb3 (sboxes+256*2)
#define sb4 (sboxes+256*3)

#define round(a, b, c, x) \
			c ^= x; \
			a -= sb1[((c)>>(0*8))&0xFF] ^ sb2[((c)>>(2*8))&0xFF] ^ \
					 sb3[((c)>>(4*8))&0xFF] ^ sb4[((c)>>(6*8))&0xFF] ; \
			b += sb4[((c)>>(1*8))&0xFF] ^ sb3[((c)>>(3*8))&0xFF] ^ \
					 sb2[((c)>>(5*8))&0xFF] ^ sb1[((c)>>(7*8))&0xFF] ; \
			b *= mul;

void tiger_compress(const chunk *str, chunk state[3])
{
	chunk a, b, c;
	chunk aa, bb, cc;
	chunk x0, x1, x2, x3, x4, x5, x6, x7;

	a = state[0];
	b = state[1];
	c = state[2];

	x0=str[0]; x1=str[1]; x2=str[2]; x3=str[3];
	x4=str[4]; x5=str[5]; x6=str[6]; x7=str[7];

	aa = a;
	bb = b;
	cc = c;

	int pass;

	for(pass = 0; pass < TIGER_PASSES; pass++)
	{
		if(pass)
		{
			x0 -= x7 ^ 0xA5A5A5A5A5A5A5A5ULL; x1 ^= x0; x2 += x1; x3 -= x2 ^ ((~x1)<<19);
			x4 ^= x3; x5 += x4; x6 -= x5 ^ ((~x4)>>23); x7 ^= x6;
			x0 += x7; x1 -= x0 ^ ((~x7)<<19); x2 ^= x1; x3 += x2;
			x4 -= x3 ^ ((~x2)>>23); x5 ^= x4; x6 += x5; x7 -= x6 ^ 0x0123456789ABCDEFULL;
		}
		
    uint mul = !pass ? 5 : (pass==1 ? 7 : 9);
    round(a, b, c, x0) round(b, c, a, x1) round(c, a, b, x2) round(a, b, c, x3)
    round(b, c, a, x4) round(c, a, b, x5) round(a, b, c, x6) round(b, c, a, x7)

    chunk tmp = a; a = c; c = b; b = tmp;
	
	}

	a ^= aa;
	b -= bb;
	c += cc;

	state[0] = a;
	state[1] = b;
	state[2] = c;
}

void tiger_gensboxes()
{
	const char *str = "Tiger - A Fast New Hash Function, by Ross Anderson and Eli Biham";
	chunk state[3] = { 0x0123456789ABCDEFULL, 0xFEDCBA9876543210ULL, 0xF096A5B4C3B2E187ULL };
	uchar temp[64];
	int i, j, col, sb, pass;

	if(BIGENDIAN)
	{
		for(j = 0; j < 64; j++)
		{
			temp[j^7] = str[j];
		}
	}
	else 
	{
		for(j = 0; j < 64; j++)
		{
			temp[j] = str[j];
		}
	}

	for(i = 0; i < 1024; i++)
	{
		for(col = 0; col < 8; col++)
		{
			((uchar *)&sboxes[i])[col] = i&0xFF;
		}
	}

	int abc = 2;
	for(pass = 0; pass < 5; pass++)
	{
		for(i = 0; i < 256; i++)
		{
			for(sb = 0; sb < 1024; sb += 256)
			{
				abc++;
				if(abc >= 3) { abc = 0; tiger_compress((chunk *)temp, state); }
				for(col = 0; col < 8; col++)
				{
					uchar val = ((uchar *)&sboxes[sb+i])[col];
					((uchar *)&sboxes[sb+i])[col] = ((uchar *)&sboxes[sb + ((uchar *)&state[abc])[col]])[col];
					((uchar *)&sboxes[sb + ((uchar *)&state[abc])[col]])[col] = val;
				}
			}
		}
	}
}

void tiger_hash(const uchar *str, int length, union hashval *val)
{
	static int init = false;
	if(!init) { tiger_gensboxes(); init = true; }

	uchar temp[64];

	val->chunks[0] = 0x0123456789ABCDEFULL;
	val->chunks[1] = 0xFEDCBA9876543210ULL;
	val->chunks[2] = 0xF096A5B4C3B2E187ULL;

	int i, j;
	for(i = length; i >= 64; i -= 64, str += 64)
	{
	    if(BIGENDIAN)
	    {
				for(j = 0; j < 64; j++)
				{
					temp[j^7] = str[j];
				}
				
				tiger_compress((chunk *)temp, val->chunks);
	    }
	    else 
	    {
	    	tiger_compress((chunk *)str, val->chunks);
	    }
	}

	if(BIGENDIAN)
	{
	    for(j = 0; j < i; j++) temp[j^7] = str[j];
	    temp[j^7] = 0x01;
	    while(++j&7) temp[j^7] = 0;
	}
	else
	{
	    for(j = 0; j < i; j++) temp[j] = str[j];
	    temp[j] = 0x01;
	    while(++j&7) temp[j] = 0;
	}

	if(j > 56)
	{
	    while(j < 64) temp[j++] = 0;
	    tiger_compress((chunk *)temp, val->chunks);
	    j = 0;
	}
	while(j < 56) temp[j++] = 0;
	*(chunk *)(temp+56) = (chunk)length<<3;
	tiger_compress((chunk *)temp, val->chunks);
}
