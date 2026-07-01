#include <stdlib.h>

int some_arg;

int main() {
  return (int)(long)malloc(some_arg);
}
