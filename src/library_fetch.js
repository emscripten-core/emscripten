/**
 * @license
 * Copyright 2016 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

#include Fetch.js

var LibraryFetch = {
  $Fetch__postset: 'Fetch.init();',
  $Fetch__deps: ['$HandleAllocator'],
  $Fetch: Fetch,
  _emscripten_fetch_get_response_headers_length__deps: ['$lengthBytesUTF8'],
  _emscripten_fetch_get_response_headers_length: fetchGetResponseHeadersLength,
  _emscripten_fetch_get_response_headers__deps: ['$lengthBytesUTF8', '$stringToUTF8'],
  _emscripten_fetch_get_response_headers: fetchGetResponseHeaders,
  _emscripten_fetch_free: fetchFree,

#if FETCH_SUPPORT_INDEXEDDB
  $fetchDeleteCachedData: fetchDeleteCachedData,
  $fetchLoadCachedData: fetchLoadCachedData,
  $fetchCacheData: fetchCacheData,
#endif
  $fetchXHR: fetchXHR,

  emscripten_start_fetch: startFetch,
  emscripten_start_fetch__deps: [
    'malloc',
    'free',
    '$Fetch',
    '$fetchXHR',
    '$callUserCallback',
    '$writeI53ToI64',
    '$stringToUTF8',
#if FETCH_SUPPORT_INDEXEDDB
    '$fetchCacheData',
    '$fetchLoadCachedData',
    '$fetchDeleteCachedData',
#endif
  ]
};

addToLibrary(LibraryFetch);
