/*
 * Copyright 2018 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <emscripten.h>

int main(void)
{
    EM_ASM(
        var path = require("path");
        var home = process.env.HOME;
        // On Windows HOME environment variable doesn't exist, but concatenating HOMEDRIVE and HOMEPATH
        // does the same thing.
        if (!home) home = process.env.HOMEDRIVE + process.env.HOMEPATH;
        var parent = path.dirname(home);
        var relative = path.relative(parent, home);
        FS.mkdir('/nodefs_home');
        FS.mount(NODEFS, { root: parent }, '/nodefs_home');
        // Reading C:/Users/(username) on Windows, /home/(username) on Linux
        // C:/Users on Windows disallows write access but should still allow access to its children
        FS.readdir('/nodefs_home/' + relative);
    );
    printf("success\n");
    return 0;
}
