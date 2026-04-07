#include "emscripten.h"
#include "stdio.h"
#include "sys/ioctl.h"
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

    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

    fprintf (outfile, "rows %d\n", w.ws_row);
    fprintf (outfile, "columns %d\n", w.ws_col);
    return 0;
}
