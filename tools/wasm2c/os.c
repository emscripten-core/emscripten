#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#ifndef _WIN32
#include <unistd.h>
#else
#include <io.h>
#endif

IMPORT_IMPL(void, Z_wasi_snapshot_preview1Z_proc_exitZ_vi, (u32 x), {
  exit(x);
});

#define MAX_FDS 1024

static int wasm_fd_to_native[MAX_FDS];

static u32 next_wasm_fd;

#define WASM_STDIN  0
#define WASM_STDOUT 1
#define WASM_STDERR 2

static void init_fds() {
#ifndef _WIN32
  wasm_fd_to_native[WASM_STDIN] = STDIN_FILENO;
  wasm_fd_to_native[WASM_STDOUT] = STDOUT_FILENO;
  wasm_fd_to_native[WASM_STDERR] = STDERR_FILENO;
#else
  wasm_fd_to_native[WASM_STDIN] = _fileno(stdin);
  wasm_fd_to_native[WASM_STDOUT] = _fileno(stdout);
  wasm_fd_to_native[WASM_STDERR] = _fileno(stderr);
#endif
  next_wasm_fd = 3;
}

static u32 get_or_allocate_wasm_fd(int nfd) {
  // If the native fd is already mapped, return the same wasm fd for it.
  for (int i = 0; i < next_wasm_fd; i++) {
    if (wasm_fd_to_native[i] == nfd) {
      return i;
    }
  }
  if (next_wasm_fd >= MAX_FDS) {
    abort_with_message("ran out of fds");
  }
  u32 fd = next_wasm_fd;
  wasm_fd_to_native[fd] = nfd;
  next_wasm_fd++;
  return fd;
}

static int get_native_fd(u32 fd) {
  if (fd >= MAX_FDS || fd >= next_wasm_fd) {
    return -1;
  }
  return wasm_fd_to_native[fd];
}

IMPORT_IMPL(u32, Z_envZ___sys_openZ_iiii, (u32 path, u32 flags, u32 varargs), {
  VERBOSE_LOG("  open: %s %d %d\n", MEMACCESS(path), flags, wasm_i32_load(varargs));
  int nfd = open(MEMACCESS(path), flags, wasm_i32_load(varargs));
  VERBOSE_LOG("    => native %d\n", nfd);
  if (nfd >= 0) {
    u32 fd = get_or_allocate_wasm_fd(nfd);
    VERBOSE_LOG("      => wasm %d\n", fd);
    return fd;
  }
  return -1;
});

IMPORT_IMPL(u32, Z_wasi_snapshot_preview1Z_fd_writeZ_iiiii, (u32 fd, u32 iov, u32 iovcnt, u32 pnum), {
  int nfd = get_native_fd(fd);
  VERBOSE_LOG("  fd_write wasm %d => native %d\n", fd, nfd);
  if (nfd < 0) {
    return WASI_DEFAULT_ERROR;
  }
  u32 num = 0;
  for (u32 i = 0; i < iovcnt; i++) {
    u32 ptr = wasm_i32_load(iov + i * 8);
    u32 len = wasm_i32_load(iov + i * 8 + 4);
    VERBOSE_LOG("    chunk %d %d\n", ptr, len);
    ssize_t result;
    // Use stdio for stdout/stderr to avoid mixing a low-level write() with
    // other logging code, which can change the order from the expected.
    if (fd == WASM_STDOUT) {
      result = fwrite(MEMACCESS(ptr), 1, len, stdout);
    } else if (fd == WASM_STDERR) {
      result = fwrite(MEMACCESS(ptr), 1, len, stderr);
    } else {
      result = write(nfd, MEMACCESS(ptr), len);
    }
    if (result < 0) {
      VERBOSE_LOG("    error, %d %s\n", errno, strerror(errno));
      return WASI_DEFAULT_ERROR;
    }
    if (result != len) {
      VERBOSE_LOG("    amount error, %ld %d\n", result, len);
      return WASI_DEFAULT_ERROR;
    }
    num += len;
  }
  VERBOSE_LOG("    success: %d\n", num);
  wasm_i32_store(pnum, num);
  return 0;
});

IMPORT_IMPL(u32, Z_wasi_snapshot_preview1Z_fd_readZ_iiiii, (u32 fd, u32 iov, u32 iovcnt, u32 pnum), {
  int nfd = get_native_fd(fd);
  VERBOSE_LOG("  fd_read wasm %d => native %d\n", fd, nfd);
  if (nfd < 0) {
    return WASI_DEFAULT_ERROR;
  }
  u32 num = 0;
  for (u32 i = 0; i < iovcnt; i++) {
    u32 ptr = wasm_i32_load(iov + i * 8);
    u32 len = wasm_i32_load(iov + i * 8 + 4);
    VERBOSE_LOG("    chunk %d %d\n", ptr, len);
    ssize_t result = read(nfd, MEMACCESS(ptr), len);
    if (result < 0) {
      VERBOSE_LOG("    error, %d %s\n", errno, strerror(errno));
      return WASI_DEFAULT_ERROR;
    }
    num += result;
    if (result != len) {
      break; // nothing more to read
    }
  }
  VERBOSE_LOG("    success: %d\n", num);
  wasm_i32_store(pnum, num);
  return 0;
});

