#include <emscripten.h>
template <typename... Args>
int call(Args... args) {
  return(EM_ASM_INT(
    {
      Module.print(Array.prototype.join.call(arguments, ','));
    },
    args...
  ));
}

int main(int argc, char **argv) {
  call(1);
  call(1, 2);
  call(1, 2, 3);
  return 0;
}
