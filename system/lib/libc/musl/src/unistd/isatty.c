#include <unistd.h>
#include <termios.h>

int isatty(int fd)
{
	struct termios t;
	return tcgetattr(fd, &t) == 0;
}
