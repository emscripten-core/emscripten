#include "stdio_impl.h"

/* This function makes no attempt to protect the user from his/her own
 * stupidity. If called any time but when then ISO C standard specifically
 * allows it, all hell can and will break loose, especially with threads!
 *
 * This implementation ignores all arguments except the buffering type,
 * and uses the existing buffer allocated alongside the FILE object.
 * In the case of stderr where the preexisting buffer is length 1, it
 * is not possible to set line buffering or full buffering. */

int setvbuf(FILE *restrict f, char *restrict buf, int type, size_t size)
{
	f->lbf = EOF;

	if (type == _IONBF)
		f->buf_size = 0;
	else if (type == _IOLBF)
		f->lbf = '\n';

	f->flags |= F_SVB;

	return 0;
}
