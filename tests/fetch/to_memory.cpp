#include <string.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <emscripten/fetch.h>

int main()
{
  emscripten_fetch_attr_t attr;
  emscripten_fetch_attr_init(&attr);
  strcpy(attr.requestType, "GET");
  attr.userData = (void*)0x12345678;
  attr.onsuccess = [](emscripten_fetch_t *fetch) {
    assert(fetch);
    printf("Finished downloading %llu bytes\n", fetch->numBytes);
    assert(fetch->url);
    assert(!strcmp(fetch->url, "gears.png"));
    assert(fetch->id != 0);
    assert((uintptr_t)fetch->userData == 0x12345678);
    assert(fetch->totalBytes == 6407);
    assert(fetch->numBytes == fetch->totalBytes);
    assert(fetch->data != 0);
    // Compute rudimentary checksum of data
    uint8_t checksum = 0;
    for(int i = 0; i < fetch->numBytes; ++i)
      checksum ^= fetch->data[i];
    printf("Data checksum: %02X\n", checksum);
    assert(checksum == 0x08);
    emscripten_fetch_close(fetch);
  };
  attr.onprogress = [](emscripten_fetch_t *fetch) {
    assert(fetch);
    printf("Downloading.. %.2f%% complete.\n", (fetch->dataOffset + fetch->numBytes) * 100.0 / fetch->totalBytes);
    assert(fetch->url);
    assert(!strcmp(fetch->url, "gears.png"));
    assert(fetch->id != 0);
    assert((uintptr_t)fetch->userData == 0x12345678);
  };
  attr.onerror = [](emscripten_fetch_t *fetch) {
    printf("Download failed!\n");
    assert(false); // The download is not supposed to fail in this test.

    assert(fetch);
    assert(fetch->id != 0);
    assert(!strcmp(fetch->url, "gears.png"));
    assert((uintptr_t)fetch->userData == 0x12345678);
  };
  attr.attributes = EMSCRIPTEN_FETCH_APPEND | EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
  emscripten_fetch_t *fetch = emscripten_fetch(&attr, "gears.png");
}
