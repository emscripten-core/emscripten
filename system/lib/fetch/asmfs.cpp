#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#define __NEED_struct_iovec
#include <libc/bits/alltypes.h>
#include <assert.h>
#include <string.h>
#include <emscripten/emscripten.h>
#include <emscripten/fetch.h>
#include <math.h>

extern "C" {

#define EM_FILEDESCRIPTOR_MAGIC 0x64666d65U // 'emfd'
struct FileDescriptor
{
	uint32_t magic;
	emscripten_fetch_t *fetch;
	ssize_t file_pos;
};

// http://man7.org/linux/man-pages/man2/open.2.html
long __syscall5(int which, ...) // open
{
	va_list vl;
	va_start(vl, which);
	const char *pathname = va_arg(vl, const char*);
	int flags = va_arg(vl, int);
	int mode = va_arg(vl, int);
	va_end(vl);

	EM_ASM_INT( { Module['printErr']('__syscall5 OPEN, which: ' + $0 + ', pathname ' + Pointer_stringify($1) + ', ' + $2 + ', ' + $3 + '.') }, 
		which, pathname, flags, mode);

	emscripten_fetch_attr_t attr;
	emscripten_fetch_attr_init(&attr);
	strcpy(attr.requestMethod, "GET");
	attr.attributes = EMSCRIPTEN_FETCH_APPEND | EMSCRIPTEN_FETCH_LOAD_TO_MEMORY | EMSCRIPTEN_FETCH_WAITABLE | EMSCRIPTEN_FETCH_PERSIST_FILE;

	FileDescriptor *desc = (FileDescriptor*)malloc(sizeof(FileDescriptor));
	desc->magic = EM_FILEDESCRIPTOR_MAGIC;
	desc->fetch = emscripten_fetch(&attr, pathname);
	desc->file_pos = 0;

	return (long)desc;
}

// http://man7.org/linux/man-pages/man2/close.2.html
long __syscall6(int which, ...) // close
{
	va_list vl;
	va_start(vl, which);
	int fd = va_arg(vl, int);
	va_end(vl);
	EM_ASM_INT( { Module['printErr']('__syscall6 CLOSE, which: ' + $0 + ', fd: ' + $1 + '.') }, which, fd);

	FileDescriptor *desc = (FileDescriptor*)fd;
	if (!desc || desc->magic != EM_FILEDESCRIPTOR_MAGIC)
	{
		fprintf(stderr, "Invalid or already closed file descriptor 0x%8X passed to close()!", (unsigned int)desc);
		return -1; // TODO: set errno
	}
	if (desc->fetch)
	{
		emscripten_fetch_wait(desc->fetch, INFINITY); // TODO: This should not be necessary- test this out
		emscripten_fetch_close(desc->fetch);
		desc->fetch = 0;
	}
	desc->magic = 0;
	free(desc);
	return 0;
}

// http://man7.org/linux/man-pages/man2/sysctl.2.html
long __syscall54(int which, ...) // sysctl
{
	EM_ASM_INT( { Module['printErr']('__syscall54 SYSCTL, which: ' + $0 + '.') }, which);
	return 0;
}

// http://man7.org/linux/man-pages/man2/llseek.2.html
long __syscall140(int which, ...) // llseek
{
	va_list vl;
	va_start(vl, which);
	unsigned int fd = va_arg(vl, unsigned int);
	unsigned long offset_high = va_arg(vl, unsigned long);
	unsigned long offset_low = va_arg(vl, unsigned long);
	off_t *result = va_arg(vl, off_t *);
	unsigned int whence = va_arg(vl, unsigned int);
	va_end(vl);
	EM_ASM_INT( { Module['printErr']('__syscall140 LLSEEK, which: ' + $0 + ', fd ' + $1 + ', offset high ' + $2 + ' offset low ' + $3 + ' result ' + $4 + ' whence ' + $5 + '.') }, 
		which, fd, offset_high, offset_low, result, whence);

	FileDescriptor *desc = (FileDescriptor*)fd;
	if (!desc || desc->magic != EM_FILEDESCRIPTOR_MAGIC)
	{
		fprintf(stderr, "Invalid or closed file descriptor 0x%8X passed to close()!", (unsigned int)desc);
		return -1; // TODO: set errno
	}
	if (!desc->fetch)
	{
		fprintf(stderr, "Internal error: no file data available for fd 0x%8X!", (unsigned int)desc);
		return -1;
	}

	emscripten_fetch_wait(desc->fetch, INFINITY);

	uint64_t offset = ((uint64_t)offset_high << 32) | (uint64_t)offset_low;
	switch(whence)
	{
		case SEEK_SET: desc->file_pos = offset; break;
		case SEEK_CUR: desc->file_pos += offset; break;
		case SEEK_END: desc->file_pos = desc->fetch->numBytes - offset; break;
		default: return -1;
	}
	if (desc->file_pos < 0) desc->file_pos = 0;
	if (desc->file_pos > desc->fetch->numBytes) desc->file_pos = desc->fetch->numBytes;
	if (result) *result = desc->file_pos;
	return 0;
}

// http://man7.org/linux/man-pages/man2/readv.2.html
long __syscall145(int which, ...) // readv
{
	va_list vl;
	va_start(vl, which);
	int fd = va_arg(vl, int);
	const iovec *iov = va_arg(vl, const iovec*);
	int iovcnt = va_arg(vl, int);
	va_end(vl);
	EM_ASM_INT( { Module['printErr']('__syscall145 READV, which: ' + $0 + ', fd ' + $1 + ', iov: ' + $2 + ', iovcnt: ' + $3 + ' .') }, which, fd, iov, iovcnt);

	FileDescriptor *desc = (FileDescriptor*)fd;
	if (!desc || desc->magic != EM_FILEDESCRIPTOR_MAGIC)
	{
		fprintf(stderr, "Invalid or closed file descriptor 0x%8X passed to readv()!", (unsigned int)desc);
		return -1; // TODO: set errno
	}
	if (!desc->fetch)
	{
		fprintf(stderr, "Internal error: no file data available for fd 0x%8X!", (unsigned int)desc);
		return -1;
	}

	emscripten_fetch_wait(desc->fetch, INFINITY); // TODO: Ensure that multiple waits are ok.

	size_t offset = desc->file_pos;
	for(int i = 0; i < iovcnt; ++i)
	{
		ssize_t dataLeft = desc->fetch->numBytes - offset;
		if (dataLeft <= 0) break;
		size_t bytesToCopy = dataLeft < iov[i].iov_len ? dataLeft : iov[i].iov_len;
		memcpy(iov[i].iov_base, &desc->fetch->data[offset], bytesToCopy);
		offset += bytesToCopy;
	}
	ssize_t numRead = offset - desc->file_pos;
	desc->file_pos = offset;
	return numRead;
}

/*
// http://man7.org/linux/man-pages/man2/writev.2.html
long __syscall146(int which, ...) // writev
{
	EM_ASM_INT( { Module['printErr']('__syscall146 WRITEV, which: ' + $0 + '.') }, which);
	return 0;
}
*/

} // ~extern "C"
