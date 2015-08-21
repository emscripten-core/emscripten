#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <emscripten.h>

extern "C" {

void EMSCRIPTEN_KEEPALIVE finish() {
  // load some file data, SYNCHRONOUSLY :)
  char buffer[100];
  int num;

  printf("load first file\n");
  FILE *f1 = fopen("files/file1.txt", "r");
  assert(f1);
  num = fread(buffer, 1, 5, f1);
  assert(num == 5);
  fclose(f1);
  buffer[5] = 0;
  assert(strcmp(buffer, "first") == 0);

  printf("load second file\n");
  FILE *f2 = fopen("files/sub/file2.txt", "r");
  assert(f2);
  num = fread(buffer, 1, 6, f2);
  assert(num == 6);
  fclose(f2);
  buffer[6] = 0;
  assert(strcmp(buffer, "second") == 0);

  // all done
  printf("success\n");
  int result = 1;
  REPORT_RESULT();
}

}

int main() {
  // Load the metadata and data of our file package. When they arrive, load the contents of the package into our filesystem.
  // The data arrives as a Blob, which could in other cases arrive from any other way a Blob can arrive:
  //   * Local file the user selected
  //   * Data loaded from IndexedDB
  // In all cases, including the one here of a network request, Blobs allow the browser to optimize them so that
  // a large file is not necessarily all in memory at once.
  EM_ASM({
    var meta, blob;
    function maybeReady() {
      if (!(meta && blob)) return;

      meta = JSON.parse(meta);

      Module.print('loading into filesystem');
      FS.mkdir('/files');
      FS.mount(WORKERFS, {
        packages: [{ metadata: meta, blob: blob }]
      }, '/files');

      Module.ccall('finish');
    }

    var meta_xhr = new XMLHttpRequest();
    meta_xhr.open("GET", "files.js.metadata", true);
    meta_xhr.responseType = "text";
    meta_xhr.onload = function() {
      Module.print('got metadata');
      meta = meta_xhr.response;
      maybeReady();
    };
    meta_xhr.send();

    var data_xhr = new XMLHttpRequest();
    data_xhr.open("GET", "files.data", true);
    data_xhr.responseType = "blob";
    data_xhr.onload = function() {
      Module.print('got data');
      blob = data_xhr.response;
      maybeReady();
    };
    data_xhr.send();
  });

  emscripten_exit_with_live_runtime();

  return 1;
}

