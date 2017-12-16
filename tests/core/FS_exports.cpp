#include<emscripten.h>

int main() {
#ifdef USE_FILES
  if (fopen("nonexistend", "r")) {
    puts("that was bad");
    return 1;
  }
#endif
#ifdef DIRECT
  EM_ASM({
    FS.createDataFile("/", "file.txt", [1, 2, 3]);
  });
#else
  EM_ASM({
    Module["FS_createDataFile"]("/", "file.txt", [1, 2, 3]);
  });
#endif
  EM_ASM({
    // use eval, so that the compiler can't see FS usage statically
    eval('Module.print("Data: " + JSON.stringify(MEMFS.getFileDataAsRegularArray(FS.root.contents["file.txt"])))');
  });
}

