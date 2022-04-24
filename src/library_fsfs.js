/**
 * @license
 * Copyright 2022 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

mergeInto(LibraryManager.library, {
  $FSFS__deps: ['$FS', '$MEMFS', '$PATH'],
  $FSFS__postset: function() {
    return '';
  },
  $FSFS: {
    DIR_MODE: Number("{{{ cDefine('S_IFDIR') }}}") | 511 /* 0777 */,
    FILE_MODE: Number("{{{ cDefine('S_IFREG') }}}") | 511 /* 0777 */,
    mount: function(mount) {
      if (!mount.opts.dirHandle) {
        throw new Error('opts.dirHandle is required');
      }

      // reuse all of the core MEMFS functionality
      return MEMFS.mount.apply(null, arguments);
    },
    syncfs: async (mount, populate, callback) => {
      try {
        const local = await FSFS.getLocalSet(mount);
        const remote = await FSFS.getRemoteSet(mount);
        const src = populate ? remote : local;
        const dst = populate ? local : remote;
        await FSFS.reconcile(mount, src, dst);
        callback(null);
      } catch (e) {
        callback(e);
      }
    },
    // Returns file set of emscripten's filesystem at the mountpoint.
    getLocalSet: (mount) => {
      var entries = Object.create(null);

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
        var stat = FS.stat(path);

        if (FS.isDir(stat.mode)) {
          check.push.apply(check, FS.readdir(path).filter(isRealDir).map(toAbsolute(path)));
        }

        entries[path] = { timestamp: stat.mtime, mode: stat.mode };
      }

      return { type: 'local', entries: entries };
    },
    // Returns file set of the real, on-disk filesystem at the mountpoint.
    getRemoteSet: async (mount) => {
      const entries = Object.create(null);

      const handles = await FSFS.getFsHandles(mount.opts.dirHandle, true);
      for (const [path, handle] of handles) {
        if (path === '.') continue;

        entries[PATH.join2(mount.mountpoint, path)] = {
          timestamp: handle.kind === 'file' ? (await handle.getFile()).lastModifiedDate : new Date(),
          mode: handle.kind === 'file' ? FSFS.FILE_MODE : FSFS.DIR_MODE,
        };
      }

      return { type: 'remote', entries, handles };
    },
    loadLocalEntry: (path) => {
      const lookup = FS.lookupPath(path);
      const node = lookup.node;
      const stat = FS.stat(path);      

      if (FS.isDir(stat.mode)) {
        return { 'timestamp': stat.mtime, 'mode': stat.mode };
      } else if (FS.isFile(stat.mode)) {
        node.contents = MEMFS.getFileDataAsTypedArray(node);
        return { timestamp: stat.mtime, mode: stat.mode, contents: node.contents };
      } else {
        throw new Error('node type not supported');
      }
    },
    storeLocalEntry: (path, entry) => {
      if (FS.isDir(entry['mode'])) {
        FS.mkdirTree(path, entry['mode']);
      } else if (FS.isFile(entry['mode'])) {
        FS.writeFile(path, entry['contents'], { canOwn: true });
      } else {
        throw new Error('node type not supported');
      }

      FS.chmod(path, entry['mode']);
      FS.utime(path, entry['timestamp'], entry['timestamp']);
    },
    removeLocalEntry: (path) => {
      var stat = FS.stat(path);

      if (FS.isDir(stat.mode)) {
        FS.rmdir(path);
      } else if (FS.isFile(stat.mode)) {
        FS.unlink(path);
      }
    },
    loadRemoteEntry: async (handle) => {
      if (handle.kind === 'file') {
        const file = await handle.getFile();
        return {
          contents: new Uint8Array(await file.arrayBuffer()),
          mode: FSFS.FILE_MODE,
          timestamp: file.lastModifiedDate,
        };
      } else if (handle.kind === 'directory') {
        return {
          mode: FSFS.DIR_MODE,
          timestamp: new Date(),
        };
      } else {
        throw new Error('unknown kind: ' + handle.kind);
      }
    },
    storeRemoteEntry: async (handles, path, entry) => {
      const parentDirHandle = handles.get(PATH.dirname(path));
      const handle = FS.isFile(entry.mode) ?
        await parentDirHandle.getFileHandle(PATH.basename(path), {create: true}) :
        await parentDirHandle.getDirectoryHandle(PATH.basename(path), {create: true});
      if (handle.kind === 'file') {
        const writable = await handle.createWritable();
        await writable.write(entry.contents);
        await writable.close();
      }
      handles.set(path, handle);
    },
    removeRemoteEntry: async (handles, path) => {
      const parentDirHandle = handles.get(PATH.dirname(path));
      await parentDirHandle.removeEntry(PATH.basename(path));
      handles.delete(path);
    },
    reconcile: async (mount, src, dst) => {
      let total = 0;

      const create = [];
      Object.keys(src.entries).forEach(function (key) {
        const e = src.entries[key];
        const e2 = dst.entries[key];
        if (!e2 || (FS.isFile(e.mode) && e['timestamp'].getTime() > e2['timestamp'].getTime())) {
          create.push(key);
          total++;
        }
      });
      // sort paths in ascending order so directory entries are created
      // before the files inside them
      create.sort();

      const remove = [];
      Object.keys(dst.entries).forEach(function (key) {
        if (!src.entries[key]) {
          remove.push(key);
          total++;
        }
      });
      // sort paths in descending order so files are deleted before their
      // parent directories
      remove.sort().reverse();

      if (!total) {
        return;
      }

      const handles = src.type === 'remote' ? src.handles : dst.handles;

      for (const path of create) {
        const relPath = PATH.normalize(path.replace(mount.mountpoint, '/')).substring(1);;
        if (dst.type === 'local') {
          const handle = handles.get(relPath);
          const entry = await FSFS.loadRemoteEntry(handle);
          FSFS.storeLocalEntry(path, entry);
        } else {
          const entry = FSFS.loadLocalEntry(path);
          await FSFS.storeRemoteEntry(handles, relPath, entry);
        }
      }

      for (const path of remove) {
        if (dst.type === 'local') {
          FSFS.removeLocalEntry(path);
        } else {
          const relPath = PATH.normalize(path.replace(mount.mountpoint, '/')).substring(1);
          await FSFS.removeRemoteEntry(handles, relPath);
        }
      }
    },
    getFsHandles: async (dirHandle) => {
      const handles = [];
    
      async function collect(curDirHandle) {
        for await (const entry of curDirHandle.values()) {
          handles.push(entry);
          if (entry.kind === 'directory') {
            await collect(entry);
          }
        }
      }
    
      await collect(dirHandle);
    
      const result = new Map();
      result.set('.', dirHandle);
      for (const handle of handles) {
        const relativePath = (await dirHandle.resolve(handle)).join('/');
        result.set(relativePath, handle);
      }
      return result;
    },
  }
});
