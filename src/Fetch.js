/**
 * @license
 * Copyright 2016 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

var Fetch = {
  // HandleAllocator for XHR request object
  // xhrs: undefined,

  // The web worker that runs proxied file I/O requests. (this field is
  // populated on demand, start as undefined to save code size)
  // worker: undefined,

  // Specifies an instance to the IndexedDB database. The database is opened
  // as a preload step before the Emscripten application starts. (this field is
  // populated on demand, start as undefined to save code size)
  // dbInstance: undefined,

#if FETCH_SUPPORT_INDEXEDDB
  // Be cautious that `onerror` may be run synchronously
  openDatabase(dbname, dbversion, onsuccess, onerror) {
    try {
#if FETCH_DEBUG
      dbg(`fetch: indexedDB.open(dbname="${dbname}", dbversion="${dbversion}");`);
#endif
      var openRequest = indexedDB.open(dbname, dbversion);
    } catch (e) {
      return onerror(e);
    }

    openRequest.onupgradeneeded = (event) => {
#if FETCH_DEBUG
      dbg('fetch: IndexedDB upgrade needed. Clearing database.');
#endif
      var db = /** @type {IDBDatabase} */ (event.target.result);
      if (db.objectStoreNames.contains('FILES')) {
        db.deleteObjectStore('FILES');
      }
      db.createObjectStore('FILES');
    };
    openRequest.onsuccess = (event) => onsuccess(event.target.result);
    openRequest.onerror = onerror;
  },
#endif

  init() {
    Fetch.xhrs = new HandleAllocator();
#if FETCH_SUPPORT_INDEXEDDB
#if PTHREADS
    if (ENVIRONMENT_IS_PTHREAD) return;
#endif
    var onsuccess = (db) => {
#if FETCH_DEBUG
      dbg('fetch: IndexedDB successfully opened.');
#endif
      Fetch.dbInstance = db;
      removeRunDependency('library_fetch_init');
    };

    var onerror = () => {
#if FETCH_DEBUG
      dbg('fetch: IndexedDB open failed.');
#endif
      Fetch.dbInstance = false;
      removeRunDependency('library_fetch_init');
    };

    addRunDependency('library_fetch_init');
    Fetch.openDatabase('emscripten_filesystem', 1, onsuccess, onerror);
#endif // ~FETCH_SUPPORT_INDEXEDDB
  }
}

#if FETCH_SUPPORT_INDEXEDDB
function fetchDeleteCachedData(db, fetch, onsuccess, onerror) {
  if (!db) {
#if FETCH_DEBUG
    dbg('fetch: IndexedDB not available!');
#endif
    onerror(fetch, 0, 'IndexedDB not available!');
    return;
  }

  var fetch_attr = fetch + {{{ C_STRUCTS.emscripten_fetch_t.__attributes }}};
  var path = {{{ makeGetValue('fetch_attr', C_STRUCTS.emscripten_fetch_attr_t.destinationPath, '*') }}};
  path ||= {{{ makeGetValue('fetch', C_STRUCTS.emscripten_fetch_t.url, '*') }}};

  var pathStr = UTF8ToString(path);

  try {
    var transaction = db.transaction(['FILES'], 'readwrite');
    var packages = transaction.objectStore('FILES');
    var request = packages.delete(pathStr);
    request.onsuccess = (event) => {
      var value = event.target.result;
#if FETCH_DEBUG
      dbg(`fetch: Deleted file ${pathStr} from IndexedDB`);
#endif
      {{{ makeSetValue('fetch', C_STRUCTS.emscripten_fetch_t.data, 0, '*') }}};
      writeI53ToI64(fetch + {{{ C_STRUCTS.emscripten_fetch_t.numBytes }}}, 0);
      writeI53ToI64(fetch + {{{ C_STRUCTS.emscripten_fetch_t.dataOffset }}}, 0);
      writeI53ToI64(fetch + {{{ C_STRUCTS.emscripten_fetch_t.totalBytes }}}, 0);
      // Mimic XHR readyState 4 === 'DONE: The operation is complete'
      {{{ makeSetValue('fetch', C_STRUCTS.emscripten_fetch_t.readyState, 4, 'i16') }}};
      // Mimic XHR HTTP status code 200 "OK"
      {{{ makeSetValue('fetch', C_STRUCTS.emscripten_fetch_t.status, 200, 'i16') }}};
      stringToUTF8("OK", fetch + {{{ C_STRUCTS.emscripten_fetch_t.statusText }}}, 64);
      onsuccess(fetch, 0, value);
    };
    request.onerror = (error) => {
#if FETCH_DEBUG
      dbg(`fetch: Failed to delete file ${pathStr} from IndexedDB! error: ${error}`);
#endif
      {{{ makeSetValue('fetch', C_STRUCTS.emscripten_fetch_t.readyState, 4, 'i16') }}} // Mimic XHR readyState 4 === 'DONE: The operation is complete'
      {{{ makeSetValue('fetch', C_STRUCTS.emscripten_fetch_t.status, 404, 'i16') }}} // Mimic XHR HTTP status code 404 "Not Found"
      stringToUTF8("Not Found", fetch + {{{ C_STRUCTS.emscripten_fetch_t.statusText }}}, 64);
      onerror(fetch, 0, error);
    };
  } catch(e) {
#if FETCH_DEBUG
    dbg(`fetch: Failed to load file ${pathStr} from IndexedDB! Got exception ${e}`);
#endif
    onerror(fetch, 0, e);
  }
}

