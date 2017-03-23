#include <emscripten.h>
#include <assert.h>

int main() {
  int x = EM_ASM_INT_V({ return Runtime.stackSave(); });
  int y = EM_ASM_INT_V({ return Runtime.stackSave(); });
  assert(x == y);
  EM_ASM_INT({ print($0); }, &x);
  EM_ASM({ Module.print('success'); });
}
