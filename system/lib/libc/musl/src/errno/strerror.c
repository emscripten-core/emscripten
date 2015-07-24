#include <errno.h>
#include <string.h>

#define E(a,b) ((unsigned char)a),
static const unsigned char errid[] = {
#include "__strerror.h"
};

#undef E
#define E(a,b) b "\0"
static const char errmsg[] =
#include "__strerror.h"
;

char *strerror(int e)
{
	const char *s;
	int i;
	/* mips has one error code outside of the 8-bit range due to a
	 * historical typo, so we just remap it. */
	if (EDQUOT==1133) {
		if (e==109) e=-1;
		else if (e==EDQUOT) e=109;
	}
	for (i=0; errid[i] && errid[i] != e; i++);
	for (s=errmsg; i; s++, i--) for (; *s; s++);
	return (char *)s;
}
