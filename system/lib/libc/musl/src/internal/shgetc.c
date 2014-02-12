#include "shgetc.h"

void __shlim(FILE *f, off_t lim)
{
	f->shlim = lim;
	f->shcnt = f->rend - f->rpos;
	if (lim && f->shcnt > lim)
		f->shend = f->rpos + lim;
	else
		f->shend = f->rend;
}

int __shgetc(FILE *f)
{
	int c;
	if (f->shlim && f->shcnt >= f->shlim || (c=__uflow(f)) < 0) {
		f->shend = 0;
		return EOF;
	}
	if (f->shlim && f->rend - f->rpos > f->shlim - f->shcnt - 1)
		f->shend = f->rpos + (f->shlim - f->shcnt - 1);
	else
		f->shend = f->rend;
	if (f->rend) f->shcnt += f->rend - f->rpos + 1;
	if (f->rpos[-1] != c) f->rpos[-1] = c;
	return c;
}
