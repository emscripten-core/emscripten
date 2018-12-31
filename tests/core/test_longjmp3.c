#include <setjmp.h>
#include <stdio.h>

typedef struct {
  jmp_buf* jmp;
} jmp_state;

void setjmp_func(jmp_state* s, int level) {
  jmp_buf* prev_jmp = s->jmp;
  jmp_buf c_jmp;

  if (level == 2) {
    printf("level is 2, perform longjmp!\n");
    longjmp(*(s->jmp), 1);
  }

  if (setjmp(c_jmp) == 0) {
    printf("setjmp normal execution path, level: %d\n", level);
    s->jmp = &c_jmp;
    setjmp_func(s, level + 1);
  } else {
    printf("setjmp exception execution path, level: %d\n", level);
    if (prev_jmp) {
      printf("prev_jmp is not empty, continue with longjmp!\n");
      s->jmp = prev_jmp;
      longjmp(*(s->jmp), 1);
    }
  }

  printf("Exiting setjmp function, level: %d\n", level);
}

int main(int argc, char* argv[]) {
  jmp_state s;
  s.jmp = NULL;

  setjmp_func(&s, 0);

  return 0;
}
