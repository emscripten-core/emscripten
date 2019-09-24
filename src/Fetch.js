// Copyright 2016 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

var Fetch = {
  xhrs: [],

  // The web worker that runs proxied file I/O requests. (this field is populated on demand, start as undefined to save code size)
  // worker: undefined,

  // Specifies an instance to the IndexedDB database. The database is opened
  // as a preload step before the Emscripten application starts. (this field is populated on demand, start as undefined to save code size)
  // dbInstance: undefined,

  setu64: function(addr, val) {
    HEAPU32[addr >> 2] = val;
    HEAPU32[addr + 4 >> 2] = (val / 4294967296)|0;
  },

#if FETCH_SUPPORT_INDEXEDDB
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
#endif

#if USE_PTHREADS
  initFetchWorker: function() {
    var stackSize = 128*1024;
    var stack = allocate(stackSize>>2, "i32*", ALLOC_DYNAMIC);
    Fetch.worker.postMessage({cmd: 'init', DYNAMICTOP_PTR: DYNAMICTOP_PTR, STACKTOP: stack, STACK_MAX: stack + stackSize, queuePtr: _fetch_work_queue, buffer: HEAPU8.buffer});
  },
#endif

  staticInit: function() {
#if USE_PTHREADS
    var isMainThread = (typeof ENVIRONMENT_IS_FETCH_WORKER === 'undefined' && !ENVIRONMENT_IS_PTHREAD);
#else
    var isMainThread = (typeof ENVIRONMENT_IS_FETCH_WORKER === 'undefined');
#endif

#if FETCH_SUPPORT_INDEXEDDB
    var onsuccess = function(db) {
#if FETCH_DEBUG
      console.log('fetch: IndexedDB successfully opened.');
#endif
      Fetch.dbInstance = db;

      if (isMainThread) {
#if USE_PTHREADS
        Fetch.initFetchWorker();
#endif
        removeRunDependency('library_fetch_init');
      }
    };
    var onerror = function() {
#if FETCH_DEBUG
      console.error('fetch: IndexedDB open failed.');
#endif
      Fetch.dbInstance = false;

      if (isMainThread) {
#if USE_PTHREADS
        Fetch.initFetchWorker();
#endif
        removeRunDependency('library_fetch_init');
      }
    };
    Fetch.openDatabase('emscripten_filesystem', 1, onsuccess, onerror);
#endif // ~FETCH_SUPPORT_INDEXEDDB

#if USE_PTHREADS
    if (isMainThread) {
#if FETCH_SUPPORT_INDEXEDDB
      addRunDependency('library_fetch_init');
#endif

      // Allow HTML module to configure the location where the 'worker.js' file will be loaded from,
      // via Module.locateFile() function. If not specified, then the default URL 'worker.js' relative
      // to the main html file is loaded.
      var fetchJs = locateFile('{{{ FETCH_WORKER_FILE }}}');
      Fetch.worker = new Worker(fetchJs);
      Fetch.worker.onmessage = function(e) {
        out('fetch-worker sent a message: ' + e.filename + ':' + e.lineno + ': ' + e.message);
      };
      Fetch.worker.onerror = function(e) {
        err('fetch-worker sent an error! ' + e.filename + ':' + e.lineno + ': ' + e.message);
      };
    }
#else
#if FETCH_SUPPORT_INDEXEDDB
    if (typeof ENVIRONMENT_IS_FETCH_WORKER === 'undefined' || !ENVIRONMENT_IS_FETCH_WORKER) addRunDependency('library_fetch_init');
#endif
#endif
  }
}

