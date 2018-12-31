#include <setjmp.h>
#include <stdio.h>

typedef struct {
  jmp_buf* jmp;
} jmp_state;

void second_func(jmp_state* s);

void first_func(jmp_state* s) {
  jmp_buf* prev_jmp = s->jmp;
  jmp_buf c_jmp;
  volatile int once = 0;

  if (setjmp(c_jmp) == 0) {
    printf("Normal execution path of first function!\n");

    s->jmp = &c_jmp;
    second_func(s);
  } else {
    printf("Exception execution path of first function! %d\n", once);

    if (!once) {
      printf("Calling longjmp the second time!\n");
      once = 1;
      longjmp(*(s->jmp), 1);
    }
  }
}

void second_func(jmp_state* s) { longjmp(*(s->jmp), 1); }

int main(int argc, char* argv[]) {
  jmp_state s;
  s.jmp = NULL;

  first_func(&s);

  return 0;
}
