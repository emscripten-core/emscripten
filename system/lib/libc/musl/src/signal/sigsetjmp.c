#include <setjmp.h>
#include <signal.h>

/* !!! This function will not work unless the compiler performs
 * tail call optimization. Machine-specific asm versions should
 * be created instead even though the workaround (tail call)
 * is entirely non-machine-specific... */

int sigsetjmp(sigjmp_buf buf, int save)
{
	if ((buf->__fl = save))
		pthread_sigmask(SIG_SETMASK, 0, (sigset_t *)buf->__ss);
	return setjmp(buf);
}
