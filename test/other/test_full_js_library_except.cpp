#include <cstdio>

int main(int argc, char* argv[]) {
  try {
    puts("throw...");
    throw 1;
    puts("(never reached)");
  } catch(...) {
    puts("catch!");
  }

  return 0;
}
