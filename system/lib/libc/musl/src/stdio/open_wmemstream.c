#include "stdio_impl.h"
#include <wchar.h>
#include <errno.h>
#include <limits.h>
#include <string.h>

struct cookie {
	wchar_t **bufp;
	size_t *sizep;
	size_t pos;
	wchar_t *buf;
	size_t len;
	size_t space;
	mbstate_t mbs;
};

static off_t wms_seek(FILE *f, off_t off, int whence)
{
	ssize_t base;
	struct cookie *c = f->cookie;
	if (whence>2U) {
fail:
		errno = EINVAL;
		return -1;
	}
	base = (size_t [3]){0, c->pos, c->len}[whence];
	if (off < -base || off > SSIZE_MAX/4-base) goto fail;
	memset(&c->mbs, 0, sizeof c->mbs);
	return c->pos = base+off;
}

static size_t wms_write(FILE *f, const unsigned char *buf, size_t len)
{
	struct cookie *c = f->cookie;
	size_t len2;
	wchar_t *newbuf;
	if (len + c->pos >= c->space) {
		len2 = 2*c->space+1 | c->pos+len+1;
		if (len2 > SSIZE_MAX/4) return 0;
		newbuf = realloc(c->buf, len2*4);
		if (!newbuf) return 0;
		*c->bufp = c->buf = newbuf;
		memset(c->buf + c->space, 0, 4*(len2 - c->space));
		c->space = len2;
	}
	
	len2 = mbsnrtowcs(c->buf+c->pos, (void *)&buf, len, c->space-c->pos, &c->mbs);
	if (len2 == -1) return 0;
	c->pos += len2;
	if (c->pos >= c->len) c->len = c->pos;
	*c->sizep = c->pos;
	return len;
}

static int wms_close(FILE *f)
{
	return 0;
}

FILE *open_wmemstream(wchar_t **bufp, size_t *sizep)
{
	FILE *f;
	struct cookie *c;
	if (!(f=malloc(sizeof *f + sizeof *c))) return 0;
	memset(f, 0, sizeof *f + sizeof *c);
	f->cookie = c = (void *)(f+1);

	c->bufp = bufp;
	c->sizep = sizep;
	c->pos = c->len = c->space = 0;
	c->buf = 0;

	f->flags = F_NORD;
	f->fd = -1;
	f->buf = (void *)(c+1);
	f->buf_size = 0;
	f->lbf = EOF;
	f->write = wms_write;
	f->seek = wms_seek;
	f->close = wms_close;

	if (!libc.threaded) f->lock = -1;

	OFLLOCK();
	f->next = libc.ofl_head;
	if (libc.ofl_head) libc.ofl_head->prev = f;
	libc.ofl_head = f;
	OFLUNLOCK();

	return f;
}
