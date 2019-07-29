#include <emscripten.h>
#include <stdio.h>

void scan(void* x, void* y) {
  printf("scan\n");
  int* p = (int*)x;
  int* q = (int*)y;
  while (p < q) {
    printf("scan sees: %d\n", *p);
    p++;
  }
}

int main() {
  int x = EM_ASM_INT({ return 314159 });
  int y = EM_ASM_INT({ return 21828 });
  emscripten_scan_registers(scan);
  printf("%d, %d\n", x, y);
}

