/**
 * @license
 * Copyright 2016 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

#include Fetch.js

var LibraryFetch = {
  $Fetch__postset: 'Fetch.init();',
  $Fetch: Fetch,
  _emscripten_fetch_get_response_headers_length: fetchGetResponseHeadersLength,
  _emscripten_fetch_get_response_headers: fetchGetResponseHeaders,
  emscripten_fetch_free: fetchFree,

#if FETCH_SUPPORT_INDEXEDDB
  $fetchDeleteCachedData: fetchDeleteCachedData,
  $fetchLoadCachedData: fetchLoadCachedData,
  $fetchCacheData: fetchCacheData,
#endif
  $fetchXHR: fetchXHR,
#if FETCH_STREAMING
  $FetchXHR: FetchXHR,
#endif

  emscripten_start_fetch: startFetch,
  emscripten_start_fetch__deps: [
    'malloc',
    'realloc',
    '$writeI53ToI64',
    '$stringToUTF8',
    '$stringToNewUTF8',
#if FETCH_STREAMING
    '$FetchXHR',
#endif
  ],
};

addToLibrary(LibraryFetch);
