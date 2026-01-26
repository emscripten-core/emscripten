// Copyright 2025 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <emscripten/fetch.h>

// 301: Moved Permanently                      - https://developer.mozilla.org/en-US/docs/Web/HTTP/Status/301
// 302: Found (Previously "Moved Temporarily") - https://developer.mozilla.org/en-US/docs/Web/HTTP/Status/302
// 303: See Other                              - https://developer.mozilla.org/en-US/docs/Web/HTTP/Status/303
// 307: Temporary Redirect                     - https://developer.mozilla.org/en-US/docs/Web/HTTP/Status/307
// 308: Permanent Redirect                     - https://developer.mozilla.org/en-US/docs/Web/HTTP/Status/308
const int redirect_codes[] = {301, 302, 303, 307, 308};
const int num_codes = sizeof(redirect_codes) / sizeof(redirect_codes[0]);

void check_fetch_result(emscripten_fetch_t *fetch, int expected_status, const char *expected_url) {
  printf("Fetch finished with status %d\n", fetch->status);
  assert(fetch->status == expected_status);
  printf("Downloaded %llu bytes\n", fetch->numBytes);
  assert(strcmp(fetch->responseUrl, expected_url) == 0);
}

void fetchSyncTest(int code, const char *method) {
  char url[128];
  snprintf(url, sizeof(url), SERVER "/status/%d", code);
  emscripten_fetch_attr_t attr;
  emscripten_fetch_attr_init(&attr);
  strcpy(attr.requestMethod, method);
  attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY | EMSCRIPTEN_FETCH_SYNCHRONOUS | EMSCRIPTEN_FETCH_REPLACE;
  emscripten_fetch_t *fetch = emscripten_fetch(&attr, url);
  assert(fetch);
  check_fetch_result(fetch, 200, SERVER "/status/200");
  emscripten_fetch_close(fetch);
}

void onsuccess(emscripten_fetch_t *fetch);
void onreadystatechange(emscripten_fetch_t *fetch);

// State for async test
static int async_code_idx = 0;
static int async_method_idx = 0;
const char *methods[] = {"GET", "POST"};
const int num_methods = 2;

void start_next_async_fetch();

void onsuccess(emscripten_fetch_t *fetch) {
  check_fetch_result(fetch, 200, SERVER "/status/200");
  emscripten_fetch_close(fetch);
  start_next_async_fetch();
}

void onreadystatechange(emscripten_fetch_t *fetch) {
  printf("Fetch readyState %d responseUrl %s\n", fetch->readyState, fetch->responseUrl ? fetch->responseUrl : "is null");
  if (fetch->readyState < 2) {
    assert(NULL == fetch->responseUrl);
  } else {
    assert(0 == strcmp(fetch->responseUrl, SERVER "/status/200"));
  }
}

void start_next_async_fetch() {
  if (async_code_idx >= num_codes) {
    async_code_idx = 0;
    async_method_idx++;
    if (async_method_idx >= num_methods) {
      // All async tests done, now run sync tests
      for (int m = 0; m < num_methods; ++m) {
        for (int i = 0; i < num_codes; ++i) {
          fetchSyncTest(redirect_codes[i], methods[m]);
        }
      }
      exit(0);
    }
  }
  int code = redirect_codes[async_code_idx++];
  const char *method = methods[async_method_idx];
  char url[128];
  snprintf(url, sizeof(url), SERVER "/status/%d", code);
  emscripten_fetch_attr_t attr;
  emscripten_fetch_attr_init(&attr);
  strcpy(attr.requestMethod, method);
  attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
  attr.onsuccess = onsuccess;
  attr.onreadystatechange = onreadystatechange;
  emscripten_fetch_t *fetch = emscripten_fetch(&attr, url);
  assert(fetch);
}

int main() {
  start_next_async_fetch();
  return 99;
}
