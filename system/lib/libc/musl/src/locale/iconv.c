#include <iconv.h>
#include <errno.h>
#include <wchar.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <stdint.h>

#define UTF_32BE    0300
#define UTF_16LE    0301
#define UTF_16BE    0302
#define UTF_32LE    0303
#define UCS2BE      0304
#define UCS2LE      0305
#define WCHAR_T     0306
#define US_ASCII    0307
#define UTF_8       0310
#define EUC_JP      0320
#define SHIFT_JIS   0321
#define GB18030     0330
#define GBK         0331
#define GB2312      0332
#define BIG5        0340
#define EUC_KR      0350

/* FIXME: these are not implemented yet
 * EUC:   A1-FE A1-FE
 * GBK:   81-FE 40-7E,80-FE
 * Big5:  A1-FE 40-7E,A1-FE
 */

/* Definitions of charmaps. Each charmap consists of:
 * 1. Empty-string-terminated list of null-terminated aliases.
 * 2. Special type code or number of elided entries.
 * 3. Character table (size determined by field 2). */

static const unsigned char charmaps[] =
"utf8\0\0\310"
"wchart\0\0\306"
"ucs2\0ucs2be\0\0\304"
"ucs2le\0\0\305"
"utf16\0utf16be\0\0\302"
"utf16le\0\0\301"
"ucs4\0ucs4be\0utf32\0utf32be\0\0\300"
"ucs4le\0utf32le\0\0\303"
"ascii\0usascii\0iso646\0iso646us\0\0\307"
"eucjp\0\0\320"
"shiftjis\0sjis\0\0\321"
"gb18030\0\0\330"
"gbk\0\0\331"
"gb2312\0\0\332"
"big5\0bigfive\0cp950\0big5hkscs\0\0\340"
"euckr\0ksc5601\0ksx1001\0cp949\0\0\350"
#include "codepages.h"
;

static const unsigned short legacy_chars[] = {
#include "legacychars.h"
};

static const unsigned short jis0208[84][94] = {
#include "jis0208.h"
};

static const unsigned short gb18030[126][190] = {
#include "gb18030.h"
};

static const unsigned short big5[89][157] = {
#include "big5.h"
};

static const unsigned short hkscs[] = {
#include "hkscs.h"
};

static const unsigned short ksc[93][94] = {
#include "ksc.h"
};

static int fuzzycmp(const unsigned char *a, const unsigned char *b)
{
	for (; *a && *b; a++, b++) {
		while (*a && (*a|32U)-'a'>26 && *a-'0'>10U) a++;
		if ((*a|32U) != *b) return 1;
	}
	return *a != *b;
}

static size_t find_charmap(const void *name)
{
	const unsigned char *s;
	for (s=charmaps; *s; ) {
		if (!fuzzycmp(name, s)) {
			for (; *s; s+=strlen((void *)s)+1);
			return s+1-charmaps;
		}
		s += strlen((void *)s)+1;
		if (!*s) {
			if (s[1] > 0200) s+=2;
			else s+=2+(128U-s[1])/4*5;
		}
	}
	return -1;
}

iconv_t iconv_open(const char *to, const char *from)
{
	size_t f, t;

	if ((t = find_charmap(to))==-1
	 || (f = find_charmap(from))==-1
	 || (charmaps[t] >= 0320)) {
		errno = EINVAL;
		return (iconv_t)-1;
	}

	return (void *)(f<<16 | t);
}

int iconv_close(iconv_t cd)
{
	return 0;
}

static unsigned get_16(const unsigned char *s, int e)
{
	e &= 1;
	return s[e]<<8 | s[1-e];
}

static void put_16(unsigned char *s, unsigned c, int e)
{
	e &= 1;
	s[e] = c>>8;
	s[1-e] = c;
}

static unsigned get_32(const unsigned char *s, int e)
{
	e &= 3;
	return s[e]+0U<<24 | s[e^1]<<16 | s[e^2]<<8 | s[e^3];
}

static void put_32(unsigned char *s, unsigned c, int e)
{
	e &= 3;
	s[e^0] = c>>24;
	s[e^1] = c>>16;
	s[e^2] = c>>8;
	s[e^3] = c;
}

/* Adapt as needed */
#define mbrtowc_utf8 mbrtowc
#define wctomb_utf8 wctomb

