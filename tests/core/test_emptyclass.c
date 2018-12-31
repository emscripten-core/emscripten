#include <stdio.h>

struct Randomized {
  Randomized(int x) { printf("*zzcheezzz*\n"); }
};

int main(int argc, const char *argv[]) {
  new Randomized(55);

  return 0;
}
