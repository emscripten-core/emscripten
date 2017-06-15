var Fetch = {
  attr_t_offset_requestMethod: 0,
  attr_t_offset_userData: 32,
  attr_t_offset_onsuccess: 36,
  attr_t_offset_onerror: 40,
  attr_t_offset_onprogress: 44,
  attr_t_offset_attributes: 48,
  attr_t_offset_timeoutMSecs: 52,
  attr_t_offset_withCredentials: 56,
  attr_t_offset_destinationPath: 60,
  attr_t_offset_userName: 64,
  attr_t_offset_password: 68,
  attr_t_offset_requestHeaders: 72,
  attr_t_offset_overriddenMimeType: 76,
  attr_t_offset_requestData: 80,
  attr_t_offset_requestDataSize: 84,

  fetch_t_offset_id: 0,
  fetch_t_offset_userData: 4,
  fetch_t_offset_url: 8,
  fetch_t_offset_data: 12,
  fetch_t_offset_numBytes: 16,
  fetch_t_offset_dataOffset: 24,
  fetch_t_offset_totalBytes: 32,
  fetch_t_offset_readyState: 40,
  fetch_t_offset_status: 42,
  fetch_t_offset_statusText: 44,
  fetch_t_offset___proxyState: 108,
  fetch_t_offset___attributes: 112,

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
    Fetch.worker.postMessage({cmd: 'init', TOTAL_MEMORY: TOTAL_MEMORY, DYNAMICTOP_PTR: DYNAMICTOP_PTR, STACKTOP: stack, STACK_MAX: stack + stackSize, queuePtr: _fetch_work_queue, buffer: HEAPU8.buffer});
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

#if USE_PTHREADS
      if (isMainThread) {
        if (typeof SharedArrayBuffer !== 'undefined') Fetch.initFetchWorker();
        removeRunDependency('library_fetch_init');
      }
#else
      if (typeof ENVIRONMENT_IS_FETCH_WORKER === 'undefined' || !ENVIRONMENT_IS_FETCH_WORKER) removeRunDependency('library_fetch_init');
#endif
    };
    var onerror = function() {
#if FETCH_DEBUG
      console.error('fetch: IndexedDB open failed.');
#endif
      Fetch.dbInstance = false;

#if USE_PTHREADS
      if (isMainThread) {
        if (typeof SharedArrayBuffer !== 'undefined') Fetch.initFetchWorker();
        removeRunDependency('library_fetch_init');
      }
#endif
    };
    Fetch.openDatabase('emscripten_filesystem', 1, onsuccess, onerror);

#if USE_PTHREADS
    if (isMainThread) {
      addRunDependency('library_fetch_init');

      var fetchJs = 'fetch-worker.js';
      // Allow HTML module to configure the location where the 'pthread-main.js' file will be loaded from,
      // either via Module.locateFile() function, or via Module.pthreadMainPrefixURL string. If neither
      // of these are passed, then the default URL 'pthread-main.js' relative to the main html file is loaded.
      if (typeof Module['locateFile'] === 'function') fetchJs = Module['locateFile'](fetchJs);
      else if (Module['pthreadMainPrefixURL']) fetchJs = Module['pthreadMainPrefixURL'] + fetchJs;
      Fetch.worker = new Worker(fetchJs);
      Fetch.worker.onmessage = function(e) {
        Module['print']('fetch-worker sent a message: ' + e.filename + ':' + e.lineno + ': ' + e.message);
      };
      Fetch.worker.onerror = function(e) {
        Module['printErr']('fetch-worker sent an error! ' + e.filename + ':' + e.lineno + ': ' + e.message);
      };
    }
#else
    if (typeof ENVIRONMENT_IS_FETCH_WORKER === 'undefined' || !ENVIRONMENT_IS_FETCH_WORKER) addRunDependency('library_fetch_init');
#endif
  }
}

