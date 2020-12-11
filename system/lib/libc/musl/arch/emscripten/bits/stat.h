// XXX Emscripten in sync with musl/arch/i386/bits/stat.h except for the padding and 64-bit time_t redirections change.

/* copied from kernel definition, but with padding replaced
 * by the corresponding correctly-sized userspace types. */
struct stat {
	dev_t st_dev;
#ifndef __EMSCRIPTEN__
	int __st_dev_padding;
	long __st_ino_truncated;
#endif
	mode_t st_mode;
	nlink_t st_nlink;
	uid_t st_uid;
	gid_t st_gid;
	dev_t st_rdev;
#ifndef __EMSCRIPTEN__
	int __st_rdev_padding;
#endif
	off_t st_size;
	blksize_t st_blksize;
	blkcnt_t st_blocks;
#ifndef __EMSCRIPTEN__ // XXX Emscripten no need to activate the symbol redirections for 64-bit time_t.
	struct {
		long tv_sec;
		long tv_nsec;
	} __st_atim32, __st_mtim32, __st_ctim32;
#endif
	struct timespec st_atim;
	struct timespec st_mtim;
	struct timespec st_ctim;
	ino_t st_ino;
};
