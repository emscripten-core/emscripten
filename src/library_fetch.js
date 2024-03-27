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

  // New promise-based fetch function
  emscripten_fetch_promise: function(fetch_attr, url) {
    // Convert pointers to strings 
    var attrs = Pointer_stringify(fetch_attr);
    var urlStr = Pointer_stringify(url);

    // Handles the asynchronous fetch operation
    return Asyncify.handleAsync(async () => {
      try {
        let attrsObj = JSON.parse(attrs);
        let response = await fetch(urlStr, attrsObj);

        if (!response.ok) throw new Error('Network response was not ok');

        let data = await response.
        

        
        return 0; // Indicate success
      } catch (error) {
        console.error('Fetch error:', error);
        return 1; // Indicate failure
      }
    });


};

addToLibrary(LibraryFetch);
