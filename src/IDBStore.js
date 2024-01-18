/**
 * @license
 * Copyright 2015 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

var IDBStore = {
  indexedDB() {
    if (typeof indexedDB != 'undefined') return indexedDB;
    var ret = null;
    if (typeof window == 'object') ret = window.indexedDB || window.mozIndexedDB || window.webkitIndexedDB || window.msIndexedDB;
    assert(ret, 'IDBStore used, but indexedDB not supported');
    return ret;
  },
  DB_VERSION: 22,
  DB_STORE_NAME: 'FILE_DATA',
  dbs: {},
  blobs: [0],
  getDB(name, callback) {
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
    req.onupgradeneeded = (e) => {
      var db = /** @type {IDBDatabase} */ (e.target.result);
      var transaction = e.target.transaction;
      var fileStore;
      if (db.objectStoreNames.contains(IDBStore.DB_STORE_NAME)) {
        fileStore = transaction.objectStore(IDBStore.DB_STORE_NAME);
      } else {
        fileStore = db.createObjectStore(IDBStore.DB_STORE_NAME);
      }
    };
    req.onsuccess = () => {
      db = /** @type {IDBDatabase} */ (req.result);
      // add to the cache
      IDBStore.dbs[name] = db;
      callback(null, db);
    };
    req.onerror = function(event) {
      callback(event.target.error || 'unknown error');
      event.preventDefault();
    };
  },
  getStore(dbName, type, callback) {
    IDBStore.getDB(dbName, (error, db) => {
      if (error) return callback(error);
      var transaction = db.transaction([IDBStore.DB_STORE_NAME], type);
      transaction.onerror = (event) => {
        callback(event.target.error || 'unknown error');
        event.preventDefault();
      };
      var store = transaction.objectStore(IDBStore.DB_STORE_NAME);
      callback(null, store);
    });
  },
  // External API
  getFile(dbName, id, callback) {
    IDBStore.getStore(dbName, 'readonly', (err, store) => {
      if (err) return callback(err);
      var req = store.get(id);
      req.onsuccess = (event) => {
        var result = event.target.result;
        if (!result) {
          return callback(`file ${id} not found`);
        }
        return callback(null, result);
      };
      req.onerror = callback;
    });
  },
  setFile(dbName, id, data, callback) {
    IDBStore.getStore(dbName, 'readwrite', (err, store) => {
      if (err) return callback(err);
      var req = store.put(data, id);
      req.onsuccess = (event) => callback();
      req.onerror = callback;
    });
  },
  deleteFile(dbName, id, callback) {
    IDBStore.getStore(dbName, 'readwrite', (err, store) => {
      if (err) return callback(err);
      var req = store.delete(id);
      req.onsuccess = (event) => callback();
      req.onerror = callback;
    });
  },
  existsFile(dbName, id, callback) {
    IDBStore.getStore(dbName, 'readonly', (err, store) => {
      if (err) return callback(err);
      var req = store.count(id);
      req.onsuccess = (event) => callback(null, event.target.result > 0);
      req.onerror = callback;
    });
  },
  clearStore(dbName, callback) {
    IDBStore.getStore(dbName, 'readwrite', (err, store) => {
      if (err) return callback(err);
      var req = store.clear();
      req.onsuccess = (event) => callback();
      req.onerror = callback;
    });
  },
};
