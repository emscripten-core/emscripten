// basic file operations
#include <iostream>
#include <fstream>
#include <string>
#include <emscripten.h>
#include "pthreadfs.h"

int main () {
  emscripten_init_pthreadfs();
  std::cout << "Proof that stdout works fine.\n";
  std::ofstream myfile;
  myfile.open ("filesystemaccess/example");
  myfile << "Writing a few characters.\n";
  myfile.close();

  std::string line;
  std::ifstream myfile_read ("filesystemaccess/example");
 
  if (myfile_read.is_open()) {
    std::getline(myfile_read, line);
    EM_ASM({console.log("Read line" + UTF8ToString($0));
    }, line.c_str());
    myfile_read.close();
  }

  std::ofstream stream1 ("filesystemaccess/multistreamexample");
  std::ofstream stream2 ("filesystemaccess/multistreamexample");
  stream1 << "Write a line through stream1.\n";
  stream2 << "Write a line through stream2.\n";
  stream1.close();
  stream2.close();

  std::remove("filesystemaccess/multistreamexample"); 
  bool can_open_deleted_file = (bool) std::ifstream("filesystemaccess/multistreamexample");
  if(!can_open_deleted_file) { 
    std::cout << "Opening deleted file failed, as expected.\n"; 
  }

  EM_ASM(console.log('after close'););
  EM_PTHREADFS_ASM( function timeout(ms) { return new Promise(resolve => setTimeout(resolve, ms)); }
    await timeout(3000);
    console.log("3 seconds after close");)
  EM_PTHREADFS_ASM( function timeout(ms) { return new Promise(resolve => setTimeout(resolve, ms)); }
    await timeout(1000);
    console.log("4 seconds after close");)
  return 0;
}
