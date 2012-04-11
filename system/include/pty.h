
#include <sys/termios.h>

int openpty(int *amaster, int *aslave, char *name, struct termios *termp, struct winsize *winp);
int forkpty(int *amaster, char *name, struct termios *termp, struct winsize *winp);

