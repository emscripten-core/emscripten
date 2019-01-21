// Copyright 2016 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

/**
 * This is the main test runner file for tests
 */
#include <iostream>
#include <stdio.h>

struct Before {
  bool allow() { return true; }

  std::string m_assertionInfo;
};

void do_throw2() {
  printf("- Throwing\n");
  throw std::runtime_error("runtime_error() thrown");
}

void runtest() {
  printf("- Run test\n");
  Before bef;
  if ( bef.allow() ) {
    try {
      do_throw2();
    }
    catch( std::runtime_error ) {
      printf("- Caught expected\n"); 
    }
  }
  else { printf("- not allowed\n"); }

}

int main( int argc, char* argv[] ) {
  runtest();
}

