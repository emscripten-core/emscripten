#include <stdio.h>
#include <exception>

// multiple inheritance of base classes that are not empty, so adjustments are necessary

struct boost_exception {
  int x;
};

struct my_error: boost_exception, std::exception { };

int main() {
  try {
    throw my_error();
  } catch( boost_exception & ) {
    puts("a");
    try {
      puts("b");
      throw;
     } catch( my_error & ) {
        puts("c");
     }
  }
  puts("d");
  return 0;
}
