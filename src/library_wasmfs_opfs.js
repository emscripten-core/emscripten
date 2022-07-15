/**
 * @license
 * Copyright 2022 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

mergeInto(LibraryManager.library, {
  // TODO: Generate these ID pools from a common utility.
  $wasmfsOPFSDirectoryHandles: {
    allocated: [],
    free: [],
    get: function(i) {
#if ASSERTIONS
      assert(this.allocated[i] !== undefined);
#endif
      return this.allocated[i];
    }
  },

  $wasmfsOPFSFileHandles: {
    allocated: [],
    free: [],
    get: function(i) {
#if ASSERTIONS
      assert(this.allocated[i] !== undefined);
#endif
      return this.allocated[i];
    }
  },

  $wasmfsOPFSAccessHandles: {
    allocated: [],
    free: [],
    get: function(i) {
#if ASSERTIONS
      assert(this.allocated[i] !== undefined);
#endif
      return this.allocated[i];
    }
  },

  $wasmfsOPFSBlobs: {
    allocated: [],
    free: [],
    get: function(i) {
#if ASSERTIONS
      assert(this.allocated[i] !== undefined);
#endif
      return this.allocated[i];
    }
  },

  $wasmfsOPFSAllocate: function(ids, handle) {
    let id;
    if (ids.free.length > 0) {
      id = ids.free.pop();
      ids.allocated[id] = handle;
    } else {
      id = ids.allocated.length;
      ids.allocated.push(handle);
    }
    return id;
  },

  $wasmfsOPFSFree: function(ids, id) {
#if ASSERTIONS
    assert(ids.allocated[id] !== undefined);
#endif
    delete ids.allocated[id];
    ids.free.push(id);
  },

  _wasmfs_opfs_init_root_directory__deps: ['$wasmfsOPFSDirectoryHandles'],
  _wasmfs_opfs_init_root_directory: async function(ctx) {
    if (wasmfsOPFSDirectoryHandles.allocated.length == 0) {
      // Directory 0 is reserved as the root
      let root = await navigator.storage.getDirectory();
      wasmfsOPFSDirectoryHandles.allocated.push(root);
    }
    _emscripten_proxy_finish(ctx);
  },

  // Return the file ID for the file with `name` under `parent`, creating it if
  // it doesn't exist and `create` or otherwise return one of the following
  // error codes:
  //
  // -1: file does not exist.
  // -2: file exists but it is actually a directory.
  // -3: file exists but an access handle cannot be created for it.
  $wasmfsOPFSGetOrCreateFile__deps: ['$wasmfsOPFSAllocate',
                                     '$wasmfsOPFSDirectoryHandles',
                                     '$wasmfsOPFSFileHandles'],
  $wasmfsOPFSGetOrCreateFile: async function(parent, name, create) {
    let parentHandle = wasmfsOPFSDirectoryHandles.get(parent);
    let fileHandle;
    try {
      fileHandle = await parentHandle.getFileHandle(name, {create: create});
    } catch (e) {
      if (e.name === "NotFoundError") {
        return -1;
      }
      if (e.name === "TypeMismatchError") {
        return -2;
      }
      throw e;
    }
    return wasmfsOPFSAllocate(wasmfsOPFSFileHandles, fileHandle);
  },

  // Return the file ID for the directory with `name` under `parent`, creating
  // it if it doesn't exist and `create` or otherwise one of the following error
  // codes:
  //
  // -1: directory does not exist.
  // -2: directory exists but is actually a data file.
  $wasmfsOPFSGetOrCreateDir__deps: ['$wasmfsOPFSAllocate',
                                    '$wasmfsOPFSDirectoryHandles'],
  $wasmfsOPFSGetOrCreateDir: async function(parent, name, create) {
    let parentHandle = wasmfsOPFSDirectoryHandles.get(parent);
    let childHandle;
    try {
      childHandle =
          await parentHandle.getDirectoryHandle(name, {create: create});
    } catch (e) {
      if (e.name === "NotFoundError") {
        return -1;
      }
      if (e.name === "TypeMismatchError") {
        return -2;
      }
      throw e;
    }
    return wasmfsOPFSAllocate(wasmfsOPFSDirectoryHandles, childHandle);
  },

  _wasmfs_opfs_get_child__deps: ['$wasmfsOPFSGetOrCreateFile',
                                 '$wasmfsOPFSGetOrCreateDir'],
  _wasmfs_opfs_get_child:
      async function(ctx, parent, namePtr, childTypePtr, childIDPtr) {
    let name = UTF8ToString(namePtr);
    let childType = 1;
    let childID = await wasmfsOPFSGetOrCreateFile(parent, name, false);
    if (childID == -2) {
      childType = 2;
      childID = await wasmfsOPFSGetOrCreateDir(parent, name, false);
    }
    {{{ makeSetValue('childTypePtr', 0, 'childType', 'i32') }}};
    {{{ makeSetValue('childIDPtr', 0, 'childID', 'i32') }}};
    _emscripten_proxy_finish(ctx);
  },

  _wasmfs_opfs_get_entries__deps: [],
  _wasmfs_opfs_get_entries: async function(ctx, dirID, entries) {
    let dirHandle = wasmfsOPFSDirectoryHandles.get(dirID);

    // TODO: Use 'for await' once Acorn supports that.
    let iter = dirHandle.entries();
    for (let entry; entry = await iter.next(), !entry.done;) {
      let [name, child] = entry.value;
      withStackSave(() => {
        let namePtr = allocateUTF8OnStack(name);
        let type = child.kind == "file" ?
            {{{ cDefine('File::DataFileKind') }}} :
            {{{ cDefine('File::DirectoryKind') }}};
        __wasmfs_opfs_record_entry(entries, namePtr, type)
      });
    }
    _emscripten_proxy_finish(ctx);
  },

  _wasmfs_opfs_insert_file__deps: ['$wasmfsOPFSGetOrCreateFile'],
  _wasmfs_opfs_insert_file: async function(ctx, parent, namePtr, childIDPtr) {
    let name = UTF8ToString(namePtr);
    let childID = await wasmfsOPFSGetOrCreateFile(parent, name, true);
    {{{ makeSetValue('childIDPtr', 0, 'childID', 'i32') }}};
    _emscripten_proxy_finish(ctx);
  },

  _wasmfs_opfs_insert_directory__deps: ['$wasmfsOPFSGetOrCreateDir'],
  _wasmfs_opfs_insert_directory:
      async function(ctx, parent, namePtr, childIDPtr) {
    let name = UTF8ToString(namePtr);
    let childID = await wasmfsOPFSGetOrCreateDir(parent, name, true);
    {{{ makeSetValue('childIDPtr', 0, 'childID', 'i32') }}};
    _emscripten_proxy_finish(ctx);
  },

  _wasmfs_opfs_move__deps: ['$wasmfsOPFSFileHandles',
                            '$wasmfsOPFSDirectoryHandles'],
  _wasmfs_opfs_move: async function(ctx, fileID, newDirID, namePtr) {
    let name = UTF8ToString(namePtr);
    let fileHandle = wasmfsOPFSFileHandles.get(fileID);
    let newDirHandle = wasmfsOPFSDirectoryHandles.get(newDirID);
    await fileHandle.move(newDirHandle, name);
    _emscripten_proxy_finish(ctx);
  },

  _wasmfs_opfs_remove_child__deps: ['$wasmfsOPFSFree',
                                    '$wasmfsOPFSDirectoryHandles'],
  _wasmfs_opfs_remove_child: async function(ctx, dirID, namePtr) {
    let name = UTF8ToString(namePtr);
    let dirHandle = wasmfsOPFSDirectoryHandles.get(dirID);
    await dirHandle.removeEntry(name);
    _emscripten_proxy_finish(ctx);
  },

  _wasmfs_opfs_free_file__deps: ['$wasmfsOPFSFree', '$wasmfsOPFSFileHandles'],
  _wasmfs_opfs_free_file: function(fileID) {
    wasmfsOPFSFree(wasmfsOPFSFileHandles, fileID);
  },

  _wasmfs_opfs_free_directory__deps: ['$wasmfsOPFSFree',
                                      '$wasmfsOPFSDirectoryHandles'],
  _wasmfs_opfs_free_directory: function(dirID) {
    wasmfsOPFSFree(wasmfsOPFSDirectoryHandles, dirID);
  },

  _wasmfs_opfs_open_access__deps: ['$wasmfsOPFSAllocate',
                                   '$wasmfsOPFSFileHandles',
                                   '$wasmfsOPFSAccessHandles'],
  _wasmfs_opfs_open_access: async function(ctx, fileID, accessIDPtr) {
    let fileHandle = wasmfsOPFSFileHandles.get(fileID);
    let accessID;
    try {
      let accessHandle;
      // TODO: Remove this once the Access Handles API has settled.
      if (FileSystemFileHandle.prototype.createSyncAccessHandle.length == 0) {
        accessHandle = await fileHandle.createSyncAccessHandle();
      } else {
        accessHandle = await fileHandle.createSyncAccessHandle(
            {mode: "in-place"});
      }
      accessID = wasmfsOPFSAllocate(wasmfsOPFSAccessHandles, accessHandle);
    } catch (e) {
      if (e.name === "InvalidStateError") {
        accessID = -1;
      } else {
        throw e;
      }
    }
    {{{ makeSetValue('accessIDPtr', 0, 'accessID', 'i32') }}};
    _emscripten_proxy_finish(ctx);
  },

  _wasmfs_opfs_open_blob__deps: ['$wasmfsOPFSAllocate',
                                 '$wasmfsOPFSFileHandles',
                                 '$wasmfsOPFSBlobs'],
  _wasmfs_opfs_open_blob: async function(ctx, fileID, blobIDPtr) {
    let fileHandle = wasmfsOPFSFileHandles.get(fileID);
    let blobID;
    try {
      let blob = await fileHandle.getFile();
      blobID = wasmfsOPFSAllocate(wasmfsOPFSBlobs, blob);
    } catch (e) {
      if (e.name === "NotAllowedError") {
        blobID = -1;
      } else {
        throw e;
      }
    }
    {{{ makeSetValue('blobIDPtr', 0, 'blobID', 'i32') }}};
    _emscripten_proxy_finish(ctx);
  },

  _wasmfs_opfs_close_access__deps: ['$wasmfsOPFSFree',
                                    '$wasmfsOPFSAccessHandles'],
  _wasmfs_opfs_close_access: async function(ctx, accessID) {
    let accessHandle = wasmfsOPFSAccessHandles.get(accessID);
    await accessHandle.close();
    wasmfsOPFSFree(wasmfsOPFSAccessHandles, accessID);
    _emscripten_proxy_finish(ctx);
  },

  _wasmfs_opfs_close_blob__deps: ['$wasmfsOPFSFree',
                                  '$wasmfsOPFSBlobs'],
  _wasmfs_opfs_close_blob: function(blobID) {
    wasmfsOPFSFree(wasmfsOPFSBlobs, blobID);
  },

  _wasmfs_opfs_read_access__deps: ['$wasmfsOPFSAccessHandles'],
  _wasmfs_opfs_read_access: function(accessID, bufPtr, len, pos) {
    let accessHandle = wasmfsOPFSAccessHandles.get(accessID);
    let data = HEAPU8.subarray(bufPtr, bufPtr + len);
    return accessHandle.read(data, {at: pos});
  },

  _wasmfs_opfs_read_blob__deps: ['$wasmfsOPFSBlobs'],
  _wasmfs_opfs_read_blob: async function(ctx, blobID, bufPtr, len, pos, nreadPtr) {
    let blob = wasmfsOPFSBlobs.get(blobID);
    let slice = blob.slice(pos, pos + len);
    let nread = 0;

    try {
      // TODO: Use ReadableStreamBYOBReader once
      // https://bugs.chromium.org/p/chromium/issues/detail?id=1189621 is
      // resolved.
      let buf = await slice.arrayBuffer();
      let data = new Uint8Array(buf);
      HEAPU8.set(data, bufPtr);
      nread += data.length;
    } catch (e) {
      if (e instanceof RangeError) {
        nread = -1;
      } else {
        throw e;
      }
    }

    {{{ makeSetValue('nreadPtr', 0, 'nread', 'i32') }}};
    _emscripten_proxy_finish(ctx);
  },

  _wasmfs_opfs_write_access__deps: ['$wasmfsOPFSAccessHandles'],
  _wasmfs_opfs_write_access: function(accessID, bufPtr, len, pos, nwrittenPtr) {
    let accessHandle = wasmfsOPFSAccessHandles.get(accessID);
    let data = HEAPU8.subarray(bufPtr, bufPtr + len);
    return accessHandle.write(data, {at: pos});
  },

  _wasmfs_opfs_get_size_access__deps: ['$wasmfsOPFSAccessHandles'],
  _wasmfs_opfs_get_size_access: async function(ctx, accessID, sizePtr) {
    let accessHandle = wasmfsOPFSAccessHandles.get(accessID);
    let size = await accessHandle.getSize();
    {{{ makeSetValue('sizePtr', 0, 'size', 'i32') }}};
    _emscripten_proxy_finish(ctx);
  },

  _wasmfs_opfs_get_size_blob__deps: ['$wasmfsOPFSBlobs'],
  _wasmfs_opfs_get_size_blob: function(blobID) {
    return wasmfsOPFSBlobs.get(blobID).size;
  },

  _wasmfs_opfs_get_size_file__deps: ['$wasmfsOPFSFileHandles'],
  _wasmfs_opfs_get_size_file: async function(ctx, fileID, sizePtr) {
    let fileHandle = wasmfsOPFSFileHandles.get(fileID);
    let size = (await fileHandle.getFile()).size;
    {{{ makeSetValue('sizePtr', 0, 'size', 'i32') }}};
    _emscripten_proxy_finish(ctx);
  },

  _wasmfs_opfs_set_size_access__deps: ['$wasmfsOPFSAccessHandles'],
  _wasmfs_opfs_set_size_access: async function(ctx, accessID, size) {
    let accessHandle = wasmfsOPFSAccessHandles.get(accessID);
    await accessHandle.truncate(size);
    _emscripten_proxy_finish(ctx);
  },

  _wasmfs_opfs_set_size_file__deps: ['$wasmfsOPFSFileHandles'],
  _wasmfs_opfs_set_size_file: async function(ctx, fileID, size, errPtr) {
    let fileHandle = wasmfsOPFSFileHandles.get(fileID);
    try {
      let writable = await fileHandle.createWritable({keepExistingData: true});
      await writable.truncate(size);
      await writable.close();
      {{{ makeSetValue('errPtr', 0, '0', 'i32') }}};
    } catch {
      {{{ makeSetValue('errPtr', 0, '1', 'i32') }}};
    }
    _emscripten_proxy_finish(ctx);
  },

  _wasmfs_opfs_flush_access__deps: ['$wasmfsOPFSAccessHandles'],
  _wasmfs_opfs_flush_access: async function(ctx, accessID) {
    let accessHandle = wasmfsOPFSAccessHandles.get(accessID);
    await accessHandle.flush();
    _emscripten_proxy_finish(ctx);
  }
});
