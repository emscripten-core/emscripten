#include Fetch.js

var LibraryFetch = {
  $Fetch__postset: 'Fetch.staticInit();',
  $Fetch: Fetch,
  _emscripten_fetch_load_cached_data: _emscripten_fetch_load_cached_data,
  _emscripten_fetch_cache_data: _emscripten_fetch_cache_data,
  _emscripten_fetch_xhr: _emscripten_fetch_xhr,
  emscripten_start_fetch__deps: ['$Fetch', '_emscripten_fetch_xhr', '_emscripten_fetch_cache_data', '_emscripten_fetch_load_cached_data'],
  emscripten_start_fetch: emscripten_start_fetch
};

mergeInto(LibraryManager.library, LibraryFetch);
