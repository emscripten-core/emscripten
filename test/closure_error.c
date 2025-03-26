#include <emscripten.h>

int main() {
  EM_ASM({
    thisVarDoesNotExist++;
    thisVarDoesNotExistEither++;
  });
}
