#include <stdio.h>
#include <emscripten.h>

int main(void)
{
    EM_ASM(
        var path = require("path");
        var home = process.env.HOME;
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
