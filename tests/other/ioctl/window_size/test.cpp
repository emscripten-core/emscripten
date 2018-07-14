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
