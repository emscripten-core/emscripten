#include <features.h>

extern hidden _Atomic int *const __at_quick_exit_lockptr;
extern hidden _Atomic int *const __atexit_lockptr;
extern hidden _Atomic int *const __gettext_lockptr;
extern hidden _Atomic int *const __locale_lockptr;
extern hidden _Atomic int *const __random_lockptr;
extern hidden _Atomic int *const __sem_open_lockptr;
extern hidden _Atomic int *const __stdio_ofl_lockptr;
extern hidden _Atomic int *const __syslog_lockptr;
extern hidden _Atomic int *const __timezone_lockptr;

extern hidden _Atomic int *const __bump_lockptr;

extern hidden _Atomic int *const __vmlock_lockptr;

hidden void __malloc_atfork(int);
hidden void __ldso_atfork(int);
hidden void __pthread_key_atfork(int);

hidden void __post_Fork(int);
