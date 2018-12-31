#include <stdio.h>
#include <thread>

int main() {
  int x = std::this_thread::get_id() == std::this_thread::get_id();
  printf("is this thread the same as this thread? %d\n", x);
}

