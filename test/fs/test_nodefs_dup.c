/*
 * Copyright 2018 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <fcntl.h>
#include <emscripten.h>
#include <unistd.h>

#ifdef NODERAWFS
#define CWD ""
#else
#define CWD "/working/"
#endif

int main(void)
{
    EM_ASM(
#ifdef NODERAWFS
        FS.close(FS.open('test.txt', 'w'));
#else
        FS.mkdir('/working');
        FS.mount(NODEFS, {root: '.'}, '/working');
        FS.close(FS.open('/working/test.txt', 'w'));
#endif
    );
    int fd1 = open(CWD "test.txt", O_RDONLY);
    int fd2 = dup(fd1);
    int fd3 = fcntl(fd1, F_DUPFD_CLOEXEC, 0);

    printf("fd1: %d, fd2: %d\n", fd1, fd2);
    printf("close(fd1): %d\n", close(fd1));
    printf("close(fd2): %d\n", close(fd2));
    printf("close(fd3): %d\n", close(fd3));
    return 0;
}
