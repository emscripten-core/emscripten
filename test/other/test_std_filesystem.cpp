#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

int main() {
  fs::path tmp{std::filesystem::temp_directory_path()};

  const auto foo{"foo"};
  std::ofstream{tmp / foo} << foo; // creates file containing "foo"
  std::cout << "remove(): " << fs::remove(tmp / foo) << '\n'; // success
  std::cout << "remove(): " << fs::remove(tmp / foo) << '\n'; // fail

  std::filesystem::create_directories(tmp / "abcdef/example");
  const std::uintmax_t n{fs::remove_all(tmp / "abcdef")};
  std::cout << "remove_all(): " << n << " files or directories\n";
}
