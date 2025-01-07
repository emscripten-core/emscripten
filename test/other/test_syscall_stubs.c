#include <sys/wait.h>
#include <stddef.h>

int main() {
    // The current implementation uses the stub symbol __syscall_wait4.
    // This test exists simply to ensure that it compile and link.
    waitpid(0, NULL, 0);
    wait3(NULL, 0, NULL);
    wait4(0, NULL, 0, NULL);
}
