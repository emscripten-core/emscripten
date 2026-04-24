#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <assert.h>

int main() {
  {
    int src_fd = open("file", O_CREAT | O_WRONLY, 0777);
    assert(src_fd >= 0);
    assert(close(src_fd) == 0);
  }
  {
    // POSIX: open("file/") must fail with ENOTDIR if file is a regular file.
    assert(open("file/", O_RDONLY) == -1);
    assert(errno == ENOTDIR);

    assert(open("file/.", O_RDONLY) == -1);
    assert(errno == ENOTDIR);

    assert(open("file/..", O_RDONLY) == -1);
    assert(errno == ENOTDIR);

    assert(mkdir("file/blah", 0777) == -1);
    assert(errno == ENOTDIR);
  }
  {
    assert(open("./does-not-exist/", O_CREAT, 0777) == -1);
    assert(errno == EISDIR);
  }
  {
    assert(mkdir("dir", 0777) == 0);
    assert(symlink("dir", "link_to_dir") == 0);
    assert(symlink("file", "link_to_file") == 0);

    // link_to_dir/ should resolve to the directory.
    int fd = open("link_to_dir/", O_RDONLY);
    assert(fd >= 0);
    close(fd);

    // link_to_file/ should fail with ENOTDIR.
    assert(open("link_to_file/", O_RDONLY) == -1);
    assert(errno == ENOTDIR);
  }
  printf("success\n");
}
