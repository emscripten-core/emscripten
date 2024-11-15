// Copyright 2022 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <emscripten/fetch.h>
#include <emscripten/em_asm.h>

void handleSuccess(emscripten_fetch_t *fetch) {
  assert(false && "Should not succeed");
  emscripten_fetch_close(fetch);
}

void handleError(emscripten_fetch_t *fetch) {
  printf("handleError: %d %s\n", fetch->status, fetch->statusText);
  bool isAbortedStatus = fetch->status == (unsigned short) -1;
  assert(isAbortedStatus); // should have aborted status
  EM_ASM({
    const xhr = Fetch.xhrs.get($0);
    const oldReadyStateChangeHandler = xhr.onreadystatechange;
    // Overriding xhr handlers to check if xhr.abort() was called
    xhr.onreadystatechange = (e) => {
      console.log("fetch: xhr.readyState: " + xhr.readyState + ', status: ' + xhr.status);
      assert(xhr.readyState === 0 || xhr.status === 0, "readyState should be equal to UNSENT(0) or status should be equal to 0 when calling xhr.abort()");
      oldReadyStateChangeHandler(e);
    };
    xhr.onload = () => {
      assert(false, "xhr.onload should not be called after xhr.abort()");
    };
    xhr.onerror = () => {
      assert(false, "xhr.onerror should not be called after xhr.abort()");
    };
    xhr.onprogress = () => {
      assert(false, "xhr.onprogress should not be called after xhr.abort()");
    };
  }, fetch->id);
}

void handleStateChange(emscripten_fetch_t *fetch) {
  emscripten_fetch_close(fetch); // Abort the fetch
}

int main() {
  emscripten_fetch_attr_t attr;
  emscripten_fetch_attr_init(&attr);
  strcpy(attr.requestMethod, "GET");
  attr.attributes = EMSCRIPTEN_FETCH_REPLACE;
  attr.onsuccess = handleSuccess;
  attr.onerror = handleError;
  attr.onreadystatechange = handleStateChange;
  auto fetch = emscripten_fetch(&attr, "gears.png");
  return 0;
}
