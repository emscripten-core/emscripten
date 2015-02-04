var LibraryIDBStore = {
  // A simple IDB-backed storage mechanism. Suitable for saving and loading large files asynchronously. This does
  // *NOT* use the emscripten filesystem, intentionally, to avoid overhead. It lets you application define whatever
  // filesystem-like layer you want, with the overhead 100% controlled by you. At the extremes, you could either
  // just store large files, with almost no extra code; or you could implement a file b-tree using posix-compliant
  // filesystem on top.
  $IDBStore: {
    indexedDB: function() {
      if (typeof indexedDB !== 'undefined') return indexedDB;
      var ret = null;
      if (typeof window === 'object') ret = window.indexedDB || window.mozIndexedDB || window.webkitIndexedDB || window.msIndexedDB;
      assert(ret, 'IDBStore used, but indexedDB not supported');
      return ret;
    },
    DB_VERSION: 22,
    DB_STORE_NAME: 'FILE_DATA',
    dbs: {},
    getDB: function(name, callback) {
      // check the cache first
      var db = IDBStore.dbs[name];
      if (db) {
        return callback(null, db);
      }
      var req;
      try {
        req = IDBStore.indexedDB().open(name, IDBStore.DB_VERSION);
      } catch (e) {
        return callback(e);
      }
      req.onupgradeneeded = function(e) {
        var db = e.target.result;
        var transaction = e.target.transaction;
        var fileStore;
        if (db.objectStoreNames.contains(IDBStore.DB_STORE_NAME)) {
          fileStore = transaction.objectStore(IDBStore.DB_STORE_NAME);
        } else {
          fileStore = db.createObjectStore(IDBStore.DB_STORE_NAME);
        }
      };
      req.onsuccess = function() {
        db = req.result;
        // add to the cache
        IDBStore.dbs[name] = db;
        callback(null, db);
      };
      req.onerror = function(e) {
        callback(this.error);
        e.preventDefault();
      };
    },
    getStore: function(dbName, type, callback) {
      IDBStore.getDB(dbName, function(error, db) {
        var transaction = db.transaction([IDBStore.DB_STORE_NAME], type);
        transaction.onerror = function(e) {
          callback(this.error);
          e.preventDefault();
        };
        var store = transaction.objectStore(IDBStore.DB_STORE_NAME);
        callback(null, store);
      });
    },
    // External API
    getFile: function(dbName, id, callback) {
      IDBStore.getStore(dbName, 'readonly', function(err, store) {
        var req = store.get(id);
        req.onsuccess = function(event) {
          var result = event.target.result;
          if (!result) {
            return callback('file ' + id + ' not found');
          } else {
            return callback(null, result);
          }
        };
        req.onerror = function(error) {
          callback(error);
        };
      });
    },
    setFile: function(dbName, id, data, callback) {
      IDBStore.getStore(dbName, 'readwrite', function(err, store) {
        var req = store.put(data, id);
        req.onsuccess = function(event) {
          callback();
        };
        req.onerror = function(error) {
          errback(error);
        };
      });
    },
    deleteFile: function(dbName, id, callback) {
      IDBStore.getStore(dbName, 'readwrite', function(err, store) {
        var req = store.delete(id);
        req.onsuccess = function(event) {
          callback();
        };
        req.onerror = function(error) {
          errback(error);
        };
      });
    },
  },

  emscripten_idb_async_load: function(db, id, arg, onload, onerror) {
    IDBStore.getFile(Pointer_stringify(db), Pointer_stringify(id), function(error, byteArray) {
      if (error) {
        if (onerror) Runtime.dynCall('vi', onerror, [arg]);
        return;
      }
      var buffer = _malloc(byteArray.length);
      HEAPU8.set(byteArray, buffer);
      Runtime.dynCall('viii', onload, [arg, buffer, byteArray.length]);
      _free(buffer);
    });
  },
  emscripten_idb_async_store: function(db, id, ptr, num, arg, onstore, onerror) {
    // note that we copy the data here, as these are async operatins - changes to HEAPU8 meanwhile should not affect us!
    IDBStore.setFile(Pointer_stringify(db), Pointer_stringify(id), new Uint8Array(HEAPU8.subarray(ptr, ptr+num)), function(error) {
      if (error) {
        if (onerror) Runtime.dynCall('vi', onerror, [arg]);
        return;
      }
      if (onstore) Runtime.dynCall('vi', onstore, [arg]);
    });
  },
  emscripten_idb_async_delete: function(db, id, arg, ondelete, onerror) {
    IDBStore.deleteFile(Pointer_stringify(db), Pointer_stringify(id), function(error) {
      if (error) {
        if (onerror) Runtime.dynCall('vi', onerror, [arg]);
        return;
      }
      if (ondelete) Runtime.dynCall('vi', ondelete, [arg]);
    });
  },
};

autoAddDeps(LibraryIDBStore, '$IDBStore');
mergeInto(LibraryManager.library, LibraryIDBStore);

