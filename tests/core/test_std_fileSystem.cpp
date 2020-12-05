#include <stdio.h>
#include <iostream>
#include <filesystem>
#include <fstream>

void printResult(int value) {
  std::cout<< value << std::endl;
}

int main() {

  std::error_code ec;

  // Creates a unique filename that does not name a currently existing file
  const auto fileName = std::tmpnam(nullptr);

  std::wofstream ofs{ fileName };
  ofs.close();

  // check whether file exist or not
  if (std::__fs::filesystem::exists(fileName))
  {
    //std::cout <<__FILE__<<" "<<__FUNCTION__ <<" "<<__LINE__<<"\n" ;
  }
  
  // now set the permission to the file 
  std::__fs::filesystem::permissions(fileName,std::__fs::filesystem::perms::group_read,ec);
  if (ec) {
     //std::cout <<__FILE__<<" "<<__FUNCTION__ <<" "<<__LINE__<< " " << ec << "\n" ;
  }
  // Now get the File Status
  auto s = std::__fs::filesystem::status(fileName, ec);
  // Now get the file permission which was set above
  std::__fs::filesystem::perms perm = s.permissions();
 
   printResult((int)perm);
  return 0;
}