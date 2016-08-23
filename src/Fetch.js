var Fetch = {
  xhrs: [],
  // The web worker that runs proxied file I/O requests.
  worker: undefined,
  // Specifies an instance to the IndexedDB database. The database is opened
  // as a preload step before the Emscripten application starts.
  dbInstance: undefined,

  setu64: function(addr, val) {
    HEAPU32[addr >> 2] = val;
    HEAPU32[addr + 4 >> 2] = (val / 4294967296)|0;
  },

  openDatabase: function(dbname, dbversion, onsuccess, onerror) {
    try {
#if FETCH_DEBUG
      console.log('fetch: indexedDB.open(dbname="' + dbname + '", dbversion="' + dbversion + '");');
#endif
      var openRequest = indexedDB.open(dbname, dbversion);
    } catch (e) { return onerror(e); }

    openRequest.onupgradeneeded = function(event) {
#if FETCH_DEBUG
      console.log('fetch: IndexedDB upgrade needed. Clearing database.');
#endif
      var db = event.target.result;
      if (db.objectStoreNames.contains('FILES')) {
        db.deleteObjectStore('FILES');
      }
      db.createObjectStore('FILES');
    };
    openRequest.onsuccess = function(event) { onsuccess(event.target.result); };
    openRequest.onerror = function(error) { onerror(error); };
  },

  initFetchWorker: function() {
    var stackSize = 128*1024;
    var stack = allocate(stackSize>>2, "i32*", ALLOC_DYNAMIC);
    Fetch.worker.postMessage({cmd: 'init', TOTAL_MEMORY: TOTAL_MEMORY, DYNAMICTOP_PTR: DYNAMICTOP_PTR, STACKTOP: stack, STACK_MAX: stack + stackSize, queuePtr: _fetch_work_queue, buffer: HEAPU8.buffer}, [HEAPU8.buffer]);
  },

  staticInit: function() {
#if USE_PTHREADS
    var isMainThread = (typeof ENVIRONMENT_IS_FETCH_WORKER === 'undefined' && !ENVIRONMENT_IS_PTHREAD);
#else
    var isMainThread = (typeof ENVIRONMENT_IS_FETCH_WORKER === 'undefined');
#endif

    var onsuccess = function(db) {
#if FETCH_DEBUG
      console.log('fetch: IndexedDB successfully opened.');
#endif
      Fetch.dbInstance = db;
      if (isMainThread) {
        Fetch.initFetchWorker();
        removeRunDependency('library_fetch_init');
      }
    };
    var onerror = function() {
#if FETCH_DEBUG
      console.error('fetch: IndexedDB open failed.');
#endif
      Fetch.dbInstance = false;
      if (isMainThread) {
        Fetch.initFetchWorker();
        removeRunDependency('library_fetch_init');
      }
    };
    Fetch.openDatabase('emscripten_filesystem', 1, onsuccess, onerror);

    if (isMainThread) {
      addRunDependency('library_fetch_init');

      var fetchJs = 'fetch-worker.js';
      // Allow HTML module to configure the location where the 'pthread-main.js' file will be loaded from,
      // either via Module.locateFile() function, or via Module.pthreadMainPrefixURL string. If neither
      // of these are passed, then the default URL 'pthread-main.js' relative to the main html file is loaded.
      if (typeof Module['locateFile'] === 'function') fetchJs = Module['locateFile'](fetchJs);
      else if (Module['pthreadMainPrefixURL']) fetchJs = Module['pthreadMainPrefixURL'] + fetchJs;
      Fetch.worker = new Worker(fetchJs);
    }
  }
}

