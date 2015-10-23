#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <grp.h>
#include <assert.h>
#include <emscripten.h>

int main() {
  EM_ASM(
    FS.mkdir('working');
#if NODEFS
    FS.mount(NODEFS, { root: '.' }, 'working');
#endif
  );

  int f = open("working", O_RDONLY);
  assert(f);
  int t = open("/dev/stdin", O_RDONLY);
  assert(t);

  sync();

  printf("fsync(good): %d", fsync(f));
  printf(", errno: %d\n", errno);
  errno = 0;
  printf("fsync(bad): %d", fsync(42));
  printf(", errno: %d\n", errno);
  errno = 0;

  printf("fdatasync(good): %d", fdatasync(f));
  printf(", errno: %d\n", errno);
  errno = 0;
  printf("fdatasync(bad): %d", fdatasync(42));
  printf(", errno: %d\n", errno);
  errno = 0;

  printf("tcgetpgrp(good): %d", tcgetpgrp(t));
  printf(", errno: %d\n", errno);
  errno = 0;
  printf("tcgetpgrp(bad): %d", tcgetpgrp(42));
  printf(", errno: %d\n", errno);
  errno = 0;

  printf("tcsetpgrp(good): %d", tcsetpgrp(t, 123));
  printf(", errno: %d\n", errno);
  errno = 0;
  printf("tcsetpgrp(bad): %d", tcsetpgrp(42, 123));
  printf(", errno: %d\n", errno);
  errno = 0;

  printf("link: %d", link("working/here", "working/there"));
  printf(", errno: %d\n", errno);
  errno = 0;

  printf("lockf(good): %d", lockf(f, F_LOCK, 456));
  printf(", errno: %d\n", errno);
  errno = 0;
  printf("lockf(bad): %d", lockf(42, F_LOCK, 456));
  printf(", errno: %d\n", errno);
  errno = 0;

  printf("nice: %d", nice(42));
  printf(", errno: %d\n", errno);
  errno = 0;

  printf("pause: %d", pause());
  printf(", errno: %d\n", errno);
  errno = 0;

  int pipe_arg[2];
  printf("pipe(good): %d", pipe(pipe_arg));
  printf(", errno: %d\n", errno);
  errno = 0;
  printf("pipe(bad): %d", pipe(0));
  printf(", errno: %d\n", errno);
  errno = 0;

  char* exec_argv[] = {"arg", 0};
  char* exec_env[] = {"a=b", 0};
  printf("execl: %d", execl("working/program", "arg", 0));
  printf(", errno: %d\n", errno);
  errno = 0;
  printf("execle: %d", execle("working/program", "arg", 0, exec_env));
  printf(", errno: %d\n", errno);
  errno = 0;
  printf("execlp: %d", execlp("program", "arg", 0));
  printf(", errno: %d\n", errno);
  errno = 0;
  printf("execv: %d", execv("program", exec_argv));
  printf(", errno: %d\n", errno);
  errno = 0;
  printf("execve: %d", execve("program", exec_argv, exec_env));
  printf(", errno: %d\n", errno);
  errno = 0;
  printf("execvp: %d", execvp("program", exec_argv));
  printf(", errno: %d\n", errno);
  errno = 0;

  printf("chown(good): %d", chown("working", 123, 456));
  printf(", errno: %d\n", errno);
  errno = 0;
  printf("chown(bad): %d", chown("working/noexist", 123, 456));
  printf(", errno: %d\n", errno);
  errno = 0;
  printf("lchown(good): %d", lchown("working", 123, 456));
  printf(", errno: %d\n", errno);
  errno = 0;
  printf("lchown(bad): %d", lchown("working/noexist", 123, 456));
  printf(", errno: %d\n", errno);
  errno = 0;
  printf("fchown(good): %d", fchown(f, 123, 456));
  printf(", errno: %d\n", errno);
  errno = 0;
  printf("fchown(bad): %d", fchown(42, 123, 456));
  printf(", errno: %d\n", errno);
  errno = 0;

  printf("fork: %d", fork());
  printf(", errno: %d\n", errno);
  errno = 0;
  printf("vfork: %d", vfork());
  printf(", errno: %d\n", errno);
  errno = 0;

  printf("crypt: %s", crypt("foo", "bar"));
  printf(", errno: %d\n", errno);
  errno = 0;
  char encrypt_block[64] = {0};
  printf("encrypt");
  encrypt(encrypt_block, 42);
  printf(", errno: %d\n", errno);
  errno = 0;

  printf("getgid: %d", getgid());
  printf(", errno: %d\n", errno);
  errno = 0;
  printf("getegid: %d", getegid());
  printf(", errno: %d\n", errno);
  errno = 0;
  printf("getuid: %d", getuid());
  printf(", errno: %d\n", errno);
  errno = 0;
  printf("geteuid: %d", geteuid());
  printf(", errno: %d\n", errno);
  errno = 0;

  printf("getpgrp: %d", getpgrp());
  printf(", errno: %d\n", errno);
  errno = 0;
  pid_t mypid = getpid();
  printf("getpid: %d", mypid);
  printf(", errno: %d\n", errno);
  errno = 0;
  printf("getppid: %d", getppid());
  printf(", errno: %d\n", errno);
  errno = 0;
  printf("getpgid: %d", getpgid(mypid));
  printf(", errno: %d\n", errno);
  errno = 0;
  printf("getsid: %d", getsid(mypid));
  printf(", errno: %d\n", errno);
  errno = 0;
  printf("setgid: %d", setgid(0));
  printf(", errno: %d\n", errno);
  errno = 0;
  printf("setegid: %d", setegid(0));
  printf(", errno: %d\n", errno);
  errno = 0;
  printf("setuid: %d", setuid(0));
  printf(", errno: %d\n", errno);
  errno = 0;
  printf("seteuid: %d", seteuid(0));
  printf(", errno: %d\n", errno);
  errno = 0;
  printf("setpgrp: %d", setpgrp());
  printf(", errno: %d\n", errno);
  errno = 0;
  printf("setsid: %d", setsid());
  printf(", errno: %d\n", errno);
  errno = 0;
  printf("setpgid: %d", setpgid(mypid, mypid));
  printf(", errno: %d\n", errno);
  errno = 0;
  printf("setpgid: %d", setpgid(123, 456));
  printf(", errno: %d\n", errno);
  errno = 0;
  printf("setregid: %d", setregid(123, 456));
  printf(", errno: %d\n", errno);
  errno = 0;
  printf("setreuid: %d", setreuid(123, 456));
  printf(", errno: %d\n", errno);
  errno = 0;

  gid_t groups[10] = {42};
  printf("getgroups: %d", getgroups(10, groups));
  printf(", result: %d", groups[0]);
  printf(", errno: %d\n", errno);
  errno = 0;

  gid_t groups2[1] = {0};
  printf("setgroups: %d", setgroups(1, groups2));
  printf(", errno: %d\n", errno);
  errno = 0;

  return 0;
}
