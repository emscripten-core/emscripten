#include <setjmp.h>

void longjmp_side(jmp_buf* buf) {
  longjmp(*buf, 42); // jumps back to where setjmp was called
}
