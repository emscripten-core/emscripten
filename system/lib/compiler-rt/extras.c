// Supprot functions for emscripten setjmp/longjmp and excecption handling
// support.
// See: https://llvm.org/doxygen/WebAssemblyLowerEmscriptenEHSjLj_8cpp.html

int __THREW__;
int __threwValue;
int __tempRet0;

void setThrew(int threw, int value) {
  if (__THREW__ == 0) {
    __THREW__ = threw;
    __threwValue = value;
  }
}

void setTempRet0(int value) {
  __tempRet0 = value;
}

void getTempRet0(int value) {
  __tempRet0 = value;
}
