// Copyright 2016 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <bits/errno.h>
#include <math.h>
#include <memory.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>
#include <stdbool.h>

#include <emscripten/emscripten.h>
#include <emscripten/fetch.h>
#include <emscripten/html5.h>
#include <emscripten/threading.h>
#include <emscripten/console.h>

#include "emscripten_internal.h"

// From https://developer.mozilla.org/en-US/docs/Web/API/XMLHttpRequest/readyState
#define STATE_UNSENT           0 // Client has been created. open() not called yet.
#define STATE_OPENED           1 // open() has been called.
#define STATE_HEADERS_RECEIVED 2 // send() has been called, and headers and status are available.
#define STATE_LOADING          3 // Downloading; responseText holds partial data.
#define STATE_DONE             4 // The operation is complete.

#define STATE_MAX STATE_DONE

// Uncomment the following and clear the cache with emcc --clear-cache to rebuild this file to
// enable internal debugging. #define FETCH_DEBUG

static void fetch_free(emscripten_fetch_t* fetch);

typedef struct emscripten_fetch_queue {
  emscripten_fetch_t** queuedOperations;
  int numQueuedItems;
  int queueSize;
} emscripten_fetch_queue;

emscripten_fetch_queue* _emscripten_get_fetch_queue() {
  static thread_local emscripten_fetch_queue g_queue;

  if (!g_queue.queuedOperations) {
    g_queue.queueSize = 64;
    g_queue.numQueuedItems = 0;
    g_queue.queuedOperations = malloc(sizeof(emscripten_fetch_t*) * g_queue.queueSize);
  }
  return &g_queue;
}

void emscripten_proxy_fetch(emscripten_fetch_t* fetch) {
  // TODO: mutex lock
  emscripten_fetch_queue* queue = _emscripten_get_fetch_queue();
  // TODO handle case when queue->numQueuedItems >= queue->queueSize
  queue->queuedOperations[queue->numQueuedItems++] = fetch;
#ifdef FETCH_DEBUG
  emscripten_dbgf("Queued fetch to fetch-worker to process. There are "
                          "now %d operations in the queue.", queue->numQueuedItems);
#endif
  // TODO: mutex unlock
}

void emscripten_fetch_attr_init(emscripten_fetch_attr_t* fetch_attr) {
  memset(fetch_attr, 0, sizeof(emscripten_fetch_attr_t));
}

emscripten_fetch_t* emscripten_fetch(emscripten_fetch_attr_t* fetch_attr, const char* url) {
  if (!fetch_attr || !url) {
    return NULL;
  }

  const bool synchronous = (fetch_attr->attributes & EMSCRIPTEN_FETCH_SYNCHRONOUS) != 0;
  const bool readFromIndexedDB =
    (fetch_attr->attributes & (EMSCRIPTEN_FETCH_APPEND | EMSCRIPTEN_FETCH_NO_DOWNLOAD)) != 0 ||
    ((fetch_attr->attributes & EMSCRIPTEN_FETCH_REPLACE) == 0);
  const bool writeToIndexedDB = (fetch_attr->attributes & EMSCRIPTEN_FETCH_PERSIST_FILE) != 0 ||
                                !strncmp(fetch_attr->requestMethod, "EM_IDB_", strlen("EM_IDB_"));
  const bool performXhr = (fetch_attr->attributes & EMSCRIPTEN_FETCH_NO_DOWNLOAD) == 0;
  if (emscripten_is_main_browser_thread() && synchronous && (performXhr || readFromIndexedDB || writeToIndexedDB)) {
#ifdef FETCH_DEBUG
    emscripten_errf("emscripten_fetch('%s') failed! Synchronous blocking XHRs and IndexedDB operations are not supported on the main browser thread. Try dropping the EMSCRIPTEN_FETCH_SYNCHRONOUS flag, or run with the linker flag --proxy-to-worker to decouple main C runtime thread from the main browser thread.", url);
#endif
    return NULL;
  }

  emscripten_fetch_t* fetch = (emscripten_fetch_t*)calloc(1, sizeof(emscripten_fetch_t));
  if (!fetch) {
    return NULL;
  }
  fetch->userData = fetch_attr->userData;
  fetch->__attributes.timeoutMSecs = fetch_attr->timeoutMSecs;
  fetch->__attributes.attributes = fetch_attr->attributes;
  fetch->__attributes.withCredentials = fetch_attr->withCredentials;
  fetch->__attributes.requestData = fetch_attr->requestData;
  fetch->__attributes.requestDataSize = fetch_attr->requestDataSize;
  strcpy(fetch->__attributes.requestMethod, fetch_attr->requestMethod);
  fetch->__attributes.onerror = fetch_attr->onerror;
  fetch->__attributes.onsuccess = fetch_attr->onsuccess;
  fetch->__attributes.onprogress = fetch_attr->onprogress;
  fetch->__attributes.onreadystatechange = fetch_attr->onreadystatechange;
#define STRDUP_OR_ABORT(s, str_to_dup)                                                             \
  if (str_to_dup) {                                                                                \
    s = strdup(str_to_dup);                                                                        \
    if (!s) {                                                                                      \
      fetch_free(fetch);                                                                           \
      return 0;                                                                                    \
    }                                                                                              \
  }
  STRDUP_OR_ABORT(fetch->url, url);
  STRDUP_OR_ABORT(fetch->__attributes.destinationPath, fetch_attr->destinationPath);
  STRDUP_OR_ABORT(fetch->__attributes.userName, fetch_attr->userName);
  STRDUP_OR_ABORT(fetch->__attributes.password, fetch_attr->password);
  STRDUP_OR_ABORT(fetch->__attributes.overriddenMimeType, fetch_attr->overriddenMimeType);
#undef STRDUP_OR_ABORT

  if (fetch_attr->requestHeaders) {
    size_t headersCount = 0;
    while (fetch_attr->requestHeaders[headersCount]) {
      ++headersCount;
    }
    const char** headers = (const char**)malloc((headersCount + 1) * sizeof(const char*));
    if (!headers) {
      fetch_free(fetch);
      return NULL;
    }
    memset((void*)headers, 0, (headersCount + 1) * sizeof(const char*));

    for (size_t i = 0; i < headersCount; ++i) {
      headers[i] = strdup(fetch_attr->requestHeaders[i]);
      if (!headers[i]) {
        for (size_t j = 0; j < i; ++j) {
          free((void*)headers[j]);
        }
        free((void*)headers);
        fetch_free(fetch);
        return NULL;
      }
    }
    headers[headersCount] = 0;
    fetch->__attributes.requestHeaders = headers;
  }

  emscripten_start_fetch(fetch);
  return fetch;
}

