{
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
  blobs: [0],
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
      if (error) return callback(error);
      var transaction = db.transaction([IDBStore.DB_STORE_NAME], type);
      transaction.onerror = function(e) {
        callback(this.error || 'unknown error');
        e.preventDefault();
      };
      var store = transaction.objectStore(IDBStore.DB_STORE_NAME);
      callback(null, store);
    });
  },
  // External API
  getFile: function(dbName, id, callback) {
    IDBStore.getStore(dbName, 'readonly', function(err, store) {
      if (err) return callback(err);
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
      if (err) return callback(err);
      var req = store.put(data, id);
      req.onsuccess = function(event) {
        callback();
      };
      req.onerror = function(error) {
        callback(error);
      };
    });
  },
  deleteFile: function(dbName, id, callback) {
    IDBStore.getStore(dbName, 'readwrite', function(err, store) {
      if (err) return callback(err);
      var req = store.delete(id);
      req.onsuccess = function(event) {
        callback();
      };
      req.onerror = function(error) {
        callback(error);
      };
    });
  },
  existsFile: function(dbName, id, callback) {
    IDBStore.getStore(dbName, 'readonly', function(err, store) {
      if (err) return callback(err);
      var req = store.count(id);
      req.onsuccess = function(event) {
        callback(null, event.target.result > 0);
      };
      req.onerror = function(error) {
        callback(error);
      };
    });
  },
}

