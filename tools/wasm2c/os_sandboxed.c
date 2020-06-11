// Stubs for OS functions, for a sandboxed environment. Nothing is allowed
// exit the sandbox, calls to printf will fail, etc.

IMPORT_IMPL(void, Z_wasi_snapshot_preview1Z_proc_exitZ_vi, (u32 x), {
  abort_with_message("exit() called");
});

STUB_IMPORT_IMPL(u32, Z_envZ___sys_openZ_iiii, (u32 path, u32 flags, u32 varargs), -1);
STUB_IMPORT_IMPL(u32, Z_wasi_snapshot_preview1Z_fd_writeZ_iiiii, (u32 fd, u32 iov, u32 iovcnt, u32 pnum), WASI_DEFAULT_ERROR);
STUB_IMPORT_IMPL(u32, Z_wasi_snapshot_preview1Z_fd_readZ_iiiii, (u32 fd, u32 iov, u32 iovcnt, u32 pnum), WASI_DEFAULT_ERROR);
STUB_IMPORT_IMPL(u32, Z_wasi_snapshot_preview1Z_fd_closeZ_ii, (u32 fd), WASI_DEFAULT_ERROR);
STUB_IMPORT_IMPL(u32, Z_wasi_snapshot_preview1Z_environ_sizes_getZ_iii, (u32 pcount, u32 pbuf_size), WASI_DEFAULT_ERROR);
STUB_IMPORT_IMPL(u32, Z_wasi_snapshot_preview1Z_environ_getZ_iii, (u32 __environ, u32 environ_buf), WASI_DEFAULT_ERROR);
STUB_IMPORT_IMPL(u32, Z_wasi_snapshot_preview1Z_fd_seekZ_iijii, (u32 fd, u64 offset, u32 whence, u32 new_offset), WASI_DEFAULT_ERROR);
STUB_IMPORT_IMPL(u32, Z_wasi_snapshot_preview1Z_fd_seekZ_iiiiii, (u32 a, u32 b, u32 c, u32 d, u32 e), WASI_DEFAULT_ERROR);
STUB_IMPORT_IMPL(u32, Z_envZ___sys_unlinkZ_ii, (u32 path), WASI_DEFAULT_ERROR);
STUB_IMPORT_IMPL(u32, Z_envZ___sys_fstat64Z_iii, (u32 fd, u32 buf), EM_EACCES);
STUB_IMPORT_IMPL(u32, Z_envZ___sys_stat64Z_iii, (u32 path, u32 buf), EM_EACCES);
STUB_IMPORT_IMPL(u32, Z_envZ___sys_readZ_iiii, (u32 fd, u32 buf, u32 count), EM_EACCES);
STUB_IMPORT_IMPL(u32, Z_envZ___sys_accessZ_iii, (u32 pathname, u32 mode), EM_EACCES);
STUB_IMPORT_IMPL(u32, Z_wasi_snapshot_preview1Z_clock_time_getZ_iiji, (u32 clock_id, u64 max_lag, u32 out), WASI_EINVAL);
STUB_IMPORT_IMPL(u32, Z_wasi_snapshot_preview1Z_clock_res_getZ_iii, (u32 clock_id, u32 out), WASI_EINVAL);
