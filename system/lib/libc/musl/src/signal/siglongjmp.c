#include <setjmp.h>
#include <signal.h>
#include "syscall.h"
#include "pthread_impl.h"

_Noreturn void siglongjmp(sigjmp_buf buf, int ret)
{
	if (buf->__fl) __restore_sigs(buf->__ss);
	longjmp(buf, ret);
}