#if FETCH_SUPPORT_INDEXEDDB
function __emscripten_fetch_delete_cached_data(db, fetch, onsuccess, onerror) {
  if (!db) {
#if FETCH_DEBUG
    console.error('fetch: IndexedDB not available!');
#endif
    onerror(fetch, 0, 'IndexedDB not available!');
    return;
  }

  var fetch_attr = fetch + {{{ C_STRUCTS.emscripten_fetch_t.__attributes }}};
  var path = HEAPU32[fetch_attr + {{{ C_STRUCTS.emscripten_fetch_attr_t.destinationPath }}} >> 2];
  if (!path) path = HEAPU32[fetch + {{{ C_STRUCTS.emscripten_fetch_t.url }}} >> 2];
  var pathStr = UTF8ToString(path);

  try {
    var transaction = db.transaction(['FILES'], 'readwrite');
    var packages = transaction.objectStore('FILES');
    var request = packages.delete(pathStr);
    request.onsuccess = function(event) {
      var value = event.target.result;
#if FETCH_DEBUG
      console.log('fetch: Deleted file ' + pathStr + ' from IndexedDB');
#endif
      HEAPU32[fetch + {{{ C_STRUCTS.emscripten_fetch_t.data }}} >> 2] = 0;
      Fetch.setu64(fetch + {{{ C_STRUCTS.emscripten_fetch_t.numBytes }}}, 0);
      Fetch.setu64(fetch + {{{ C_STRUCTS.emscripten_fetch_t.dataOffset }}}, 0);
      Fetch.setu64(fetch + {{{ C_STRUCTS.emscripten_fetch_t.totalBytes }}}, 0);
      HEAPU16[fetch + {{{ C_STRUCTS.emscripten_fetch_t.readyState }}} >> 1] = 4; // Mimic XHR readyState 4 === 'DONE: The operation is complete'
      HEAPU16[fetch + {{{ C_STRUCTS.emscripten_fetch_t.status }}} >> 1] = 200; // Mimic XHR HTTP status code 200 "OK"
      stringToUTF8("OK", fetch + {{{ C_STRUCTS.emscripten_fetch_t.statusText }}}, 64);
      onsuccess(fetch, 0, value);
    };
    request.onerror = function(error) {
#if FETCH_DEBUG
      console.error('fetch: Failed to delete file ' + pathStr + ' from IndexedDB! error: ' + error);
#endif
      HEAPU16[fetch + {{{ C_STRUCTS.emscripten_fetch_t.readyState }}} >> 1] = 4; // Mimic XHR readyState 4 === 'DONE: The operation is complete'
      HEAPU16[fetch + {{{ C_STRUCTS.emscripten_fetch_t.status }}} >> 1] = 404; // Mimic XHR HTTP status code 404 "Not Found"
      stringToUTF8("Not Found", fetch + {{{ C_STRUCTS.emscripten_fetch_t.statusText }}}, 64);
      onerror(fetch, 0, error);
    };
  } catch(e) {
#if FETCH_DEBUG
    console.error('fetch: Failed to load file ' + pathStr + ' from IndexedDB! Got exception ' + e);
#endif
    onerror(fetch, 0, e);
  }
}

