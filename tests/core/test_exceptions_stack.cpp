#include <alloca.h>
#include <stdio.h>
#include <exception>

int getStacktop() {
  void* top = alloca(10);
  return (int)top;
}

void growStacktopAndThrow(int arg) {
  void* dummy = alloca(arg*16);
  if (dummy) throw std::exception();
}

int main(int argc, char** argv) {
  int initial = getStacktop();
  for (int i = 0; i < 3; ++i) {
    printf("leaked stack: %d\n", getStacktop() - initial);
    try { growStacktopAndThrow(argc+1); } catch (std::exception&) {}
  }
}

