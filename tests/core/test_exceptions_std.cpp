#include <stdio.h>
#include <exception>

int main() {
  std::exception e;
  try {
    throw e;
  }
  catch (std::exception e) {
    printf("what? %s\n", e.what());
    printf("caught std::exception\n");
  }
  catch (int x) {
    throw std::bad_exception();
  }
  return 0;
}
