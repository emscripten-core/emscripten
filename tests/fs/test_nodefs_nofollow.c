/*
 * Copyright 2020 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

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
    assert(open("/working/test.txt", O_NOFOLLOW) != -1);
    printf("success\n");
    return 0;
}
