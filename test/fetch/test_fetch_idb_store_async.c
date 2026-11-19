// Copyright 2016 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <assert.h>
#include <emscripten.h>
#include <emscripten/eventloop.h>
#include <emscripten/fetch.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FILE_SIZE 10240

static uint8_t *expected_data;

void failure(emscripten_fetch_t *fetch) {
  printf("IDB operation failed: status=%d\n", fetch->status);
  emscripten_fetch_close(fetch);
  assert(false && "IDB operation failed");
}

void delete_success(emscripten_fetch_t *fetch) {
  printf("IDB delete succeeded.\n");
  emscripten_fetch_close(fetch);
  free(expected_data);
  printf("done\n");
}

void load_success(emscripten_fetch_t *fetch) {
  printf("IDB load succeeded.\n");
  assert(fetch->status == 200);
  assert(fetch->numBytes == FILE_SIZE);
  assert(fetch->data != NULL);
  assert(memcmp(fetch->data, expected_data, FILE_SIZE) == 0);
  emscripten_fetch_close(fetch);

  emscripten_fetch_attr_t attr;
  emscripten_fetch_attr_init(&attr);
  strcpy(attr.requestMethod, "EM_IDB_DELETE");
  attr.onsuccess = delete_success;
  attr.onerror = failure;
  emscripten_fetch(&attr, "outputfile.dat");
}

EMSCRIPTEN_KEEPALIVE void on_idb_verified() {
  emscripten_fetch_attr_t attr;
  emscripten_fetch_attr_init(&attr);
  strcpy(attr.requestMethod, "GET");
  attr.attributes = EMSCRIPTEN_FETCH_NO_DOWNLOAD | EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
  attr.onsuccess = load_success;
  attr.onerror = failure;
  emscripten_fetch(&attr, "outputfile.dat");
  emscripten_runtime_keepalive_pop();
}

EM_JS(void, check_idb_entry, (const char *filename, int expected_size), {
  var key = UTF8ToString(Number(filename));
  var req = indexedDB.open('emscripten_filesystem', 1);
  req.onerror = () => abort('indexedDB.open failed');
  req.onsuccess = () => {
    var db = req.result;
    var tx = db.transaction(['FILES'], 'readonly');
    var getReq = tx.objectStore('FILES').get(key);
    getReq.onerror = () => abort('objectStore.get failed');
    getReq.onsuccess = () => {
      var val = getReq.result;
      if (!val || val.byteLength !== expected_size ||
          val.byteOffset !== 0 || val.buffer.byteLength !== expected_size) {
        abort(`unexpected IDB value: byteLength=${val?.byteLength}, byteOffset=${val?.byteOffset}, buffer.byteLength=${val?.buffer?.byteLength}`);
      }
      db.close();
      _on_idb_verified();
    };
  };
});

void store_success(emscripten_fetch_t *fetch) {
  printf("IDB store succeeded.\n");
  emscripten_fetch_close(fetch);
  emscripten_runtime_keepalive_push();
  check_idb_entry("outputfile.dat", FILE_SIZE);
}

void persistFileToIndexedDB(const char *outputFilename, uint8_t *data, size_t numBytes) {
  emscripten_fetch_attr_t attr;
  emscripten_fetch_attr_init(&attr);
  strcpy(attr.requestMethod, "EM_IDB_STORE");
  attr.attributes = EMSCRIPTEN_FETCH_REPLACE | EMSCRIPTEN_FETCH_PERSIST_FILE;
  attr.requestData = (char *)data;
  attr.requestDataSize = numBytes;
  attr.onsuccess = store_success;
  attr.onerror = failure;
  emscripten_fetch(&attr, outputFilename);
}

int main() {
  // Create data
  expected_data = (uint8_t*)malloc(FILE_SIZE);
  for (int i = 0; i < FILE_SIZE; ++i) {
    expected_data[i] = (uint8_t)rand();
  }

  persistFileToIndexedDB("outputfile.dat", expected_data, FILE_SIZE);
}
