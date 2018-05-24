#include <setjmp.h>
#include <stdio.h>

typedef struct {
  jmp_buf* jmp;
} jmp_state;

void stack_manipulate_func(jmp_state* s, int level) {
  jmp_buf buf;

  printf("Entering stack_manipulate_func, level: %d\n", level);

  if (level == 0) {
    s->jmp = &buf;
    if (setjmp(*(s->jmp)) == 0) {
      printf("Setjmp normal execution path, level: %d\n", level);
      stack_manipulate_func(s, level + 1);
    } else {
      printf("Setjmp error execution path, level: %d\n", level);
    }
  } else {
    printf("Perform longjmp at level %d\n", level);
    longjmp(*(s->jmp), 1);
  }

  printf("Exiting stack_manipulate_func, level: %d\n", level);
}

int main(int argc, char* argv[]) {
  jmp_state s;
  s.jmp = NULL;
  stack_manipulate_func(&s, 0);

  return 0;
}
