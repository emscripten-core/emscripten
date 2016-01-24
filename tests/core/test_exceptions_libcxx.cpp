#include <string>
#include <iostream>
#include <stdexcept>

int main() {
  std::string s;
  try {
    std::cout << s.at(0) << std::endl;
  } catch (const std::out_of_range& e) {
    std::cout << "Exception caught:" << std::endl;
    std::cout << e.what() << std::endl;
  }
  return 0;
}

