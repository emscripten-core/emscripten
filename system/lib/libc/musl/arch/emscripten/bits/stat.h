/*
 * This definition happens to match the x32 and x86-64 stat layouts.
 * Direct compatibility with them isn't essential, but they are good
 * examples to follow. Note that there's no implicit padding, and
 * there's room for future expansion.
 */
struct stat
{
	dev_t st_dev;
	ino_t st_ino;
	nlink_t st_nlink;

	mode_t st_mode;
	uid_t st_uid;
	gid_t st_gid;
	unsigned int __pad0;
	dev_t st_rdev;
	off_t st_size;
	blksize_t st_blksize;
	blkcnt_t st_blocks;

	struct timespec st_atim;
	struct timespec st_mtim;
	struct timespec st_ctim;
	long long __unused[3];
};
