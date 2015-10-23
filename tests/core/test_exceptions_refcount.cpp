#include <stdio.h>
#include <exception>

class MyException : public std::exception {
public:
  MyException() { printf("me now!\n"); }
  ~MyException() _NOEXCEPT { printf("i'm over!\n"); }
};

int main() {
  {
    const std::exception_ptr p;
  }
  {
    MyException m;
  }
  printf("ok.\n");
  return 0;
}

