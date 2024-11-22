#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    open("a/b", O_RDWR | O_CREAT | O_EXCL);
    printf("Error %d: %s\n", errno, strerror(errno));
}
