#include <stdio.h>

// We could also include the .wasm.h file for
// these, but let's declare things for the example.

extern void wasmbox_init(void);

extern int (*Z_do_bad_thingZ_ii)(int);

extern int (*Z_twiceZ_ii)(int);

int main() {
  puts("Initializing sandboxed unsafe library");
  wasmbox_init();
  printf("Calling twice on 21 returns %d\n", Z_twiceZ_ii(21));
  puts("Calling something bad now...");
  int num = Z_do_bad_thingZ_ii(1);
  printf("The sandbox should not have been able to print anything.\n"
         "It claims it printed %d chars but the test proves it didn't!\n", num);
}
