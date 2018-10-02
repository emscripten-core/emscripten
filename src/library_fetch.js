// Copyright 2016 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include Fetch.js

var LibraryFetch = {
#if USE_PTHREADS
  $Fetch__postset: 'if (!ENVIRONMENT_IS_PTHREAD) Fetch.staticInit();',
  fetch_work_queue: '; if (ENVIRONMENT_IS_PTHREAD) _fetch_work_queue = PthreadWorkerInit._fetch_work_queue; else PthreadWorkerInit._fetch_work_queue = _fetch_work_queue = allocate(12, "i32*", ALLOC_STATIC)',
#else
  $Fetch__postset: 'Fetch.staticInit();',
  fetch_work_queue: 'allocate(12, "i32*", ALLOC_STATIC)',
#endif
  $Fetch: Fetch,
  _emscripten_get_fetch_work_queue__deps: ['fetch_work_queue'],
  _emscripten_get_fetch_work_queue: function() {
    return _fetch_work_queue;
  },

  $__emscripten_fetch_delete_cached_data: __emscripten_fetch_delete_cached_data,
  $__emscripten_fetch_load_cached_data: __emscripten_fetch_load_cached_data,
  $__emscripten_fetch_cache_data: __emscripten_fetch_cache_data,
  $__emscripten_fetch_xhr: __emscripten_fetch_xhr,
  emscripten_start_fetch__deps: ['$Fetch', '$__emscripten_fetch_xhr', '$__emscripten_fetch_cache_data', '$__emscripten_fetch_load_cached_data', '$__emscripten_fetch_delete_cached_data', '_emscripten_get_fetch_work_queue', 'emscripten_is_main_runtime_thread'],
  emscripten_start_fetch: emscripten_start_fetch
};

mergeInto(LibraryManager.library, LibraryFetch);