size_t iconv(iconv_t cd0, char **restrict in, size_t *restrict inb, char **restrict out, size_t *restrict outb)
{
	size_t x=0;
	unsigned long cd = (unsigned long)cd0;
	unsigned to = cd & 0xffff;
	unsigned from = cd >> 16;
	const unsigned char *map = charmaps+from+1;
	const unsigned char *tomap = charmaps+to+1;
	mbstate_t st = {0};
	wchar_t wc;
	unsigned c, d;
	size_t k, l;
	int err;
	unsigned char type = map[-1];
	unsigned char totype = tomap[-1];

	if (!in || !*in || !*inb) return 0;

	for (; *inb; *in+=l, *inb-=l) {
		c = *(unsigned char *)*in;
		l = 1;

		if (c >= 128 || type-UTF_32BE < 7U) switch (type) {
		case UTF_8:
			l = mbrtowc_utf8(&wc, *in, *inb, &st);
			if (!l) l++;
			else if (l == (size_t)-1) goto ilseq;
			else if (l == (size_t)-2) goto starved;
			c = wc;
			break;
		case US_ASCII:
			goto ilseq;
		case WCHAR_T:
			l = sizeof(wchar_t);
			if (*inb < l) goto starved;
			c = *(wchar_t *)*in;
			if (0) {
		case UTF_32BE:
		case UTF_32LE:
			l = 4;
			if (*inb < 4) goto starved;
			c = get_32((void *)*in, type);
			}
			if (c-0xd800u < 0x800u || c >= 0x110000u) goto ilseq;
			break;
		case UCS2BE:
		case UCS2LE:
		case UTF_16BE:
		case UTF_16LE:
			l = 2;
			if (*inb < 2) goto starved;
			c = get_16((void *)*in, type);
			if ((unsigned)(c-0xdc00) < 0x400) goto ilseq;
			if ((unsigned)(c-0xd800) < 0x400) {
				if (type-UCS2BE < 2U) goto ilseq;
				l = 4;
				if (*inb < 4) goto starved;
				d = get_16((void *)(*in + 2), type);
				if ((unsigned)(d-0xdc00) >= 0x400) goto ilseq;
				c = ((c-0xd7c0)<<10) + (d-0xdc00);
			}
			break;
		case SHIFT_JIS:
			if (c-0xa1 <= 0xdf-0xa1) {
				c += 0xff61-0xa1;
				break;
			}
			l = 2;
			if (*inb < 2) goto starved;
			d = *((unsigned char *)*in + 1);
			if (c-129 <= 159-129) c -= 129;
			else if (c-224 <= 239-224) c -= 193;
			else goto ilseq;
			c *= 2;
			if (d-64 <= 158-64) {
				if (d==127) goto ilseq;
				if (d>127) d--;
				d -= 64;
			} else if (d-159 <= 252-159) {
				c++;
				d -= 159;
			}
			c = jis0208[c][d];
			if (!c) goto ilseq;
			break;
		case EUC_JP:
			l = 2;
			if (*inb < 2) goto starved;
			d = *((unsigned char *)*in + 1);
			if (c==0x8e) {
				c = d;
				if (c-0xa1 > 0xdf-0xa1) goto ilseq;
				c += 0xff61 - 0xa1;
				break;
			}
			c -= 0xa1;
			d -= 0xa1;
			if (c >= 84 || d >= 94) goto ilseq;
			c = jis0208[c][d];
			if (!c) goto ilseq;
			break;
		case GB2312:
			if (c < 0xa1) goto ilseq;
		case GBK:
		case GB18030:
			c -= 0x81;
			if (c >= 126) goto ilseq;
			l = 2;
			if (*inb < 2) goto starved;
			d = *((unsigned char *)*in + 1);
			if (d < 0xa1 && type == GB2312) goto ilseq;
			if (d-0x40>=191 || d==127) {
				if (d-'0'>9 || type != GB18030)
					goto ilseq;
				l = 4;
				if (*inb < 4) goto starved;
				c = (10*c + d-'0') * 1260;
				d = *((unsigned char *)*in + 2);
				if (d-0x81>126) goto ilseq;
				c += 10*(d-0x81);
				d = *((unsigned char *)*in + 3);
				if (d-'0'>9) goto ilseq;
				c += d-'0';
				c += 128;
				for (d=0; d<=c; ) {
					k = 0;
					for (int i=0; i<126; i++)
						for (int j=0; j<190; j++)
							if (gb18030[i][j]-d <= c-d)
								k++;
					d = c+1;
					c += k;
				}
				break;
			}
			d -= 0x40;
			if (d>63) d--;
			c = gb18030[c][d];
			break;
		case BIG5:
			l = 2;
			if (*inb < 2) goto starved;
			d = *((unsigned char *)*in + 1);
			if (d-0x40>=0xff-0x40 || d-0x7f<0xa1-0x7f) goto ilseq;
			d -= 0x40;
			if (d > 0x3e) d -= 0x22;
			if (c-0xa1>=0xfa-0xa1) {
				if (c-0x87>=0xff-0x87) goto ilseq;
				if (c < 0xa1) c -= 0x87;
				else c -= 0x87 + (0xfa-0xa1);
				c = (hkscs[4867+(c*157+d)/16]>>(c*157+d)%16)%2<<17
					| hkscs[c*157+d];
				/* A few HKSCS characters map to pairs of UCS
				 * characters. These are mapped to surrogate
				 * range in the hkscs table then hard-coded
				 * here. Ugly, yes. */
				if (c/256 == 0xdc) {
					if (totype-0300U > 8) k = 2;
					else k = "\10\4\4\10\4\4\10\2\4"[totype-0300];
					if (k > *outb) goto toobig;
					x += iconv((iconv_t)(uintptr_t)to,
						&(char *){"\303\212\314\204"
						"\303\212\314\214"
						"\303\252\314\204"
						"\303\252\314\214"
						+c%256}, &(size_t){4},
						out, outb);
					continue;
				}
				if (!c) goto ilseq;
				break;
			}
			c -= 0xa1;
			c = big5[c][d]|(c==0x27&&(d==0x3a||d==0x3c||d==0x42))<<17;
			if (!c) goto ilseq;
			break;
		case EUC_KR:
			l = 2;
			if (*inb < 2) goto starved;
			d = *((unsigned char *)*in + 1);
			c -= 0xa1;
			d -= 0xa1;
			if (c >= 93 || d >= 94) {
				c += (0xa1-0x81);
				d += 0xa1;
				if (c >= 93 || c>=0xc6-0x81 && d>0x52)
					goto ilseq;
				if (d-'A'<26) d = d-'A';
				else if (d-'a'<26) d = d-'a'+26;
				else if (d-0x81<0xff-0x81) d = d-0x81+52;
				else goto ilseq;
				if (c < 0x20) c = 178*c + d;
				else c = 178*0x20 + 84*(c-0x20) + d;
				c += 0xac00;
				for (d=0xac00; d<=c; ) {
					k = 0;
					for (int i=0; i<93; i++)
						for (int j=0; j<94; j++)
							if (ksc[i][j]-d <= c-d)
								k++;
					d = c+1;
					c += k;
				}
				break;
			}
			c = ksc[c][d];
			if (!c) goto ilseq;
			break;
		default:
			if (c < 128+type) break;
			c -= 128+type;
			c = legacy_chars[ map[c*5/4]>>2*c%8 |
				map[c*5/4+1]<<8-2*c%8 & 1023 ];
			if (!c) c = *(unsigned char *)*in;
			if (c==1) goto ilseq;
		}

		switch (totype) {
		case WCHAR_T:
			if (*outb < sizeof(wchar_t)) goto toobig;
			*(wchar_t *)*out = c;
			*out += sizeof(wchar_t);
			*outb -= sizeof(wchar_t);
			break;
		case UTF_8:
			if (*outb < 4) {
				char tmp[4];
				k = wctomb_utf8(tmp, c);
				if (*outb < k) goto toobig;
				memcpy(*out, tmp, k);
			} else k = wctomb_utf8(*out, c);
			*out += k;
			*outb -= k;
			break;
		case US_ASCII:
			if (c > 0x7f) subst: x++, c='*';
		default:
			if (*outb < 1) goto toobig;
			if (c < 128+totype) {
			revout:
				*(*out)++ = c;
				*outb -= 1;
				break;
			}
			d = c;
			for (c=0; c<128-totype; c++) {
				if (d == legacy_chars[ tomap[c*5/4]>>2*c%8 |
					tomap[c*5/4+1]<<8-2*c%8 & 1023 ]) {
					c += 128;
					goto revout;
				}
			}
			goto subst;
		case UCS2BE:
		case UCS2LE:
		case UTF_16BE:
		case UTF_16LE:
			if (c < 0x10000 || type-UCS2BE < 2U) {
				if (c >= 0x10000) c = 0xFFFD;
				if (*outb < 2) goto toobig;
				put_16((void *)*out, c, totype);
				*out += 2;
				*outb -= 2;
				break;
			}
			if (*outb < 4) goto toobig;
			c -= 0x10000;
			put_16((void *)*out, (c>>10)|0xd800, totype);
			put_16((void *)(*out + 2), (c&0x3ff)|0xdc00, totype);
			*out += 4;
			*outb -= 4;
			break;
		case UTF_32BE:
		case UTF_32LE:
			if (*outb < 4) goto toobig;
			put_32((void *)*out, c, totype);
			*out += 4;
			*outb -= 4;
			break;
		}
	}
	return x;
ilseq:
	err = EILSEQ;
	x = -1;
	goto end;
toobig:
	err = E2BIG;
	x = -1;
	goto end;
starved:
	err = EINVAL;
	x = -1;
end:
	errno = err;
	return x;
}
