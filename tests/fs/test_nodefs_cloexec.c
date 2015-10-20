#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <emscripten.h>

int main(void)
{
    EM_ASM(
        FS.mkdir('/working');
        FS.mount(NODEFS, {root: '.'}, '/working');
        FS.close(FS.open('/working/test.txt', 'w'));
    );
    assert(open("/working/test.txt", O_RDONLY | O_CLOEXEC) != -1);
    printf("success\n");
    return 0;
}
