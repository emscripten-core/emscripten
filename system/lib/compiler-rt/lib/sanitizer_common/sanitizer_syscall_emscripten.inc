#define __scc(X) ((long) (X))

#define __syscall_emscripten(n, ...) __syscall##n(n, ##__VA_ARGS__)
#define __syscall_emscripten0(n) __syscall_emscripten(n)
#define __syscall_emscripten1(n,a) __syscall_emscripten(n,__scc(a))
#define __syscall_emscripten2(n,a,b) __syscall_emscripten(n,__scc(a),__scc(b))
#define __syscall_emscripten3(n,a,b,c) __syscall_emscripten(n,__scc(a),__scc(b),__scc(c))
#define __syscall_emscripten4(n,a,b,c,d) __syscall_emscripten(n,__scc(a),__scc(b),__scc(c),__scc(d))
#define __syscall_emscripten5(n,a,b,c,d,e) __syscall_emscripten(n,__scc(a),__scc(b),__scc(c),__scc(d),__scc(e))
#define __syscall_emscripten6(n,a,b,c,d,e,f) __syscall_emscripten(n,__scc(a),__scc(b),__scc(c),__scc(d),__scc(e),__scc(f))
#define __syscall_emscripten7(n,a,b,c,d,e,f,g) __syscall_emscripten(n,__scc(a),__scc(b),__scc(c),__scc(d),__scc(e),__scc(f),__scc(g))

#define __SYSCALL_NARGS_X(a,b,c,d,e,f,g,h,n,...) n
#define __SYSCALL_NARGS(...) __SYSCALL_NARGS_X(__VA_ARGS__,7,6,5,4,3,2,1,0,)
#define __SYSCALL_CONCAT_X(a,b) a##b
#define __SYSCALL_CONCAT(a,b) __SYSCALL_CONCAT_X(a,b)
#define __SYSCALL_DISP(b,...) __SYSCALL_CONCAT(b,__SYSCALL_NARGS(__VA_ARGS__))(__VA_ARGS__)
#define internal_syscall(...) __SYSCALL_DISP(__syscall_emscripten,__VA_ARGS__)

#define DECLARE_SYSCALL2(x) extern "C" long __syscall##x(int which, ...)
#define DECLARE_SYSCALL1(x) DECLARE_SYSCALL2(x)
#define DECLARE_SYSCALL(x) DECLARE_SYSCALL1(SYS_##x)

DECLARE_SYSCALL(dup);
DECLARE_SYSCALL(mmap2);
DECLARE_SYSCALL(munmap);
DECLARE_SYSCALL(mprotect);
DECLARE_SYSCALL(close);
DECLARE_SYSCALL(open);
DECLARE_SYSCALL(read);
DECLARE_SYSCALL(write);
DECLARE_SYSCALL(ftruncate);
DECLARE_SYSCALL(stat64);
DECLARE_SYSCALL(lstat64);
DECLARE_SYSCALL(fstat64);
DECLARE_SYSCALL(dup2);
DECLARE_SYSCALL(readlink);
DECLARE_SYSCALL(unlink);
DECLARE_SYSCALL(rename);
DECLARE_SYSCALL(exit_group);
DECLARE_SYSCALL(nanosleep);
DECLARE_SYSCALL(execve);
DECLARE_SYSCALL(gettid);
DECLARE_SYSCALL(getpid);
DECLARE_SYSCALL(getppid);
DECLARE_SYSCALL(getdents);
DECLARE_SYSCALL(wait4);
DECLARE_SYSCALL(lseek);
DECLARE_SYSCALL(sigaltstack);
DECLARE_SYSCALL(ptrace);
DECLARE_SYSCALL(tgkill);
DECLARE_SYSCALL(gettimeofday);
DECLARE_SYSCALL(rt_sigprocmask);

#define SYSCALL(x) SYS_##x

bool internal_iserror(uptr retval, int *rverrno) {
  if (retval >= (uptr)-4095) {
    if (rverrno)
      *rverrno = -retval;
    return true;
  }
  return false;
}
