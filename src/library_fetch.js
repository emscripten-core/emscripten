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

  _emscripten_fetch_load_cached_data: _emscripten_fetch_load_cached_data,
  _emscripten_fetch_cache_data: _emscripten_fetch_cache_data,
  _emscripten_fetch_xhr: _emscripten_fetch_xhr,
  emscripten_start_fetch__deps: ['$Fetch', '_emscripten_fetch_xhr', '_emscripten_fetch_cache_data', '_emscripten_fetch_load_cached_data', '_emscripten_get_fetch_work_queue'],
  emscripten_start_fetch: emscripten_start_fetch
};

mergeInto(LibraryManager.library, LibraryFetch);