function fetchLoadCachedData(db, fetch, onsuccess, onerror) {
  if (!db) {
#if FETCH_DEBUG
    dbg('fetch: IndexedDB not available!');
#endif
    onerror(fetch, 0, 'IndexedDB not available!');
    return;
  }

  var fetch_attr = fetch + {{{ C_STRUCTS.emscripten_fetch_t.__attributes }}};
  var path = {{{ makeGetValue('fetch_attr', C_STRUCTS.emscripten_fetch_attr_t.destinationPath, '*') }}};
  path ||= {{{ makeGetValue('fetch', C_STRUCTS.emscripten_fetch_t.url, '*') }}};
  var pathStr = UTF8ToString(path);

  try {
    var transaction = db.transaction(['FILES'], 'readonly');
    var packages = transaction.objectStore('FILES');
    var getRequest = packages.get(pathStr);
    getRequest.onsuccess = (event) => {
      if (event.target.result) {
        var value = event.target.result;
        var len = value.byteLength || value.length;
#if FETCH_DEBUG
        dbg(`fetch: Loaded file ${pathStr} from IndexedDB, length: ${len}`);
#endif
        // The data pointer malloc()ed here has the same lifetime as the emscripten_fetch_t structure itself has, and is
        // freed when emscripten_fetch_close() is called.
        var ptr = _malloc(len);
        HEAPU8.set(new Uint8Array(value), ptr);
        {{{ makeSetValue('fetch', C_STRUCTS.emscripten_fetch_t.data, 'ptr', '*') }}};
        writeI53ToI64(fetch + {{{ C_STRUCTS.emscripten_fetch_t.numBytes }}}, len);
        writeI53ToI64(fetch + {{{ C_STRUCTS.emscripten_fetch_t.dataOffset }}}, 0);
        writeI53ToI64(fetch + {{{ C_STRUCTS.emscripten_fetch_t.totalBytes }}}, len);
        {{{ makeSetValue('fetch', C_STRUCTS.emscripten_fetch_t.readyState, 4, 'i16') }}} // Mimic XHR readyState 4 === 'DONE: The operation is complete'
        {{{ makeSetValue('fetch', C_STRUCTS.emscripten_fetch_t.status, 200, 'i16') }}} // Mimic XHR HTTP status code 200 "OK"
        stringToUTF8("OK", fetch + {{{ C_STRUCTS.emscripten_fetch_t.statusText }}}, 64);
        onsuccess(fetch, 0, value);
      } else {
        // Succeeded to load, but the load came back with the value of undefined, treat that as an error since we never store undefined in db.
#if FETCH_DEBUG
        dbg(`fetch: File ${pathStr} not found in IndexedDB`);
#endif
        {{{ makeSetValue('fetch', C_STRUCTS.emscripten_fetch_t.readyState, 4, 'i16') }}} // Mimic XHR readyState 4 === 'DONE: The operation is complete'
        {{{ makeSetValue('fetch', C_STRUCTS.emscripten_fetch_t.status, 404, 'i16') }}} // Mimic XHR HTTP status code 404 "Not Found"
        stringToUTF8("Not Found", fetch + {{{ C_STRUCTS.emscripten_fetch_t.statusText }}}, 64);
        onerror(fetch, 0, 'no data');
      }
    };
    getRequest.onerror = (error) => {
#if FETCH_DEBUG
      dbg(`fetch: Failed to load file ${pathStr} from IndexedDB!`);
#endif
      {{{ makeSetValue('fetch', C_STRUCTS.emscripten_fetch_t.readyState, 4, 'i16') }}} // Mimic XHR readyState 4 === 'DONE: The operation is complete'
      {{{ makeSetValue('fetch', C_STRUCTS.emscripten_fetch_t.status, 404, 'i16') }}} // Mimic XHR HTTP status code 404 "Not Found"
      stringToUTF8("Not Found", fetch + {{{ C_STRUCTS.emscripten_fetch_t.statusText }}}, 64);
      onerror(fetch, 0, error);
    };
  } catch(e) {
#if FETCH_DEBUG
    dbg(`fetch: Failed to load file ${pathStr} from IndexedDB! Got exception ${e}`);
#endif
    onerror(fetch, 0, e);
  }
}

