var Fetch = {
  xhrs: [],
  // Specifies an instance to the IndexedDB database. The database is opened
  // as a preload step before the Emscripten application starts.
  dbInstance: undefined,

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

  staticInit: function() {
    var onsuccess = function(db) {
#if FETCH_DEBUG
      console.log('fetch: IndexedDB successfully opened.');
#endif
      Fetch.dbInstance = db;
      removeRunDependency('library_fetch_init');
    };
    var onerror = function() {
#if FETCH_DEBUG
      console.error('fetch: IndexedDB open failed.');
#endif
      Fetch.dbInstance = false;
      removeRunDependency('library_fetch_init');
    };
    Fetch.openDatabase('emscripten_filesystem', 1, onsuccess, onerror);
    addRunDependency('library_fetch_init');
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

  var fetch_attr = fetch + 108/*TODO:structs_info*/;
  var path = {{{ makeGetValue('fetch_attr', 64/*TODO*/, 'i32') }}};
  if (!path) path = {{{ makeGetValue('fetch', 8/*TODO:structs_info.jsonify this*/, 'i32') }}};
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
        {{{ makeSetValue('fetch', 12/*TODO:jsonify*/, 'ptr', 'i32')}}};
        {{{ makeSetValue('fetch', 16/*TODO:jsonify*/, 'len', 'i64')}}};
        {{{ makeSetValue('fetch', 24/*TODO:jsonify*/, '0', 'i64')}}};
        {{{ makeSetValue('fetch', 32/*TODO:jsonify*/, 'len', 'i64')}}};

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

  var fetch_attr = fetch + 108/*TODO:structs_info*/;
  var destinationPath = {{{ makeGetValue('fetch_attr', 64/*TODO*/, 'i32') }}};
  if (!destinationPath) destinationPath = {{{ makeGetValue('fetch', 8/*TODO*/, 'i32') }}};
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
  var url = {{{ makeGetValue('fetch', 8/*TODO:structs_info.jsonify this*/, 'i32') }}};
  if (!url) {
#if FETCH_DEBUG
    console.error('fetch: XHR failed, no URL specified!');
#endif
    onerror(fetch, 0, 'no url specified!');
    return;
  }
  var url_ = Pointer_stringify(url);

  var fetch_attr = fetch + 108/*TODO:structs_info*/;
  var requestMethod = fetch_attr ? Pointer_stringify(fetch_attr) : 'GET';
  var userData = {{{ makeGetValue('fetch_attr', 32/*TODO:structs_info.jsonify this*/, 'i32') }}};
  var fetchAttributes = {{{ makeGetValue('fetch_attr', 48/*TODO*/, 'i32') }}};
  var timeoutMsecs = {{{ makeGetValue('fetch_attr', 52/*TODO*/, 'i32') }}};
  var withCredentials = !!({{{ makeGetValue('fetch_attr', 56/*TODO*/, 'i32') }}});
  var destinationPath = {{{ makeGetValue('fetch_attr', 64/*TODO*/, 'i32') }}};
  var userName = {{{ makeGetValue('fetch_attr', 68/*TODO*/, 'i32') }}};
  var password = {{{ makeGetValue('fetch_attr', 72/*TODO*/, 'i32') }}};
  var requestHeaders = {{{ makeGetValue('fetch_attr', 76/*TODO*/, 'i32') }}};
  var overriddenMimeType = {{{ makeGetValue('fetch_attr', 80/*TODO*/, 'i32') }}};

  var fetchAttrLoadToMemory = !!(fetchAttributes & 1/*EMSCRIPTEN_FETCH_LOAD_TO_MEMORY*/);
  var fetchAttrStreamData = !!(fetchAttributes & 2/*EMSCRIPTEN_FETCH_STREAM_DATA*/);
  var fetchAttrPersistFile = !!(fetchAttributes & 4/*EMSCRIPTEN_FETCH_PERSIST_FILE*/);
  var fetchAttrAppend = !!(fetchAttributes & 8/*EMSCRIPTEN_FETCH_APPEND*/);
  var fetchAttrReplace = !!(fetchAttributes & 16/*EMSCRIPTEN_FETCH_REPLACE*/);
  var fetchAttrNoDownload = !!(fetchAttributes & 32/*EMSCRIPTEN_FETCH_NO_DOWNLOAD*/);
  var fetchAttrSynchronous = !!(fetchAttributes & 64/*EMSCRIPTEN_FETCH_SYNCHRONOUS*/);

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
      var key = {{{ makeGetValue('requestHeaders', 0, 'i32') }}};
      if (!key) break;
      var value = {{{ makeGetValue('requestHeaders', 4, 'i32') }}};
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
  {{{ makeSetValue('fetch', 0/*TODO:jsonify*/, 'id', 'i32')}}};
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
      HEAPU8.set(new Uint8Array(xhr.response), ptr);
    }
    {{{ makeSetValue('fetch', 12/*TODO:jsonify*/, 'ptr', 'i32')}}};
    {{{ makeSetValue('fetch', 16/*TODO:jsonify*/, 'ptrLen', 'i64')}}};
    {{{ makeSetValue('fetch', 24/*TODO:jsonify*/, '0', 'i64')}}};
    if (len) {
      {{{ makeSetValue('fetch', 32/*TODO:jsonify*/, 'len', 'i64')}}};
    }
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
    var ptrLen = (fetchAttrLoadToMemory && fetchAttrStreamData && xhr.response) ? xhr.response.byteLength : 0;
    var ptr = 0;
    if (fetchAttrLoadToMemory && fetchAttrStreamData) {
#if FETCH_DEBUG
      console.log('fetch: allocating ' + ptrLen + ' bytes in Emscripten heap for xhr data');
#endif
      ptr = _malloc(ptrLen); // TODO: make emscripten_fetch_close() free() this data.
      HEAPU8.set(new Uint8Array(xhr.response), ptr);
    }
    {{{ makeSetValue('fetch', 12/*TODO:jsonify*/, 'ptr', 'i32')}}};
    {{{ makeSetValue('fetch', 16/*TODO:jsonify*/, 'ptrLen', 'i64')}}};
    {{{ makeSetValue('fetch', 24/*TODO:jsonify*/, 'e.loaded - ptrLen', 'i64')}}};
    {{{ makeSetValue('fetch', 32/*TODO:jsonify*/, 'e.total', 'i64')}}};
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

function emscripten_start_fetch(fetch) {
  Module['noExitRuntime'] = true; // TODO: assumes we are the main Emscripten runtime thread, in the future won't.

  var fetch_attr = fetch + 108/*TODO:structs_info*/;
  var onsuccess = {{{ makeGetValue('fetch_attr', 36/*TODO:structs_info.jsonify this*/, 'i32') }}};
  var onerror = {{{ makeGetValue('fetch_attr', 40/*TODO:structs_info.jsonify this*/, 'i32') }}};
  var onprogress = {{{ makeGetValue('fetch_attr', 44/*TODO:structs_info.jsonify this*/, 'i32') }}};
  var fetchAttributes = {{{ makeGetValue('fetch_attr', 48/*TODO*/, 'i32') }}};
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
    if (onsuccess) Runtime.dynCall('vi', onsuccess, [fetch]);
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
    if (onsuccess) Runtime.dynCall('vi', onsuccess, [fetch]);
  };

  var reportProgress = function(fetch, xhr, e) {
    if (onprogress) Runtime.dynCall('vi', onprogress, [fetch]);      
  };

  var reportError = function(fetch, xhr, e) {
#if FETCH_DEBUG
    console.error('fetch: operation failed: ' + e);
#endif
    if (onerror) Runtime.dynCall('vi', onerror, [fetch]);
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
