#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

int main() {
  // libc++'s temp_directory_path depends on one of these env vars
  // being set, and falls back to /tmp as a default.
  // Log these values in case this test ever fails:
  const char* env_paths[] = {"TMPDIR", "TMP", "TEMP", "TEMPDIR"};
  for (int i = 0; i < sizeof(env_paths)/sizeof(char*); i++) {
    const char* p = getenv(env_paths[i]);
    std::cout << env_paths[i] << " -> " << (p ? p : "(NULL)") << "\n";
    if (p) {
      std::cout <<  env_paths[i] << " exists: " << fs::exists(env_paths[i]) << "\n";
    }
  }
  std::cout << "default /tmp exists: " << fs::exists("/tmp") << "\n";

  fs::path tmp{fs::temp_directory_path()};
  std::cout << "temp_directory_path: " << tmp << "\n";
  assert(fs::exists(tmp));
  std::cout << "exists ok!" << "\n";
  return 0;
}
