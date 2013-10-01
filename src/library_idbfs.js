mergeInto(LibraryManager.library, {
  $IDBFS__deps: ['$FS', '$MEMFS', '$PATH'],
  $IDBFS: {
    dbs: {},
    indexedDB: function() {
      return window.indexedDB || window.mozIndexedDB || window.webkitIndexedDB || window.msIndexedDB;
    },
    DB_VERSION: 20,
    DB_STORE_NAME: 'FILE_DATA',
    // reuse all of the core MEMFS functionality
    mount: function(mount) {
      return MEMFS.mount.apply(null, arguments);
    },
    // the only custom function IDBFS implements is to handle
    // synchronizing the wrapped MEMFS with a backing IDB instance
    syncfs: function(mount, populate, callback) {
      IDBFS.getLocalSet(mount, function(err, local) {
        if (err) return callback(err);

        IDBFS.getRemoteSet(mount, function(err, remote) {
          if (err) return callback(err);

          var src = populate ? remote : local;
          var dst = populate ? local : remote;

          IDBFS.reconcile(src, dst, callback);
        });
      });
    },
    reconcile: function(src, dst, callback) {
      var total = 0;

      var create = {};
      for (var key in src.files) {
        if (!src.files.hasOwnProperty(key)) continue;
        var e = src.files[key];
        var e2 = dst.files[key];
        if (!e2 || e.timestamp > e2.timestamp) {
          create[key] = e;
          total++;
        }
      }

      var remove = {};
      for (var key in dst.files) {
        if (!dst.files.hasOwnProperty(key)) continue;
        var e = dst.files[key];
        var e2 = src.files[key];
        if (!e2) {
          remove[key] = e;
          total++;
        }
      }

      if (!total) {
        // early out
        return callback(null);
      }

      var completed = 0;
      var done = function(err) {
        if (err) return callback(err);
        if (++completed >= total) {
          return callback(null);
        }
      };

      // create a single transaction to handle and IDB reads / writes we'll need to do
      var db = src.type === 'remote' ? src.db : dst.db;
      var transaction = db.transaction([IDBFS.DB_STORE_NAME], 'readwrite');
      transaction.onerror = function() { callback(this.error); };
      var store = transaction.objectStore(IDBFS.DB_STORE_NAME);

      for (var path in create) {
        if (!create.hasOwnProperty(path)) continue;
        var entry = create[path];

        if (dst.type === 'local') {
          // save file to local
          try {
            if (FS.isDir(entry.mode)) {
              FS.mkdir(path, entry.mode);
            } else if (FS.isFile(entry.mode)) {
              var stream = FS.open(path, 'w+', 0666);
              FS.write(stream, entry.contents, 0, entry.contents.length, 0, true /* canOwn */);
              FS.close(stream);
            }
            done(null);
          } catch (e) {
            return done(e);
          }
        } else {
          // save file to IDB
          var req = store.put(entry, path);
          req.onsuccess = function() { done(null); };
          req.onerror = function() { done(this.error); };
        }
      }

      for (var path in remove) {
        if (!remove.hasOwnProperty(path)) continue;
        var entry = remove[path];

        if (dst.type === 'local') {
          // delete file from local
          try {
            if (FS.isDir(entry.mode)) {
              // TODO recursive delete?
              FS.rmdir(path);
            } else if (FS.isFile(entry.mode)) {
              FS.unlink(path);
            }
            done(null);
          } catch (e) {
            return done(e);
          }
        } else {
          // delete file from IDB
          var req = store.delete(path);
          req.onsuccess = function() { done(null); };
          req.onerror = function() { done(this.error); };
        }
      }
    },
    getLocalSet: function(mount, callback) {
      var files = {};

      var isRealDir = function(p) {
        return p !== '.' && p !== '..';
      };
      var toAbsolute = function(root) {
        return function(p) {
          return PATH.join(root, p);
        }
      };

      var check = FS.readdir(mount.mountpoint)
        .filter(isRealDir)
        .map(toAbsolute(mount.mountpoint));

      while (check.length) {
        var path = check.pop();
        var stat, node;

        try {
          var lookup = FS.lookupPath(path);
          node = lookup.node;
          stat = FS.stat(path);
        } catch (e) {
          return callback(e);
        }

        if (FS.isDir(stat.mode)) {
          check.push.apply(check, FS.readdir(path)
            .filter(isRealDir)
            .map(toAbsolute(path)));

          files[path] = { mode: stat.mode, timestamp: stat.mtime };
        } else if (FS.isFile(stat.mode)) {
          files[path] = { contents: node.contents, mode: stat.mode, timestamp: stat.mtime };
        } else {
          return callback(new Error('node type not supported'));
        }
      }

      return callback(null, { type: 'local', files: files });
    },
    getDB: function(name, callback) {
      // look it up in the cache
      var db = IDBFS.dbs[name];
      if (db) {
        return callback(null, db);
      }
      var req;
      try {
        req = IDBFS.indexedDB().open(name, IDBFS.DB_VERSION);
      } catch (e) {
        return onerror(e);
      }
      req.onupgradeneeded = function() {
        db = req.result;
        db.createObjectStore(IDBFS.DB_STORE_NAME);
      };
      req.onsuccess = function() {
        db = req.result;
        // add to the cache
        IDBFS.dbs[name] = db;
        callback(null, db);
      };
      req.onerror = function() {
        callback(this.error);
      };
    },
    getRemoteSet: function(mount, callback) {
      var files = {};

      IDBFS.getDB(mount.mountpoint, function(err, db) {
        if (err) return callback(err);

        var transaction = db.transaction([IDBFS.DB_STORE_NAME], 'readonly');
        transaction.onerror = function() { callback(this.error); };

        var store = transaction.objectStore(IDBFS.DB_STORE_NAME);
        store.openCursor().onsuccess = function(event) {
          var cursor = event.target.result;
          if (!cursor) {
            return callback(null, { type: 'remote', db: db, files: files });
          }

          files[cursor.key] = cursor.value;
          cursor.continue();
        };
      });
    }
  }
});
