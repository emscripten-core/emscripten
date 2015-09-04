#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include <emscripten.h>

double before_it_all;

extern "C" {

void EMSCRIPTEN_KEEPALIVE finish() {
  // load some file data, SYNCHRONOUSLY :)
  char buffer[100];
  int num;

  printf("load files\n");
  FILE *f1 = fopen("files/file1.txt", "r");
  assert(f1);
  FILE *f2 = fopen("files/file2.txt", "r");
  assert(f2);
  FILE *f3 = fopen("files/file3.txt", "r");
  assert(f3);
  FILE *files[] = { f1, f2, f3 };
  double before = emscripten_get_now();
  for (int i = 0; i < 100*1024*1024 - 5*1024*1024; i += 100*1024) {
    i += random() % 10;
    int which = i % 3;
    FILE *f = files[which];
    int off = i % 10;
    //printf("read %d: %d (%d)\n", which, i, i % 10);
    int ret = fseek(f, i, SEEK_SET);
    assert(ret == 0);
    num = fread(buffer, 1, 5, f);
    assert(num == 5);
    buffer[5] = 0;
    char correct[] = "01234567890123456789";
    assert(strncmp(buffer, correct + off, 5) == 0);
  }
  double after = emscripten_get_now();
  fclose(f1);
  fclose(f2);
  fclose(f3);
  printf("read IO time: %f\n", after - before);

  printf("total time: %f\n", after - before_it_all);

  // all done
  printf("success\n");
  int result = 1;
  REPORT_RESULT();
}

}

int main() {
  before_it_all = emscripten_get_now();

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