function __emscripten_fetch_delete_cached_data(db, fetch, onsuccess, onerror) {
  if (!db) {
#if FETCH_DEBUG
    console.error('fetch: IndexedDB not available!');
#endif
    onerror(fetch, 0, 'IndexedDB not available!');
    return;
  }

  var fetch_attr = fetch + Fetch.fetch_t_offset___attributes;
  var path = HEAPU32[fetch_attr + Fetch.attr_t_offset_destinationPath >> 2];
  if (!path) path = HEAPU32[fetch + Fetch.fetch_t_offset_url >> 2];
  var pathStr = Pointer_stringify(path);

  try {
    var transaction = db.transaction(['FILES'], 'readwrite');
    var packages = transaction.objectStore('FILES');
    var request = packages.delete(pathStr);
    request.onsuccess = function(event) {
      var value = event.target.result;
#if FETCH_DEBUG
      console.log('fetch: Deleted file ' + pathStr + ' from IndexedDB');
#endif
      HEAPU32[fetch + Fetch.fetch_t_offset_data >> 2] = 0;
      Fetch.setu64(fetch + Fetch.fetch_t_offset_numBytes, 0);
      Fetch.setu64(fetch + Fetch.fetch_t_offset_dataOffset, 0);
      Fetch.setu64(fetch + Fetch.fetch_t_offset_dataOffset, 0);
      HEAPU16[fetch + Fetch.fetch_t_offset_readyState >> 1] = 4; // Mimic XHR readyState 4 === 'DONE: The operation is complete'
      HEAPU16[fetch + Fetch.fetch_t_offset_status >> 1] = 200; // Mimic XHR HTTP status code 200 "OK"
      stringToUTF8("OK", fetch + Fetch.fetch_t_offset_statusText, 64);
      onsuccess(fetch, 0, value);
    };
    request.onerror = function(error) {
#if FETCH_DEBUG
      console.error('fetch: Failed to delete file ' + pathStr + ' from IndexedDB! error: ' + error);
#endif
      HEAPU16[fetch + Fetch.fetch_t_offset_readyState >> 1] = 4; // Mimic XHR readyState 4 === 'DONE: The operation is complete'
      HEAPU16[fetch + Fetch.fetch_t_offset_status >> 1] = 404; // Mimic XHR HTTP status code 404 "Not Found"
      stringToUTF8("Not Found", fetch + Fetch.fetch_t_offset_statusText, 64);
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

  var fetch_attr = fetch + Fetch.fetch_t_offset___attributes;
  var path = HEAPU32[fetch_attr + Fetch.attr_t_offset_destinationPath >> 2];
  if (!path) path = HEAPU32[fetch + Fetch.fetch_t_offset_url >> 2];
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

        // The data pointer malloc()ed here has the same lifetime as the emscripten_fetch_t structure itself has, and is
        // freed when emscripten_fetch_close() is called.
        var ptr = _malloc(len);
        HEAPU8.set(new Uint8Array(value), ptr);
        HEAPU32[fetch + Fetch.fetch_t_offset_data >> 2] = ptr;
        Fetch.setu64(fetch + Fetch.fetch_t_offset_numBytes, len);
        Fetch.setu64(fetch + Fetch.fetch_t_offset_dataOffset, 0);
        Fetch.setu64(fetch + Fetch.fetch_t_offset_totalBytes, len);
        HEAPU16[fetch + Fetch.fetch_t_offset_readyState >> 1] = 4; // Mimic XHR readyState 4 === 'DONE: The operation is complete'
        HEAPU16[fetch + Fetch.fetch_t_offset_status >> 1] = 200; // Mimic XHR HTTP status code 200 "OK"
        stringToUTF8("OK", fetch + Fetch.fetch_t_offset_statusText, 64);
        onsuccess(fetch, 0, value);
      } else {
        // Succeeded to load, but the load came back with the value of undefined, treat that as an error since we never store undefined in db.
#if FETCH_DEBUG
        console.error('fetch: File ' + pathStr + ' not found in IndexedDB');
#endif
        HEAPU16[fetch + Fetch.fetch_t_offset_readyState >> 1] = 4; // Mimic XHR readyState 4 === 'DONE: The operation is complete'
        HEAPU16[fetch + Fetch.fetch_t_offset_status >> 1] = 404; // Mimic XHR HTTP status code 404 "Not Found"
        stringToUTF8("Not Found", fetch + Fetch.fetch_t_offset_statusText, 64);
        onerror(fetch, 0, 'no data');
      }
    };
    getRequest.onerror = function(error) {
#if FETCH_DEBUG
      console.error('fetch: Failed to load file ' + pathStr + ' from IndexedDB!');
#endif
      HEAPU16[fetch + Fetch.fetch_t_offset_readyState >> 1] = 4; // Mimic XHR readyState 4 === 'DONE: The operation is complete'
      HEAPU16[fetch + Fetch.fetch_t_offset_status >> 1] = 404; // Mimic XHR HTTP status code 404 "Not Found"
      stringToUTF8("Not Found", fetch + Fetch.fetch_t_offset_statusText, 64);
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

  var fetch_attr = fetch + Fetch.fetch_t_offset___attributes;
  var destinationPath = HEAPU32[fetch_attr + Fetch.attr_t_offset_destinationPath >> 2];
  if (!destinationPath) destinationPath = HEAPU32[fetch + Fetch.fetch_t_offset_url >> 2];
  var destinationPathStr = Pointer_stringify(destinationPath);

  try {
    var transaction = db.transaction(['FILES'], 'readwrite');
    var packages = transaction.objectStore('FILES');
    var putRequest = packages.put(data, destinationPathStr);
    putRequest.onsuccess = function(event) {
#if FETCH_DEBUG
      console.log('fetch: Stored file "' + destinationPathStr + '" to IndexedDB cache.');
#endif
      HEAPU16[fetch + Fetch.fetch_t_offset_readyState >> 1] = 4; // Mimic XHR readyState 4 === 'DONE: The operation is complete'
      HEAPU16[fetch + Fetch.fetch_t_offset_status >> 1] = 200; // Mimic XHR HTTP status code 200 "OK"
      stringToUTF8("OK", fetch + Fetch.fetch_t_offset_statusText, 64);
      onsuccess(fetch, 0, destinationPathStr);
    };
    putRequest.onerror = function(error) {
#if FETCH_DEBUG
      console.error('fetch: Failed to store file "' + destinationPathStr + '" to IndexedDB cache!');
#endif
      // Most likely we got an error if IndexedDB is unwilling to store any more data for this page.
      // TODO: Can we identify and break down different IndexedDB-provided errors and convert those
      // to more HTTP status codes for more information?
      HEAPU16[fetch + Fetch.fetch_t_offset_readyState >> 1] = 4; // Mimic XHR readyState 4 === 'DONE: The operation is complete'
      HEAPU16[fetch + Fetch.fetch_t_offset_status >> 1] = 413; // Mimic XHR HTTP status code 413 "Payload Too Large"
      stringToUTF8("Payload Too Large", fetch + Fetch.fetch_t_offset_statusText, 64);
      onerror(fetch, 0, error);
    };
  } catch(e) {
#if FETCH_DEBUG
      console.error('fetch: Failed to store file "' + destinationPathStr + '" to IndexedDB cache! Exception: ' + e);
#endif
    onerror(fetch, 0, e);
  }
}

function __emscripten_fetch_xhr(fetch, onsuccess, onerror, onprogress) {
  var url = HEAPU32[fetch + Fetch.fetch_t_offset_url >> 2];
  if (!url) {
#if FETCH_DEBUG
    console.error('fetch: XHR failed, no URL specified!');
#endif
    onerror(fetch, 0, 'no url specified!');
    return;
  }
  var url_ = Pointer_stringify(url);

  var fetch_attr = fetch + Fetch.fetch_t_offset___attributes;
  var requestMethod = Pointer_stringify(fetch_attr);
  if (!requestMethod) requestMethod = 'GET';
  var userData = HEAPU32[fetch_attr + Fetch.attr_t_offset_userData >> 2];
  var fetchAttributes = HEAPU32[fetch_attr + Fetch.attr_t_offset_attributes >> 2];
  var timeoutMsecs = HEAPU32[fetch_attr + Fetch.attr_t_offset_timeoutMSecs >> 2];
  var withCredentials = !!HEAPU32[fetch_attr + Fetch.attr_t_offset_withCredentials >> 2];
  var destinationPath = HEAPU32[fetch_attr + Fetch.attr_t_offset_destinationPath >> 2];
  var userName = HEAPU32[fetch_attr + Fetch.attr_t_offset_userName >> 2];
  var password = HEAPU32[fetch_attr + Fetch.attr_t_offset_password >> 2];
  var requestHeaders = HEAPU32[fetch_attr + Fetch.attr_t_offset_requestHeaders >> 2];
  var overriddenMimeType = HEAPU32[fetch_attr + Fetch.attr_t_offset_overriddenMimeType >> 2];
  var dataPtr = HEAPU32[fetch_attr + Fetch.attr_t_offset_requestData >> 2];
  var dataLength = HEAPU32[fetch_attr + Fetch.attr_t_offset_requestDataSize >> 2];

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
  xhr.withCredentials = withCredentials;
#if FETCH_DEBUG
  console.log('fetch: xhr.timeout: ' + xhr.timeout + ', xhr.withCredentials: ' + xhr.withCredentials);
  console.log('fetch: xhr.open(requestMethod="' + requestMethod + '", url: "' + url_ +'", userName: ' + userNameStr + ', password: ' + passwordStr + ');');
#endif
  xhr.open(requestMethod, url_, !fetchAttrSynchronous, userNameStr, passwordStr);
  if (!fetchAttrSynchronous) xhr.timeout = timeoutMsecs; // XHR timeout field is only accessible in async XHRs, and must be set after .open() but before .send().
  xhr.url_ = url_; // Save the url for debugging purposes (and for comparing to the responseURL that server side advertised)
  xhr.responseType = fetchAttrStreamData ? 'moz-chunked-arraybuffer' : 'arraybuffer';

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
  HEAPU32[fetch + Fetch.fetch_t_offset_id >> 2] = id;
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
    HEAPU32[fetch + Fetch.fetch_t_offset_data >> 2] = ptr;
    Fetch.setu64(fetch + Fetch.fetch_t_offset_numBytes, ptrLen);
    Fetch.setu64(fetch + Fetch.fetch_t_offset_dataOffset, 0);
    if (len) {
      // If the final XHR.onload handler receives the bytedata to compute total length, report that,
      // otherwise don't write anything out here, which will retain the latest byte size reported in
      // the most recent XHR.onprogress handler.
      Fetch.setu64(fetch + Fetch.fetch_t_offset_totalBytes, len);
    }
    HEAPU16[fetch + Fetch.fetch_t_offset_readyState >> 1] = xhr.readyState;
    if (xhr.readyState === 4 && xhr.status === 0) {
      if (len > 0) xhr.status = 200; // If loading files from a source that does not give HTTP status code, assume success if we got data bytes.
      else xhr.status = 404; // Conversely, no data bytes is 404.
    }
    HEAPU16[fetch + Fetch.fetch_t_offset_status >> 1] = xhr.status;
    if (xhr.statusText) stringToUTF8(xhr.statusText, fetch + Fetch.fetch_t_offset_statusText, 64);
    if (xhr.status == 200) {
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
  }
  xhr.onerror = function(e) {
    var status = xhr.status; // XXX TODO: Overwriting xhr.status doesn't work here, so don't override anywhere else either.
    if (xhr.readyState == 4 && status == 0) status = 404; // If no error recorded, pretend it was 404 Not Found.
#if FETCH_DEBUG
    console.error('fetch: xhr of URL "' + xhr.url_ + '" / responseURL "' + xhr.responseURL + '" finished with error, readyState ' + xhr.readyState + ' and status ' + status);
#endif
    HEAPU32[fetch + Fetch.fetch_t_offset_data >> 2] = 0;
    Fetch.setu64(fetch + Fetch.fetch_t_offset_numBytes, 0);
    Fetch.setu64(fetch + Fetch.fetch_t_offset_dataOffset, 0);
    Fetch.setu64(fetch + Fetch.fetch_t_offset_totalBytes, 0);
    HEAPU16[fetch + Fetch.fetch_t_offset_readyState >> 1] = xhr.readyState;
    HEAPU16[fetch + Fetch.fetch_t_offset_status >> 1] = status;
    if (onerror) onerror(fetch, xhr, e);
  }
  xhr.ontimeout = function(e) {
#if FETCH_DEBUG
    console.error('fetch: xhr of URL "' + xhr.url_ + '" / responseURL "' + xhr.responseURL + '" timed out, readyState ' + xhr.readyState + ' and status ' + xhr.status);
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
      // The data pointer malloc()ed here has the same lifetime as the emscripten_fetch_t structure itself has, and is
      // freed when emscripten_fetch_close() is called.
      ptr = _malloc(ptrLen);
      HEAPU8.set(new Uint8Array(xhr.response), ptr);
    }
    HEAPU32[fetch + Fetch.fetch_t_offset_data >> 2] = ptr;
    Fetch.setu64(fetch + Fetch.fetch_t_offset_numBytes, ptrLen);
    Fetch.setu64(fetch + Fetch.fetch_t_offset_dataOffset, e.loaded - ptrLen);
    Fetch.setu64(fetch + Fetch.fetch_t_offset_totalBytes, e.total);
    HEAPU16[fetch + Fetch.fetch_t_offset_readyState >> 1] = xhr.readyState;
    if (xhr.readyState >= 3 && xhr.status === 0 && e.loaded > 0) xhr.status = 200; // If loading files from a source that does not give HTTP status code, assume success if we get data bytes
    HEAPU16[fetch + Fetch.fetch_t_offset_status >> 1] = xhr.status;
    if (xhr.statusText) stringToUTF8(xhr.statusText, fetch + Fetch.fetch_t_offset_statusText, 64);
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

  var fetch_attr = fetch + Fetch.fetch_t_offset___attributes;
  var requestMethod = Pointer_stringify(fetch_attr);
  var onsuccess = HEAPU32[fetch_attr + Fetch.attr_t_offset_onsuccess >> 2];
  var onerror = HEAPU32[fetch_attr + Fetch.attr_t_offset_onerror >> 2];
  var onprogress = HEAPU32[fetch_attr + Fetch.attr_t_offset_onprogress >> 2];
  var fetchAttributes = HEAPU32[fetch_attr + Fetch.attr_t_offset_attributes >> 2];
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
    if (onsuccess && Runtime.dynCall) Module['dynCall_vi'](onsuccess, fetch);
    else if (successcb) successcb(fetch);
  };

  var cacheResultAndReportSuccess = function(fetch, xhr, e) {
#if FETCH_DEBUG
    console.log('fetch: operation success. Caching result.. e: ' + e);
#endif
    var storeSuccess = function(fetch, xhr, e) {
#if FETCH_DEBUG
      console.log('fetch: IndexedDB store succeeded.');
#endif
      if (onsuccess && Runtime.dynCall) Module['dynCall_vi'](onsuccess, fetch);
      else if (successcb) successcb(fetch);
    };
    var storeError = function(fetch, xhr, e) {
#if FETCH_DEBUG
      console.error('fetch: IndexedDB store failed.');
#endif
      if (onsuccess && Runtime.dynCall) Module['dynCall_vi'](onsuccess, fetch);
      else if (successcb) successcb(fetch);
    };
    __emscripten_fetch_cache_data(Fetch.dbInstance, fetch, xhr.response, storeSuccess, storeError);
  };

  var reportProgress = function(fetch, xhr, e) {
    if (onprogress && Runtime.dynCall) Module['dynCall_vi'](onprogress, fetch);
    else if (progresscb) progresscb(fetch);
  };

  var reportError = function(fetch, xhr, e) {
#if FETCH_DEBUG
    console.error('fetch: operation failed: ' + e);
#endif
    if (onerror && Runtime.dynCall) Module['dynCall_vi'](onerror, fetch);
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
  if (!fetchAttrReplace || requestMethod === 'EM_IDB_STORE' || requestMethod === 'EM_IDB_DELETE') {
    if (!Fetch.dbInstance) {
#if FETCH_DEBUG
      console.error('fetch: failed to read IndexedDB! Database is not open.');
#endif
      reportError(fetch, 0, 'IndexedDB is not open');
      return 0; // todo: free
    }

    if (requestMethod === 'EM_IDB_STORE') {
      var dataPtr = HEAPU32[fetch_attr + Fetch.attr_t_offset_requestData >> 2];
      var dataLength = HEAPU32[fetch_attr + Fetch.attr_t_offset_requestDataSize >> 2];
      var data = HEAPU8.slice(dataPtr, dataPtr + dataLength); // TODO(?): Here we perform a clone of the data, because storing shared typed arrays to IndexedDB does not seem to be allowed.
      __emscripten_fetch_cache_data(Fetch.dbInstance, fetch, data, reportSuccess, reportError);
    } else if (requestMethod === 'EM_IDB_DELETE') {
      __emscripten_fetch_delete_cached_data(Fetch.dbInstance, fetch, reportSuccess, reportError);
    } else if (fetchAttrNoDownload) {
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
