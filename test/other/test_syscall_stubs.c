#include <sys/wait.h>
#include <stddef.h>

int main() {
    // The current implementation uses the stub symbol __syscall_wait4.
    // This test exists simply to ensure that it compiles and links.
    waitpid(0, NULL, 0);
    wait3(NULL, 0, NULL);
    wait4(0, NULL, 0, NULL);
}
