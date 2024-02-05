// Copyright 2018 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <sys/ioctl.h>
#include <unistd.h>
#include <stdio.h>

int main() {
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) != 0) {
        puts("TIOCGWINSZ failed");
        return -1;
    }
    if (ioctl(STDOUT_FILENO, TIOCSWINSZ, &ws) != 0) {
        puts("TIOCSWINSZ failed");
        return -1;
    }
    puts("success");
    return 0;
}
