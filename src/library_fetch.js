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
  _emscripten_fetch_get_response_headers_length: _fetch_get_response_headers_length,
  _emscripten_fetch_get_response_headers: _fetch_get_response_headers,
  _emscripten_fetch_free: _fetch_free,

#if FETCH_SUPPORT_INDEXEDDB
  $__emscripten_fetch_delete_cached_data: __emscripten_fetch_delete_cached_data,
  $__emscripten_fetch_load_cached_data: __emscripten_fetch_load_cached_data,
  $__emscripten_fetch_cache_data: __emscripten_fetch_cache_data,
#endif
  $__emscripten_fetch_xhr: __emscripten_fetch_xhr,

  emscripten_start_fetch: emscripten_start_fetch,
  emscripten_start_fetch__deps: ['$Fetch', '$__emscripten_fetch_xhr',
#if FETCH_SUPPORT_INDEXEDDB
  '$__emscripten_fetch_cache_data', '$__emscripten_fetch_load_cached_data', '$__emscripten_fetch_delete_cached_data',
#endif
  'emscripten_is_main_browser_thread']
};

mergeInto(LibraryManager.library, LibraryFetch);
