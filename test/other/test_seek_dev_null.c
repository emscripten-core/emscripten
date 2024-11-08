#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

int main() {
    int fd = open("/dev/null", O_WRONLY);
    printf("fd: %d\n", fd);
    if (fd == -1) {
        printf("open failed: %s\n", strerror(errno));
        return 1;
    }
    int res = lseek(fd, 10, SEEK_CUR);
    printf("res: %d\n", res);
    if (res == -1) {
        printf("lseek failed: %s\n", strerror(errno));
        return 1;
    }
    return 0;
}
