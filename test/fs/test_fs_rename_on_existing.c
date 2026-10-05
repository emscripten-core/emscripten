#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <assert.h>
#include <errno.h>
#include <string.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

static void create_file(const char *path, const char *buffer) {
  printf("creating: %s\n", path);
  int fd = open(path, O_WRONLY | O_CREAT | O_EXCL, 0666);
  printf("error: %s\n", strerror(errno));
  assert(fd >= 0);

  int err = write(fd, buffer, sizeof(char) * strlen(buffer));
  assert(err ==  (sizeof(char) * strlen(buffer)));

  close(fd);
}

#if defined(NODEFS) && !defined(WASMFS)
// Rename in a second mount of the host cwd while the VFS cwd is elsewhere, so
// host and VFS paths differ (#27860). No absolute host path: PATH is POSIX-only.
static void test_second_mount() {
  EM_ASM({
    FS.mkdir('/other');
    FS.mount(NODEFS, { root: '.' }, '/other');
  });
  assert(mkdir("/memcwd", 0777) == 0);
  create_file("/memcwd/other_b", "memfs");
  assert(chdir("/memcwd") == 0);

  create_file("/other/other_a", "abc");
  create_file("/other/other_b", "xyz");
  assert(rename("/other/other_a", "/other/other_b") == 0);
  assert(access("/memcwd/other_b", F_OK) == 0);

  assert(unlink("/other/other_b") == 0);
  assert(access("/other/other_b", F_OK) == -1 && errno == ENOENT);
  create_file("/other/other_b", "xyz");
}
#endif

#if defined(MEMFS) && !defined(WASMFS)
static void test_proxyfs() {
  EM_ASM({
    FS.mkdir('/proxied');
    FS.mkdir('/proxy');
    FS.mount(PROXYFS, { root: '/proxied', fs: FS }, '/proxy');
  });

  create_file("/proxy/a", "abc");
  create_file("/proxy/b", "xyz");
  assert(rename("/proxy/a", "/proxy/b") == 0);

  assert(unlink("/proxy/b") == 0);
  assert(access("/proxy/b", F_OK) == -1 && errno == ENOENT);
  create_file("/proxy/b", "xyz");
}
#endif

int main() {
  create_file("a", "abc");
  create_file("b", "xyz");
  assert(rename("a", "b") == 0);
  assert(unlink("b") == 0);
  create_file("b", "xyz");
#if defined(NODEFS) && !defined(WASMFS)
  test_second_mount();
#endif
#if defined(MEMFS) && !defined(WASMFS)
  test_proxyfs();
#endif
  printf("done\n");
}
