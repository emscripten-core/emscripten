/**
 * @license
 * Copyright 2015 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

#include "IDBStore.js"

var LibraryIDBStore = {
  // A simple IDB-backed storage mechanism. Suitable for saving and loading
  // large files asynchronously. This does *NOT* use the emscripten filesystem,
  // intentionally, to avoid overhead. It lets you application define whatever
  // filesystem-like layer you want, with the overhead 100% controlled by you.
  // At the extremes, you could either just store large files, with almost no
  // extra code; or you could implement a file b-tree using posix-compliant
  // filesystem on top.
  $IDBStore: IDBStore,
  emscripten_idb_async_load__deps: ['$UTF8ToString', '$callUserCallback', 'malloc', 'free'],
  emscripten_idb_async_load: (db, id, arg, onload, onerror) => {
    {{{ runtimeKeepalivePush() }}};
    IDBStore.getFile(UTF8ToString(db), UTF8ToString(id), (error, byteArray) => {
      {{{ runtimeKeepalivePop() }}}
      callUserCallback(() => {
        if (error) {
          if (onerror) {{{ makeDynCall('vp', 'onerror') }}}(arg);
          return;
        }
        var buffer = _malloc(byteArray.length);
        HEAPU8.set(byteArray, buffer);
        {{{ makeDynCall('vppi', 'onload') }}}(arg, buffer, byteArray.length);
        _free(buffer);
      });
    });
  },
  emscripten_idb_async_store__deps: ['$UTF8ToString', 'free', '$callUserCallback'],
  emscripten_idb_async_store: (db, id, ptr, num, arg, onstore, onerror) => {
    // note that we copy the data here, as these are async operatins - changes
    // to HEAPU8 meanwhile should not affect us!
    {{{ runtimeKeepalivePush() }}};
    IDBStore.setFile(UTF8ToString(db), UTF8ToString(id), new Uint8Array(HEAPU8.subarray(ptr, ptr+num)), (error) => {
      {{{ runtimeKeepalivePop() }}}
      callUserCallback(() => {
        if (error) {
          if (onerror) {{{ makeDynCall('vp', 'onerror') }}}(arg);
          return;
        }
        if (onstore) {{{ makeDynCall('vp', 'onstore') }}}(arg);
      });
    });
  },
  emscripten_idb_async_delete__deps: ['$UTF8ToString', '$callUserCallback'],
  emscripten_idb_async_delete: (db, id, arg, ondelete, onerror) => {
    {{{ runtimeKeepalivePush() }}};
    IDBStore.deleteFile(UTF8ToString(db), UTF8ToString(id), (error) => {
      {{{ runtimeKeepalivePop() }}}
      callUserCallback(() => {
        if (error) {
          if (onerror) {{{ makeDynCall('vp', 'onerror') }}}(arg);
          return;
        }
        if (ondelete) {{{ makeDynCall('vp', 'ondelete') }}}(arg);
      });
    });
  },
  emscripten_idb_async_exists__deps: ['$UTF8ToString', '$callUserCallback'],
  emscripten_idb_async_exists: (db, id, arg, oncheck, onerror) => {
    {{{ runtimeKeepalivePush() }}};
    IDBStore.existsFile(UTF8ToString(db), UTF8ToString(id), (error, exists) => {
      {{{ runtimeKeepalivePop() }}}
      callUserCallback(() => {
        if (error) {
          if (onerror) {{{ makeDynCall('vp', 'onerror') }}}(arg);
          return;
        }
        if (oncheck) {{{ makeDynCall('vpi', 'oncheck') }}}(arg, exists);
      });
    });
  },
  emscripten_idb_async_clear__deps: ['$UTF8ToString', '$callUserCallback'],
  emscripten_idb_async_clear: (db, arg, onclear, onerror) => {
    {{{ runtimeKeepalivePush() }}};
    IDBStore.clearStore(UTF8ToString(db), (error) => {
      {{{ runtimeKeepalivePop() }}}
      callUserCallback(() => {
        if (error) {
          if (onerror) {{{ makeDynCall('vp', 'onerror') }}}(arg);
          return;
        }
        if (onclear) {{{ makeDynCall('vp', 'onclear') }}}(arg);
      });
    });
  },

#if ASYNCIFY
  emscripten_idb_load__async: true,
  emscripten_idb_load__deps: ['malloc'],
  emscripten_idb_load: (db, id, pbuffer, pnum, perror) => Asyncify.handleSleep((wakeUp) => {
    IDBStore.getFile(UTF8ToString(db), UTF8ToString(id), (error, byteArray) => {
      if (error) {
        {{{ makeSetValue('perror', 0, '1', 'i32') }}};
        wakeUp();
        return;
      }
      var buffer = _malloc(byteArray.length); // must be freed by the caller!
      HEAPU8.set(byteArray, buffer);
      {{{ makeSetValue('pbuffer', 0, 'buffer', '*') }}};
      {{{ makeSetValue('pnum',    0, 'byteArray.length', 'i32') }}};
      {{{ makeSetValue('perror',  0, '0', 'i32') }}};
      wakeUp();
    });
  }),
  emscripten_idb_store__async: true,
  emscripten_idb_store: (db, id, ptr, num, perror) => Asyncify.handleSleep((wakeUp) => {
    IDBStore.setFile(UTF8ToString(db), UTF8ToString(id), new Uint8Array(HEAPU8.subarray(ptr, ptr+num)), (error) => {
      {{{ makeSetValue('perror', 0, '!!error', 'i32') }}};
      wakeUp();
    });
  }),
  emscripten_idb_delete__async: true,
  emscripten_idb_delete: (db, id, perror) => Asyncify.handleSleep((wakeUp) => {
    IDBStore.deleteFile(UTF8ToString(db), UTF8ToString(id), (error) => {
      {{{ makeSetValue('perror', 0, '!!error', 'i32') }}};
      wakeUp();
    });
  }),
  emscripten_idb_exists__async: true,
  emscripten_idb_exists: (db, id, pexists, perror) => Asyncify.handleSleep((wakeUp) => {
    IDBStore.existsFile(UTF8ToString(db), UTF8ToString(id), (error, exists) => {
      {{{ makeSetValue('pexists', 0, '!!exists', 'i32') }}};
      {{{ makeSetValue('perror',  0, '!!error', 'i32') }}};
      wakeUp();
    });
  }),
  emscripten_idb_clear__async: true,
  emscripten_idb_clear: (db, perror) => Asyncify.handleSleep((wakeUp) => {
    IDBStore.clearStore(UTF8ToString(db), (error) => {
      {{{ makeSetValue('perror', 0, '!!error', 'i32') }}};
      wakeUp();
    });
  }),
  // extra worker methods - proxied
  emscripten_idb_load_blob__async: true,
  emscripten_idb_load_blob: (db, id, pblob, perror) => Asyncify.handleSleep((wakeUp) => {
    assert(!IDBStore.pending);
    IDBStore.pending = (msg) => {
      IDBStore.pending = null;
      var blob = msg.blob;
      if (!blob) {
        {{{ makeSetValue('perror', 0, '1', 'i32') }}};
        wakeUp();
        return;
      }
      assert(blob instanceof Blob);
      var blobId = IDBStore.blobs.length;
      IDBStore.blobs.push(blob);
      {{{ makeSetValue('pblob', 0, 'blobId', 'i32') }}};
      wakeUp();
    };
    postMessage({
      target: 'IDBStore',
      method: 'loadBlob',
      db: UTF8ToString(db),
      id: UTF8ToString(id)
    });
  }),
  emscripten_idb_store_blob__async: true,
  emscripten_idb_store_blob: (db, id, ptr, num, perror) => Asyncify.handleSleep((wakeUp) => {
    assert(!IDBStore.pending);
    IDBStore.pending = (msg) => {
      IDBStore.pending = null;
      {{{ makeSetValue('perror', 0, '!!msg.error', 'i32') }}};
      wakeUp();
    };
    postMessage({
      target: 'IDBStore',
      method: 'storeBlob',
      db: UTF8ToString(db),
      id: UTF8ToString(id),
      blob: new Blob([new Uint8Array(HEAPU8.subarray(ptr, ptr+num))])
    });
  }),
  emscripten_idb_read_from_blob: (blobId, start, num, buffer) => {
    var blob = IDBStore.blobs[blobId];
    if (!blob) return 1;
    if (start+num > blob.size) return 2;
    var byteArray = (new FileReaderSync()).readAsArrayBuffer(blob.slice(start, start+num));
    HEAPU8.set(new Uint8Array(byteArray), buffer);
    return 0;
  },
  emscripten_idb_free_blob: (blobId) => {
    assert(IDBStore.blobs[blobId]);
    IDBStore.blobs[blobId] = null;
  },
#else
  emscripten_idb_load: (db, id, pbuffer, pnum, perror) => {
    throw 'Please compile your program with async support in order to use synchronous operations like emscripten_idb_load, etc.';
  },
  emscripten_idb_store: (db, id, ptr, num, perror) => {
    throw 'Please compile your program with async support in order to use synchronous operations like emscripten_idb_store, etc.';
  },
  emscripten_idb_delete: (db, id, perror) => {
    throw 'Please compile your program with async support in order to use synchronous operations like emscripten_idb_delete, etc.';
  },
  emscripten_idb_exists: (db, id, pexists, perror) => {
    throw 'Please compile your program with async support in order to use synchronous operations like emscripten_idb_exists, etc.';
  },
  emscripten_idb_clear: (db, perror) => {
    throw 'Please compile your program with async support in order to use synchronous operations like emscripten_idb_clear, etc.';
  },
#endif // ASYNCIFY
};

autoAddDeps(LibraryIDBStore, '$IDBStore');
addToLibrary(LibraryIDBStore);