function __emscripten_fetch_load_cached_data(db, fetch, onsuccess, onerror) {
  if (!db) {
#if FETCH_DEBUG
    console.error('fetch: IndexedDB not available!');
#endif
    onerror(fetch, 0, 'IndexedDB not available!');
    return;
  }

  var fetch_attr = fetch + {{{ C_STRUCTS.emscripten_fetch_t.__attributes }}};
  var path = HEAPU32[fetch_attr + {{{ C_STRUCTS.emscripten_fetch_attr_t.destinationPath }}} >> 2];
  if (!path) path = HEAPU32[fetch + {{{ C_STRUCTS.emscripten_fetch_t.url }}} >> 2];
  var pathStr = UTF8ToString(path);

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
        // The data pointer malloc()ed here has the same lifetime as the emscripten_fetch_t structure itself has, and is
        // freed when emscripten_fetch_close() is called.
        var ptr = _malloc(len);
        HEAPU8.set(new Uint8Array(value), ptr);
        HEAPU32[fetch + {{{ C_STRUCTS.emscripten_fetch_t.data }}} >> 2] = ptr;
        Fetch.setu64(fetch + {{{ C_STRUCTS.emscripten_fetch_t.numBytes }}}, len);
        Fetch.setu64(fetch + {{{ C_STRUCTS.emscripten_fetch_t.dataOffset }}}, 0);
        Fetch.setu64(fetch + {{{ C_STRUCTS.emscripten_fetch_t.totalBytes }}}, len);
        HEAPU16[fetch + {{{ C_STRUCTS.emscripten_fetch_t.readyState }}} >> 1] = 4; // Mimic XHR readyState 4 === 'DONE: The operation is complete'
        HEAPU16[fetch + {{{ C_STRUCTS.emscripten_fetch_t.status }}} >> 1] = 200; // Mimic XHR HTTP status code 200 "OK"
        stringToUTF8("OK", fetch + {{{ C_STRUCTS.emscripten_fetch_t.statusText }}}, 64);
        onsuccess(fetch, 0, value);
      } else {
        // Succeeded to load, but the load came back with the value of undefined, treat that as an error since we never store undefined in db.
#if FETCH_DEBUG
        console.error('fetch: File ' + pathStr + ' not found in IndexedDB');
#endif
        HEAPU16[fetch + {{{ C_STRUCTS.emscripten_fetch_t.readyState }}} >> 1] = 4; // Mimic XHR readyState 4 === 'DONE: The operation is complete'
        HEAPU16[fetch + {{{ C_STRUCTS.emscripten_fetch_t.status }}} >> 1] = 404; // Mimic XHR HTTP status code 404 "Not Found"
        stringToUTF8("Not Found", fetch + {{{ C_STRUCTS.emscripten_fetch_t.statusText }}}, 64);
        onerror(fetch, 0, 'no data');
      }
    };
    getRequest.onerror = function(error) {
#if FETCH_DEBUG
      console.error('fetch: Failed to load file ' + pathStr + ' from IndexedDB!');
#endif
      HEAPU16[fetch + {{{ C_STRUCTS.emscripten_fetch_t.readyState }}} >> 1] = 4; // Mimic XHR readyState 4 === 'DONE: The operation is complete'
      HEAPU16[fetch + {{{ C_STRUCTS.emscripten_fetch_t.status }}} >> 1] = 404; // Mimic XHR HTTP status code 404 "Not Found"
      stringToUTF8("Not Found", fetch + {{{ C_STRUCTS.emscripten_fetch_t.statusText }}}, 64);
      onerror(fetch, 0, error);
    };
  } catch(e) {
#if FETCH_DEBUG
    console.error('fetch: Failed to load file ' + pathStr + ' from IndexedDB! Got exception ' + e);
#endif
    onerror(fetch, 0, e);
  }
}

function __emscripten_fetch_cache_data(db, fetch, data, onsuccess, onerror) {
  if (!db) {
#if FETCH_DEBUG
    console.error('fetch: IndexedDB not available!');
#endif
    onerror(fetch, 0, 'IndexedDB not available!');
    return;
  }

  var fetch_attr = fetch + {{{ C_STRUCTS.emscripten_fetch_t.__attributes }}};
  var destinationPath = HEAPU32[fetch_attr + {{{ C_STRUCTS.emscripten_fetch_attr_t.destinationPath }}} >> 2];
  if (!destinationPath) destinationPath = HEAPU32[fetch + {{{ C_STRUCTS.emscripten_fetch_t.url }}} >> 2];
  var destinationPathStr = UTF8ToString(destinationPath);

  try {
    var transaction = db.transaction(['FILES'], 'readwrite');
    var packages = transaction.objectStore('FILES');
    var putRequest = packages.put(data, destinationPathStr);
    putRequest.onsuccess = function(event) {
#if FETCH_DEBUG
      console.log('fetch: Stored file "' + destinationPathStr + '" to IndexedDB cache.');
#endif
      HEAPU16[fetch + {{{ C_STRUCTS.emscripten_fetch_t.readyState }}} >> 1] = 4; // Mimic XHR readyState 4 === 'DONE: The operation is complete'
      HEAPU16[fetch + {{{ C_STRUCTS.emscripten_fetch_t.status }}} >> 1] = 200; // Mimic XHR HTTP status code 200 "OK"
      stringToUTF8("OK", fetch + {{{ C_STRUCTS.emscripten_fetch_t.statusText }}}, 64);
      onsuccess(fetch, 0, destinationPathStr);
    };
    putRequest.onerror = function(error) {
#if FETCH_DEBUG
      console.error('fetch: Failed to store file "' + destinationPathStr + '" to IndexedDB cache!');
#endif
      // Most likely we got an error if IndexedDB is unwilling to store any more data for this page.
      // TODO: Can we identify and break down different IndexedDB-provided errors and convert those
      // to more HTTP status codes for more information?
      HEAPU16[fetch + {{{ C_STRUCTS.emscripten_fetch_t.readyState }}} >> 1] = 4; // Mimic XHR readyState 4 === 'DONE: The operation is complete'
      HEAPU16[fetch + {{{ C_STRUCTS.emscripten_fetch_t.status }}} >> 1] = 413; // Mimic XHR HTTP status code 413 "Payload Too Large"
      stringToUTF8("Payload Too Large", fetch + {{{ C_STRUCTS.emscripten_fetch_t.statusText }}}, 64);
      onerror(fetch, 0, error);
    };
  } catch(e) {
#if FETCH_DEBUG
      console.error('fetch: Failed to store file "' + destinationPathStr + '" to IndexedDB cache! Exception: ' + e);
#endif
    onerror(fetch, 0, e);
  }
}
#endif // ~FETCH_SUPPORT_INDEXEDDB

