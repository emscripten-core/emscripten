#ifndef  _COMPAT_EXECINFO_H_
#define  _COMPAT_EXECINFO_H_

int backtrace(void **buffer, int size);

char **backtrace_symbols(void *const *buffer, int size);

void backtrace_symbols_fd(void *const *buffer, int size, int fd);

#endif  _COMPAT_EXECINFO_H_
