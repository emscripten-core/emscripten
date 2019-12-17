// Copyright 2016 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include Fetch.js

var LibraryFetch = {
#if USE_PTHREADS
  $Fetch__postset: 'if (!ENVIRONMENT_IS_PTHREAD) Fetch.staticInit();',
#else
  $Fetch__postset: 'Fetch.staticInit();',
#endif
  fetch_work_queue: '{{{ makeStaticAlloc(12) }}}',
  $Fetch: Fetch,
  _emscripten_get_fetch_work_queue__deps: ['fetch_work_queue'],
  _emscripten_get_fetch_work_queue: function() {
    return _fetch_work_queue;
  },
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
  '_emscripten_get_fetch_work_queue', 'emscripten_is_main_browser_thread']
};

mergeInto(LibraryManager.library, LibraryFetch);
