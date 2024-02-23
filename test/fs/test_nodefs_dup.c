/*
 * Copyright 2018 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <emscripten.h>
#include <fcntl.h>
#include <stdio.h>
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
    int fd1 = open(CWD "test.txt", O_WRONLY);
    int fd2 = dup(fd1);
    int fd3 = fcntl(fd1, F_DUPFD_CLOEXEC, 0);

    assert(fd1 == 3);
    assert(fd2 == 4);
    assert(fd3 == 5);
    assert(close(fd1) == 0);
    assert(write(fd2, "abcdef", 6) == 6);
    assert(close(fd2) == 0);
    assert(write(fd3, "ghijkl", 6) == 6);
    assert(close(fd3) == 0);
    printf("success\n");
    return 0;
}
