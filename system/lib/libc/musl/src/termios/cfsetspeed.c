#define _BSD_SOURCE
#include <termios.h>
#include <sys/ioctl.h>
#include <errno.h>

int cfsetspeed(struct termios *tio, speed_t speed)
{
	int r = cfsetospeed(tio, speed);
	if (!r) cfsetispeed(tio, 0);
	return r;
}