function _emscripten_fetch_load_cached_data(db, fetch, onsuccess, onerror) {
  if (!db) {
#if FETCH_DEBUG
    console.error('fetch: IndexedDB not available!');
#endif
    onerror(fetch, 0, 'IndexedDB not available!');
    return;
  }

  var fetch_attr = fetch + 112/*TODO:structs_info*/;
  var path = HEAPU32[fetch_attr + 64 >> 2];//{{{ makeGetValue('fetch_attr', 64/*TODO*/, 'i32') }}};
  if (!path) path = HEAPU32[fetch + 8 >> 2];//{{{ makeGetValue('fetch', 8/*TODO:structs_info.jsonify this*/, 'i32') }}};
  var pathStr = Pointer_stringify(path);

  try {
    var transaction = db.transaction(['FILES'], 'readonly');
    var packages = transaction.objectStore('FILES');
    var getRequest = packages.get(pathStr);
    getRequest.onsuccess = function(event) {
      if (event.target.result) {
        var value = event.target.result;
        var len = value.byteLength || value.length;
#if FETCH_DEBUG
        console.log('fetch: Loaded file ' + pathStr + ' from IndexedDB, length: ' + len);
#endif

        var ptr = _malloc(len); // TODO: make emscripten_fetch_close() free() this data.
        HEAPU8.set(new Uint8Array(value), ptr);
        HEAPU32[fetch + 12 >> 2] = ptr;//{{{ makeSetValue('fetch', 12/*TODO:jsonify*/, 'ptr', 'i32')}}};
        Fetch.setu64(fetch + 16, len);//{{{ makeSetValue('fetch', 16/*TODO:jsonify*/, 'len', 'i64')}}};
        Fetch.setu64(fetch + 24, 0);//{{{ makeSetValue('fetch', 24/*TODO:jsonify*/, '0', 'i64')}}};
        Fetch.setu64(fetch + 32, len);//{{{ makeSetValue('fetch', 32/*TODO:jsonify*/, 'len', 'i64')}}};

        onsuccess(fetch, 0, value);
      } else {
        // Succeeded to load, but the load came back with the value of undefined, treat that as an error since we never store undefined in db.
#if FETCH_DEBUG
        console.error('fetch: Loaded file ' + pathStr + ' from IndexedDB, but it had 0 length!');
#endif
        onerror(fetch, 0, 'no data');
      }
    };
    getRequest.onerror = function(error) {
#if FETCH_DEBUG
      console.error('fetch: Failed to load file ' + pathStr + ' from IndexedDB!');
#endif
      onerror(fetch, 0, error);
    };
  } catch(e) {
#if FETCH_DEBUG
    console.error('fetch: Failed to load file ' + pathStr + ' from IndexedDB! Got exception ' + e);
#endif
    onerror(fetch, 0, e);
  }
}

function _emscripten_fetch_cache_data(db, fetch, data, onsuccess, onerror) {
  if (!db) {
#if FETCH_DEBUG
    console.error('fetch: IndexedDB not available!');
#endif
    onerror(fetch, 0, 'IndexedDB not available!');
    return;
  }

  var fetch_attr = fetch + 112/*TODO:structs_info*/;
  var destinationPath = HEAPU32[fetch_attr + 64 >> 2];//{{{ makeGetValue('fetch_attr', 64/*TODO*/, 'i32') }}};
  if (!destinationPath) destinationPath = HEAPU32[fetch + 8 >> 2];//{{{ makeGetValue('fetch', 8/*TODO*/, 'i32') }}};
  var destinationPathStr = Pointer_stringify(destinationPath);

  try {
    var transaction = db.transaction(['FILES'], 'readwrite');
    var packages = transaction.objectStore('FILES');
    var putRequest = packages.put(data, destinationPathStr);
    putRequest.onsuccess = function(event) {
#if FETCH_DEBUG
      console.log('fetch: Stored file "' + destinationPathStr + '" to IndexedDB cache.');
#endif
      onsuccess(fetch, 0, destinationPathStr);
    };
    putRequest.onerror = function(error) {
#if FETCH_DEBUG
      console.error('fetch: Failed to store file "' + destinationPathStr + '" to IndexedDB cache!');
#endif
      onerror(fetch, 0, error);
    };
  } catch(e) {
#if FETCH_DEBUG
      console.error('fetch: Failed to store file "' + destinationPathStr + '" to IndexedDB cache! Exception: ' + e);
#endif
    onerror(fetch, 0, e);
  }
}

