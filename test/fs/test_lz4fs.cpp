// Copyright 2015 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>

#include <emscripten.h>

#define TOTAL_SIZE (10*1024*128)

double before_it_all;

extern "C" void EMSCRIPTEN_KEEPALIVE finish() {
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
  EM_ASM((
    assert(!Module['decompressedChunks']);
    Module['compressedData']['debug'] = true;
    console.log('last cached indexes ' + Module['compressedData']['cachedIndexes']);
    assert(Module['compressedData']['cachedIndexes'].indexOf(0) < 0); // 0 is not cached
  ));
  printf("multiple reads of same byte\n");
  for (int i = 0; i < 100; i++) {
    ret = fseek(f1, 0, SEEK_SET); // read near the start, should trigger one decompress, then all cache hits
    assert(ret == 0);
    num = fread(buffer, 1, 1, f1);
    assert(num == 1);
  }
  EM_ASM((
    assert(Module['decompressedChunks'] == 1, ['seeing', Module['decompressedChunks'], 'decompressed chunks']);
  ));
  printf("multiple reads of adjoining byte\n");
  for (int i = 0; i < 100; i++) {
    ret = fseek(f1, i, SEEK_SET);
    assert(ret == 0);
    num = fread(buffer, 1, 1, f1);
    assert(num == 1);
  }
  EM_ASM((
    assert(Module['decompressedChunks'] == 1, ['seeing', Module['decompressedChunks'], 'decompressed chunks']);
  ));
  printf("multiple reads across two chunks\n");
  for (int i = 0; i < 2100; i++) {
    ret = fseek(f1, i, SEEK_SET);
    assert(ret == 0);
    num = fread(buffer, 1, 1, f1);
    assert(num == 1);
  }
  EM_ASM((
    assert(Module['decompressedChunks'] == 2, ['seeing', Module['decompressedChunks'], 'decompressed chunks']);
  ));
  printf("caching test ok\n");
#endif

  fclose(f1);
  fclose(f2);
  fclose(f3);

  // attemping to read a lz4 node as a link should be invalid
  buffer[0] = '\0';
  assert(readlink("file1.txt", buffer, sizeof(buffer)) == -1);
  assert(buffer[0] == '\0');
  assert(errno == EINVAL);
  assert(readlink("subdir/file2.txt", buffer, sizeof(buffer)) == -1);
  assert(buffer[0] == '\0');
  assert(errno == EINVAL);

  // all done
  int result;
#if LOAD_MANUALLY
  result = 1;
#else
  result = 2;
#endif
  emscripten_force_exit(result);
}

int main() {
  before_it_all = emscripten_get_now();

#if LOAD_MANUALLY
  EM_ASM((
    var COMPLETE_SIZE = 10*1024*128*3;

    var meta, data;
    function maybeReady() {
      if (!(meta && data)) return;

      meta = JSON.parse(meta);

      out('loading into filesystem');
      FS.mkdir('/files');
      LZ4.loadPackage({ 'metadata': meta, 'data': data });

      Module['compressedData'] = FS.root.contents['file1.txt'].contents.compressedData;
      var compressedSize = Module['compressedData']['data'].length;
      var low = COMPLETE_SIZE/3;
      var high = COMPLETE_SIZE/2;
      console.log('seeing compressed size of ' + compressedSize + ', expect in ' + [low, high]);
      assert(compressedSize > low && compressedSize < high); // more than 1/3, because 1/3 is uncompressible, but still, less than 1/2

      ccall('finish');
    }

    fetch("files.js.metadata")
      .then((rsp) => rsp.text())
      .then((text) => {
        meta = text;
        maybeReady();
      });

    fetch("files.data")
      .then((rsp) => rsp.arrayBuffer())
      .then((buf) => {
        out('got data');
        data = buf;
        maybeReady();
      });
  ));

  emscripten_exit_with_live_runtime();
#else
  finish();
#endif

  return 1;
}

