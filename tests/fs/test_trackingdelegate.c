#include <stdio.h>
#include <stdlib.h>
#include <emscripten.h>

int main() {

  EM_ASM(
    FS.trackingDelegate['willMovePath'] = function(oldpath, newpath) {
      Module.print('About to move "' + oldpath + '" to "' + newpath + '"');
    };
    FS.trackingDelegate['onMovePath'] = function(oldpath, newpath) {
      Module.print('Moved "' + oldpath + '" to "' + newpath + '"');
    };
    FS.trackingDelegate['willDeletePath'] = function(path) {
      Module.print('About to delete "' + path + '"');
    };
    FS.trackingDelegate['onDeletePath'] = function(path) {
      Module.print('Deleted "' + path + '"');
    };
    FS.trackingDelegate['onOpenFile'] = function(path, flags) { 
      Module.print('Opened "' + path + '" with flags ' + flags);
    };
    FS.trackingDelegate['onWriteToFile'] = function(path) {
      Module.print('Wrote to file "' + path + '"');
    };
  );

  FILE *file;
  file = fopen("/file.txt", "w");
  fputs("hello!", file);
  fclose(file);
  rename("/file.txt", "/renamed.txt");
  file = fopen("/renamed.txt", "r");
  char str[256] = {};
  fgets(str, 255, file);
  printf("File read returned '%s'\n", str);
  fclose(file);
  remove("/renamed.txt");
}
