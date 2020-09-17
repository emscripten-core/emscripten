#ifdef __cplusplus
extern "C" {
#endif

// This is used by libc++ as an efficient way to get high-quality random data
// (more efficiently than via the filesystem using /dev/urandom).
// Upstream musl added support for this, so we can switch to that, but it isn't
// where libc++ looks for it (which is here and not unistd.h), and it uses a
// syscall which is unnecessary indirection for us.
int getentropy(void *buffer, size_t length);

#ifdef __cplusplus
}
#endif
