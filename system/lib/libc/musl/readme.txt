These sources were downloaded from the musl-0.9.13 release on August 30, 2013.

Differences from upstream musl include:

* file descriptor numbers for stdio are 1-3 rather than 0-2.
* various 64 bit types are 32 bit instead including off_t,
  ino_t, dev_t, blkcnt_t, fsblkcnt_t, fsfilcnt_t, rlim_t.
* We don't define _POSIX_SHARED_MEMORY_OBJECTS.
* We flag __assert_fail as _Noreturn.
