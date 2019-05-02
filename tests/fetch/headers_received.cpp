// Copyright 2016 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <emscripten/fetch.h>

void headersReceived(emscripten_fetch_t *fetch)
{
  assert(fetch->responseHeaders);

  int numHeaders = 0;
  for(; fetch->responseHeaders[numHeaders * 2]; ++numHeaders)
  {
    // Check both the header and its value are present.
    assert(fetch->responseHeaders[(numHeaders * 2) + 1]);
    printf("Got response header: %s:%s\n", fetch->responseHeaders[numHeaders * 2], fetch->responseHeaders[(numHeaders * 2) + 1]);
  }

  printf("Finished receiving %d headers from URL %s.\n", numHeaders, fetch->url);

#ifdef REPORT_RESULT
  REPORT_RESULT(0);
#endif
}

void success(emscripten_fetch_t *fetch)
{
  printf("Finished downloading %llu bytes from URL %s.\n", fetch->numBytes, fetch->url);
  // The data is now available at fetch->data[0] through fetch->data[fetch->numBytes-1];
  emscripten_fetch_close(fetch); // Free data associated with the fetch.
}

int main()
{
  emscripten_fetch_attr_t attr;
  emscripten_fetch_attr_init(&attr);
  strcpy(attr.requestMethod, "GET");
  attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY | EMSCRIPTEN_FETCH_REPLACE | EMSCRIPTEN_FETCH_RESPONSE_HEADERS;
  attr.onsuccess = success;
  attr.onheadersreceived = headersReceived;
  attr.timeoutMSecs = 2*60;
  emscripten_fetch(&attr, "myfile.dat");
}
