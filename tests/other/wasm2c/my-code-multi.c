#include <stdio.h>

// Note the prefixing (a_, b_) on all functions from the two libraries.

extern void a_wasmbox_init(void);
extern void b_wasmbox_init(void);

extern int (*a_Z_twiceZ_ii)(int);
extern int (*b_Z_thriceZ_ii)(int);

int main() {
  puts("Initializing sandboxed unsafe libraries");
  a_wasmbox_init();
  b_wasmbox_init();
  printf("Calling twice on 21 returns %d\n", a_Z_twiceZ_ii(21));
  printf("Calling thrice on 10 returns %d\n", b_Z_thriceZ_ii(10));
}
