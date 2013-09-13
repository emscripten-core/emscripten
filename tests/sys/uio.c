#include <stdio.h>
#include <strings.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>

int main()
{
    int fd;
    char buf1[3], buf2[3], buf3[3];
    struct iovec iovecs[3];
    ssize_t written, readed;

    buf1[0] = 1;
    buf1[1] = 2;
    buf2[0] = 3;
    buf2[1] = 4;
    buf2[2] = 5;

    iovecs[0].iov_base = buf1;
    iovecs[1].iov_base = buf2;
    iovecs[0].iov_len = 2;
    iovecs[1].iov_len = 3;

    fd = open("/testfile", O_WRONLY | O_CREAT, 0666);
    assert(fd >= 0);

    assert(writev(fd, NULL, 2) == -1);
    assert(writev(fd, iovecs, -1) == -1);

    written = writev(fd, iovecs, 2);
    printf("written = %d\n", (int) written);
    assert(written == 5);

    close(fd);

    bzero(buf1, 3);
    bzero(buf2, 3);
    bzero(buf3, 3);

    iovecs[0].iov_base = buf1;
    iovecs[1].iov_base = buf2;
    iovecs[2].iov_base = buf3;
    iovecs[0].iov_len = 1;
    iovecs[1].iov_len = 2;
    iovecs[2].iov_len = 3;

    fd = open("/testfile", O_RDONLY, 0666);
    assert(fd >= 0);

    assert(readv(fd, NULL, 3) == -1);
    assert(readv(fd, iovecs, -1) == -1);

    readed = readv(fd, iovecs, 3);
    printf("readed = %d\n", (int) readed);
    assert(readed == 5);

    close(fd);

    printf("buf1[0] = %d\n", buf1[0]);
    printf("buf2[0] = %d\n", buf2[0]);
    printf("buf2[1] = %d\n", buf2[1]);
    printf("buf3[0] = %d\n", buf3[0]);
    printf("buf3[1] = %d\n", buf3[1]);
}
