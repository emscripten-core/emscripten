#include <assert.h>
#include <emscripten/console.h>
#include <setjmp.h>
#include <malloc.h>
#include <stdlib.h>

jmp_buf env;

void luaWork(int d){
  int x;
  emscripten_outf("d is at %d", d);
  assert(d <= 10);

  longjmp(env, 1);
}

void dump() {
  struct mallinfo m = mallinfo();
  emscripten_outf("dump: %d , %d", m.arena, m.uordblks);
}

void work(int n) {
  emscripten_outf("work %d", n);
  dump();

  emscripten_outf("calling luaWork with n=%d", n);
  if (!setjmp(env)){
    luaWork(n);
  }

  if (n > 0) work(n-1);
}

int main() {
  struct mallinfo m1 = mallinfo();
  dump();
  work(10);
  dump();
  struct mallinfo m2 = mallinfo();
  assert(m1.uordblks == m2.uordblks);
  emscripten_out("ok.");
}
