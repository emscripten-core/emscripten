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
#include <time.h>

extern "C" {

#define INODE_TYPE uint32_t
#define INODE_FILE 1
#define INODE_DIR  2

struct inode
{
	char name[NAME_MAX+1]; // NAME_MAX actual bytes + one byte for null termination.
	inode *parent; // ID of the parent node
	inode *sibling; // ID of a sibling node (these form a singular linked list that specifies the content under a directory)
	inode *child; // ID of the first child node in a chain of children (the root of a linked list of inodes)
	uint32_t uid; // User ID of the owner
	uint32_t gid; // Group ID of the owning group
	uint32_t mode; // r/w/x modes
	time_t ctime; // Time when the inode was last modified
	time_t mtime; // Time when the content was last modified
	time_t atime; // Time when the content was last accessed
	size_t size; // Size of the file in bytes
	size_t capacity; // Amount of bytes allocated to pointer data
	uint8_t *data; // The actual file contents.

	INODE_TYPE type;

	emscripten_fetch_t *fetch;
};

#define EM_FILEDESCRIPTOR_MAGIC 0x64666d65U // 'emfd'
struct FileDescriptor
{
	uint32_t magic;
	ssize_t file_pos;
	uint32_t mode;
	uint32_t flags;

	inode *node;
};

static inode *create_inode(INODE_TYPE type)
{
	inode *i = (inode*)malloc(sizeof(inode));
	memset(i, 0, sizeof(inode));
	i->ctime = i->mtime = i->atime = time(0);
	i->type = type;
	EM_ASM(Module['print']('create_inode allocated new inode object.'));
	return i;
}

// The current working directory of the application process.
static inode *cwd_inode = 0;

static inode *filesystem_root()
{
	static inode *root_node = create_inode(INODE_DIR);
	return root_node;
}

static inode *get_cwd()
{
	if (!cwd_inode) cwd_inode = filesystem_root();
	return cwd_inode;
}

static void set_cwd(inode *node)
{
	cwd_inode = node;
}

static void delete_inode(inode *node)
{
	free(node);
}

// Makes node the child of parent.
static void link_inode(inode *node, inode *parent)
{
	EM_ASM_INT( { Module['printErr']('link_inode: node ' + Pointer_stringify($0) + ' to parent ' + Pointer_stringify($1) + '.') }, 
		node->name, parent->name);
	// When linking a node, it can't be part of the filesystem tree (but it can have children of its own)
	assert(!node->parent);
	assert(!node->sibling);

	// The inode pointed by 'node' is not yet part of the filesystem, so it's not shared memory and only this thread
	// is accessing it. Therefore setting the node's parent here is not yet racy, do that operation first.
	node->parent = parent;

	// This node is to become the first child of the parent, and the old first child of the parent should
	// become the sibling of this node, i.e.
	//  1) node->sibling = parent->child;
	//  2) parent->child = node;
	// However these two operations need to occur atomically in order to be coherent. To ensure that, run the two
	// operations in a CAS loop, which is possible because the first operation is not racy until the node is 'published'
	// to the filesystem tree by the compare_exchange operation.
	do { __atomic_load(&parent->child, &node->sibling, __ATOMIC_SEQ_CST); // node->sibling <- parent->child
	} while (!__atomic_compare_exchange(&parent->child, &node->sibling, &node, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)); // parent->child <- node if it had not raced to change value in between
}

// Traverse back in sibling linked list, or 0 if no such node exist.
static inode *find_predecessor_sibling(inode *node, inode *parent)
{
	inode *child = parent->child;
	if (child == node)
		return 0;
	while(child && child->sibling != node)
		child = child->sibling;
	if (!child->sibling) return 0;
	return child;
}

static void unlink_inode(inode *node)
{
	EM_ASM_INT( { Module['printErr']('unlink_inode: node ' + Pointer_stringify($0) + ' from its parent ' + Pointer_stringify($1) + '.') }, 
		node->name, node->parent->name);
	inode *parent = node->parent;
	if (!parent) return;
	node->parent = 0;

	if (parent->child == node)
	{
		parent->child = node->sibling;
	}
	else
	{
		inode *predecessor = find_predecessor_sibling(node, parent);
		if (predecessor)
			predecessor->sibling = node->sibling;
	}
	node->parent = node->sibling = 0;
}

// Compares two strings for equality until a '\0' or a '/' is hit. Returns 0 if the strings differ,
// or a pointer to the beginning of the next directory component name of s1 if the strings are equal.
static const char *path_cmp(const char *s1, const char *s2)
{
	while(*s1 == *s2)
	{
		if (*s1 == '/') return s1+1;
		if (*s1 == '\0') return s1;
		++s1;
		++s2;
	}
	if (*s1 == '/' && *s2 == '\0') return s1+1;
	if (*s1 == '\0' && *s2 == '/') return s1;
	return 0;
}

// Copies string 'path' to 'dst', but stops on the first forward slash '/' character.
// Returns number of bytes written, excluding null terminator
static int strcpy_inodename(char *dst, const char *path)
{
	char *d = dst;
	while(*path && *path != '/')
		*dst++ = *path++;
	*dst = '\0';
	return dst - d;
}

// Returns a pointer to the basename part of the string, i.e. the string after the last occurrence of a forward slash character
static const char *basename_part(const char *path)
{
	const char *s = path;
	while(*path)
	{
		if (*path == '/') s = path+1;
		++path;
	}
	return s;
}

static inode *create_directory_hierarchy_for_file(inode *root, const char *path_to_file)
{
	assert(root);
	if (!root) return 0;

	inode *node = root->child;
	while(node)
	{
		const char *child_path = path_cmp(path_to_file, node->name);
		EM_ASM_INT( { Module['printErr']('path_cmp ' + Pointer_stringify($0) + ', ' + Pointer_stringify($1) + ', ' + Pointer_stringify($2) + ' .') }, path_to_file, node->name, child_path);
		if (child_path)
		{
			// The directory name matches.
			path_to_file = child_path;
			if (path_to_file[0] == '\0') return node;
			if (path_to_file[0] == '/' && path_to_file[1] == '\0' /* && node is a directory*/) return node;
			root = node;
			node = node->child;
		}
		else
		{
			node = node->sibling;
		}
	}
	EM_ASM_INT( { Module['printErr']('path_to_file ' + Pointer_stringify($0) + ' .') }, path_to_file);
	const char *basename_pos = basename_part(path_to_file);
	EM_ASM_INT( { Module['printErr']('basename_pos ' + Pointer_stringify($0) + ' .') }, basename_pos);
	while(*path_to_file && path_to_file < basename_pos)
	{
		node = create_inode(INODE_DIR);
		path_to_file += strcpy_inodename(node->name, path_to_file) + 1;
		link_inode(node, root);
		EM_ASM_INT( { Module['print']('create_directory_hierarchy_for_file: created directory ' + Pointer_stringify($0) + ' under parent ' + Pointer_stringify($1) + '.') }, 
			node->name, node->parent->name);
		root = node;
	}
	return root;
}

// Given a path to a file, finds the inode of the parent directory that contains the file, or 0 if the intermediate path doesn't exist.
static inode *find_parent_inode(inode *root, const char *path)
{
	if (!root) return 0;
	const char *basename = basename_part(path);
	inode *node = root->child;
	while(node)
	{
		const char *child_path = path_cmp(path, node->name);
		if (child_path)
		{
			// The directory name matches.
			path = child_path;
			if (path >= basename) return node;
			if (!*path) return 0;
			node = node->child;
		}
		else
		{
			node = node->sibling;
		}
	}
	return 0;
}

// Given a root inode of the filesystem and a path relative to it, e.g. "some/directory/dir_or_file",
// returns the inode that corresponds to "dir_or_file", or 0 if it doesn't exist.
// If the parameter out_closest_parent is specified, the closest (grand)parent node will be returned.
static inode *find_inode(inode *root, const char *path, inode **out_closest_parent = 0)
{
	if (out_closest_parent) *out_closest_parent = root;
	if (!root)
	{
		return 0;
	}

	inode *node = root->child;
	while(node)
	{
		const char *child_path = path_cmp(path, node->name);
		if (child_path)
		{
			// The directory name matches.
			path = child_path;
			if (path[0] == '\0') return node;
			if (path[0] == '/' && path[1] == '\0' /* && node is a directory*/) return node;
			if (out_closest_parent) *out_closest_parent = node;
			node = node->child;
		}
		else
		{
			node = node->sibling;
		}
	}
	return 0;
}

// Debug function that dumps out the filesystem tree to console.
void emscripten_dump_fs_tree(inode *root, char *path)
{
	printf("%s:\n", path);
	// Print out:
	// file mode | number of links | owner name | group name | file size in bytes | file last modified time | path name
	// which aligns with "ls -AFTRl" on console
	inode *child = root->child;
	uint64_t totalSize = 0;
	while(child)
	{
		printf("%c%c%c%c%c%c%c%c%c%c  %d user%u group%u %u Jan 1 1970 %s%c\n",
			child->type == INODE_DIR ? 'd' : '-',
			(child->mode & S_IRUSR) ? 'r' : '-',
			(child->mode & S_IWUSR) ? 'w' : '-',
			(child->mode & S_IXUSR) ? 'x' : '-',
			(child->mode & S_IRGRP) ? 'r' : '-',
			(child->mode & S_IWGRP) ? 'w' : '-',
			(child->mode & S_IXGRP) ? 'x' : '-',
			(child->mode & S_IROTH) ? 'r' : '-',
			(child->mode & S_IWOTH) ? 'w' : '-',
			(child->mode & S_IXOTH) ? 'x' : '-',
			1, // number of links to this file
			child->uid,
			child->gid,
			child->size,
			child->name,
			child->type == INODE_DIR ? '/' : ' ');
		totalSize += child->size;
		child = child->sibling;
	}
	printf("total %llu bytes\n\n", totalSize);

	child = root->child;
	char *path_end = path + strlen(path);
	while(child)
	{
		if (child->type == INODE_DIR)
		{
			strcpy(path_end, child->name);
			strcat(path_end, "/");
			emscripten_dump_fs_tree(child, path);
		}
		child = child->sibling;
	}
}

void emscripten_dump_fs_root()
{
	char path[PATH_MAX] = "/";
	emscripten_dump_fs_tree(filesystem_root(), path);
}

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
	/*
	if (accessMode != O_RDONLY) // O_WRONLY or O_RDWR
	{
		EM_ASM(Module['printErr']('open() syscall failed! pathname refers to a file on a read-only filesystem and write access was requested.'));
		errno = EROFS; // "pathname refers to a file on a read-only filesystem and write access was requested."
		return -1;
	}
	*/

	if ((flags & O_ASYNC))
	{
		EM_ASM(Module['printErr']('open() syscall failed! Opening files with O_ASYNC flag is not supported in ASMFS (TODO?)'));
		return -1;
	}

	// The flags:O_CLOEXEC flag is ignored, doesn't have meaning for Emscripten
/*
	if ((flags & O_CREAT))
	{
		EM_ASM(Module['printErr']('open() syscall failed! Opening files with O_CREAT flag is not yet supported in ASMFS (TODO)'));
		return -1;
	}
*/
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

	if ((flags & O_EXCL) && !(flags & O_CREAT))
	{
		EM_ASM(Module['printErr']('open() syscall failed! Opening files with O_EXCL flag needs to always be paired with O_CREAT'));
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
/*
	if ((flags & O_TRUNC))
	{
		EM_ASM(Module['printErr']('open() syscall failed! Opening files with O_TRUNC flags is not yet supported in ASMFS (TODO)'));
		return -1;
	}
*/
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

	// Find if this file exists already in the filesystem?
	inode *root;
	if (pathname[0] == '/')
	{
		root = filesystem_root();
		++pathname;
	}
	else
		root = get_cwd();

	inode *grandparent = 0;
	inode *node = find_inode(root, pathname, &grandparent);
	if (node)
	{
		if ((flags & O_CREAT) && (flags & O_EXCL))
		{
			EM_ASM_INT( { Module['printErr']('__syscall5 OPEN: pathname ' + Pointer_stringify($0) + ', inode open failed because file exists and open flags had O_CREAT and O_EXCL') }, 
				pathname);
			errno = EEXIST;
			return -1;
		}
		EM_ASM_INT( { Module['print']('__syscall5 OPEN: pathname ' + Pointer_stringify($0) + ', inode exists. data ptr: ' + $1 + ', data size: ' + $2 + ', fetch ptr: ' + $3) }, 
			pathname, node->data, node->size, node->fetch);
	}

	if (node && node->fetch) emscripten_fetch_wait(node->fetch, INFINITY);

	if ((flags & O_CREAT) || (flags & O_TRUNC) || (flags & O_EXCL))
	{
		// Create a new empty file or truncate existing one.
		if (node)
		{
			if (node->fetch)
			{
				emscripten_fetch_close(node->fetch);
				node->fetch = 0;
			}
			node->size = 0;
		}
		else
		{
			inode *directory = create_directory_hierarchy_for_file(root, pathname);
			node = create_inode(INODE_FILE);
			strcpy(node->name, basename_part(pathname));
			link_inode(node, directory);
			printf("Created file %s in directory %s\n", node->name, directory->name);
		}

		emscripten_dump_fs_root();
	}
	else if (!node || (!node->fetch && !node->data))
	{
		// If not, we'll need to fetch it.
		emscripten_fetch_attr_t attr;
		emscripten_fetch_attr_init(&attr);
		strcpy(attr.requestMethod, "GET");
		attr.attributes = EMSCRIPTEN_FETCH_APPEND | EMSCRIPTEN_FETCH_LOAD_TO_MEMORY | EMSCRIPTEN_FETCH_WAITABLE | EMSCRIPTEN_FETCH_PERSIST_FILE;
		emscripten_fetch_t *fetch = emscripten_fetch(&attr, pathname);

	// switch(fopen_mode)
	// {
	// case synchronous_fopen:
		emscripten_fetch_wait(fetch, INFINITY);

		if (fetch->status != 200 || fetch->totalBytes == 0)
		{
			EM_ASM_INT( { Module['printErr']('__syscall5 OPEN failed! File ' + Pointer_stringify($0) + ' does not exist: XHR returned status code ' + $1 + ', and file length was ' + $2 + '.') }, 
				pathname, (int)fetch->status, (int)fetch->totalBytes);
			emscripten_fetch_close(fetch);
			errno = ENOENT;
			return -1;
		}
	//  break;
	// case asynchronous_fopen:
	//  break;
	// }

		if (node)
		{
			node->fetch = fetch;
		}
		else
		{
			inode *directory = create_directory_hierarchy_for_file(root, pathname);
			node = create_inode(INODE_FILE);
			strcpy(node->name, basename_part(pathname));
			node->fetch = fetch;
			link_inode(node, directory);
			printf("Created file %s in directory %s\n", node->name, directory->name);
		}
		node->size = node->fetch->totalBytes;
		emscripten_dump_fs_root();
	}

	FileDescriptor *desc = (FileDescriptor*)malloc(sizeof(FileDescriptor));
	desc->magic = EM_FILEDESCRIPTOR_MAGIC;
	desc->node = node;
	desc->file_pos = (flags & O_APPEND) ? node->fetch->totalBytes : 0;
	desc->mode = mode;
	desc->flags = flags;

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
	if (desc->node && desc->node->fetch)
	{
		emscripten_fetch_wait(desc->node->fetch, INFINITY); // TODO: This should not be necessary- test this out
		emscripten_fetch_close(desc->node->fetch);
		desc->node->fetch = 0;
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
	if (!desc->node->fetch)
	{
		fprintf(stderr, "Internal error: no file data available for fd 0x%8X!", (unsigned int)desc);
		return -1;
	}

	emscripten_fetch_wait(desc->node->fetch, INFINITY);

	int64_t offset = (int64_t)(((uint64_t)offset_high << 32) | (uint64_t)offset_low);
	int64_t newPos;
	switch(whence)
	{
		case SEEK_SET: newPos = offset; break;
		case SEEK_CUR: newPos = desc->file_pos + offset; break;
		case SEEK_END: newPos = desc->node->fetch->numBytes + offset; break;
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
		EM_ASM_INT( { Module['printErr']('Invalid or closed file descriptor ' + $0 + ' passed to readv!') }, fd);
		errno = EBADF; // "fd is not a valid file descriptor or is not open for reading."
		return -1;
	}

	// TODO: Test and detect to return EISDIR.

	// TODO: Support nonblocking IO and check for EAGAIN/EWOULDBLOCK
	if (desc->node->fetch) emscripten_fetch_wait(desc->node->fetch, INFINITY);

	if (iovcnt < 0)
	{
		errno = EINVAL; // "The vector count, iovcnt, is less than zero or greater than the permitted maximum."
		return -1;
	}

	ssize_t total_read_amount = 0;
	for(int i = 0; i < iovcnt; ++i)
	{
		ssize_t n = total_read_amount + iov[i].iov_len;
		if (n < total_read_amount || (!iov[i].iov_base && iov[i].iov_len > 0))
		{
			errno = EINVAL; // "The sum of the iov_len values overflows an ssize_t value." or "the address specified in buf is not valid"
			return -1;
		}
		total_read_amount = n;
	}

	size_t offset = desc->file_pos;
	inode *node = desc->node;
	uint8_t *data = node->data ? node->data : (uint8_t *)node->fetch->data;
	for(int i = 0; i < iovcnt; ++i)
	{
		ssize_t dataLeft = node->size - offset;
		if (dataLeft <= 0) break;
		size_t bytesToCopy = (size_t)dataLeft < iov[i].iov_len ? dataLeft : iov[i].iov_len;
		memcpy(iov[i].iov_base, &data[offset], bytesToCopy);
		offset += bytesToCopy;
	}
	ssize_t numRead = offset - desc->file_pos;
	desc->file_pos = offset;
	return numRead;
}

static char stdout_buffer[4096] = {};
static int stdout_buffer_end = 0;
static char stderr_buffer[4096] = {};
static int stderr_buffer_end = 0;

static void print_stream(void *bytes, int numBytes, bool stdout)
{
	char *buffer = stdout ? stdout_buffer : stderr_buffer;
	int &buffer_end = stdout ? stdout_buffer_end : stderr_buffer_end;

	memcpy(buffer + buffer_end, bytes, numBytes);
	buffer_end += numBytes;
	int new_buffer_start = 0;
	for(int i = 0; i < buffer_end; ++i)
	{
		if (buffer[i] == '\n')
		{
			buffer[i] = 0;
			EM_ASM_INT( { Module['print'](Pointer_stringify($0)) }, buffer+new_buffer_start);
			new_buffer_start = i+1;
		}
	}
	size_t new_buffer_size = buffer_end - new_buffer_start;
	memmove(buffer, buffer + new_buffer_start, new_buffer_size);
	buffer_end = new_buffer_size;
}

// http://man7.org/linux/man-pages/man2/writev.2.html
long __syscall146(int which, ...) // writev
{
	va_list vl;
	va_start(vl, which);
	int fd = va_arg(vl, int);
	const iovec *iov = va_arg(vl, const iovec*);
	int iovcnt = va_arg(vl, int);
	va_end(vl);

	FileDescriptor *desc = (FileDescriptor*)fd;
	if (fd != 1/*stdout*/ && fd != 2/*stderr*/)
	{
//		EM_ASM_INT( { Module['printErr']('__syscall146 WRITEV, which: ' + $0 + ', fd ' + $1 + ', iov: ' + $2 + ', iovcnt: ' + $3 + ' .') }, which, fd, iov, iovcnt);
		if (!desc || desc->magic != EM_FILEDESCRIPTOR_MAGIC)
		{
			EM_ASM_INT( { Module['printErr']('Invalid or closed file descriptor ' + $0 + ' passed to writev!') }, fd);
			errno = EBADF; // "fd is not a valid file descriptor or is not open for reading."
			return -1;
		}
	}

	if (iovcnt < 0)
	{
		errno = EINVAL; // "The vector count, iovcnt, is less than zero or greater than the permitted maximum."
		return -1;
	}

	ssize_t total_write_amount = 0;
	for(int i = 0; i < iovcnt; ++i)
	{
		ssize_t n = total_write_amount + iov[i].iov_len;
		if (n < total_write_amount || (!iov[i].iov_base && iov[i].iov_len > 0))
		{
			errno = EINVAL; // "The sum of the iov_len values overflows an ssize_t value." or "the address specified in buf is not valid"
			return -1;
		}
		total_write_amount = n;
	}
//	EM_ASM_INT( { Module['printErr']('__syscall146 WRITEV, write amount to fd ' + $0 + ' is ' + $1 + '.') }, fd, total_write_amount);

	if (fd == 1/*stdout*/ || fd == 2/*stderr*/)
	{
//		EM_ASM_INT( { Module['printErr']('__syscall146 WRITEV, printing to stdout/stderr: ' + $0 + ' .') }, fd);
		ssize_t bytesWritten = 0;
		for(int i = 0; i < iovcnt; ++i)
		{
			print_stream(iov[i].iov_base, iov[i].iov_len, fd == 1);
			bytesWritten += iov[i].iov_len;
		}
		return bytesWritten;
	}
	else
	{
//		EM_ASM_INT( { Module['printErr']('__syscall146 WRITEV, printing to file data ptr: ' + $0 + ' .') }, fd);
		// Enlarge the file in memory to fit space for the new data
		size_t newSize = desc->file_pos + total_write_amount;
		inode *node = desc->node;
		if (node->capacity < newSize)
		{
//			EM_ASM_INT( { Module['printErr']('__syscall146 WRITEV, enlarging data ptr: ' + $0 + ' -> ' + $1 + ' .') }, node->capacity, newSize);
			size_t newCapacity = (newSize > (size_t)(node->capacity*1.25) ? newSize : (size_t)(node->capacity*1.25)); // Geometric increases in size for amortized O(1) behavior
			uint8_t *newData = (uint8_t *)realloc(node->data, newCapacity);
			if (!newData)
			{
				newData = (uint8_t *)malloc(newCapacity);
				memcpy(newData, node->data, node->size);
				// TODO: init gaps with zeroes.
				free(node->data);
			}
			node->data = newData;
			node->size = newSize;
			node->capacity = newCapacity;
		}

		for(int i = 0; i < iovcnt; ++i)
		{
			memcpy((uint8_t*)node->data + desc->file_pos, iov[i].iov_base, iov[i].iov_len);
			desc->file_pos += iov[i].iov_len;
		}
//		EM_ASM_INT( { Module['print']('__syscall5 WRITE: wrote pathname ' + Pointer_stringify($0) + ', inode exists. data ptr: ' + $1 + ', data size: ' + $2 + ', fetch ptr: ' + $3) }, 
//			node->name, node->data, node->size, node->fetch);
	}
	return total_write_amount;
}

// http://man7.org/linux/man-pages/man2/write.2.html
long __syscall4(int which, ...) // write
{
	va_list vl;
	va_start(vl, which);
	int fd = va_arg(vl, int);
	void *buf = va_arg(vl, void *);
	size_t count = va_arg(vl, size_t);
	va_end(vl);

	iovec io = { buf, count };
	return __syscall146(146, fd, &io, 1);
}

// http://man7.org/linux/man-pages/man2/mkdir.2.html
long __syscall39(int which, ...) // mkdir
{
	va_list vl;
	va_start(vl, which);
	const char *pathname = va_arg(vl, const char *);
	mode_t mode = va_arg(vl, mode_t);
	va_end(vl);

	EM_ASM_INT( { Module['printErr']('__syscall145 MKDIR, which: ' + $0 + ', pathname ' + Pointer_stringify($1) + ', mode: ' + $2 + ' .') }, which, pathname, mode);

	inode *root;
	if (pathname[0] == '/')
	{
		root = filesystem_root();
		++pathname;
	}
	else
		root = get_cwd();
	inode *parent_dir = find_parent_inode(root, pathname); // TODO: This is wrong, shouldn't be recursive
	inode *directory = create_inode(INODE_DIR);
	strcpy(directory->name, basename_part(pathname));
	directory->mode = mode;
	link_inode(directory, parent_dir);
	emscripten_dump_fs_root();
	return 0; // TODO: error checking
}

// http://man7.org/linux/man-pages/man2/rmdir.2.html
long __syscall40(int which, ...) // rmdir
{
	va_list vl;
	va_start(vl, which);
	const char *pathname = va_arg(vl, const char *);
	va_end(vl);

	inode *root;
	if (pathname[0] == '/')
	{
		root = filesystem_root();
		++pathname;
	}
	else
		root = get_cwd();
	inode *node = find_inode(root, pathname);
	if (node && node->type == INODE_DIR && !node->child)
	{
		EM_ASM_INT( { Module['print']('__syscall145 RMDIR, pathname: ' + Pointer_stringify($0) + ' removed.') }, pathname);
		unlink_inode(node);
		emscripten_dump_fs_root();
		return 0;
	}
	else
	{
		EM_ASM_INT( { Module['printErr']('__syscall145 RMDIR, pathname: ' + Pointer_stringify($0) + ' not deleted.') }, pathname);
		errno = ENOENT;
		return -1;
	}

}

// http://man7.org/linux/man-pages/man2/unlink.2.html
long __syscall10(int which, ...) // unlink
{
	va_list vl;
	va_start(vl, which);
	const char *pathname = va_arg(vl, const char *);
	va_end(vl);

	inode *root;
	if (pathname[0] == '/')
	{
		root = filesystem_root();
		++pathname;
	}
	else
		root = get_cwd();
	inode *node = find_inode(root, pathname);
	if (node && !node->child)
	{
		EM_ASM_INT( { Module['print']('__syscall145 UNLINK, pathname: ' + Pointer_stringify($0) + ' removed.') }, pathname);
		unlink_inode(node);
		emscripten_dump_fs_root();
		return 0;
	}
	else
	{
		EM_ASM_INT( { Module['printErr']('__syscall145 UNLINK, pathname: ' + Pointer_stringify($0) + ' not deleted.') }, pathname);
		errno = ENOENT;
		return -1;
	}
}

} // ~extern "C"
