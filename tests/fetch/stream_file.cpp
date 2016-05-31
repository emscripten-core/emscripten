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
  attr.onsuccess = [](emscripten_fetch_t *fetch) {
    assert(fetch->data == 0); // The data was streamed via onprogress, no bytes available here.
    assert(fetch->numBytes == 0);
    assert(fetch->totalBytes > 0);
    printf("Finished downloading %llu bytes\n", fetch->totalBytes);
    emscripten_fetch_close(fetch);
  };
  attr.onprogress = [](emscripten_fetch_t *fetch) {
    assert(fetch->data != 0);
    assert(fetch->numBytes > 0);
    assert(fetch->numBytes <= fetch->totalBytes);
    printf("Downloading.. %.2f%% complete.\n", (fetch->dataOffset + fetch->numBytes) * 100.0 / fetch->totalBytes);
  };
  attr.attributes = EMSCRIPTEN_FETCH_APPEND | EMSCRIPTEN_FETCH_STREAM_DATA;
  emscripten_fetch_t *fetch = emscripten_fetch(&attr, "myfile.dat");
}