function fetchCacheData(/** @type {IDBDatabase} */ db, fetch, data, onsuccess, onerror) {
  if (!db) {
#if FETCH_DEBUG
    dbg('fetch: IndexedDB not available!');
#endif
    onerror(fetch, 0, 'IndexedDB not available!');
    return;
  }

  var fetch_attr = fetch + {{{ C_STRUCTS.emscripten_fetch_t.__attributes }}};
  var destinationPath = {{{ makeGetValue('fetch_attr', C_STRUCTS.emscripten_fetch_attr_t.destinationPath, '*') }}};
  destinationPath ||= {{{ makeGetValue('fetch', C_STRUCTS.emscripten_fetch_t.url, '*') }}};
  var destinationPathStr = UTF8ToString(destinationPath);

  try {
    var transaction = db.transaction(['FILES'], 'readwrite');
    var packages = transaction.objectStore('FILES');
    var putRequest = packages.put(data, destinationPathStr);
    putRequest.onsuccess = (event) => {
#if FETCH_DEBUG
      dbg(`fetch: Stored file "${destinationPathStr}" to IndexedDB cache.`);
#endif
      {{{ makeSetValue('fetch', C_STRUCTS.emscripten_fetch_t.readyState, 4, 'i16') }}} // Mimic XHR readyState 4 === 'DONE: The operation is complete'
      {{{ makeSetValue('fetch', C_STRUCTS.emscripten_fetch_t.status, 200, 'i16') }}} // Mimic XHR HTTP status code 200 "OK"
      stringToUTF8("OK", fetch + {{{ C_STRUCTS.emscripten_fetch_t.statusText }}}, 64);
      onsuccess(fetch, 0, destinationPathStr);
    };
    putRequest.onerror = (error) => {
#if FETCH_DEBUG
      dbg(`fetch: Failed to store file "${destinationPathStr}" to IndexedDB cache!`);
#endif
      // Most likely we got an error if IndexedDB is unwilling to store any more data for this page.
      // TODO: Can we identify and break down different IndexedDB-provided errors and convert those
      // to more HTTP status codes for more information?
      {{{ makeSetValue('fetch', C_STRUCTS.emscripten_fetch_t.readyState, 4, 'i16') }}} // Mimic XHR readyState 4 === 'DONE: The operation is complete'
      {{{ makeSetValue('fetch', C_STRUCTS.emscripten_fetch_t.status, 413, 'i16') }}} // Mimic XHR HTTP status code 413 "Payload Too Large"
      stringToUTF8("Payload Too Large", fetch + {{{ C_STRUCTS.emscripten_fetch_t.statusText }}}, 64);
      onerror(fetch, 0, error);
    };
  } catch(e) {
#if FETCH_DEBUG
      dbg(`fetch: Failed to store file "${destinationPathStr}" to IndexedDB cache! Exception: ${e}`);
#endif
    onerror(fetch, 0, e);
  }
}
#endif // ~FETCH_SUPPORT_INDEXEDDB