IMPORT_IMPL(u32, Z_wasi_snapshot_preview1Z_fd_closeZ_ii, (u32 fd), {
  // TODO full file support
  int nfd = get_native_fd(fd);
  VERBOSE_LOG("  close wasm %d => native %d\n", fd, nfd);
  if (nfd < 0) {
    return WASI_DEFAULT_ERROR;
  }
  close(nfd);
  return 0;
});

IMPORT_IMPL(u32, Z_wasi_snapshot_preview1Z_environ_sizes_getZ_iii, (u32 pcount, u32 pbuf_size), {
  // TODO: connect to actual env?
  wasm_i32_store(pcount, 0);
  wasm_i32_store(pbuf_size, 0);
  return 0;
});

IMPORT_IMPL(u32, Z_wasi_snapshot_preview1Z_environ_getZ_iii, (u32 __environ, u32 environ_buf), {
  // TODO: connect to actual env?
  return 0;
});

static int whence_to_native(u32 whence) {
  if (whence == 0) return SEEK_SET;
  if (whence == 1) return SEEK_CUR;
  if (whence == 2) return SEEK_END;
  return -1;
}

IMPORT_IMPL(u32, Z_wasi_snapshot_preview1Z_fd_seekZ_iijii, (u32 fd, u64 offset, u32 whence, u32 new_offset), {
  int nfd = get_native_fd(fd);
  int nwhence = whence_to_native(whence);
  VERBOSE_LOG("  seek %d (=> native %d) %ld %d (=> %d) %d\n", fd, nfd, offset, whence, nwhence, new_offset);
  if (nfd < 0) {
    return WASI_DEFAULT_ERROR;
  }
  off_t off = lseek(nfd, offset, nwhence);
  VERBOSE_LOG("    off: %ld\n", off);
  if (off == (off_t)-1) {
    VERBOSE_LOG("    error, %d %s\n", errno, strerror(errno));
    return WASI_DEFAULT_ERROR;
  }
  wasm_i64_store(new_offset, off);
  return 0;
});
IMPORT_IMPL(u32, Z_wasi_snapshot_preview1Z_fd_seekZ_iiiiii, (u32 a, u32 b, u32 c, u32 d, u32 e), {
  return Z_wasi_snapshot_preview1Z_fd_seekZ_iijii(a, b + (((u64)c) << 32), d, e);
});

// TODO: set errno in wasm for things that need it

IMPORT_IMPL(u32, Z_envZ___sys_unlinkZ_ii, (u32 path), {
  VERBOSE_LOG("  unlink %s\n", MEMACCESS(path));
  if (unlink(MEMACCESS(path))) {
    VERBOSE_LOG("    error, %d %s\n", errno, strerror(errno));
    return EM_EACCES;
  }
  return 0;
});

static u32 do_stat(int nfd, u32 buf) {
  struct stat nbuf;
  if (fstat(nfd, &nbuf)) {
    VERBOSE_LOG("    error, %d %s\n", errno, strerror(errno));
    return EM_EACCES;
  }
  VERBOSE_LOG("    success, size=%ld\n", nbuf.st_size);
  wasm_i32_store(buf + 0, nbuf.st_dev);
  wasm_i32_store(buf + 4, 0);
  wasm_i32_store(buf + 8, nbuf.st_ino);
  wasm_i32_store(buf + 12, nbuf.st_mode);
  wasm_i32_store(buf + 16, nbuf.st_nlink);
  wasm_i32_store(buf + 20, nbuf.st_uid);
  wasm_i32_store(buf + 24, nbuf.st_gid);
  wasm_i32_store(buf + 28, nbuf.st_rdev);
  wasm_i32_store(buf + 32, 0);
  wasm_i64_store(buf + 40, nbuf.st_size);
#ifdef _WIN32
  wasm_i32_store(buf + 48, 512); // fixed blocksize on windows
  wasm_i32_store(buf + 52, 0);   // but no reported blocks...
#else
  wasm_i32_store(buf + 48, nbuf.st_blksize);
  wasm_i32_store(buf + 52, nbuf.st_blocks);
#endif
#if defined(__APPLE__) || defined(__NetBSD__)
  wasm_i32_store(buf + 56, nbuf.st_atimespec.tv_sec);
  wasm_i32_store(buf + 60, nbuf.st_atimespec.tv_nsec);
  wasm_i32_store(buf + 64, nbuf.st_mtimespec.tv_sec);
  wasm_i32_store(buf + 68, nbuf.st_mtimespec.tv_nsec);
  wasm_i32_store(buf + 72, nbuf.st_ctimespec.tv_sec);
  wasm_i32_store(buf + 76, nbuf.st_ctimespec.tv_nsec);
#elif defined(_WIN32)
  wasm_i32_store(buf + 56, gmtime(&nbuf.st_atime)->tm_sec);
  wasm_i32_store(buf + 60, 0);
  wasm_i32_store(buf + 64, gmtime(&nbuf.st_mtime)->tm_sec);
  wasm_i32_store(buf + 68, 0);
  wasm_i32_store(buf + 72, gmtime(&nbuf.st_ctime)->tm_sec);
  wasm_i32_store(buf + 76, 0);
#else
  wasm_i32_store(buf + 56, nbuf.st_atim.tv_sec);
  wasm_i32_store(buf + 60, nbuf.st_atim.tv_nsec);
  wasm_i32_store(buf + 64, nbuf.st_mtim.tv_sec);
  wasm_i32_store(buf + 68, nbuf.st_mtim.tv_nsec);
  wasm_i32_store(buf + 72, nbuf.st_ctim.tv_sec);
  wasm_i32_store(buf + 76, nbuf.st_ctim.tv_nsec);
#endif
  wasm_i64_store(buf + 80, nbuf.st_ino);
  return 0;
}

