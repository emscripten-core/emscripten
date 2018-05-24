#include <emscripten.h>
#include <assert.h>

int main() {
  int x = EM_ASM_INT({ return stackSave(); });
  int y = EM_ASM_INT({ return stackSave(); });
  EM_ASM_INT({ Module.print($0); }, &x);
  EM_ASM_INT({ Module.print($0); }, &y);
  assert(x == y);
  EM_ASM({ Module.print('success'); });
}
