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
    Module.print("Data: " + JSON.stringify(Array.prototype.slice.call(FS.root.contents["file.txt"].contents)));
  });
#else
  EM_ASM({
    Module["FS_createDataFile"]("/", "file.txt", [1, 2, 3]);
    Module.print("Data: " + JSON.stringify(Array.prototype.slice.call(Module["FS"].root.contents["file.txt"].contents)));
  });
#endif
}