function __emscripten_fetch_xhr(fetch, onsuccess, onerror, onprogress, onreadystatechange) {
  var url = HEAPU32[fetch + {{{ C_STRUCTS.emscripten_fetch_t.url }}} >> 2];
  if (!url) {
#if FETCH_DEBUG
    console.error('fetch: XHR failed, no URL specified!');
#endif
    onerror(fetch, 0, 'no url specified!');
    return;
  }
  var url_ = UTF8ToString(url);

  var fetch_attr = fetch + {{{ C_STRUCTS.emscripten_fetch_t.__attributes }}};
  var requestMethod = UTF8ToString(fetch_attr);
  if (!requestMethod) requestMethod = 'GET';
  var userData = HEAPU32[fetch_attr + {{{ C_STRUCTS.emscripten_fetch_attr_t.userData }}} >> 2];
  var fetchAttributes = HEAPU32[fetch_attr + {{{ C_STRUCTS.emscripten_fetch_attr_t.attributes }}} >> 2];
  var timeoutMsecs = HEAPU32[fetch_attr + {{{ C_STRUCTS.emscripten_fetch_attr_t.timeoutMSecs }}} >> 2];
  var withCredentials = !!HEAPU32[fetch_attr + {{{ C_STRUCTS.emscripten_fetch_attr_t.withCredentials }}} >> 2];
  var destinationPath = HEAPU32[fetch_attr + {{{ C_STRUCTS.emscripten_fetch_attr_t.destinationPath }}} >> 2];
  var userName = HEAPU32[fetch_attr + {{{ C_STRUCTS.emscripten_fetch_attr_t.userName }}} >> 2];
  var password = HEAPU32[fetch_attr + {{{ C_STRUCTS.emscripten_fetch_attr_t.password }}} >> 2];
  var requestHeaders = HEAPU32[fetch_attr + {{{ C_STRUCTS.emscripten_fetch_attr_t.requestHeaders }}} >> 2];
  var overriddenMimeType = HEAPU32[fetch_attr + {{{ C_STRUCTS.emscripten_fetch_attr_t.overriddenMimeType }}} >> 2];
  var dataPtr = HEAPU32[fetch_attr + {{{ C_STRUCTS.emscripten_fetch_attr_t.requestData }}} >> 2];
  var dataLength = HEAPU32[fetch_attr + {{{ C_STRUCTS.emscripten_fetch_attr_t.requestDataSize }}} >> 2];

  var fetchAttrLoadToMemory = !!(fetchAttributes & {{{ cDefine('EMSCRIPTEN_FETCH_LOAD_TO_MEMORY') }}});
  var fetchAttrStreamData = !!(fetchAttributes & {{{ cDefine('EMSCRIPTEN_FETCH_STREAM_DATA') }}});
#if FETCH_SUPPORT_INDEXEDDB
  var fetchAttrPersistFile = !!(fetchAttributes & {{{ cDefine('EMSCRIPTEN_FETCH_PERSIST_FILE') }}});
#endif
  var fetchAttrAppend = !!(fetchAttributes & {{{ cDefine('EMSCRIPTEN_FETCH_APPEND') }}});
  var fetchAttrReplace = !!(fetchAttributes & {{{ cDefine('EMSCRIPTEN_FETCH_REPLACE') }}});
  var fetchAttrSynchronous = !!(fetchAttributes & {{{ cDefine('EMSCRIPTEN_FETCH_SYNCHRONOUS') }}});
  var fetchAttrWaitable = !!(fetchAttributes & {{{ cDefine('EMSCRIPTEN_FETCH_WAITABLE') }}});

  var userNameStr = userName ? UTF8ToString(userName) : undefined;
  var passwordStr = password ? UTF8ToString(password) : undefined;
  var overriddenMimeTypeStr = overriddenMimeType ? UTF8ToString(overriddenMimeType) : undefined;

  var xhr = new XMLHttpRequest();
  xhr.withCredentials = withCredentials;
#if FETCH_DEBUG
  console.log('fetch: xhr.timeout: ' + xhr.timeout + ', xhr.withCredentials: ' + xhr.withCredentials);
  console.log('fetch: xhr.open(requestMethod="' + requestMethod + '", url: "' + url_ +'", userName: ' + userNameStr + ', password: ' + passwordStr + ');');
#endif
  xhr.open(requestMethod, url_, !fetchAttrSynchronous, userNameStr, passwordStr);
  if (!fetchAttrSynchronous) xhr.timeout = timeoutMsecs; // XHR timeout field is only accessible in async XHRs, and must be set after .open() but before .send().
  xhr.url_ = url_; // Save the url for debugging purposes (and for comparing to the responseURL that server side advertised)
  assert(!fetchAttrStreamData, 'streaming uses moz-chunked-arraybuffer which is no longer supported; TODO: rewrite using fetch()');
  xhr.responseType = 'arraybuffer';

  if (overriddenMimeType) {
#if FETCH_DEBUG
    console.log('fetch: xhr.overrideMimeType("' + overriddenMimeTypeStr + '");');
#endif
    xhr.overrideMimeType(overriddenMimeTypeStr);
  }
  if (requestHeaders) {
    for(;;) {
      var key = HEAPU32[requestHeaders >> 2];
      if (!key) break;
      var value = HEAPU32[requestHeaders + 4 >> 2];
      if (!value) break;
      requestHeaders += 8;
      var keyStr = UTF8ToString(key);
      var valueStr = UTF8ToString(value);
#if FETCH_DEBUG
      console.log('fetch: xhr.setRequestHeader("' + keyStr + '", "' + valueStr + '");');
#endif
      xhr.setRequestHeader(keyStr, valueStr);
    }
  }
  Fetch.xhrs.push(xhr);
  var id = Fetch.xhrs.length;
  HEAPU32[fetch + {{{ C_STRUCTS.emscripten_fetch_t.id }}} >> 2] = id;
  var data = (dataPtr && dataLength) ? HEAPU8.slice(dataPtr, dataPtr + dataLength) : null;
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
      // The data pointer malloc()ed here has the same lifetime as the emscripten_fetch_t structure itself has, and is
      // freed when emscripten_fetch_close() is called.
      ptr = _malloc(ptrLen);
      HEAPU8.set(new Uint8Array(xhr.response), ptr);
    }
    HEAPU32[fetch + {{{ C_STRUCTS.emscripten_fetch_t.data }}} >> 2] = ptr;
    Fetch.setu64(fetch + {{{ C_STRUCTS.emscripten_fetch_t.numBytes }}}, ptrLen);
    Fetch.setu64(fetch + {{{ C_STRUCTS.emscripten_fetch_t.dataOffset }}}, 0);
    if (len) {
      // If the final XHR.onload handler receives the bytedata to compute total length, report that,
      // otherwise don't write anything out here, which will retain the latest byte size reported in
      // the most recent XHR.onprogress handler.
      Fetch.setu64(fetch + {{{ C_STRUCTS.emscripten_fetch_t.totalBytes }}}, len);
    }
    HEAPU16[fetch + {{{ C_STRUCTS.emscripten_fetch_t.readyState }}} >> 1] = xhr.readyState;
    if (xhr.readyState === 4 && xhr.status === 0) {
      if (len > 0) xhr.status = 200; // If loading files from a source that does not give HTTP status code, assume success if we got data bytes.
      else xhr.status = 404; // Conversely, no data bytes is 404.
    }
    HEAPU16[fetch + {{{ C_STRUCTS.emscripten_fetch_t.status }}} >> 1] = xhr.status;
    if (xhr.statusText) stringToUTF8(xhr.statusText, fetch + {{{ C_STRUCTS.emscripten_fetch_t.statusText }}}, 64);
    if (xhr.status >= 200 && xhr.status < 300) {
#if FETCH_DEBUG
      console.log('fetch: xhr of URL "' + xhr.url_ + '" / responseURL "' + xhr.responseURL + '" succeeded with status 200');
#endif
      if (onsuccess) onsuccess(fetch, xhr, e);
    } else {
#if FETCH_DEBUG
      console.error('fetch: xhr of URL "' + xhr.url_ + '" / responseURL "' + xhr.responseURL + '" failed with status ' + xhr.status);
#endif
      if (onerror) onerror(fetch, xhr, e);
    }
  };
  xhr.onerror = function(e) {
    var status = xhr.status; // XXX TODO: Overwriting xhr.status doesn't work here, so don't override anywhere else either.
    if (xhr.readyState === 4 && status === 0) status = 404; // If no error recorded, pretend it was 404 Not Found.
#if FETCH_DEBUG
    console.error('fetch: xhr of URL "' + xhr.url_ + '" / responseURL "' + xhr.responseURL + '" finished with error, readyState ' + xhr.readyState + ' and status ' + status);
#endif
    HEAPU32[fetch + {{{ C_STRUCTS.emscripten_fetch_t.data }}} >> 2] = 0;
    Fetch.setu64(fetch + {{{ C_STRUCTS.emscripten_fetch_t.numBytes }}}, 0);
    Fetch.setu64(fetch + {{{ C_STRUCTS.emscripten_fetch_t.dataOffset }}}, 0);
    Fetch.setu64(fetch + {{{ C_STRUCTS.emscripten_fetch_t.totalBytes }}}, 0);
    HEAPU16[fetch + {{{ C_STRUCTS.emscripten_fetch_t.readyState }}} >> 1] = xhr.readyState;
    HEAPU16[fetch + {{{ C_STRUCTS.emscripten_fetch_t.status }}} >> 1] = status;
    if (onerror) onerror(fetch, xhr, e);
  };
  xhr.ontimeout = function(e) {
#if FETCH_DEBUG
    console.error('fetch: xhr of URL "' + xhr.url_ + '" / responseURL "' + xhr.responseURL + '" timed out, readyState ' + xhr.readyState + ' and status ' + xhr.status);
#endif
    if (onerror) onerror(fetch, xhr, e);
  };
  xhr.onprogress = function(e) {
    var ptrLen = (fetchAttrLoadToMemory && fetchAttrStreamData && xhr.response) ? xhr.response.byteLength : 0;
    var ptr = 0;
    if (fetchAttrLoadToMemory && fetchAttrStreamData) {
#if FETCH_DEBUG
      console.log('fetch: allocating ' + ptrLen + ' bytes in Emscripten heap for xhr data');
#endif
      // The data pointer malloc()ed here has the same lifetime as the emscripten_fetch_t structure itself has, and is
      // freed when emscripten_fetch_close() is called.
      ptr = _malloc(ptrLen);
      HEAPU8.set(new Uint8Array(xhr.response), ptr);
    }
    HEAPU32[fetch + {{{ C_STRUCTS.emscripten_fetch_t.data }}} >> 2] = ptr;
    Fetch.setu64(fetch + {{{ C_STRUCTS.emscripten_fetch_t.numBytes }}}, ptrLen);
    Fetch.setu64(fetch + {{{ C_STRUCTS.emscripten_fetch_t.dataOffset }}}, e.loaded - ptrLen);
    Fetch.setu64(fetch + {{{ C_STRUCTS.emscripten_fetch_t.totalBytes }}}, e.total);
    HEAPU16[fetch + {{{ C_STRUCTS.emscripten_fetch_t.readyState }}} >> 1] = xhr.readyState;
    if (xhr.readyState >= 3 && xhr.status === 0 && e.loaded > 0) xhr.status = 200; // If loading files from a source that does not give HTTP status code, assume success if we get data bytes
    HEAPU16[fetch + {{{ C_STRUCTS.emscripten_fetch_t.status }}} >> 1] = xhr.status;
    if (xhr.statusText) stringToUTF8(xhr.statusText, fetch + {{{ C_STRUCTS.emscripten_fetch_t.statusText }}}, 64);
    if (onprogress) onprogress(fetch, xhr, e);
  };
  xhr.onreadystatechange = function(e) {
    HEAPU16[fetch + {{{ C_STRUCTS.emscripten_fetch_t.readyState }}} >> 1] = xhr.readyState;
    if (xhr.readyState >= 2) {
      HEAPU16[fetch + {{{ C_STRUCTS.emscripten_fetch_t.status }}} >> 1] = xhr.status;
    }
    if (onreadystatechange) onreadystatechange(fetch, xhr, e);
  };
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

