#include "stdio_impl.h"

int fclose(FILE *f)
{
	int r;
	int perm;
	
	FFINALLOCK(f);

	if (!(perm = f->flags & F_PERM)) {
		OFLLOCK();
		if (f->prev) f->prev->next = f->next;
		if (f->next) f->next->prev = f->prev;
		if (libc.ofl_head == f) libc.ofl_head = f->next;
		OFLUNLOCK();
	}

	r = fflush(f);
	r |= f->close(f);

	if (f->getln_buf) free(f->getln_buf);
	if (!perm) free(f);
	
	return r;
}
