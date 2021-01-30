#include <thread>

int main() {
  std::thread([]{}).join();
  return 0;
}
