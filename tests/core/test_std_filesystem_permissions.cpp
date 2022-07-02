#include <stdio.h>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <cassert>

void printResult(int value) {
    std::cout<< "file permissions: " << value << std::endl;
}

int main() {

  std::error_code ec;

  // Creates a unique filename that does not name a currently existing file
  const auto fileName = std::tmpnam(nullptr);

  std::wofstream ofs{ fileName };
  ofs.close();

  // check whether file exist or not
  assert(std::__fs::filesystem::exists(fileName));
  
  // now set the permission to the file 
  // ref: https://en.cppreference.com/w/cpp/filesystem/perms to see the value of group_read
  std::__fs::filesystem::permissions(fileName, std::__fs::filesystem::perms::group_read, ec);
  assert(!ec);
  // Now get the File Status
  auto s = std::__fs::filesystem::status(fileName, ec);
  // Now get the file permission which was set above
  std::__fs::filesystem::perms perm = s.permissions(); 

  printResult((int)perm);
  return 0;
}

