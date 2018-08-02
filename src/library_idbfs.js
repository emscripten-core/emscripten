mergeInto(LibraryManager.library, {
  $IDBFS__deps: ['$FS', '$MEMFS', '$PATH'],
  $IDBFS: {
    dbs: {},
    indexedDB: function() {
      if (typeof indexedDB !== 'undefined') return indexedDB;
      var ret = null;
      if (typeof window === 'object') ret = window.indexedDB || window.mozIndexedDB || window.webkitIndexedDB || window.msIndexedDB;
      assert(ret, 'IDBFS used, but indexedDB not supported');
      return ret;
    },
    DB_VERSION: 21,
    DB_STORE_NAME: 'FILE_DATA',
	DB_MAXRECORDSIZE: 133000000,
	DB_PARTSSUFFIX:'.idbfsp.',
	
    mount: function(mount) {
      // reuse all of the core MEMFS functionality
      return MEMFS.mount.apply(null, arguments);
    },
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
    getDB: function(name, callback) {
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
      req.onupgradeneeded = function(e) {
        var db = e.target.result;
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
      req.onsuccess = function() {
        db = req.result;

        // add to the cache
        IDBFS.dbs[name] = db;
        callback(null, db);
      };
      req.onerror = function(e) {
        callback(this.error);
        e.preventDefault();
      };
    },
    getLocalSet: function(mount, callback) {
      var entries = {};

      function isRealDir(p) {
        return p !== '.' && p !== '..';
      };
      function toAbsolute(root) {
        return function(p) {
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

        entries[path] = { timestamp: stat.mtime };
      }

      return callback(null, { type: 'local', entries: entries });
    },
    getRemoteSet: function(mount, callback) {
      var entries = {};

      IDBFS.getDB(mount.mountpoint, function(err, db) {
        if (err) return callback(err);

        try {
          var transaction = db.transaction([IDBFS.DB_STORE_NAME], 'readonly');
          transaction.onerror = function(e) {
            callback(this.error);
            e.preventDefault();
          };

          var store = transaction.objectStore(IDBFS.DB_STORE_NAME);
          var index = store.index('timestamp');

          index.openKeyCursor().onsuccess = function(event) {
            var cursor = event.target.result;

            if (!cursor) {
              return callback(null, { type: 'remote', db: db, entries: entries });
            }
			
			var regex = new RegExp( IDBFS.DB_PARTSSUFFIX+'(\\d)(\\d)');
			var match=cursor.primaryKey.match(regex);
			if (match)
			{
				if (match[1]==='0' && match[2]==='0')
				{
					entries[cursor.primaryKey.substring(0, cursor.primaryKey.length-(IDBFS.DB_PARTSSUFFIX.length+2))] = { timestamp: cursor.key , nbParts:match[2]};
				}
			}
			else
			{
				entries[cursor.primaryKey] = { timestamp: cursor.key };
			}

            cursor.continue();
          };
        } catch (e) {
          return callback(e);
        }
      });
    },
    loadLocalEntry: function(path, callback) {
      var stat, node;

      try {
        var lookup = FS.lookupPath(path);
        node = lookup.node;
        stat = FS.stat(path);
      } catch (e) {
        return callback(e);
      }

      if (FS.isDir(stat.mode)) {
        return callback(null, { timestamp: stat.mtime, mode: stat.mode });
      } else if (FS.isFile(stat.mode)) {
        // Performance consideration: storing a normal JavaScript array to a IndexedDB is much slower than storing a typed array.
        // Therefore always convert the file contents to a typed array first before writing the data to IndexedDB.
        node.contents = MEMFS.getFileDataAsTypedArray(node);
        return callback(null, { timestamp: stat.mtime, mode: stat.mode, contents: node.contents });
      } else {
        return callback(new Error('node type not supported'));
      }
    },
    storeLocalEntry: function(path, entry, callback) {
      try {
        if (FS.isDir(entry.mode)) {
          FS.mkdir(path, entry.mode);
        } else if (FS.isFile(entry.mode)) {
          FS.writeFile(path, entry.contents, { canOwn: true });
        } else {
          return callback(new Error('node type not supported'));
        }

        FS.chmod(path, entry.mode);
        FS.utime(path, entry.timestamp, entry.timestamp);
      } catch (e) {
        return callback(e);
      }

      callback(null);
    },
    removeLocalEntry: function(path, callback) {
      try {
        var lookup = FS.lookupPath(path);
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
    loadRemoteEntry: function(store, path, callback) {
		function loadRemoteMulti(store, path, contents,index,total,callback)
		{
			var req = store.get(path+IDBFS.DB_PARTSSUFFIX+index+total);
			req.onsuccess = function(event) 
			{
				if (total===0)
				{
					total=event.target.result.contents[0];
					loadRemoteMulti(store, path, contents,1,total,callback);
				}
				else if (event && event.target && event.target.result && event.target.result.contents)
				{
					var cl=new event.target.result.contents.constructor(contents.length+event.target.result.contents.length);
					cl.set(contents,0);
					cl.set(event.target.result.contents,contents.length);
					contents=cl;
					if (index<total)
					{
						loadRemoteMulti(store, path, contents,index+1,total,callback);
					}
					else
					{
						event.target.result.contents=contents;
						event.target.result.nbParts=total;
						callback(null,event.target.result);
					}
				}
				else if (event && event.target)
				{
					callback(null,event.target.result);
				}
				else
				{
					callback(null,undefined);
				}
			};
			req.onerror = function(e) {
				callback(this.error);
				e.preventDefault();
			};
		}
		var req = store.get(path);
		req.onsuccess = function(event) 
		{
			if (typeof event.target.result === 'undefined' )
			{
				var contents=[];
				loadRemoteMulti(store, path, contents,0,0,callback);
			}
			else
			{
				callback(null, event.target.result); 
			}
		};
		req.onerror = function(e) {
			callback(this.error);
			e.preventDefault();
		};
    },
    storeRemoteEntry: function(store, path, entry, callback) 
	{
		function storeRemoteEntryMulti(store, path, entry,contents, index,total, callback) 
		{
			if (index>0)
			{
				entry.contents=contents.slice((index-1)*IDBFS.DB_MAXRECORDSIZE,index*IDBFS.DB_MAXRECORDSIZE);
			}
			else
			{
				entry.contents=contents;
			}
			var req = store.put(entry, path+IDBFS.DB_PARTSSUFFIX+index+total);
			req.onsuccess = function() 
			{
				if (index>0 && index<total)
				{
					storeRemoteEntryMulti(store, path, entry,contents,index+1,total, callback);
				}
				else
				{
					callback(null);
				}
			};
			req.onerror = function(e) 
			{
				callback(this.error);
				e.preventDefault();
			};
		}
		
		var clength=(entry && entry.contents)?entry.contents.length:0;
		var nbParts=Math.ceil(clength/IDBFS.DB_MAXRECORDSIZE);
		var req;
		if (nbParts>1 && nbParts<10)
		{
			var contents=entry.contents;
			entry.contents=new entry.contents.constructor(1);
			entry.contents[0]=nbParts;
			req = store.put(entry, path+IDBFS.DB_PARTSSUFFIX+'00');
			req.onsuccess = function() 
			{
				storeRemoteEntryMulti(store, path, entry,contents,1,nbParts, callback);
			};
		}
		else
		{
			req = store.put(entry, path);
			req.onsuccess = function() 
			{
				callback(null);
			};
		}
		req.onerror = function(e) 
		{
			callback(this.error);
			e.preventDefault();
		};
    },
    removeRemoteEntry: function(store, path, callback) {
		function remoteDelete(store,path,index,total,callback)
		{
			var addstr=(index>=0?IDBFS.DB_PARTSSUFFIX+index+total:'');
			  var req = store.delete(path+addstr);
			  req.onsuccess = function() 
			  { 
				if (total>0 && index>0)
				{
					remoteDelete(store,path,index-1,total,callback);
				}
				else if (total>0 && index===0 )
				{
					remoteDelete(store,path,0,0,callback);
				}
				else
				{
					callback(null); 
				}
			  };
			  req.onerror = function(e) {
				callback(this.error);
				e.preventDefault();
			  };
		}
/*		function remoteExists(store,path,cbk)
		{
          var index = store.index('timestamp');
          index.openKeyCursor().onsuccess = function(event) 
		  {
            var cursor = event.target.result;

            if (!cursor) {
              return cbk(false);
            }
			if (cursor.primaryKey===path)
			{
				return cbk(true);
			}
            cursor.continue();
          };
		}*/
		//IF EXISTS path+'p00'
		// get nbParts
		var regex = new RegExp( IDBFS.DB_PARTSSUFFIX+'(\\d)(\\d)');
		if (path.match(regex))
		{
			path=path.substring(path.length-(IDBFS.DB_PARTSSUFFIX.length+2));
		}
		var ereq = store.get(path+IDBFS.DB_PARTSSUFFIX+'00');
		ereq.onsuccess = function(event) 
		{
			  if (event.target.result)
			  {
				  var total=event.target.result.contents[0];
				  remoteDelete(store,path,total,total,callback);
			  }
			  else
			  {
				  remoteDelete(store,path,-1,0,callback);
			  }
		};
		ereq.onerror = function(e) {
        callback(this.error);
        e.preventDefault();
       };
    },
    reconcile: function(src, dst, callback) {
      var total = 0;

      var create = [];
      Object.keys(src.entries).forEach(function (key) {
        var e = src.entries[key];
        var e2 = dst.entries[key];
        if (!e2 || e.timestamp > e2.timestamp) {
          create.push(key);
          total++;
        }
      });

      var remove = [];
      Object.keys(dst.entries).forEach(function (key) {
        var e = dst.entries[key];
        var e2 = src.entries[key];
        if (!e2) {
          remove.push(key);
          total++;
        }
      });

      if (!total) {
        return callback(null);
      }

      var errored = false;
      var completed = 0;
      var db = src.type === 'remote' ? src.db : dst.db;
      var transaction = db.transaction([IDBFS.DB_STORE_NAME], 'readwrite');
      var store = transaction.objectStore(IDBFS.DB_STORE_NAME);

      function done(err) {
        if (err) {
          if (!done.errored) {
            done.errored = true;
            return callback(err);
          }
          return;
        }
        if (++completed >= total) {
          return callback(null);
        }
      };

      transaction.onerror = function(e) {
        done(this.error);
        e.preventDefault();
      };

      // sort paths in ascending order so directory entries are created
      // before the files inside them
      create.sort().forEach(function (path) {
        if (dst.type === 'local') {
          IDBFS.loadRemoteEntry(store, path, function (err, entry) {
            if (err) return done(err);
					IDBFS.storeLocalEntry(path, entry, done);
		  }	);
        } else {
          IDBFS.loadLocalEntry(path, function (err, entry) {
            if (err) return done(err);
				IDBFS.storeRemoteEntry(store, path, entry, done);
          });
        }
      });

      // sort paths in descending order so files are deleted before their
      // parent directories
      remove.sort().reverse().forEach(function(path) {
        if (dst.type === 'local') {
          IDBFS.removeLocalEntry(path, done);
        } else {
          IDBFS.removeRemoteEntry(store, path, done);
        }
      });
    }
  }
});
