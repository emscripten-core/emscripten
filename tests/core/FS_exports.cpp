#include<emscripten.h>

int main() {
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
    Module.print("Data: " + JSON.stringify(MEMFS.getFileDataAsRegularArray(FS.root.contents["file.txt"])));
  });
}

