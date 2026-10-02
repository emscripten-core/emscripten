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
// Mount by absolute path so host and VFS paths differ: setup_nodefs.js mounts
// root '.' at the cwd, where they coincide and hid #27860.
static void test_absolute_root() {
  EM_ASM({
    var root = process.cwd();
    FS.mkdirTree(root);
    FS.writeFile(root + '/abs_b', 'memfs');
    FS.mkdir('/abs');
    FS.mount(NODEFS, { root }, '/abs');
  });

  create_file("/abs/abs_a", "abc");
  create_file("/abs/abs_b", "xyz");
  assert(rename("/abs/abs_a", "/abs/abs_b") == 0);

  int memfs_file_intact = EM_ASM_INT({
    try {
      return FS.readFile(process.cwd() + '/abs_b', { encoding: 'utf8' }) === 'memfs';
    } catch (e) {
      return false;
    }
  });
  assert(memfs_file_intact);

  assert(unlink("/abs/abs_b") == 0);
  assert(access("/abs/abs_b", F_OK) == -1 && errno == ENOENT);
  create_file("/abs/abs_b", "xyz");
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
  test_absolute_root();
#endif
#if defined(MEMFS) && !defined(WASMFS)
  test_proxyfs();
#endif
  printf("done\n");
}
