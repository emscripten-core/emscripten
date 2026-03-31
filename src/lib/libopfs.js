/**
 * @license
 * Copyright 2023 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

if (WASMFS) {
  addToLibrary({
    $OPFS__deps: ['wasmfs_create_opfs_backend'],
    $OPFS: {
      createBackend(opts) {
        return _wasmfs_create_opfs_backend();
      }
    },
  });
} else {
  addToLibrary({
    $OPFS__deps: ['$FS', '$MEMFS', '$PATH'],
    $OPFS__postset: () => {
      addAtExit('OPFS.quit();');
      return '';
    },
    $OPFS: {
      requestId: 0,
      worker: null,
      syncfsQueue: new Map(),
      getWorker: () => {
        if (OPFS.worker) {
          return OPFS.worker;
        }

        const opfsSyncWorkerSource = `
const OPFS_TIMESTAMPS_FILE = '.emscripten-opfs-stats';
const textDecoder = new TextDecoder();
const textEncoder = new TextEncoder();
const rootDirectoryCache = new Map();

function splitPath(path) {
  if (!path) return [];
  return path.split('/').filter((p) => p.length > 0);
}

async function openSyncAccessHandleCompat(fileHandle) {
  // Keep compatibility with old and new Safari signatures.
  const len = FileSystemFileHandle.prototype.createSyncAccessHandle.length;
  if (len == 0) {
    return await fileHandle.createSyncAccessHandle();
  }
  try {
    return await fileHandle.createSyncAccessHandle({ mode: 'in-place' });
  } catch (e) {
    if (e instanceof TypeError) {
      return await fileHandle.createSyncAccessHandle();
    }
    throw e;
  }
}

async function getRootDirectory(root) {
  const key = root || '';
  const cached = rootDirectoryCache.get(key);
  if (cached) return cached;

  let dir = await navigator.storage.getDirectory();
  for (const part of splitPath(key)) {
    dir = await dir.getDirectoryHandle(part, { create: true });

  }

  let meta;
  let accessHandle;
  try {
    const fileHandle = await dir.getFileHandle(OPFS_TIMESTAMPS_FILE, { create: false });
    accessHandle = await openSyncAccessHandleCompat(fileHandle);
    const fileSize = accessHandle.getSize();
    if (fileSize > 0) {
      const buffer = new Uint8Array(fileSize);
      accessHandle.read(buffer, { at: 0 });
      meta = JSON.parse(textDecoder.decode(buffer));
    }
  } catch (e) {
    // ignore
  } finally {
    if (accessHandle) {
      accessHandle.close();
    }
  }

  if (!meta || !meta.nodes) {
    meta = { nodes: {} };
  }

  rootDirectoryCache.set(key, {
    dir,
    meta,
  });

  return rootDirectoryCache.get(key);
}

async function updateMeta(root, anyPath, timestamp, mode, isDir, length) {
  const path = anyPath.startsWith("/") ? anyPath.substring(1) : anyPath;
  const cached = rootDirectoryCache.get(root);
  if (!cached) {
    throw new Error('root "' + root + '" directory not found');
  }
  cached.meta.nodes[path] = cached.meta.nodes[path] || {};
  cached.meta.nodes[path].t = timestamp.getTime();
  cached.meta.nodes[path].m = mode;
  cached.meta.nodes[path].d = isDir;
  cached.meta.nodes[path].l = length;
}

async function getMeta(root, anyPath) {
  const path = anyPath.startsWith("/") ? anyPath.substring(1) : anyPath;
  const cached = rootDirectoryCache.get(root);
  if (!cached) {
    throw new Error('root "' + root + '" directory not found');
  }
  const meta = cached.meta.nodes[path];
  return meta ? { timestamp: new Date(meta.t), mode: meta.m, isDir: meta.d, length: meta.l } : null;
}

async function removeMeta(root, anyPath) {
  const path = anyPath.startsWith("/") ? anyPath.substring(1) : anyPath;
  const cached = rootDirectoryCache.get(root);
  if (!cached) {
    throw new Error('root "' + root + '" directory not found');
  }
  delete cached.meta.nodes[path];
}

async function flushMeta(root) {
  const cached = rootDirectoryCache.get(root);
  if (!cached) {
    throw new Error('root "' + root + '" directory not found');
  }
  if (Object.keys(cached.meta.nodes).length === 0) {
    try {
      await cached.dir.removeEntry(OPFS_TIMESTAMPS_FILE);
    } catch(e) {
      // ignore 
    }
  } else {
    const fileHandle = await cached.dir.getFileHandle(OPFS_TIMESTAMPS_FILE, { create: true });
    const accessHandle = await openSyncAccessHandleCompat(fileHandle);
    accessHandle.truncate(0);
    accessHandle.write(textEncoder.encode(JSON.stringify(cached.meta)));
    accessHandle.flush();
    accessHandle.close();
  }
}

async function getDirectory(root, path, create) {
  const rootDir = (await getRootDirectory(root)).dir;
  const parts = typeof path === 'string' ? splitPath(path) : path;
  let dir = rootDir;
  for (let i = 0; i < parts.length; i++) {
    dir = await dir.getDirectoryHandle(parts[i], { create });
  }
  return dir;
}

async function getParentDirectory(root, path, create) {
  const parts = splitPath(path)
  const dir = await getDirectory(root, parts.slice(0, -1), create);
  return { dir, name: parts[parts.length - 1] };
}

async function openHandle(root, path, create) {
  const { dir, name } = await getParentDirectory(root, path, create);
  const fileHandle = await dir.getFileHandle(name, { create });
  return await openSyncAccessHandleCompat(fileHandle);
}

function toUint8Array(contents, length) {
  if (contents instanceof ArrayBuffer) {
    contents = new Uint8Array(contents);
  } else if (ArrayBuffer.isView(contents)) {
    contents = new Uint8Array(contents.buffer, contents.byteOffset, contents.byteLength);
  }

  if (length === undefined) return contents;
  return contents.subarray(0, Math.min(length, contents.length));
}

function postSuccess(type, requestId, result, transferable) {
  self.postMessage({ type, requestId, ok: true, result }, transferable);
}

function postError(type, requestId, error) {
  self.postMessage({
    type,
    requestId,
    ok: false,
    error: {
      name: error?.name || 'Error',
      message: error?.message || String(error),
    },
  });
}

const onmessage = async (event) => {
  const { type, payload = {}, requestId } = event.data || {};
  try {
    switch (type) {
      case 'write': {
        const { root, path, offset = 0, contents = null, timestamp, mode, writeBeyondEnd = false } = payload;
        if (contents) { // file
          const accessHandle = await openHandle(root, path, true);
          const data = toUint8Array(contents);
          const written = accessHandle.write(data, { at: offset });
          await updateMeta(root, path, timestamp, mode, false, accessHandle.getSize());
          accessHandle.close();
          postSuccess(type, requestId, { written });
        } else {
          await getDirectory(root, path, true);
          await updateMeta(root, path, timestamp, mode, true, 0);
          postSuccess(type, requestId, { written: 0 });
        }
        break;
      }
      case 'read': {
        let length = payload.length;
        const { root, path, offset = 0 } = payload;
        const { timestamp, mode } = await getMeta(root, path);
        const accessHandle = await openHandle(root, path, false);
        const size = accessHandle.getSize();
        if (!length) {
          length = size;
        }
        if (offset + length > size) {
          length = size - offset;
        }
        const contents = new Uint8Array(length);
        const read = accessHandle.read(contents, { at: offset });
        accessHandle.close();
        postSuccess(type, requestId, {
          contents,
          read,
          timestamp,
          mode,
        }, [contents.buffer]);
        break;
      }
      case 'unlink': {
        const { root, path } = payload;
        await removeMeta(root, path);
        const { dir, name } = await getParentDirectory(root, path, false);
        await dir.removeEntry(name);
        postSuccess(type, requestId, { deleted: true });
        break;
      }
      case 'list': {
        const { root } = payload;
        const { dir: rootDir, meta } = await getRootDirectory(root);
        const entries = {};

        for (const [path, node] of Object.entries(meta.nodes)) {
          if (node.d) {
            entries[root + '/' + path] = { timestamp: new Date(node.t), mode: node.m, isDir: node.d, length: 0 };
          }
        }

        async function walk(dir, prefix) {
          for await (const [name, handle] of dir.entries()) {
            const path = prefix ? prefix + '/' + name : name;
            if (handle.kind === 'directory') {
              await walk(handle, path);
            } else if (handle.kind === 'file') {
              const meta = await getMeta(root, path);
              if (meta) {
                entries[root + '/' + path] = meta;
              }
            }
          }
        }

        await walk(rootDir, '');
        postSuccess(type, requestId, entries);
        break;
      }
      case 'flush': {
        const { root } = payload;
        await flushMeta(root);
        postSuccess(type, requestId, { flushed: true });
        break;
      }
      default:
        throw new Error('unknown OPFS worker message type: ' + type);
    }
  } catch (error) {
    postError(type, requestId, error);
  }
};
self.onmessage = (event) => onmessage(event).catch(console.error);
        `;
        OPFS.opfsSyncWorkerUrl = URL.createObjectURL(new Blob([opfsSyncWorkerSource], { type: 'text/javascript' }));
        OPFS.worker = new Worker(OPFS.opfsSyncWorkerUrl);
        return OPFS.worker;
      },

      callWorker: (type, payload, callback) => {
        const worker = OPFS.getWorker();
        if (!worker) {
          return callback(new Error('OPFS worker not initialized'));
        }

        const selfId = OPFS.requestId++;
        const handler = (event) => {
          const { type, requestId, ok, result, error } = event.data;
          if (requestId === selfId) {
            worker.removeEventListener('message', handler);
            if (ok) {
              callback(null, result);
            } else {
              callback(new Error(error.message));
            }
          }
        };
        worker.addEventListener('message', handler);
        worker.postMessage({ type, payload, requestId: selfId });
      },

      // Queues a new VFS -> OPFS synchronization operation
      queuePersist: (mount) => {
        function onPersistComplete() {
          if (mount.opfsPersistState === 'again') startPersist(); // If a new sync request has appeared in between, kick off a new sync
          else mount.opfsPersistState = 0; // Otherwise reset sync state back to idle to wait for a new sync later
        }
        function startPersist() {
          mount.opfsPersistState = 'opfs'; // Mark that we are currently running a sync operation
          OPFS.syncfs(mount, /*populate:*/false, onPersistComplete);
        }

        if (!mount.opfsPersistState) {
          // Programs typically write/copy/move multiple files in the in-memory
          // filesystem within a single app frame, so when a filesystem sync
          // command is triggered, do not start it immediately, but only after
          // the current frame is finished. This way all the modified files
          // inside the main loop tick will be batched up to the same sync.
          mount.opfsPersistState = setTimeout(startPersist, 0);
        } else if (mount.opfsPersistState === 'opfs') {
          // There is an active OPFS sync operation in-flight, but we now
          // have accumulated more files to sync. We should therefore queue up
          // a new sync after the current one finishes so that all writes
          // will be properly persisted.
          mount.opfsPersistState = 'again';
        }
      },

      mount: (mount) => {
        // reuse core MEMFS functionality
        var mnt = MEMFS.mount(mount);
        // If the automatic OPFS persistence option has been selected, then automatically persist
        // all modifications to the filesystem as they occur.
        if (mount?.opts?.autoPersist) {
          mount.opfsPersistState = 0; // IndexedDB sync starts in idle state
          var memfs_node_ops = mnt.node_ops;
          mnt.node_ops = { ...mnt.node_ops }; // Clone node_ops to inject write tracking
          mnt.node_ops.mknod = (parent, name, mode, dev) => {
            var node = memfs_node_ops.mknod(parent, name, mode, dev);
            // Propagate injected node_ops to the newly created child node
            node.node_ops = mnt.node_ops;
            // Remember for each OPFS node which OPFS mount point they came from so we know which mount to persist on modification.
            node.opfs_mount = mnt.mount;
            // Remember original MEMFS stream_ops for this node
            node.memfs_stream_ops = node.stream_ops;
            // Clone stream_ops to inject write tracking
            node.stream_ops = { ...node.stream_ops };

            // Track all file writes
            node.stream_ops.write = (stream, buffer, offset, length, position, canOwn) => {
              // This file has been modified, we must persist IndexedDB when this file closes
              stream.node.isModified = true;
              return node.memfs_stream_ops.write(stream, buffer, offset, length, position, canOwn);
            };

            // Persist OPFS on file close
            node.stream_ops.close = (stream) => {
              var n = stream.node;
              if (n.isModified) {
                OPFS.queuePersist(n.opfs_mount);
                n.isModified = false;
              }
              if (n.memfs_stream_ops.close) return n.memfs_stream_ops.close(stream);
            };

            // Persist the node we just created to OPFS
            OPFS.queuePersist(mnt.mount);

            return node;
          };
          // Also kick off persisting the filesystem on other operations that modify the filesystem.
          mnt.node_ops.rmdir = (...args) => (OPFS.queuePersist(mnt.mount), memfs_node_ops.rmdir(...args));
          mnt.node_ops.symlink = (...args) => (OPFS.queuePersist(mnt.mount), memfs_node_ops.symlink(...args));
          mnt.node_ops.unlink = (...args) => (OPFS.queuePersist(mnt.mount), memfs_node_ops.unlink(...args));
          mnt.node_ops.rename = (...args) => (OPFS.queuePersist(mnt.mount), memfs_node_ops.rename(...args));
        }

        OPFS.syncfsQueue.set(mount.mountpoint, { active: false, delayed: [] });
        return mnt;
      },

      syncfs: (mount, populate, callback) => {
        const queue = OPFS.syncfsQueue.get(mount.mountpoint);
        if (queue.active) {
          queue.delayed.push({ populate, callback });
          return;
        }

        queue.active = true;
        OPFS.getLocalSet(mount, (err, local) => {
          if (err) return callback(err);

          OPFS.getRemoteSet(mount, (err, remote) => {
            if (err) return callback(err);

            var src = populate ? remote : local;
            var dst = populate ? local : remote;

            OPFS.reconcile(mount, src, dst, (err) => {
              queue.active = false;
              callback(err);
              if (!queue.active && queue.delayed.length > 0) {
                const { populate, callback } = queue.delayed.shift();
                OPFS.syncfs(mount, populate, callback);
              }
            });
          });
        });
      },
      getLocalSet: (mount, callback) => {
        var entries = {};

        function isRealDir(p) {
          return p !== '.' && p !== '..';
        };
        function toAbsolute(root) {
          return (p) => PATH.join2(root, p);
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
            check.push(...FS.readdir(path).filter(isRealDir).map(toAbsolute(path)));
          }

          entries[path] = { 'timestamp': stat.mtime };
        }

        return callback(null, { type: 'local', entries: entries });
      },
      getRemoteSet: (mount, callback) => {
        OPFS.callWorker('list', { root: mount.mountpoint }, (err, entries) => {
          callback(err, { type: 'remote', entries });
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
      loadRemoteEntry: (mount, path, callback) => {
        if (path.startsWith(mount.mountpoint)) {
          OPFS.callWorker('read', {
            root: mount.mountpoint,
            path: path.substring(mount.mountpoint.length),
          }, callback);
        } else {
          callback(new Error('path is not in the mount point'));
        }
      },
      storeRemoteEntry: (mount, path, entry, callback) => {
        if (path.startsWith(mount.mountpoint)) {
          OPFS.callWorker('write', {
            root: mount.mountpoint,
            path: path.substring(mount.mountpoint.length),
            contents: entry.contents,
            timestamp: entry.timestamp,
            mode: entry.mode,
          }, callback);
        } else {
          callback(new Error('path is not in the mount point'));
        }
      },
      removeRemoteEntry: (mount, path, callback) => {
        if (path.startsWith(mount.mountpoint)) {
          OPFS.callWorker('unlink', {
            root: mount.mountpoint,
            path: path.substring(mount.mountpoint.length),
          }, callback);
        } else {
          callback(new Error('path is not in the mount point'));
        }
      },
      reconcile: (mount, src, dst, callback) => {
        var total = 0;

        var create = [];
        for (var [key, e] of Object.entries(src.entries)) {
          var e2 = dst.entries[key];
          if (!e2 || e['timestamp'].getTime() != e2['timestamp'].getTime()) {
            create.push(key);
            total++;
          }
        }

        var remove = [];
        for (var key of Object.keys(dst.entries)) {
          if (!src.entries[key]) {
            remove.push(key);
            total++;
          }
        }

        if (!total) {
          return callback(null);
        }

        const promise = (async () => {
          // sort paths in ascending order so directory entries are created
          // before the files inside them
          for (const path of create.sort()) {
            await(new Promise((resolve, reject) => {
              if (dst.type === 'local') {
                const entry = src.entries[path];
                if (entry.isDir) {
                  OPFS.storeLocalEntry(path, entry, (e) => e ? reject(e) : resolve());
                } else {
                  OPFS.loadRemoteEntry(mount, path, (err, entry) => {
                    if (err) return reject(err);
                    OPFS.storeLocalEntry(path, entry, (e) => e ? reject(e) : resolve());
                  });
                }
              } else {
                OPFS.loadLocalEntry(path, (err, entry) => {
                  if (err) return reject(err);
                  OPFS.storeRemoteEntry(mount, path, entry, (e) => e ? reject(e) : resolve());
                });
              }
            }));
          }

          // sort paths in descending order so files are deleted before their
          // parent directories
          for (var path of remove.sort().reverse()) {
            await(new Promise((resolve, reject) => {
              if (dst.type === 'local') {
                OPFS.removeLocalEntry(path, (e) => e ? reject(e) : resolve());
              } else {
                OPFS.removeRemoteEntry(mount, path, (e) => e ? reject(e) : resolve());
              }
            }));
          }

          await (new Promise((resolve, reject) => {
            OPFS.callWorker('flush', { root: mount.mountpoint }, (e) => e ? reject(e) : resolve());
          }));
        })();
        promise.then(callback);
        promise.catch(callback);
      },
      quit: () => {
        if (OPFS.worker) {
          OPFS.worker.terminate();
          OPFS.worker = null;
          URL.revokeObjectURL(OPFS.opfsSyncWorkerUrl);
        }
      },
    }
  });
}
