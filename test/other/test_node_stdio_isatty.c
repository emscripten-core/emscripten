#include "emscripten.h"
#include "stdio.h"
#include "unistd.h"

EM_JS(int, init, (void), {
    var dev = FS.makedev(FS.createDevice.major++, 0);
    DEV.register(dev, DEV.nodeOutputDevice({fd: Module.outFd}));
    FS.mkdev('/dev/origout', dev);
    return FS.open('/dev/origout', 1).fd;
});

int main(void) {
    int outfd = init();
    FILE* outfile = fdopen(outfd, "w");

    fprintf(outfile, "%d%d%d\n", isatty(0), isatty(1), isatty(2));
}
