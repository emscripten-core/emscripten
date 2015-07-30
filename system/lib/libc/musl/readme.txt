These sources were downloaded from the musl-1.0.3 release on Jul 2, 2014.

Differences from upstream musl include:

* file descriptor numbers for stdio are 1-3 rather than 0-2.
* various 64 bit types are 32 bit instead including off_t,
  ino_t, dev_t, blkcnt_t, fsblkcnt_t, fsfilcnt_t, rlim_t.
* We don't define _POSIX_SHARED_MEMORY_OBJECTS.
* We flag __assert_fail as _Noreturn.
* Disable FLOCK, FUNLOCK and FFINALLOCK
* Simplify fputwc to not rely on musl stream internals
* signgam is no longer a weak alias of __signgam.
* __toread and __towrite have had shutdown functionality removed.
* Expand aliases for strto*_l() to short functions to remove warnings about incompatible pointer types.
