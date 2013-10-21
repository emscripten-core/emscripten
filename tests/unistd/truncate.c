#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <emscripten.h>

int main() {
  EM_ASM(
    FS.mkdir('working');
#if NODEFS
    FS.mount(NODEFS, { root: '.' }, 'working');
#endif
    FS.chdir('working');
    FS.writeFile('towrite', 'abcdef');
    FS.writeFile('toread', 'abcdef');
    FS.chmod('toread', 0444);
  );

  struct stat s;
  int f = open("towrite", O_WRONLY);
  int f2 = open("toread", O_RDONLY);
  printf("f2: %d\n", f2);

  fstat(f, &s);
  printf("st_size: %d\n", s.st_size);
  memset(&s, 0, sizeof s);
  errno = 0;
  printf("\n");

  printf("ftruncate(10): %d\n", ftruncate(f, 10));
  printf("errno: %d\n", errno);
  fstat(f, &s);
  printf("st_size: %d\n", s.st_size);
  memset(&s, 0, sizeof s);
  errno = 0;
  printf("\n");

  printf("ftruncate(4): %d\n", ftruncate(f, 4));
  printf("errno: %d\n", errno);
  fstat(f, &s);
  printf("st_size: %d\n", s.st_size);
  memset(&s, 0, sizeof s);
  errno = 0;
  printf("\n");

  printf("ftruncate(-1): %d\n", ftruncate(f, -1));
  printf("errno: %d\n", errno);
  fstat(f, &s);
  printf("st_size: %d\n", s.st_size);
  memset(&s, 0, sizeof s);
  errno = 0;
  printf("\n");

  printf("truncate(2): %d\n", truncate("towrite", 2));
  printf("errno: %d\n", errno);
  stat("towrite", &s);
  printf("st_size: %d\n", s.st_size);
  memset(&s, 0, sizeof s);
  errno = 0;
  printf("\n");

  printf("truncate(readonly, 2): %d\n", truncate("toread", 2));
  printf("errno: %d\n", errno);
  stat("toread", &s);
  printf("st_size: %d\n", s.st_size);
  memset(&s, 0, sizeof s);
  errno = 0;
  printf("\n");

  printf("ftruncate(readonly, 4): %d\n", ftruncate(f2, 4));
  printf("errno: %d\n", errno);
  fstat(f2, &s);
  printf("st_size: %d\n", s.st_size);
  memset(&s, 0, sizeof s);
  errno = 0;

  // Restore full permissions on all created files so that python test runner rmtree
  // won't have problems on deleting the files. On Windows, calling shutil.rmtree()
  // will fail if any of the files are read-only.
  EM_ASM(
    FS.chmod('toread', 0777);
  );
  return 0;
}
