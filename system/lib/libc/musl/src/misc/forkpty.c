#include <pty.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>

int forkpty(int *m, char *name, const struct termios *tio, const struct winsize *ws)
{
	int s, t, i, istmp[3]={0};
	pid_t pid;

	if (openpty(m, &s, name, tio, ws) < 0) return -1;

	/* Ensure before forking that we don't exceed fd limit */
	for (i=0; i<3; i++) {
		if (fcntl(i, F_GETFL) < 0) {
			t = fcntl(s, F_DUPFD, i);
			if (t<0) break;
			else if (t!=i) close(t);
			else istmp[i] = 1;
		}
	}
	pid = i==3 ? fork() : -1;
	if (!pid) {
		close(*m);
		setsid();
		ioctl(s, TIOCSCTTY, (char *)0);
		dup2(s, 0);
		dup2(s, 1);
		dup2(s, 2);
		if (s>2) close(s);
		return 0;
	}
	for (i=0; i<3; i++)
		if (istmp[i]) close(i);
	close(s);
	if (pid < 0) close(*m);
	return pid;
}
