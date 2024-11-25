#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>


char* join_path(const char* path1, const char* path2) {
    int len1 = strlen(path1);
    int len2 = strlen(path2);
    char* result = malloc(len1 + len2 + 2);
    memcpy(result, path1, len1);
    result[len1] = '/';
    memcpy(result + len1 + 1, path2, len2);
    return result;
}

int main() {
    char template[] = "/tmp/tmpdir.XXXXXX";
    char *tmpdir = mkdtemp(template);
    char* p1 = join_path(tmpdir, "test");
    char* p2 = join_path(tmpdir, "test2");

    int res = symlink(p2, p1);
    printf("link result: %d\n", res);
    int src_fd = open(p1, O_CREAT | O_WRONLY, 0777);
    printf("source_fd: %d, errno: %d %s\n", src_fd, errno, strerror(errno));
    write(src_fd, "abc", 3);
    close(src_fd);
    {
        int target_fd = open(p2, O_RDONLY);
        printf("target_fd: %d, errno: %d %s\n", target_fd, errno, strerror(errno));
        char buf[10];
        read(target_fd, buf, 10);
        printf("buf: '%s'\n", buf);
        close(target_fd);
    }
    {
        int target_fd = open(p1, O_RDONLY);
        printf("target_fd: %d, errno: %d %s\n", target_fd, errno, strerror(errno));
        char buf[10];
        read(target_fd, buf, 10);
        printf("buf: '%s'\n", buf);
        close(target_fd);
    }
    free(p1);
    free(p2);
}
