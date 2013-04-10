#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

int main() {
  int f = open("/", O_RDONLY);

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

  printf("tcgetpgrp(good): %d", tcgetpgrp(f));
  printf(", errno: %d\n", errno);
  errno = 0;
  printf("tcgetpgrp(bad): %d", tcgetpgrp(42));
  printf(", errno: %d\n", errno);
  errno = 0;

  printf("tcsetpgrp(good): %d", tcsetpgrp(f, 123));
  printf(", errno: %d\n", errno);
  errno = 0;
  printf("tcsetpgrp(bad): %d", tcsetpgrp(42, 123));
  printf(", errno: %d\n", errno);
  errno = 0;

  printf("link: %d", link("/here", "/there"));
  printf(", errno: %d\n", errno);
  errno = 0;

  printf("lockf(good): %d", lockf(f, 123, 456));
  printf(", errno: %d\n", errno);
  errno = 0;
  printf("lockf(bad): %d", lockf(42, 123, 456));
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
  printf("execl: %d", execl("/program", "arg", 0));
  printf(", errno: %d\n", errno);
  errno = 0;
  printf("execle: %d", execle("/program", "arg", 0, exec_env));
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

  printf("chown(good): %d", chown("/", 123, 456));
  printf(", errno: %d\n", errno);
  errno = 0;
  printf("chown(bad): %d", chown("/noexist", 123, 456));
  printf(", errno: %d\n", errno);
  errno = 0;
  printf("lchown(good): %d", lchown("/", 123, 456));
  printf(", errno: %d\n", errno);
  errno = 0;
  printf("lchown(bad): %d", lchown("/noexist", 123, 456));
  printf(", errno: %d\n", errno);
  errno = 0;
  printf("fchown(good): %d", fchown(f, 123, 456));
  printf(", errno: %d\n", errno);
  errno = 0;
  printf("fchown(bad): %d", fchown(42, 123, 456));
  printf(", errno: %d\n", errno);
  errno = 0;

  printf("alarm: %d", alarm(42));
  printf(", errno: %d\n", errno);
  errno = 0;
  printf("ualarm: %ld", ualarm(123, 456));
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
  printf("getpid: %d", getpid());
  printf(", errno: %d\n", errno);
  errno = 0;
  printf("getppid: %d", getppid());
  printf(", errno: %d\n", errno);
  errno = 0;

  printf("getpgid: %d", getpgid(42));
  printf(", errno: %d\n", errno);
  errno = 0;
  printf("getsid: %d", getsid(42));
  printf(", errno: %d\n", errno);
  errno = 0;

  printf("setgid: %d", setgid(42));
  printf(", errno: %d\n", errno);
  errno = 0;
  printf("setegid: %d", setegid(42));
  printf(", errno: %d\n", errno);
  errno = 0;
  printf("setuid: %d", setuid(42));
  printf(", errno: %d\n", errno);
  errno = 0;
  printf("seteuid: %d", seteuid(42));
  printf(", errno: %d\n", errno);
  errno = 0;

  printf("setpgrp: %d", setpgrp());
  printf(", errno: %d\n", errno);
  errno = 0;
  printf("setsid: %d", setsid());
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
