#include "stdio_impl.h"
#include <errno.h>
#include <string.h>
#include <inttypes.h>

struct cookie {
	size_t pos, len, size;
	unsigned char *buf;
	int mode;
};

static off_t mseek(FILE *f, off_t off, int whence)
{
	ssize_t base;
	struct cookie *c = f->cookie;
	if (whence>2U) {
fail:
		errno = EINVAL;
		return -1;
	}
	base = (size_t [3]){0, c->pos, c->len}[whence];
	if (off < -base || off > (ssize_t)c->size-base) goto fail;
	return c->pos = base+off;
}

static size_t mread(FILE *f, unsigned char *buf, size_t len)
{
	struct cookie *c = f->cookie;
	size_t rem = c->len - c->pos;
	if (c->pos > c->len) rem = 0;
	if (len > rem) {
		len = rem;
		f->flags |= F_EOF;
	}
	memcpy(buf, c->buf+c->pos, len);
	c->pos += len;
	rem -= len;
	if (rem > f->buf_size) rem = f->buf_size;
	f->rpos = f->buf;
	f->rend = f->buf + rem;
	memcpy(f->rpos, c->buf+c->pos, rem);
	c->pos += rem;
	return len;
}

static size_t mwrite(FILE *f, const unsigned char *buf, size_t len)
{
	struct cookie *c = f->cookie;
	size_t rem;
	size_t len2 = f->wpos - f->wbase;
	if (len2) {
		f->wpos = f->wbase;
		if (mwrite(f, f->wpos, len2) < len2) return 0;
	}
	if (c->mode == 'a') c->pos = c->len;
	rem = c->size - c->pos;
	if (len > rem) len = rem;
	memcpy(c->buf+c->pos, buf, len);
	c->pos += len;
	if (c->pos > c->len) {
		c->len = c->pos;
		if (c->len < c->size) c->buf[c->len] = 0;
		else if ((f->flags&F_NORD) && c->size) c->buf[c->size-1] = 0;
	}
	return len;
}

static int mclose(FILE *m)
{
	return 0;
}

FILE *fmemopen(void *restrict buf, size_t size, const char *restrict mode)
{
	FILE *f;
	struct cookie *c;
	int plus = !!strchr(mode, '+');
	
	if (!size || !strchr("rwa", *mode)) {
		errno = EINVAL;
		return 0;
	}

	if (!buf && size > SIZE_MAX-sizeof(FILE)-BUFSIZ-UNGET) {
		errno = ENOMEM;
		return 0;
	}

	f = calloc(sizeof *f + sizeof *c + UNGET + BUFSIZ + (buf?0:size), 1);
	if (!f) return 0;
	f->cookie = c = (void *)(f+1);
	f->fd = -1;
	f->lbf = EOF;
	f->buf = (unsigned char *)(c+1) + UNGET;
	f->buf_size = BUFSIZ;
	if (!buf) buf = f->buf + BUFSIZ;

	c->buf = buf;
	c->size = size;
	c->mode = *mode;
	
	if (!plus) f->flags = (*mode == 'r') ? F_NOWR : F_NORD;
	if (*mode == 'r') c->len = size;
	else if (*mode == 'a') c->len = c->pos = strnlen(buf, size);

	f->read = mread;
	f->write = mwrite;
	f->seek = mseek;
	f->close = mclose;

	if (!libc.threaded) f->lock = -1;

	return __ofl_add(f);
}