function fetchXHR(fetch, onsuccess, onerror, onprogress, onreadystatechange) {
  var url = {{{ makeGetValue('fetch', C_STRUCTS.emscripten_fetch_t.url, '*') }}};
  if (!url) {
#if FETCH_DEBUG
    dbg('fetch: XHR failed, no URL specified!');
#endif
    onerror(fetch, 0, 'no url specified!');
    return;
  }
  var url_ = UTF8ToString(url);

  var fetch_attr = fetch + {{{ C_STRUCTS.emscripten_fetch_t.__attributes }}};
  var requestMethod = UTF8ToString(fetch_attr + {{{ C_STRUCTS.emscripten_fetch_attr_t.requestMethod }}});
  requestMethod ||= 'GET';
  var timeoutMsecs = {{{ makeGetValue('fetch_attr', C_STRUCTS.emscripten_fetch_attr_t.timeoutMSecs, 'u32') }}};
  var userName = {{{ makeGetValue('fetch_attr', C_STRUCTS.emscripten_fetch_attr_t.userName, '*') }}};
  var password = {{{ makeGetValue('fetch_attr', C_STRUCTS.emscripten_fetch_attr_t.password, '*') }}};
  var requestHeaders = {{{ makeGetValue('fetch_attr', C_STRUCTS.emscripten_fetch_attr_t.requestHeaders, '*') }}};
  var overriddenMimeType = {{{ makeGetValue('fetch_attr', C_STRUCTS.emscripten_fetch_attr_t.overriddenMimeType, '*') }}};
  var dataPtr = {{{ makeGetValue('fetch_attr', C_STRUCTS.emscripten_fetch_attr_t.requestData, '*') }}};
  var dataLength = {{{ makeGetValue('fetch_attr', C_STRUCTS.emscripten_fetch_attr_t.requestDataSize, '*') }}};

  var fetchAttributes = {{{ makeGetValue('fetch_attr', C_STRUCTS.emscripten_fetch_attr_t.attributes, 'u32') }}};
  var fetchAttrLoadToMemory = !!(fetchAttributes & {{{ cDefs.EMSCRIPTEN_FETCH_LOAD_TO_MEMORY }}});
  var fetchAttrStreamData = !!(fetchAttributes & {{{ cDefs.EMSCRIPTEN_FETCH_STREAM_DATA }}});
  var fetchAttrSynchronous = !!(fetchAttributes & {{{ cDefs.EMSCRIPTEN_FETCH_SYNCHRONOUS }}});

  var userNameStr = userName ? UTF8ToString(userName) : undefined;
  var passwordStr = password ? UTF8ToString(password) : undefined;

  var xhr = new XMLHttpRequest();
  xhr.withCredentials = !!{{{ makeGetValue('fetch_attr', C_STRUCTS.emscripten_fetch_attr_t.withCredentials, 'u8') }}};;
#if FETCH_DEBUG
  dbg(`fetch: xhr.timeout: ${xhr.timeout}, xhr.withCredentials: ${xhr.withCredentials}`);
  dbg(`fetch: xhr.open(requestMethod="${requestMethod}", url: "${url}", userName: ${userNameStr}, password: ${passwordStr}`);
#endif
  xhr.open(requestMethod, url_, !fetchAttrSynchronous, userNameStr, passwordStr);
  if (!fetchAttrSynchronous) xhr.timeout = timeoutMsecs; // XHR timeout field is only accessible in async XHRs, and must be set after .open() but before .send().
  xhr.url_ = url_; // Save the url for debugging purposes (and for comparing to the responseURL that server side advertised)
#if ASSERTIONS
  assert(!fetchAttrStreamData, 'streaming uses moz-chunked-arraybuffer which is no longer supported; TODO: rewrite using fetch()');
#endif
  xhr.responseType = 'arraybuffer';

  if (overriddenMimeType) {
    var overriddenMimeTypeStr = UTF8ToString(overriddenMimeType);
#if FETCH_DEBUG
    dbg(`fetch: xhr.overrideMimeType("${overriddenMimeTypeStr}");`);
#endif
    xhr.overrideMimeType(overriddenMimeTypeStr);
  }
  if (requestHeaders) {
    for (;;) {
      var key = {{{ makeGetValue('requestHeaders', 0, '*') }}};
      if (!key) break;
      var value = {{{ makeGetValue('requestHeaders', POINTER_SIZE, '*') }}};
      if (!value) break;
      requestHeaders += {{{ 2 * POINTER_SIZE }}};
      var keyStr = UTF8ToString(key);
      var valueStr = UTF8ToString(value);
#if FETCH_DEBUG
      dbg(`fetch: xhr.setRequestHeader("${keyStr}", "${valueStr}");`);
#endif
      xhr.setRequestHeader(keyStr, valueStr);
    }
  }

  var id = Fetch.xhrs.allocate(xhr);
#if FETCH_DEBUG
  dbg(`fetch: id=${id}`);
#endif
  {{{ makeSetValue('fetch', C_STRUCTS.emscripten_fetch_t.id, 'id', 'u32') }}};
  var data = (dataPtr && dataLength) ? HEAPU8.slice(dataPtr, dataPtr + dataLength) : null;
  // TODO: Support specifying custom headers to the request.

  // Share the code to save the response, as we need to do so both on success
  // and on error (despite an error, there may be a response, like a 404 page).
  // This receives a condition, which determines whether to save the xhr's
  // response, or just 0.
  function saveResponseAndStatus() {
    var ptr = 0;
    var ptrLen = 0;
    if (xhr.response && fetchAttrLoadToMemory && {{{ makeGetValue('fetch', C_STRUCTS.emscripten_fetch_t.data, '*') }}} === 0) {
      ptrLen = xhr.response.byteLength;
    }
    if (ptrLen > 0) {
#if FETCH_DEBUG
      dbg(`fetch: allocating ${ptrLen} bytes in Emscripten heap for xhr data`);
#endif
      // The data pointer malloc()ed here has the same lifetime as the emscripten_fetch_t structure itself has, and is
      // freed when emscripten_fetch_close() is called.
      ptr = _malloc(ptrLen);
      HEAPU8.set(new Uint8Array(/** @type{Array<number>} */(xhr.response)), ptr);
    }
    {{{ makeSetValue('fetch', C_STRUCTS.emscripten_fetch_t.data, 'ptr', '*') }}}
    writeI53ToI64(fetch + {{{ C_STRUCTS.emscripten_fetch_t.numBytes }}}, ptrLen);
    writeI53ToI64(fetch + {{{ C_STRUCTS.emscripten_fetch_t.dataOffset }}}, 0);
    var len = xhr.response ? xhr.response.byteLength : 0;
    if (len) {
      // If the final XHR.onload handler receives the bytedata to compute total length, report that,
      // otherwise don't write anything out here, which will retain the latest byte size reported in
      // the most recent XHR.onprogress handler.
      writeI53ToI64(fetch + {{{ C_STRUCTS.emscripten_fetch_t.totalBytes }}}, len);
    }
    {{{ makeSetValue('fetch', C_STRUCTS.emscripten_fetch_t.readyState, 'xhr.readyState', 'i16') }}}
    {{{ makeSetValue('fetch', C_STRUCTS.emscripten_fetch_t.status, 'xhr.status', 'i16') }}}
    if (xhr.statusText) stringToUTF8(xhr.statusText, fetch + {{{ C_STRUCTS.emscripten_fetch_t.statusText }}}, 64);
  }

  xhr.onload = (e) => {
    // check if xhr was aborted by user and don't try to call back
    if (!Fetch.xhrs.has(id)) {
      return;
    }
    saveResponseAndStatus();
    if (xhr.status >= 200 && xhr.status < 300) {
#if FETCH_DEBUG
      dbg(`fetch: xhr of URL "${xhr.url_}" / responseURL "${xhr.responseURL}" succeeded with status ${xhr.status}`);
#endif
      onsuccess?.(fetch, xhr, e);
    } else {
#if FETCH_DEBUG
      dbg(`fetch: xhr of URL "${xhr.url_}" / responseURL "${xhr.responseURL}" failed with status ${xhr.status}`);
#endif
      onerror?.(fetch, xhr, e);
    }
  };
  xhr.onerror = (e) => {
    // check if xhr was aborted by user and don't try to call back
    if (!Fetch.xhrs.has(id)) {
      return;
    }
#if FETCH_DEBUG
    dbg(`fetch: xhr of URL "${xhr.url_}" / responseURL "${xhr.responseURL}" finished with error, readyState ${xhr.readyState} and status ${xhr.status}`);
#endif
    saveResponseAndStatus();
    onerror?.(fetch, xhr, e);
  };
  xhr.ontimeout = (e) => {
    // check if xhr was aborted by user and don't try to call back
    if (!Fetch.xhrs.has(id)) {
      return;
    }
#if FETCH_DEBUG
    dbg(`fetch: xhr of URL "${xhr.url_}" / responseURL "${xhr.responseURL}" timed out, readyState ${xhr.readyState} and status ${xhr.status}`);
#endif
    onerror?.(fetch, xhr, e);
  };
  xhr.onprogress = (e) => {
    // check if xhr was aborted by user and don't try to call back
    if (!Fetch.xhrs.has(id)) {
      return;
    }
    var ptrLen = (fetchAttrLoadToMemory && fetchAttrStreamData && xhr.response) ? xhr.response.byteLength : 0;
    var ptr = 0;
    if (ptrLen > 0 && fetchAttrLoadToMemory && fetchAttrStreamData) {
#if FETCH_DEBUG
      dbg(`fetch: allocating ${ptrLen} bytes in Emscripten heap for xhr data`);
#endif
#if ASSERTIONS
      assert(onprogress, 'When doing a streaming fetch, you should have an onprogress handler registered to receive the chunks!');
#endif
      // Allocate byte data in Emscripten heap for the streamed memory block (freed immediately after onprogress call)
      ptr = _malloc(ptrLen);
      HEAPU8.set(new Uint8Array(/** @type{Array<number>} */(xhr.response)), ptr);
    }
    {{{ makeSetValue('fetch', C_STRUCTS.emscripten_fetch_t.data, 'ptr', '*') }}}
    writeI53ToI64(fetch + {{{ C_STRUCTS.emscripten_fetch_t.numBytes }}}, ptrLen);
    writeI53ToI64(fetch + {{{ C_STRUCTS.emscripten_fetch_t.dataOffset }}}, e.loaded - ptrLen);
    writeI53ToI64(fetch + {{{ C_STRUCTS.emscripten_fetch_t.totalBytes }}}, e.total);
    {{{ makeSetValue('fetch', C_STRUCTS.emscripten_fetch_t.readyState, 'xhr.readyState', 'i16') }}}
    // If loading files from a source that does not give HTTP status code, assume success if we get data bytes
    if (xhr.readyState >= 3 && xhr.status === 0 && e.loaded > 0) xhr.status = 200;
    {{{ makeSetValue('fetch', C_STRUCTS.emscripten_fetch_t.status, 'xhr.status', 'i16') }}}
    if (xhr.statusText) stringToUTF8(xhr.statusText, fetch + {{{ C_STRUCTS.emscripten_fetch_t.statusText }}}, 64);
    onprogress?.(fetch, xhr, e);
    if (ptr) {
      _free(ptr);
    }
  };
  xhr.onreadystatechange = (e) => {
    // check if xhr was aborted by user and don't try to call back
    if (!Fetch.xhrs.has(id)) {
      {{{ runtimeKeepalivePop() }}}
      return;
    }
    {{{ makeSetValue('fetch', C_STRUCTS.emscripten_fetch_t.readyState, 'xhr.readyState', 'i16') }}}
    if (xhr.readyState >= 2) {
      {{{ makeSetValue('fetch', C_STRUCTS.emscripten_fetch_t.status, 'xhr.status', 'i16') }}}
    }
    onreadystatechange?.(fetch, xhr, e);
  };
#if FETCH_DEBUG
  dbg(`fetch: xhr.send(data=${data})`);
#endif
  try {
    xhr.send(data);
  } catch(e) {
#if FETCH_DEBUG
    dbg(`fetch: xhr failed with exception: ${e}`);
#endif
    onerror?.(fetch, xhr, e);
  }
}

