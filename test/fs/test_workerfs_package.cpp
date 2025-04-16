// Copyright 2015 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

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
  REPORT_RESULT(1);
}

}

int main() {
  // Load the metadata and data of our file package. When they arrive, load the contents of the package into our filesystem.
  // The data arrives as a Blob, which could in other cases arrive from any other way a Blob can arrive:
  //   * Local file the user selected
  //   * Data loaded from IndexedDB
  // In all cases, including the one here of a network request, Blobs allow the browser to optimize them so that
  // a large file is not necessarily all in memory at once.
  EM_ASM((
    var meta, blob;
    function maybeReady() {
      if (!(meta && blob)) return;

      meta = JSON.parse(meta);

      out('loading into filesystem');
      FS.mkdir('/files');
      FS.mount(WORKERFS, {
        packages: [{ metadata: meta, blob: blob }]
      }, '/files');

      ccall('finish');
    }

    fetch("files.js.metadata")
      .then((rsp) => rsp.text())
      .then((text) => {
        out('got metadata');
        meta = text;
        maybeReady();
      });

    fetch("files.data")
      .then((rsp) => rsp.blob())
      .then((data) => {
        blob = data;
        maybeReady();
      });
  ));

  emscripten_exit_with_live_runtime();

  return 1;
}

