#include <sys/ioctl.h>
#include <unistd.h>

int main() {
    struct winsize ws;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
    ioctl(STDOUT_FILENO, TIOCSWINSZ, &ws);
    return 0;
}
