#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>

int
main() {
    int fd, dupfd, status, bytes;
    fclose(stdin);
    fd = open("a.txt", O_CREAT | O_WRONLY, 0644);
    assert(fd == 0);
    printf("fd: %d errno: %d\n", fd, errno);
    bytes = write(fd, "hi there!\n", 11);
    assert(bytes == 11);
    printf("fd: %d errno: %d bytes: %d\n", fd, errno, bytes);

    dupfd = fcntl(fd, F_DUPFD, 0);
    assert(dupfd != fd);
    printf("dupfd: %d\n", dupfd);
    status = close(dupfd);
    printf("status: %d\n", status);
    assert(status == 0);
    bytes = write(fd, "hi again!", 10);
    printf("fd: %d errno: %d bytes: %d\n", fd, errno, bytes);
    assert(bytes == 10);
    status = close(fd);
    assert(status == 0);
}
