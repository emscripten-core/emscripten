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
#include <libc/fcntl.h>
#include <errno.h>

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

	int accessMode = (flags & O_ACCMODE);
	if (accessMode != O_RDONLY) // O_WRONLY or O_RDWR
	{
		EM_ASM(Module['printErr']('open() syscall failed! pathname refers to a file on a read-only filesystem and write access was requested.'));
		errno = EROFS; // "pathname refers to a file on a read-only filesystem and write access was requested."
		return -1;
	}

	if ((flags & O_ASYNC))
	{
		EM_ASM(Module['printErr']('open() syscall failed! Opening files with O_ASYNC flag is not supported in ASMFS (TODO?)'));
		return -1;
	}

	// The flags:O_CLOEXEC flag is ignored, doesn't have meaning for Emscripten

	if ((flags & O_CREAT))
	{
		EM_ASM(Module['printErr']('open() syscall failed! Opening files with O_CREAT flag is not yet supported in ASMFS (TODO)'));
		return -1;
	}

	// TODO: the flags:O_DIRECT flag seems like a great way to let applications explicitly control XHR/IndexedDB read/write buffering behavior?
	if ((flags & O_DIRECT))
	{
		EM_ASM(Module['printErr']('open() syscall failed! The filesystem does not support the O_DIRECT flag.'));
		errno = EINVAL; // "The filesystem does not support the O_DIRECT flag."
		return -1;
	}

	if ((flags & O_DIRECTORY))
	{
		EM_ASM(Module['printErr']('open() syscall failed! Opening directories with O_DIRECTORY flag is not yet supported in ASMFS (TODO)'));
		return -1;
	}

	if ((flags & O_DSYNC))
	{
		EM_ASM(Module['printErr']('open() syscall failed! Opening files with O_DSYNC flag is not yet supported in ASMFS (TODO)'));
		return -1;
	}

	if ((flags & O_EXCL))
	{
		EM_ASM(Module['printErr']('open() syscall failed! Opening files with O_EXCL flag is not yet supported in ASMFS (TODO)'));

		// TODO: Check if the file exists and if so, return EEXIST error (the file should not exist, but with O_EXCL should always create a new one)
		return -1;
	}

	// The flags:O_LARGEFILE flag is ignored, we should always be largefile-compatible

	// TODO: The flags:O_NOATIME is ignored, file access times have not been implemented yet
	// The flags O_NOCTTY, O_NOFOLLOW

	if ((flags & (O_NONBLOCK|O_NDELAY)))
	{
		EM_ASM(Module['printErr']('open() syscall failed! Opening files with O_NONBLOCK or O_NDELAY flags is not yet supported in ASMFS (TODO)'));
		return -1;
	}

	if ((flags & O_PATH))
	{
		EM_ASM(Module['printErr']('open() syscall failed! Opening files with O_PATH flags is not yet supported in ASMFS (TODO)'));
		return -1;
	}

	if ((flags & O_SYNC))
	{
		EM_ASM(Module['printErr']('open() syscall failed! Opening files with O_SYNC flags is not yet supported in ASMFS (TODO)'));
		return -1;
	}

	if ((flags & O_TMPFILE))
	{
		if (accessMode != O_WRONLY && accessMode != O_RDWR)
		{
			EM_ASM(Module['printErr']('open() syscall failed! O_TMPFILE was specified in flags, but neither O_WRONLY nor O_RDWR was specified.'));
			errno = EINVAL; // "O_TMPFILE was specified in flags, but neither O_WRONLY nor O_RDWR was specified."
			return -1;
		}
		EM_ASM(Module['printErr']('open() syscall failed! "The filesystem containing pathname does not support O_TMPFILE.'));
		errno = EOPNOTSUPP; // "The filesystem containing pathname does not support O_TMPFILE."
		return -1;
	}

	if ((flags & O_TRUNC))
	{
		EM_ASM(Module['printErr']('open() syscall failed! Opening files with O_TRUNC flags is not yet supported in ASMFS (TODO)'));
		return -1;
	}

	/* TODO:
	if (is_directory and (accessMode == O_WRONLY || accessMode == O_RDWR))
	{
		errno = EISDIR; // "pathname refers to a directory and the access requested involved writing (that is, O_WRONLY or O_RDWR is set)."
		return -1;
	}
	*/

	/* TODO:
	if (too_many_files_open)
	{
		errno = EMFILE; // "The per-process limit on the number of open file descriptors has been reached, see getrlimit(RLIMIT_NOFILE)"
		return -1;
	}
	*/

	// http://stackoverflow.com/questions/417142/what-is-the-maximum-length-of-a-url-in-different-browsers
	if (strlen(pathname) > 2000)
	{
		errno = ENAMETOOLONG;
		EM_ASM(Module['printErr']('open() syscall failed! The URL to open was more than 2000 characters long'));
		return -1;
	}

	emscripten_fetch_attr_t attr;
	emscripten_fetch_attr_init(&attr);
	strcpy(attr.requestMethod, "GET");
	attr.attributes = EMSCRIPTEN_FETCH_APPEND | EMSCRIPTEN_FETCH_LOAD_TO_MEMORY | EMSCRIPTEN_FETCH_WAITABLE | EMSCRIPTEN_FETCH_PERSIST_FILE;

	FileDescriptor *desc = (FileDescriptor*)malloc(sizeof(FileDescriptor));
	desc->magic = EM_FILEDESCRIPTOR_MAGIC;
	desc->fetch = emscripten_fetch(&attr, pathname);

	// switch(fopen_mode)
	// {
	// case synchronous_fopen:
		emscripten_fetch_wait(desc->fetch, INFINITY);

		if (desc->fetch->status != 200 || desc->fetch->totalBytes == 0)
		{
			EM_ASM_INT( { Module['printErr']('__syscall5 OPEN failed! File ' + Pointer_stringify($0) + ' does not exist: XHR returned status code ' + $1 + ', and file length was ' + $2 + '.') }, 
				pathname, (int)desc->fetch->status, (int)desc->fetch->totalBytes);
			emscripten_fetch_close(desc->fetch);
			errno = ENOENT;
			return -1;
		}

		desc->file_pos = (flags & O_APPEND) ? desc->fetch->totalBytes : 0;

	//  break;
	// case asynchronous_fopen:
	//  break;
	// }

	// TODO: The file descriptor needs to be a small number, man page:
	// "a small, nonnegative integer for use in subsequent system calls
	// (read(2), write(2), lseek(2), fcntl(2), etc.).  The file descriptor
	// returned by a successful call will be the lowest-numbered file
	// descriptor not currently open for the process."
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
		errno = EBADF; // "fd isn't a valid open file descriptor."
		return -1;
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
// also useful: http://man7.org/linux/man-pages/man2/lseek.2.html
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
		errno = EBADF; // "fd isn't a valid open file descriptor."
		return -1;
	}
	if (!desc->fetch)
	{
		fprintf(stderr, "Internal error: no file data available for fd 0x%8X!", (unsigned int)desc);
		return -1;
	}

	emscripten_fetch_wait(desc->fetch, INFINITY);

	int64_t offset = (int64_t)(((uint64_t)offset_high << 32) | (uint64_t)offset_low);
	int64_t newPos;
	switch(whence)
	{
		case SEEK_SET: newPos = offset; break;
		case SEEK_CUR: newPos = desc->file_pos + offset; break;
		case SEEK_END: newPos = desc->fetch->numBytes + offset; break;
		default:
			errno = EINVAL; // "whence is invalid."
			return -1;
	}
	if (newPos < 0)
	{
		errno = EINVAL; // "the resulting file offset would be negative"
		return -1;
	}
	if (newPos > 0x7FFFFFFFLL)
	{
		errno = EOVERFLOW; // "The resulting file offset cannot be represented in an off_t."
		EM_ASM_INT( { Module['printErr']('llseek EOVERFLOW error: fd ' + $0 + 'attempted to seek past unsupported 2^31-1 file size limit (TODO?).') },
			fd);
		return -1;
	}
	desc->file_pos = newPos;

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
		errno = EBADF; // "fd is not a valid file descriptor or is not open for reading."
		return -1;
	}
	if (!desc->fetch)
	{
		fprintf(stderr, "Internal error: no file data available for fd 0x%8X!", (unsigned int)desc);
		return -1;
	}

	// TODO: Test and detect to return EISDIR.

	// TODO: Support nonblocking IO and check for EAGAIN/EWOULDBLOCK
	emscripten_fetch_wait(desc->fetch, INFINITY); // TODO: Ensure that multiple waits are ok.

	if (iovcnt < 0)
	{
		errno = EINVAL; // "The vector count, iovcnt, is less than zero or greater than the permitted maximum."
		return -1;
	}

	ssize_t total_read_amount = 0;
	for(int i = 0; i < iovcnt; ++i)
	{
		ssize_t n = total_read_amount + iov[i].iov_len;
		if (n < total_read_amount || !iov[i].iov_base)
		{
			errno = EINVAL; // "The sum of the iov_len values overflows an ssize_t value." or "the address specified in buf is not valid"
			return -1;
		}
		total_read_amount = n;
	}

	size_t offset = desc->file_pos;
	for(int i = 0; i < iovcnt; ++i)
	{
		ssize_t dataLeft = desc->fetch->numBytes - offset;
		if (dataLeft <= 0) break;
		size_t bytesToCopy = (size_t)dataLeft < iov[i].iov_len ? dataLeft : iov[i].iov_len;
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
