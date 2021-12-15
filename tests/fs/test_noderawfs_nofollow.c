#include <assert.h>
#include <fcntl.h>
#include <stdio.h>

int main() {
    assert(open("/dev/pts/0", O_NOFOLLOW) != -1);
    printf("success\n");
    return 0;
}
