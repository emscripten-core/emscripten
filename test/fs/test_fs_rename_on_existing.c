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
static int file_contains(const char *path, const char *expected) {
  char buffer[16] = {0};
  int fd = open(path, O_RDONLY);
  if (fd < 0) {
    return 0;
  }
  read(fd, buffer, sizeof(buffer) - 1);
  close(fd);
  return strcmp(buffer, expected) == 0;
}

// Mount the host cwd a second time and rename there while the VFS cwd is a
// MEMFS directory, so the target's host path ('other_b') and VFS path differ.
// setup_nodefs.js mounts root '.' at the cwd, where they coincide and hid
// #27860. Avoid absolute host paths: PATH is POSIX-only and doesn't handle
// Windows drive letters.
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
  assert(file_contains("/memcwd/other_b", "memfs"));

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
