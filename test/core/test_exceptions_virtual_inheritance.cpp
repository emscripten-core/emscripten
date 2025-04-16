// Copyright 2014 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <iostream>
#include <stdexcept>

using std::cout;
using std::endl;

struct my_exception : public virtual std::runtime_error {
  // To allow this to be thrown directly in the tests below.
  explicit my_exception(const std::string &what)
    : std::runtime_error(what)
  {}

protected:
    my_exception()
      // This won't be called because of virtual inheritance.
      : std::runtime_error("::my_exception")
  {}
};

int main(const int argc, const char * const * const argv) {
  try {
    cout << "Throwing ::my_exception" << endl;
    throw ::my_exception("my_what");
  } catch(const std::runtime_error &ex) {
    cout << "Caught std::runtime_error: " << ex.what() << endl;
  }
}