EMSCRIPTEN_RESULT emscripten_fetch_wait(emscripten_fetch_t* fetch, double timeoutMsecs) {
  return EMSCRIPTEN_RESULT_FAILED;
}

EMSCRIPTEN_RESULT emscripten_fetch_close(emscripten_fetch_t* fetch) {
  if (!fetch) {
    return EMSCRIPTEN_RESULT_SUCCESS; // Closing null pointer is ok, same as with free().
  }

  // This function frees the fetch pointer so that it is invalid to access it anymore.
  // Use a few key fields as an integrity check that we are being passed a good pointer to a valid
  // fetch structure, which has not been yet closed. (double close is an error)
  if (fetch->id == 0 || fetch->readyState > STATE_MAX) {
    return EMSCRIPTEN_RESULT_INVALID_PARAM;
  }

  // This fetch is aborted. Call the error handler if the fetch was still in progress and was
  // canceled in flight.
  if (fetch->readyState != STATE_DONE && fetch->__attributes.onerror) {
    fetch->status = (unsigned short)-1;
    strcpy(fetch->statusText, "aborted with emscripten_fetch_close()");
    fetch->__attributes.onerror(fetch);
  }

  fetch_free(fetch);
  return EMSCRIPTEN_RESULT_SUCCESS;
}

size_t emscripten_fetch_get_response_headers_length(emscripten_fetch_t *fetch) {
  if (!fetch || fetch->readyState < STATE_HEADERS_RECEIVED) {
    return 0;
  }

  return (size_t)_emscripten_fetch_get_response_headers_length(fetch->id);
}

size_t emscripten_fetch_get_response_headers(emscripten_fetch_t *fetch, char *dst, size_t dstSizeBytes) {
  if (!fetch || fetch->readyState < STATE_HEADERS_RECEIVED) {
    return 0;
  }

  return (size_t)_emscripten_fetch_get_response_headers(fetch->id, dst, dstSizeBytes);
}

char **emscripten_fetch_unpack_response_headers(const char *headersString) {
  // Get size of output array and allocate.
  size_t numHeaders = 0;
  for (const char *pos = strchr(headersString, '\n'); pos; pos = strchr(pos + 1, '\n')) {
    numHeaders++;
  }
  char **unpackedHeaders = (char**)malloc(sizeof(char*) * ((numHeaders * 2) + 1));
  unpackedHeaders[numHeaders * 2] = NULL;

  // Allocate each header.
  const char *rowStart = headersString;
  const char *rowEnd = strchr(rowStart, '\n');
  for (size_t headerNum = 0; rowEnd; headerNum += 2) {
    const char *split = strchr(rowStart, ':');
    size_t headerSize = (size_t)split - (size_t)rowStart;
    char* header = (char*)malloc(headerSize + 1);
    strncpy(header, rowStart, headerSize);
    header[headerSize] = '\0';

    size_t valueSize = (size_t)rowEnd - (size_t)split;
    char* value = (char*)malloc(valueSize + 1);
    strncpy(value, split + 1, valueSize);
    value[valueSize] = '\0';

    unpackedHeaders[headerNum] = header;
    unpackedHeaders[headerNum+1] = value;

    rowStart = rowEnd + 1;
    rowEnd = strchr(rowStart, '\n');
  }

  return unpackedHeaders;
}

void emscripten_fetch_free_unpacked_response_headers(char **unpackedHeaders) {
  if (unpackedHeaders) {
    for (size_t i = 0; unpackedHeaders[i]; ++i) {
      free((void*)unpackedHeaders[i]);
    }
    free((void*)unpackedHeaders);
  }
}

void emscripten_fetch_free(unsigned int id) {
  return _emscripten_fetch_free(id);
}

static void fetch_free(emscripten_fetch_t* fetch) {
  emscripten_fetch_free(fetch->id);
  fetch->id = 0;
  free((void*)fetch->data);
  free((void*)fetch->url);
  free((void*)fetch->__attributes.destinationPath);
  free((void*)fetch->__attributes.userName);
  free((void*)fetch->__attributes.password);
  if (fetch->__attributes.requestHeaders) {
    for (size_t i = 0; fetch->__attributes.requestHeaders[i]; ++i) {
      free((void*)fetch->__attributes.requestHeaders[i]);
    }
    free((void*)fetch->__attributes.requestHeaders);
  }
  free((void*)fetch->__attributes.overriddenMimeType);
  free(fetch);
}
