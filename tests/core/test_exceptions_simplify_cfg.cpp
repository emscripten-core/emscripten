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

