// Copyright 2014 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

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