IMPORT_IMPL(u32, Z_envZ___sys_fstat64Z_iii, (u32 fd, u32 buf), {
  int nfd = get_native_fd(fd);
  VERBOSE_LOG("  fstat64 %d (=> %d) %d\n", fd, nfd, buf);
  if (nfd < 0) {
    return EM_EACCES;
  }
  return do_stat(nfd, buf);
});

IMPORT_IMPL(u32, Z_envZ___sys_stat64Z_iii, (u32 path, u32 buf), {
  VERBOSE_LOG("  stat64: %s\n", MEMACCESS(path));
  int nfd = open(MEMACCESS(path), O_RDONLY); // could be O_PATH on linux...
  if (nfd < 0) {
    VERBOSE_LOG("    error, %d %s\n", errno, strerror(errno));
    return EM_EACCES;
  }
  return do_stat(nfd, buf);
});

IMPORT_IMPL(u32, Z_envZ___sys_readZ_iiii, (u32 fd, u32 buf, u32 count), {
  int nfd = get_native_fd(fd);
  VERBOSE_LOG("  read %d (=> %d) %d %d\n", fd, nfd, buf, count);
  if (nfd < 0) {
    VERBOSE_LOG("    bad fd\n");
    return EM_EACCES;
  }
  ssize_t ret = read(nfd, MEMACCESS(buf), count);
  VERBOSE_LOG("    native read: %ld\n", ret);
  if (ret < 0) {
    VERBOSE_LOG("    read error %d %s\n", errno, strerror(errno));
    return EM_EACCES;
  }
  return ret;
});

IMPORT_IMPL(u32, Z_envZ___sys_accessZ_iii, (u32 pathname, u32 mode), {
  VERBOSE_LOG("  access: %s 0x%x\n", MEMACCESS(pathname), mode);
  // TODO: sandboxing, convert mode
  int result = access(MEMACCESS(pathname), mode);
  if (result < 0) {
    VERBOSE_LOG("    access error: %d %s\n", errno, strerror(errno));
    return EM_EACCES;
  }
  return 0;
});

#define WASM_CLOCK_REALTIME 0
#define WASM_CLOCK_MONOTONIC 1
#define WASM_CLOCK_PROCESS_CPUTIME 2
#define WASM_CLOCK_THREAD_CPUTIME_ID 3

static int check_clock(u32 clock_id) {
  return clock_id == WASM_CLOCK_REALTIME || clock_id == WASM_CLOCK_MONOTONIC ||
         clock_id == WASM_CLOCK_PROCESS_CPUTIME || clock_id == WASM_CLOCK_THREAD_CPUTIME_ID;
}

IMPORT_IMPL(u32, Z_wasi_snapshot_preview1Z_clock_time_getZ_iiji, (u32 clock_id, u64 max_lag, u32 out), {
  if (!check_clock(clock_id)) {
    return WASI_EINVAL;
  }
  // TODO: handle realtime vs monotonic etc.
  // wasi expects a result in nanoseconds, and we know how to convert clock()
  // to seconds, so compute from there
  const double NSEC_PER_SEC = 1000.0 * 1000.0 * 1000.0;
  wasm_i64_store(out, (u64)(clock() / (CLOCKS_PER_SEC / NSEC_PER_SEC)));
  return 0;
});

IMPORT_IMPL(u32, Z_wasi_snapshot_preview1Z_clock_res_getZ_iii, (u32 clock_id, u32 out), {
  if (!check_clock(clock_id)) {
    return WASI_EINVAL;
  }
  // TODO: handle realtime vs monotonic etc. For now just report "milliseconds".
  wasm_i64_store(out, 1000 * 1000);
  return 0;
});
