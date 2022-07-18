/**
 * @license
 * Copyright 2016 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

#include Fetch.js

var LibraryFetch = {
#if USE_PTHREADS
  $Fetch__postset: 'if (!ENVIRONMENT_IS_PTHREAD) Fetch.staticInit();',
#else
  $Fetch__postset: 'Fetch.staticInit();',
#endif
  $Fetch: Fetch,
  _emscripten_fetch_get_response_headers_length__sig: 'pi',
  _emscripten_fetch_get_response_headers_length: fetchGetResponseHeadersLength,
  _emscripten_fetch_get_response_headers__sig: 'pipp',
  _emscripten_fetch_get_response_headers: fetchGetResponseHeaders,
  _emscripten_fetch_free: fetchFree,

#if FETCH_SUPPORT_INDEXEDDB
  $fetchDeleteCachedData: fetchDeleteCachedData,
  $fetchLoadCachedData: fetchLoadCachedData,
  $fetchCacheData: fetchCacheData,
#endif
  $fetchXHR: fetchXHR,

  emscripten_start_fetch__sig: 'vp',
  emscripten_start_fetch: startFetch,
  emscripten_start_fetch__deps: [
    '$Fetch',
    '$fetchXHR',
    '$callUserCallback',
#if FETCH_SUPPORT_INDEXEDDB
    '$fetchCacheData',
    '$fetchLoadCachedData',
    '$fetchDeleteCachedData',
#endif
  ]
};

mergeInto(LibraryManager.library, LibraryFetch);
