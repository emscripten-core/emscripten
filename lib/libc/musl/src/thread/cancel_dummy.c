#include "pthread_impl.h"
#include "syscall.h"

static long sccp(syscall_arg_t nr,
                 syscall_arg_t u, syscall_arg_t v, syscall_arg_t w,
                 syscall_arg_t x, syscall_arg_t y, syscall_arg_t z)
{
	return (__syscall)(nr, u, v, w, x, y, z);
}

weak_alias(sccp, __syscall_cp);

static void dummy()
{
}

weak_alias(dummy, __testcancel);
