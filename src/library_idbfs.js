/**
 * @license
 * Copyright 2013 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

mergeInto(LibraryManager.library, {
  $IDBFS__deps: ['$FS', '$MEMFS', '$PATH'],
  $IDBFS__postset: function() {
    addAtExit('IDBFS.quit();');
    return '';
  },
  $IDBFS: {
    dbs: {},
    indexedDB: () => {
      if (typeof indexedDB != 'undefined') return indexedDB;
      var ret = null;
      if (typeof window == 'object') ret = window.indexedDB || window.mozIndexedDB || window.webkitIndexedDB || window.msIndexedDB;
      assert(ret, 'IDBFS used, but indexedDB not supported');
      return ret;
    },
    DB_VERSION: 21,
    DB_STORE_NAME: 'FILE_DATA',
    mount: function(mount) {
      // reuse all of the core MEMFS functionality
      return MEMFS.mount.apply(null, arguments);
    },
    syncfs: (mount, populate, callback) => {
      IDBFS.getLocalSet(mount, (err, local) => {
        if (err) return callback(err);

        IDBFS.getRemoteSet(mount, (err, remote) => {
          if (err) return callback(err);

          var src = populate ? remote : local;
          var dst = populate ? local : remote;

          IDBFS.reconcile(src, dst, callback);
        });
      });
    },
    quit: () => {
      Object.values(IDBFS.dbs).forEach((value) => value.close());
      IDBFS.dbs = {};
    },
    getDB: (name, callback) => {
      // check the cache first
      var db = IDBFS.dbs[name];
      if (db) {
        return callback(null, db);
      }

      var req;
      try {
        req = IDBFS.indexedDB().open(name, IDBFS.DB_VERSION);
      } catch (e) {
        return callback(e);
      }
      if (!req) {
        return callback("Unable to connect to IndexedDB");
      }
      req.onupgradeneeded = (e) => {
        var db = /** @type {IDBDatabase} */ (e.target.result);
        var transaction = e.target.transaction;

        var fileStore;

        if (db.objectStoreNames.contains(IDBFS.DB_STORE_NAME)) {
          fileStore = transaction.objectStore(IDBFS.DB_STORE_NAME);
        } else {
          fileStore = db.createObjectStore(IDBFS.DB_STORE_NAME);
        }

        if (!fileStore.indexNames.contains('timestamp')) {
          fileStore.createIndex('timestamp', 'timestamp', { unique: false });
        }
      };
      req.onsuccess = () => {
        db = /** @type {IDBDatabase} */ (req.result);

        // add to the cache
        IDBFS.dbs[name] = db;
        callback(null, db);
      };
      req.onerror = (e) => {
        callback(e.target.error);
        e.preventDefault();
      };
    },
    getLocalSet: (mount, callback) => {
      var entries = {};

      function isRealDir(p) {
        return p !== '.' && p !== '..';
      };
      function toAbsolute(root) {
        return (p) => {
          return PATH.join2(root, p);
        }
      };

      var check = FS.readdir(mount.mountpoint).filter(isRealDir).map(toAbsolute(mount.mountpoint));

      while (check.length) {
        var path = check.pop();
        var stat;

        try {
          stat = FS.stat(path);
        } catch (e) {
          return callback(e);
        }

        if (FS.isDir(stat.mode)) {
          check.push.apply(check, FS.readdir(path).filter(isRealDir).map(toAbsolute(path)));
        }

        entries[path] = { 'timestamp': stat.mtime };
      }

      return callback(null, { type: 'local', entries: entries });
    },
    getRemoteSet: (mount, callback) => {
      var entries = {};

      IDBFS.getDB(mount.mountpoint, (err, db) => {
        if (err) return callback(err);

        try {
          var transaction = db.transaction([IDBFS.DB_STORE_NAME], 'readonly');
          transaction.onerror = (e) => {
            callback(e.target.error);
            e.preventDefault();
          };

          var store = transaction.objectStore(IDBFS.DB_STORE_NAME);
          var index = store.index('timestamp');

          index.openKeyCursor().onsuccess = (event) => {
            var cursor = event.target.result;

            if (!cursor) {
              return callback(null, { type: 'remote', db, entries });
            }

            entries[cursor.primaryKey] = { 'timestamp': cursor.key };

            cursor.continue();
          };
        } catch (e) {
          return callback(e);
        }
      });
    },
    loadLocalEntry: (path, callback) => {
      var stat, node;

      try {
        var lookup = FS.lookupPath(path);
        node = lookup.node;
        stat = FS.stat(path);
      } catch (e) {
        return callback(e);
      }

      if (FS.isDir(stat.mode)) {
        return callback(null, { 'timestamp': stat.mtime, 'mode': stat.mode });
      } else if (FS.isFile(stat.mode)) {
        // Performance consideration: storing a normal JavaScript array to a IndexedDB is much slower than storing a typed array.
        // Therefore always convert the file contents to a typed array first before writing the data to IndexedDB.
        node.contents = MEMFS.getFileDataAsTypedArray(node);
        return callback(null, { 'timestamp': stat.mtime, 'mode': stat.mode, 'contents': node.contents });
      } else {
        return callback(new Error('node type not supported'));
      }
    },
    storeLocalEntry: (path, entry, callback) => {
      try {
        if (FS.isDir(entry['mode'])) {
          FS.mkdirTree(path, entry['mode']);
        } else if (FS.isFile(entry['mode'])) {
          FS.writeFile(path, entry['contents'], { canOwn: true });
        } else {
          return callback(new Error('node type not supported'));
        }

        FS.chmod(path, entry['mode']);
        FS.utime(path, entry['timestamp'], entry['timestamp']);
      } catch (e) {
        return callback(e);
      }

      callback(null);
    },
    removeLocalEntry: (path, callback) => {
      try {
        var stat = FS.stat(path);

        if (FS.isDir(stat.mode)) {
          FS.rmdir(path);
        } else if (FS.isFile(stat.mode)) {
          FS.unlink(path);
        }
      } catch (e) {
        return callback(e);
      }

      callback(null);
    },
    loadRemoteEntry: (store, path, callback) => {
      var req = store.get(path);
      req.onsuccess = (event) => { callback(null, event.target.result); };
      req.onerror = (e) => {
        callback(e.target.error);
        e.preventDefault();
      };
    },
    storeRemoteEntry: (store, path, entry, callback) => {
      try {
        var req = store.put(entry, path);
      } catch (e) {
        callback(e);
        return;
      }
      req.onsuccess = () => { callback(null); };
      req.onerror = (e) => {
        callback(e.target.error);
        e.preventDefault();
      };
    },
    removeRemoteEntry: (store, path, callback) => {
      var req = store.delete(path);
      req.onsuccess = () => { callback(null); };
      req.onerror = (e) => {
        callback(e.target.error);
        e.preventDefault();
      };
    },
    reconcile: (src, dst, callback) => {
      var total = 0;

      var create = [];
      Object.keys(src.entries).forEach(function (key) {
        var e = src.entries[key];
        var e2 = dst.entries[key];
        if (!e2 || e['timestamp'].getTime() != e2['timestamp'].getTime()) {
          create.push(key);
          total++;
        }
      });

      var remove = [];
      Object.keys(dst.entries).forEach(function (key) {
        if (!src.entries[key]) {
          remove.push(key);
          total++;
        }
      });

      if (!total) {
        return callback(null);
      }

      var errored = false;
      var db = src.type === 'remote' ? src.db : dst.db;
      var transaction = db.transaction([IDBFS.DB_STORE_NAME], 'readwrite');
      var store = transaction.objectStore(IDBFS.DB_STORE_NAME);

      function done(err) {
        if (err && !errored) {
          errored = true;
          return callback(err);
        }
      };

      transaction.onerror = (e) => {
        done(this.error);
        e.preventDefault();
      };

      transaction.oncomplete = (e) => {
        if (!errored) {
          callback(null);
        }
      };

      // sort paths in ascending order so directory entries are created
      // before the files inside them
      create.sort().forEach((path) => {
        if (dst.type === 'local') {
          IDBFS.loadRemoteEntry(store, path, (err, entry) => {
            if (err) return done(err);
            IDBFS.storeLocalEntry(path, entry, done);
          });
        } else {
          IDBFS.loadLocalEntry(path, (err, entry) => {
            if (err) return done(err);
            IDBFS.storeRemoteEntry(store, path, entry, done);
          });
        }
      });

      // sort paths in descending order so files are deleted before their
      // parent directories
      remove.sort().reverse().forEach((path) => {
        if (dst.type === 'local') {
          IDBFS.removeLocalEntry(path, done);
        } else {
          IDBFS.removeRemoteEntry(store, path, done);
        }
      });
    }
  }
});
