#include <iostream>
#include <thread>

int main() {
  std::thread([]() {
    int* p = new int(1);
    delete p;
    std::cout << *p;
  }).join();
  return 0;
}