function _emscripten_fetch_xhr(fetch, onsuccess, onerror, onprogress) {
  var url = HEAPU32[fetch + 8 >> 2];//{{{ makeGetValue('fetch', 8/*TODO:structs_info.jsonify this*/, 'i32') }}};
  if (!url) {
#if FETCH_DEBUG
    console.error('fetch: XHR failed, no URL specified!');
#endif
    onerror(fetch, 0, 'no url specified!');
    return;
  }
  var url_ = Pointer_stringify(url);

  var fetch_attr = fetch + 112/*TODO:structs_info*/;
  var requestMethod = Pointer_stringify(fetch_attr);
  if (!requestMethod) requestMethod = 'GET';
  var userData = HEAPU32[fetch_attr + 32 >> 2];//{{{ makeGetValue('fetch_attr', 32/*TODO:structs_info.jsonify this*/, 'i32') }}};
  var fetchAttributes = HEAPU32[fetch_attr + 48 >> 2];//{{{ makeGetValue('fetch_attr', 48/*TODO*/, 'i32') }}};
  var timeoutMsecs = HEAPU32[fetch_attr + 52 >> 2];//{{{ makeGetValue('fetch_attr', 52/*TODO*/, 'i32') }}};
  var withCredentials = !!HEAPU32[fetch_attr + 56 >> 2];//({{{ makeGetValue('fetch_attr', 56/*TODO*/, 'i32') }}});
  var destinationPath = HEAPU32[fetch_attr + 64 >> 2];//{{{ makeGetValue('fetch_attr', 64/*TODO*/, 'i32') }}};
  var userName = HEAPU32[fetch_attr + 68 >> 2];//{{{ makeGetValue('fetch_attr', 68/*TODO*/, 'i32') }}};
  var password = HEAPU32[fetch_attr + 72 >> 2];//{{{ makeGetValue('fetch_attr', 72/*TODO*/, 'i32') }}};
  var requestHeaders = HEAPU32[fetch_attr + 76 >> 2];//{{{ makeGetValue('fetch_attr', 76/*TODO*/, 'i32') }}};
  var overriddenMimeType = HEAPU32[fetch_attr + 80 >> 2];//{{{ makeGetValue('fetch_attr', 80/*TODO*/, 'i32') }}};

  var fetchAttrLoadToMemory = !!(fetchAttributes & 1/*EMSCRIPTEN_FETCH_LOAD_TO_MEMORY*/);
  var fetchAttrStreamData = !!(fetchAttributes & 2/*EMSCRIPTEN_FETCH_STREAM_DATA*/);
  var fetchAttrPersistFile = !!(fetchAttributes & 4/*EMSCRIPTEN_FETCH_PERSIST_FILE*/);
  var fetchAttrAppend = !!(fetchAttributes & 8/*EMSCRIPTEN_FETCH_APPEND*/);
  var fetchAttrReplace = !!(fetchAttributes & 16/*EMSCRIPTEN_FETCH_REPLACE*/);
  var fetchAttrNoDownload = !!(fetchAttributes & 32/*EMSCRIPTEN_FETCH_NO_DOWNLOAD*/);
  var fetchAttrSynchronous = !!(fetchAttributes & 64/*EMSCRIPTEN_FETCH_SYNCHRONOUS*/);
  var fetchAttrWaitable = !!(fetchAttributes & 128/*EMSCRIPTEN_FETCH_WAITABLE*/);

  var userNameStr = userName ? Pointer_stringify(userName) : undefined;
  var passwordStr = password ? Pointer_stringify(password) : undefined;
  var overriddenMimeTypeStr = overriddenMimeType ? Pointer_stringify(overriddenMimeType) : undefined;

  var xhr = new XMLHttpRequest();
  xhr.timeout = timeoutMsecs;
  xhr.withCredentials = withCredentials;
#if FETCH_DEBUG
  console.log('fetch: xhr.timeout: ' + xhr.timeout + ', xhr.withCredentials: ' + xhr.withCredentials);
  console.log('fetch: xhr.open(requestMethod="' + requestMethod + '", url: "' + url_ +'", userName: ' + userNameStr + ', password: ' + passwordStr + ');');
#endif
  xhr.open(requestMethod, url_, !fetchAttrSynchronous, userNameStr, passwordStr);
  xhr.responseType = fetchAttrStreamData ? 'moz-chunked-arraybuffer' : 'arraybuffer';

  if (overriddenMimeType) {
#if FETCH_DEBUG
    console.log('fetch: xhr.overrideMimeType("' + overriddenMimeTypeStr + '");');
#endif
    xhr.overrideMimeType(overriddenMimeTypeStr);
  }
  if (requestHeaders) {
    for(;;) {
      var key = HEAPU32[requestHeaders >> 2];//{{{ makeGetValue('requestHeaders', 0, 'i32') }}};
      if (!key) break;
      var value = HEAPU32[requestHeaders + 4 >> 2];//{{{ makeGetValue('requestHeaders', 4, 'i32') }}};
      if (!value) break;
      requestHeaders += 8;
      var keyStr = Pointer_stringify(key);
      var valueStr = Pointer_stringify(value);
#if FETCH_DEBUG
      console.log('fetch: xhr.setRequestHeader("' + keyStr + '", "' + valueStr + '");');
#endif
      xhr.setRequestHeader(keyStr, valueStr);
    }
  }
  Fetch.xhrs.push(xhr);
  var id = Fetch.xhrs.length;
  HEAPU32[fetch >> 2] = id;//{{{ makeSetValue('fetch', 0/*TODO:jsonify*/, 'id', 'i32')}}};
  var data = null; // TODO: Support user to pass data to request.
  // TODO: Support specifying custom headers to the request.

  xhr.onload = function(e) {
    var len = xhr.response ? xhr.response.byteLength : 0;
    var ptr = 0;
    var ptrLen = 0;
    if (fetchAttrLoadToMemory && !fetchAttrStreamData) {
      ptrLen = len;
#if FETCH_DEBUG
      console.log('fetch: allocating ' + ptrLen + ' bytes in Emscripten heap for xhr data');
#endif
      ptr = _malloc(ptrLen); // TODO: make emscripten_fetch_close() free() this data.
      HEAPU8.set(new Uint8Array(xhr.response), ptr); // TODO: Since DYNAMICTOP is not coherent, this can corrupt
    }
    HEAPU32[fetch + 12 >> 2] = ptr;//{{{ makeSetValue('fetch', 12/*TODO:jsonify*/, 'ptr', 'i32')}}};
    Fetch.setu64(fetch + 16, ptrLen);//{{{ makeSetValue('fetch', 16/*TODO:jsonify*/, 'ptrLen', 'i64')}}};
    Fetch.setu64(fetch + 24, 0);//{{{ makeSetValue('fetch', 24/*TODO:jsonify*/, '0', 'i64')}}};
    if (len) {
      Fetch.setu64(fetch + 32, len);//{{{ makeSetValue('fetch', 32/*TODO:jsonify*/, 'len', 'i64')}}};
    }
    HEAPU16[fetch + 40 >> 1] = xhr.readyState;
    HEAPU16[fetch + 42 >> 1] = xhr.status;
//    if (xhr.statusText) writeStringToMemory(fetch + 44, xhr.statusText);
    if (xhr.status == 200 || xhr.status == 0) {
#if FETCH_DEBUG
      console.log('fetch: xhr succeeded with status 200');
#endif
      if (onsuccess) onsuccess(fetch, xhr, e);
    } else {
#if FETCH_DEBUG
      console.error('fetch: xhr failed with status ' + xhr.status);
#endif
      if (onerror) onerror(fetch, xhr, e);
    }
  }
  xhr.onerror = function(e) {
#if FETCH_DEBUG
    console.error('fetch: xhr failed with error ' + e);
#endif
    if (onerror) onerror(fetch, xhr, e);
  }
  xhr.ontimeout = function(e) {
#if FETCH_DEBUG
    console.error('fetch: xhr timed out with error ' + e);
#endif
    if (onerror) onerror(fetch, xhr, e);
  }
  xhr.onprogress = function(e) {
    console.log('fetch ptr ' + fetch + ', state ' + HEAPU32[fetch + 108 >> 2]);
    var ptrLen = (fetchAttrLoadToMemory && fetchAttrStreamData && xhr.response) ? xhr.response.byteLength : 0;
    var ptr = 0;
    if (fetchAttrLoadToMemory && fetchAttrStreamData) {
#if FETCH_DEBUG
      console.log('fetch: allocating ' + ptrLen + ' bytes in Emscripten heap for xhr data');
#endif
      ptr = _malloc(ptrLen); // TODO: make emscripten_fetch_close() free() this data.
      HEAPU8.set(new Uint8Array(xhr.response), ptr);
    }
    HEAPU32[fetch + 12 >> 2] = ptr;//{{{ makeSetValue('fetch', 12/*TODO:jsonify*/, 'ptr', 'i32')}}};
    Fetch.setu64(fetch + 16, ptrLen);//{{{ makeSetValue('fetch', 16/*TODO:jsonify*/, 'ptrLen', 'i64')}}};
    Fetch.setu64(fetch + 24, e.loaded - ptrLen);//{{{ makeSetValue('fetch', 24/*TODO:jsonify*/, 'e.loaded - ptrLen', 'i64')}}};
    Fetch.setu64(fetch + 32, e.total);//{{{ makeSetValue('fetch', 32/*TODO:jsonify*/, 'e.total', 'i64')}}};
    HEAPU16[fetch + 40 >> 1] = xhr.readyState;
    HEAPU16[fetch + 42 >> 1] = xhr.status;
    if (xhr.statusText) writeStringToMemory(fetch + 44, xhr.statusText);
    if (onprogress) onprogress(fetch, xhr, e);
  }
#if FETCH_DEBUG
  console.log('fetch: xhr.send(data=' + data + ')');
#endif
  try {
    xhr.send(data);
  } catch(e) {
#if FETCH_DEBUG
    console.error('fetch: xhr failed with exception: ' + e);
#endif
    if (onerror) onerror(fetch, xhr, e);
  }
}

