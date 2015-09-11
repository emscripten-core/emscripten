#include <stdio.h>
#include <emscripten.h>

void loop_iter() {
  EM_ASM(Module.print('loop iter!'));
}

int main(int argc, char **argv) {
  EM_ASM(Module.print('hello dere1'));
  EM_ASM("Module.print('hello dere2');");
  emscripten_debugger(); // does nothing in shells; check for validation error though
  for (int i = 0; i < 3; i++) {
    EM_ASM(Module.print('hello dere3'); Module.print('hello dere' + 4););
  }
  EM_ASM_({ Module.print('hello input ' + $0) }, 123);
  EM_ASM_ARGS({ Module.print('hello input ' + $0) }, 456);
  int sum = 0;
  for (int i = 0; i < argc * 3; i++) {
    sum += EM_ASM_INT({
                        Module.print('i: ' + [ $0, ($1).toFixed(2) ]);
                        return $0 * 2;
                      },
                      i, double(i) / 12);
  }
  EM_ASM_INT({ globalVar = $0 }, sum); // no outputs, just input
  sum = 0;
  sum = EM_ASM_INT_V({ return globalVar }); // no inputs, just output
  printf("sum: %d\n", sum);
  printf("|%.2f|\n", EM_ASM_DOUBLE({
    return $0; // return double properly
  }, 1.2));
  for (int i = 0; i < argc*2; i++) loop_iter();
  return 0;
}