function startFetch(fetch, successcb, errorcb, progresscb, readystatechangecb) {
  // Avoid shutting down the runtime since we want to wait for the async
  // response.
  {{{ runtimeKeepalivePush() }}}

  var fetch_attr = fetch + {{{ C_STRUCTS.emscripten_fetch_t.__attributes }}};
  var onsuccess = {{{ makeGetValue('fetch_attr', C_STRUCTS.emscripten_fetch_attr_t.onsuccess, '*') }}};
  var onerror = {{{ makeGetValue('fetch_attr', C_STRUCTS.emscripten_fetch_attr_t.onerror, '*') }}};
  var onprogress = {{{ makeGetValue('fetch_attr', C_STRUCTS.emscripten_fetch_attr_t.onprogress, '*') }}};
  var onreadystatechange = {{{ makeGetValue('fetch_attr', C_STRUCTS.emscripten_fetch_attr_t.onreadystatechange, '*') }}};
  var fetchAttributes = {{{ makeGetValue('fetch_attr', C_STRUCTS.emscripten_fetch_attr_t.attributes, '*') }}};
  var fetchAttrSynchronous = !!(fetchAttributes & {{{ cDefs.EMSCRIPTEN_FETCH_SYNCHRONOUS }}});

  function doCallback(f) {
    if (fetchAttrSynchronous) {
      f();
    } else {
      callUserCallback(f);
    }
  }

  var reportSuccess = (fetch, xhr, e) => {
#if FETCH_DEBUG
    dbg(`fetch: operation success. e: ${e}`);
#endif
    {{{ runtimeKeepalivePop() }}}
    doCallback(() => {
      if (onsuccess) {{{ makeDynCall('vp', 'onsuccess') }}}(fetch);
      else successcb?.(fetch);
    });
  };

  var reportProgress = (fetch, xhr, e) => {
    doCallback(() => {
      if (onprogress) {{{ makeDynCall('vp', 'onprogress') }}}(fetch);
      else progresscb?.(fetch);
    });
  };

  var reportError = (fetch, xhr, e) => {
#if FETCH_DEBUG
    dbg(`fetch: operation failed: ${e}`);
#endif
    {{{ runtimeKeepalivePop() }}}
    doCallback(() => {
      if (onerror) {{{ makeDynCall('vp', 'onerror') }}}(fetch);
      else errorcb?.(fetch);
    });
  };

  var reportReadyStateChange = (fetch, xhr, e) => {
#if FETCH_DEBUG
    dbg(`fetch: ready state change. e: ${e}`);
#endif
    doCallback(() => {
      if (onreadystatechange) {{{ makeDynCall('vp', 'onreadystatechange') }}}(fetch);
      else readystatechangecb?.(fetch);
    });
  };

  var performUncachedXhr = (fetch, xhr, e) => {
#if FETCH_DEBUG
    dbg(`fetch: starting (uncached) XHR: ${e}`);
#endif
    fetchXHR(fetch, reportSuccess, reportError, reportProgress, reportReadyStateChange);
  };

#if FETCH_SUPPORT_INDEXEDDB
  var cacheResultAndReportSuccess = (fetch, xhr, e) => {
#if FETCH_DEBUG
    dbg(`fetch: operation success. Caching result.. e: ${e}`);
#endif
    var storeSuccess = (fetch, xhr, e) => {
#if FETCH_DEBUG
      dbg('fetch: IndexedDB store succeeded.');
#endif
      {{{ runtimeKeepalivePop() }}}
      doCallback(() => {
        if (onsuccess) {{{ makeDynCall('vp', 'onsuccess') }}}(fetch);
        else successcb?.(fetch);
      });
    };
    var storeError = (fetch, xhr, e) => {
#if FETCH_DEBUG
      dbg('fetch: IndexedDB store failed.');
#endif
      {{{ runtimeKeepalivePop() }}}
      doCallback(() => {
        if (onsuccess) {{{ makeDynCall('vp', 'onsuccess') }}}(fetch);
        else successcb?.(fetch);
      });
    };
    fetchCacheData(Fetch.dbInstance, fetch, xhr.response, storeSuccess, storeError);
  };

  var performCachedXhr = (fetch, xhr, e) => {
#if FETCH_DEBUG
    dbg(`fetch: starting (cached) XHR: ${e}`);
#endif
    fetchXHR(fetch, cacheResultAndReportSuccess, reportError, reportProgress, reportReadyStateChange);
  };

  var requestMethod = UTF8ToString(fetch_attr + {{{ C_STRUCTS.emscripten_fetch_attr_t.requestMethod }}});
  var fetchAttrReplace = !!(fetchAttributes & {{{ cDefs.EMSCRIPTEN_FETCH_REPLACE }}});
  var fetchAttrPersistFile = !!(fetchAttributes & {{{ cDefs.EMSCRIPTEN_FETCH_PERSIST_FILE }}});
  var fetchAttrNoDownload = !!(fetchAttributes & {{{ cDefs.EMSCRIPTEN_FETCH_NO_DOWNLOAD }}});
  if (requestMethod === 'EM_IDB_STORE') {
    // TODO(?): Here we perform a clone of the data, because storing shared typed arrays to IndexedDB does not seem to be allowed.
    var ptr = {{{ makeGetValue('fetch_attr', C_STRUCTS.emscripten_fetch_attr_t.requestData, '*') }}};
    var size = {{{ makeGetValue('fetch_attr', C_STRUCTS.emscripten_fetch_attr_t.requestDataSize, '*') }}};
    fetchCacheData(Fetch.dbInstance, fetch, HEAPU8.slice(ptr, ptr + size), reportSuccess, reportError);
  } else if (requestMethod === 'EM_IDB_DELETE') {
    fetchDeleteCachedData(Fetch.dbInstance, fetch, reportSuccess, reportError);
  } else if (!fetchAttrReplace) {
    fetchLoadCachedData(Fetch.dbInstance, fetch, reportSuccess, fetchAttrNoDownload ? reportError : (fetchAttrPersistFile ? performCachedXhr : performUncachedXhr));
  } else if (!fetchAttrNoDownload) {
    fetchXHR(fetch, fetchAttrPersistFile ? cacheResultAndReportSuccess : reportSuccess, reportError, reportProgress, reportReadyStateChange);
  } else {
#if FETCH_DEBUG
    dbg('fetch: Invalid combination of flags passed.');
#endif
    return 0; // todo: free
  }
  return fetch;
#else // !FETCH_SUPPORT_INDEXEDDB
  fetchXHR(fetch, reportSuccess, reportError, reportProgress, reportReadyStateChange);
  return fetch;
#endif // ~FETCH_SUPPORT_INDEXEDDB
}

function fetchGetResponseHeadersLength(id) {
  return lengthBytesUTF8(Fetch.xhrs.get(id).getAllResponseHeaders()) + 1;
}

function fetchGetResponseHeaders(id, dst, dstSizeBytes) {
  var responseHeaders = Fetch.xhrs.get(id).getAllResponseHeaders();
  var lengthBytes = lengthBytesUTF8(responseHeaders) + 1;
  stringToUTF8(responseHeaders, dst, dstSizeBytes);
  return Math.min(lengthBytes, dstSizeBytes);
}

//Delete the xhr JS object, allowing it to be garbage collected.
function fetchFree(id) {
#if FETCH_DEBUG
  dbg(`fetch: fetchFree id:${id}`);
#endif
  if (Fetch.xhrs.has(id)) {
    var xhr = Fetch.xhrs.get(id);
    Fetch.xhrs.free(id);
    // check if fetch is still in progress and should be aborted
    if (xhr.readyState > 0 && xhr.readyState < 4) {
      xhr.abort();
    }
  }
}
