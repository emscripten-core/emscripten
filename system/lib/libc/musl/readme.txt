These sources were downloaded from the musl-0.9.12 release on July 29, 2013,
along with some updates from git to fix various issues that were found.

Differences from upstream musl include:

* file descriptor numbers for stdio are 1-3 rather than 0-2.
* various 64 bit types are 32 bit instead including off_t,
  ino_t, dev_t, blkcnt_t, fsblkcnt_t, fsfilcnt_t, rlim_t.
