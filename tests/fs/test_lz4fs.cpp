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
  int counter = 0;
  int i = 0;
  printf("read from files\n");
  for (int i = 0; i < 10*1024*128 - 5; i += random() % 1000) {
    int which = i % 3;
    FILE *f = files[which];
    //printf("%d read %d: %d (%d)\n", counter, which, i, i % 10);
    int off = i % 10;
    int ret = fseek(f, i, SEEK_SET);
    assert(ret == 0);
    num = fread(buffer, 1, 5, f);
    if (num != 5) {
      printf("%d read %d: %d failed num\n", counter, which, i);
      abort();
    }
    buffer[5] = 0;
    char correct[] = "01234567890123456789";
    if (strncmp(buffer, correct + off, 5) != 0) {
      printf("%d read %d: %d (%d) failed data\n", counter, which, i, i % 10);
      abort();
    }
    counter++;
  }
  double after = emscripten_get_now();
  fclose(f1);
  fclose(f2);
  fclose(f3);
  printf("success. read IO time: %f (%d reads), total time: %f\n", after - before, counter, after - before_it_all);

  // all done
  int result = 1;
  REPORT_RESULT();
}

}

int main() {
  before_it_all = emscripten_get_now();

  EM_ASM({
    var meta, data;
    function maybeReady() {
      if (!(meta && data)) return;

      meta = JSON.parse(meta);

      Module.print('loading into filesystem');
      FS.mkdir('/files');
      FS.mount(LZ4FS, {
        packages: [{ metadata: meta, data: data }]
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
    data_xhr.responseType = "arraybuffer";
    data_xhr.onload = function() {
      Module.print('got data');
      data = data_xhr.response;
      maybeReady();
    };
    data_xhr.send();
  });

  emscripten_exit_with_live_runtime();

  return 1;
}

