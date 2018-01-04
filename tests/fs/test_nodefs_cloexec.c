#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <emscripten.h>

int main(void)
{
    EM_ASM(
#ifndef NODERAWFS
        FS.mkdir('/working');
        FS.mount(NODEFS, {root: '.'}, '/working');
        FS.currentPath = '/working';
#endif
        FS.close(FS.open('test.txt', 'w'));
    );
    assert(open("test.txt", O_RDONLY | O_CLOEXEC) != -1);
    printf("success\n");
    return 0;
}
