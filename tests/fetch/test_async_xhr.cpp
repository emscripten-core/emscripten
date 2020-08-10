// Copyright 2020 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <string.h>
#include <future>
#include <iostream>
#include <assert.h>
#include <emscripten/fetch.h>

void downloadSucceeded(emscripten_fetch_t *fetch) {
    assert(strcmp(fetch->url, "gears.png") == 0);
    printf("User data: %s\n", (char*)fetch->userData);
    printf("Finished downloading %llu bytes.\n", fetch->numBytes);
    emscripten_fetch_close(fetch);
}

void downloadFailed(emscripten_fetch_t *fetch) {
    assert(strcmp(fetch->url, "gears.png") == 0);
    printf("Downloading failed, HTTP failure status code: %d.\n", fetch->status);
    emscripten_fetch_close(fetch);
}

int main() {
    char *str = (char*)"User Data";
    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
    strcpy(attr.requestMethod, "GET");
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
    attr.userData = str;
    attr.onsuccess = downloadSucceeded;
    attr.onerror = downloadFailed;
    emscripten_fetch_t *fetch = emscripten_fetch(&attr, "gears.png");
}
