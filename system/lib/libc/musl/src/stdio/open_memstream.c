#include "stdio_impl.h"
#include <errno.h>
#include <limits.h>
#include <string.h>

struct cookie {
	char **bufp;
	size_t *sizep;
	size_t pos;
	char *buf;
	size_t len;
	size_t space;
};

static off_t ms_seek(FILE *f, off_t off, int whence)
{
	ssize_t base;
	struct cookie *c = f->cookie;
	if (whence>2U) {
fail:
		errno = EINVAL;
		return -1;
	}
	base = (size_t [3]){0, c->pos, c->len}[whence];
	if (off < -base || off > SSIZE_MAX-base) goto fail;
	return c->pos = base+off;
}

static size_t ms_write(FILE *f, const unsigned char *buf, size_t len)
{
	struct cookie *c = f->cookie;
	size_t len2 = f->wpos - f->wbase;
	char *newbuf;
	if (len2) {
		f->wpos = f->wbase;
		if (ms_write(f, f->wbase, len2) < len2) return 0;
	}
	if (len + c->pos >= c->space) {
		len2 = 2*c->space+1 | c->pos+len+1;
		newbuf = realloc(c->buf, len2);
		if (!newbuf) return 0;
		*c->bufp = c->buf = newbuf;
		memset(c->buf + c->space, 0, len2 - c->space);
		c->space = len2;
	}
	memcpy(c->buf+c->pos, buf, len);
	c->pos += len;
	if (c->pos >= c->len) c->len = c->pos;
	*c->sizep = c->pos;
	return len;
}

static int ms_close(FILE *f)
{
	return 0;
}

FILE *open_memstream(char **bufp, size_t *sizep)
{
	FILE *f;
	struct cookie *c;
	if (!(f=malloc(sizeof *f + sizeof *c + BUFSIZ))) return 0;
	memset(f, 0, sizeof *f + sizeof *c);
	f->cookie = c = (void *)(f+1);

	c->bufp = bufp;
	c->sizep = sizep;
	c->pos = c->len = c->space = 0;
	c->buf = 0;

	f->flags = F_NORD;
	f->fd = -1;
	f->buf = (void *)(c+1);
	f->buf_size = BUFSIZ;
	f->lbf = EOF;
	f->write = ms_write;
	f->seek = ms_seek;
	f->close = ms_close;

	if (!libc.threaded) f->lock = -1;

	OFLLOCK();
	f->next = libc.ofl_head;
	if (libc.ofl_head) libc.ofl_head->prev = f;
	libc.ofl_head = f;
	OFLUNLOCK();

	return f;
}
