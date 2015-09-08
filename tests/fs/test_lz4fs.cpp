#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include <emscripten.h>

#define TOTAL_SIZE (10*1024*128)

double before_it_all;

extern "C" {

void EMSCRIPTEN_KEEPALIVE finish() {
  // load some file data, SYNCHRONOUSLY :)
  char buffer[100];
  int num;

  printf("load files\n");
  FILE *f1 = fopen("file1.txt", "r");
  assert(f1);
  FILE *f2 = fopen("subdir/file2.txt", "r");
  assert(f2);
  FILE *f3 = fopen("file3.txt", "r");
  assert(f3);
  FILE *files[] = { f1, f2, f3 };
  double before = emscripten_get_now();
  int counter = 0;
  int i = 0;
  printf("read from files\n");
  for (int i = 0; i < TOTAL_SIZE - 5; i += random() % 1000) {
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
    if (which != 2) {
      buffer[5] = 0;
      char correct[] = "01234567890123456789";
      if (strncmp(buffer, correct + which + off, 5) != 0) {
        printf("%d read %d: %d (%d) failed data\n", counter, which, i, i % 10);
        abort();
      }
    }
    counter++;
  }
  assert(counter == 2657);
  double after = emscripten_get_now();

  printf("final test on random data\n");
  int ret = fseek(f3, 17, SEEK_SET);
  assert(ret == 0);
  num = fread(buffer, 1, 1, f3);
  assert(num == 1);
  assert(buffer[0] == 'X');

  printf("read success. read IO time: %f (%d reads), total time: %f\n", after - before, counter, after - before_it_all);

#if LOAD_MANUALLY
  printf("caching tests\n");
  ret = fseek(f3, TOTAL_SIZE - 5, SEEK_SET); assert(ret == 0);
  num = fread(buffer, 1, 1, f3); assert(num == 1); // read near the end
  ret = fseek(f3, TOTAL_SIZE - 5000, SEEK_SET); assert(ret == 0);
  num = fread(buffer, 1, 1, f3); assert(num == 1); // also near the end
  EM_ASM({
    assert(!Module['decompressedChunks']);
    Module.compressedData.debug = true;
    console.log('last cached indexes ' + Module.compressedData.cachedIndexes);
    assert(Module.compressedData.cachedIndexes.indexOf(0) < 0); // 0 is not cached
  });
  printf("multiple reads of same byte\n");
  for (int i = 0; i < 100; i++) {
    ret = fseek(f1, 0, SEEK_SET); // read near the start, should trigger one decompress, then all cache hits
    assert(ret == 0);
    num = fread(buffer, 1, 1, f1);
    assert(num == 1);
  }
  EM_ASM({
    assert(Module['decompressedChunks'] == 1, ['seeing', Module['decompressedChunks'], 'decompressed chunks']);
  });
  printf("multiple reads of adjoining byte\n");
  for (int i = 0; i < 100; i++) {
    ret = fseek(f1, i, SEEK_SET);
    assert(ret == 0);
    num = fread(buffer, 1, 1, f1);
    assert(num == 1);
  }
  EM_ASM({
    assert(Module['decompressedChunks'] == 1, ['seeing', Module['decompressedChunks'], 'decompressed chunks']);
  });
  printf("multiple reads across two chunks\n");
  for (int i = 0; i < 2100; i++) {
    ret = fseek(f1, i, SEEK_SET);
    assert(ret == 0);
    num = fread(buffer, 1, 1, f1);
    assert(num == 1);
  }
  EM_ASM({
    assert(Module['decompressedChunks'] == 2, ['seeing', Module['decompressedChunks'], 'decompressed chunks']);
  });
  printf("caching test ok\n");
#endif

  fclose(f1);
  fclose(f2);
  fclose(f3);

  // all done
  int result;
#if LOAD_MANUALLY
  result = 1;
#else
  result = 2;
#endif
  REPORT_RESULT();
}

}

int main() {
  before_it_all = emscripten_get_now();

#if LOAD_MANUALLY
  EM_ASM({
    var COMPLETE_SIZE = 10*1024*128*3;

    var meta, data;
    function maybeReady() {
      if (!(meta && data)) return;

      meta = JSON.parse(meta);

      Module.print('loading into filesystem');
      FS.mkdir('/files');
      LZ4.loadPackage({ 'metadata': meta, 'data': data });

      Module.compressedData = FS.root.contents['file1.txt'].contents.compressedData;
      var compressedSize = Module.compressedData.data.length;
      var low = COMPLETE_SIZE/3;
      var high = COMPLETE_SIZE/2;
      console.log('seeing compressed size of ' + compressedSize + ', expect in ' + [low, high]);
      assert(compressedSize > low && compressedSize < high); // more than 1/3, because 1/3 is uncompressible, but still, less than 1/2

      Module['ccall']('finish');
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
#else
  finish();
#endif

  return 1;
}