function emscripten_start_fetch(fetch, successcb, errorcb, progresscb) {
  if (typeof Module !== 'undefined') Module['noExitRuntime'] = true; // If we are the main Emscripten runtime, we should not be closing down.

  var fetch_attr = fetch + 112/*TODO:structs_info*/;
  var onsuccess = HEAPU32[fetch_attr + 36 >> 2];//{{{ makeGetValue('fetch_attr', 36/*TODO:structs_info.jsonify this*/, 'i32') }}};
  var onerror = HEAPU32[fetch_attr + 40 >> 2];//{{{ makeGetValue('fetch_attr', 40/*TODO:structs_info.jsonify this*/, 'i32') }}};
  var onprogress = HEAPU32[fetch_attr + 44 >> 2];//{{{ makeGetValue('fetch_attr', 44/*TODO:structs_info.jsonify this*/, 'i32') }}};
  var fetchAttributes = HEAPU32[fetch_attr + 48 >> 2];//{{{ makeGetValue('fetch_attr', 48/*TODO*/, 'i32') }}};
  var fetchAttrLoadToMemory = !!(fetchAttributes & 1/*EMSCRIPTEN_FETCH_LOAD_TO_MEMORY*/);
  var fetchAttrStreamData = !!(fetchAttributes & 2/*EMSCRIPTEN_FETCH_STREAM_DATA*/);
  var fetchAttrPersistFile = !!(fetchAttributes & 4/*EMSCRIPTEN_FETCH_PERSIST_FILE*/);
  var fetchAttrAppend = !!(fetchAttributes & 8/*EMSCRIPTEN_FETCH_APPEND*/);
  var fetchAttrReplace = !!(fetchAttributes & 16/*EMSCRIPTEN_FETCH_REPLACE*/);
  var fetchAttrNoDownload = !!(fetchAttributes & 32/*EMSCRIPTEN_FETCH_NO_DOWNLOAD*/);

  var reportSuccess = function(fetch, xhr, e) {
#if FETCH_DEBUG
    console.log('fetch: operation success. e: ' + e);
#endif
    if (onsuccess && Runtime.dynCall) Runtime.dynCall('vi', onsuccess, [fetch]);
    else if (successcb) successcb(fetch);
  };

  var cacheResultAndReportSuccess = function(fetch, xhr, e) {
#if FETCH_DEBUG
    console.log('fetch: operation success. Caching result.. e: ' + e);
#endif
    var storeSuccess = function() {
#if FETCH_DEBUG
      console.log('fetch: IndexedDB store succeeded.');
#endif
    };
    var storeError = function() {
#if FETCH_DEBUG
      console.error('fetch: IndexedDB store failed.');
#endif
    };
    __emscripten_fetch_cache_data(Fetch.dbInstance, fetch, xhr.response, storeSuccess, storeError);
    if (onsuccess && Runtime.dynCall) Runtime.dynCall('vi', onsuccess, [fetch]);
    else if (successcb) successcb(fetch);
  };

  var reportProgress = function(fetch, xhr, e) {
    if (onprogress && Runtime.dynCall) Runtime.dynCall('vi', onprogress, [fetch]);      
    else if (progresscb) progresscb(fetch);
  };

  var reportError = function(fetch, xhr, e) {
#if FETCH_DEBUG
    console.error('fetch: operation failed: ' + e);
#endif
    if (onerror && Runtime.dynCall) Runtime.dynCall('vi', onerror, [fetch]);
    else if (errorcb) errorcb(fetch);
  };

  var performUncachedXhr = function(fetch, xhr, e) {
#if FETCH_DEBUG
    console.error('fetch: starting (uncached) XHR: ' + e);
#endif
    __emscripten_fetch_xhr(fetch, reportSuccess, reportError, reportProgress);
  };

  var performCachedXhr = function(fetch, xhr, e) {
#if FETCH_DEBUG
    console.error('fetch: starting (cached) XHR: ' + e);
#endif
    __emscripten_fetch_xhr(fetch, cacheResultAndReportSuccess, reportError, reportProgress);
  };

  // Should we try IndexedDB first?
  if (!fetchAttrReplace) {
    if (!Fetch.dbInstance) {
#if FETCH_DEBUG
      console.error('fetch: failed to read IndexedDB! Database is not open.');
#endif
      reportError(fetch, 0, 'IndexedDB is not open');
      return 0; // todo: free
    }

    if (fetchAttrNoDownload) {
      __emscripten_fetch_load_cached_data(Fetch.dbInstance, fetch, reportSuccess, reportError);
    } else if (fetchAttrPersistFile) {
      __emscripten_fetch_load_cached_data(Fetch.dbInstance, fetch, reportSuccess, performCachedXhr);        
    } else {
      __emscripten_fetch_load_cached_data(Fetch.dbInstance, fetch, reportSuccess, performUncachedXhr);        
    }
  } else if (!fetchAttrNoDownload) {
    if (fetchAttrPersistFile) {
      __emscripten_fetch_xhr(fetch, cacheResultAndReportSuccess, reportError, reportProgress);
    } else {
      __emscripten_fetch_xhr(fetch, reportSuccess, reportError, reportProgress);        
    }
  } else {
#if FETCH_DEBUG
    console.error('fetch: Invalid combination of flags passed.');
#endif
    return 0; // todo: free
  }
  return fetch;
}