function emscripten_start_fetch(fetch, successcb, errorcb, progresscb, readystatechangecb) {
  if (typeof noExitRuntime !== 'undefined') noExitRuntime = true; // If we are the main Emscripten runtime, we should not be closing down.

  var fetch_attr = fetch + {{{ C_STRUCTS.emscripten_fetch_t.__attributes }}};
  var requestMethod = UTF8ToString(fetch_attr);
  var onsuccess = HEAPU32[fetch_attr + {{{ C_STRUCTS.emscripten_fetch_attr_t.onsuccess }}} >> 2];
  var onerror = HEAPU32[fetch_attr + {{{ C_STRUCTS.emscripten_fetch_attr_t.onerror }}} >> 2];
  var onprogress = HEAPU32[fetch_attr + {{{ C_STRUCTS.emscripten_fetch_attr_t.onprogress }}} >> 2];
  var onreadystatechange = HEAPU32[fetch_attr + {{{ C_STRUCTS.emscripten_fetch_attr_t.onreadystatechange }}} >> 2];
  var fetchAttributes = HEAPU32[fetch_attr + {{{ C_STRUCTS.emscripten_fetch_attr_t.attributes }}} >> 2];
  var fetchAttrLoadToMemory = !!(fetchAttributes & {{{ cDefine('EMSCRIPTEN_FETCH_LOAD_TO_MEMORY') }}});
  var fetchAttrStreamData = !!(fetchAttributes & {{{ cDefine('EMSCRIPTEN_FETCH_STREAM_DATA') }}});
#if FETCH_SUPPORT_INDEXEDDB
  var fetchAttrPersistFile = !!(fetchAttributes & {{{ cDefine('EMSCRIPTEN_FETCH_PERSIST_FILE') }}});
  var fetchAttrNoDownload = !!(fetchAttributes & {{{ cDefine('EMSCRIPTEN_FETCH_NO_DOWNLOAD') }}});
#endif
  var fetchAttrAppend = !!(fetchAttributes & {{{ cDefine('EMSCRIPTEN_FETCH_APPEND') }}});
  var fetchAttrReplace = !!(fetchAttributes & {{{ cDefine('EMSCRIPTEN_FETCH_REPLACE') }}});

  var reportSuccess = function(fetch, xhr, e) {
#if FETCH_DEBUG
    console.log('fetch: operation success. e: ' + e);
#endif
    if (onsuccess) {{{ makeDynCall('vi') }}}(onsuccess, fetch);
    else if (successcb) successcb(fetch);
  };

  var reportProgress = function(fetch, xhr, e) {
    if (onprogress) {{{ makeDynCall('vi') }}}(onprogress, fetch);
    else if (progresscb) progresscb(fetch);
  };

  var reportError = function(fetch, xhr, e) {
#if FETCH_DEBUG
    console.error('fetch: operation failed: ' + e);
#endif
    if (onerror) {{{ makeDynCall('vi') }}}(onerror, fetch);
    else if (errorcb) errorcb(fetch);
  };

  var reportReadyStateChange = function(fetch, xhr, e) {
#if FETCH_DEBUG
    console.log('fetch: ready state change. e: ' + e);
#endif
    if (onreadystatechange) {{{ makeDynCall('vi') }}}(onreadystatechange, fetch);
    else if (readystatechangecb) readystatechangecb(fetch);
  };

  var performUncachedXhr = function(fetch, xhr, e) {
#if FETCH_DEBUG
    console.error('fetch: starting (uncached) XHR: ' + e);
#endif
    __emscripten_fetch_xhr(fetch, reportSuccess, reportError, reportProgress, reportReadyStateChange);
  };

#if FETCH_SUPPORT_INDEXEDDB
  var cacheResultAndReportSuccess = function(fetch, xhr, e) {
#if FETCH_DEBUG
    console.log('fetch: operation success. Caching result.. e: ' + e);
#endif
    var storeSuccess = function(fetch, xhr, e) {
#if FETCH_DEBUG
      console.log('fetch: IndexedDB store succeeded.');
#endif
      if (onsuccess) {{{ makeDynCall('vi') }}}(onsuccess, fetch);
      else if (successcb) successcb(fetch);
    };
    var storeError = function(fetch, xhr, e) {
#if FETCH_DEBUG
      console.error('fetch: IndexedDB store failed.');
#endif
      if (onsuccess) {{{ makeDynCall('vi') }}}(onsuccess, fetch);
      else if (successcb) successcb(fetch);
    };
    __emscripten_fetch_cache_data(Fetch.dbInstance, fetch, xhr.response, storeSuccess, storeError);
  };

  var performCachedXhr = function(fetch, xhr, e) {
#if FETCH_DEBUG
    console.error('fetch: starting (cached) XHR: ' + e);
#endif
    __emscripten_fetch_xhr(fetch, cacheResultAndReportSuccess, reportError, reportProgress, reportReadyStateChange);
  };

  // Should we try IndexedDB first?
  var needsIndexedDbConnection = !fetchAttrReplace || requestMethod === 'EM_IDB_STORE' || requestMethod === 'EM_IDB_DELETE';
  if (needsIndexedDbConnection && !Fetch.dbInstance) {
#if FETCH_DEBUG
    console.error('fetch: failed to read IndexedDB! Database is not open.');
#endif
    reportError(fetch, 0, 'IndexedDB is not open');
    return 0; // todo: free
  }

  if (requestMethod === 'EM_IDB_STORE') {
    // TODO(?): Here we perform a clone of the data, because storing shared typed arrays to IndexedDB does not seem to be allowed.
    var ptr = HEAPU32[fetch_attr + {{{ C_STRUCTS.emscripten_fetch_attr_t.requestData }}} >> 2];
    __emscripten_fetch_cache_data(Fetch.dbInstance, fetch, HEAPU8.slice(ptr, ptr + HEAPU32[fetch_attr + {{{ C_STRUCTS.emscripten_fetch_attr_t.requestDataSize }}} >> 2]), reportSuccess, reportError);
  } else if (requestMethod === 'EM_IDB_DELETE') {
    __emscripten_fetch_delete_cached_data(Fetch.dbInstance, fetch, reportSuccess, reportError);
  } else if (!fetchAttrReplace) {
    __emscripten_fetch_load_cached_data(Fetch.dbInstance, fetch, reportSuccess, fetchAttrNoDownload ? reportError : (fetchAttrPersistFile ? performCachedXhr : performUncachedXhr));
  } else if (!fetchAttrNoDownload) {
    __emscripten_fetch_xhr(fetch, fetchAttrPersistFile ? cacheResultAndReportSuccess : reportSuccess, reportError, reportProgress, reportReadyStateChange);
  } else {
#if FETCH_DEBUG
    console.error('fetch: Invalid combination of flags passed.');
#endif
    return 0; // todo: free
  }
  return fetch;
#else // !FETCH_SUPPORT_INDEXEDDB
  __emscripten_fetch_xhr(fetch, reportSuccess, reportError, reportProgress, reportReadyStateChange);
  return fetch;
#endif // ~FETCH_SUPPORT_INDEXEDDB
}

function _fetch_get_response_headers_length(id) {
    return lengthBytesUTF8(Fetch.xhrs[id-1].getAllResponseHeaders()) + 1;
}

function _fetch_get_response_headers(id, dst, dstSizeBytes) {
    var responseHeaders = Fetch.xhrs[id-1].getAllResponseHeaders();
    var lengthBytes = lengthBytesUTF8(responseHeaders) + 1;
    stringToUTF8(responseHeaders, dst, dstSizeBytes);
    return Math.min(lengthBytes, dstSizeBytes);
}

//Delete the xhr JS object, allowing it to be garbage collected.
function _fetch_free(id) {
  //Note: should just be [id], but indexes off by 1 (see: #8803)
#if FETCH_DEBUG
  console.log("fetch: Deleting id:" + (id-1) + " of " + Fetch.xhrs);
#endif
  delete Fetch.xhrs[id-1];
}
