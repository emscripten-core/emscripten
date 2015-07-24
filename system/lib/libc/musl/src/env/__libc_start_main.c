#include <elf.h>
#include "libc.h"

void __init_tls(size_t *);
void __init_security(size_t *);
void __init_ldso_ctors(void);

#ifndef SHARED
static void dummy() {}
weak_alias(dummy, _init);
extern void (*const __init_array_start)() __attribute__((weak));
extern void (*const __init_array_end)() __attribute__((weak));
#endif

#define AUX_CNT 38

extern size_t __hwcap, __sysinfo;
extern char *__progname, *__progname_full;

void __init_libc(char **envp, char *pn)
{
	size_t i, *auxv, aux[AUX_CNT] = { 0 };
	__environ = envp;
	for (i=0; envp[i]; i++);
	libc.auxv = auxv = (void *)(envp+i+1);
	for (i=0; auxv[i]; i+=2) if (auxv[i]<AUX_CNT) aux[auxv[i]] = auxv[i+1];
	__hwcap = aux[AT_HWCAP];
	__sysinfo = aux[AT_SYSINFO];
	libc.page_size = aux[AT_PAGESZ];

	if (pn) {
		__progname = __progname_full = pn;
		for (i=0; pn[i]; i++) if (pn[i]=='/') __progname = pn+i+1;
	}

	__init_tls(aux);
	__init_security(aux);
}

int __libc_start_main(int (*main)(int,char **,char **), int argc, char **argv)
{
	char **envp = argv+argc+1;

#ifndef SHARED
	__init_libc(envp, argv[0]);
	_init();
	uintptr_t a = (uintptr_t)&__init_array_start;
	for (; a<(uintptr_t)&__init_array_end; a+=sizeof(void(*)()))
		(*(void (**)())a)();
#endif

	/* Pass control to to application */
	exit(main(argc, argv, envp));
	return 0;
}
