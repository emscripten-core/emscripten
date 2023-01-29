/**
 * @license
 * Copyright 2022 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

mergeInto(LibraryManager.library, {
  $wasmfsOPFSDirectoryHandles__deps: ['$HandleAllocator'],
  $wasmfsOPFSDirectoryHandles: "new HandleAllocator()",
  $wasmfsOPFSFileHandles__deps: ['$HandleAllocator'],
  $wasmfsOPFSFileHandles: "new HandleAllocator()",
  $wasmfsOPFSAccessHandles__deps: ['$HandleAllocator'],
  $wasmfsOPFSAccessHandles: "new HandleAllocator()",
  $wasmfsOPFSBlobs__deps: ["$HandleAllocator"],
  $wasmfsOPFSBlobs: "new HandleAllocator()",

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
  // it doesn't exist and `create` or otherwise return a negative error code
  // corresponding to the error.
  $wasmfsOPFSGetOrCreateFile__deps: ['$wasmfsOPFSDirectoryHandles',
                                     '$wasmfsOPFSFileHandles'],
  $wasmfsOPFSGetOrCreateFile: async function(parent, name, create) {
    let parentHandle = wasmfsOPFSDirectoryHandles.get(parent);
    let fileHandle;
    try {
      fileHandle = await parentHandle.getFileHandle(name, {create: create});
    } catch (e) {
      if (e.name === "NotFoundError") {
        return -{{{ cDefine('EEXIST') }}};
      }
      if (e.name === "TypeMismatchError") {
        return -{{{ cDefine('EISDIR') }}};
      }
#if ASSERTIONS
      err('unexpected error:', e, e.stack);
#endif
      return -{{{ cDefine('EIO') }}};
    }
    return wasmfsOPFSFileHandles.allocate(fileHandle);
  },

  // Return the file ID for the directory with `name` under `parent`, creating
  // it if it doesn't exist and `create` or otherwise return a negative error
  // code corresponding to the error.
  $wasmfsOPFSGetOrCreateDir__deps: ['$wasmfsOPFSDirectoryHandles'],
  $wasmfsOPFSGetOrCreateDir: async function(parent, name, create) {
    let parentHandle = wasmfsOPFSDirectoryHandles.get(parent);
    let childHandle;
    try {
      childHandle =
          await parentHandle.getDirectoryHandle(name, {create: create});
    } catch (e) {
      if (e.name === "NotFoundError") {
        return -{{{ cDefine('EEXIST') }}};
      }
      if (e.name === "TypeMismatchError") {
        return -{{{ cDefine('ENOTDIR') }}};
      }
#if ASSERTIONS
      err('unexpected error:', e, e.stack);
#endif
      return -{{{ cDefine('EIO') }}};
    }
    return wasmfsOPFSDirectoryHandles.allocate(childHandle);
  },

  _wasmfs_opfs_get_child__deps: ['$wasmfsOPFSGetOrCreateFile',
                                 '$wasmfsOPFSGetOrCreateDir'],
  _wasmfs_opfs_get_child:
      async function(ctx, parent, namePtr, childTypePtr, childIDPtr) {
    let name = UTF8ToString(namePtr);
    let childType = 1;
    let childID = await wasmfsOPFSGetOrCreateFile(parent, name, false);
    if (childID == -{{{ cDefine('EISDIR') }}}) {
      childType = 2;
      childID = await wasmfsOPFSGetOrCreateDir(parent, name, false);
    }
    {{{ makeSetValue('childTypePtr', 0, 'childType', 'i32') }}};
    {{{ makeSetValue('childIDPtr', 0, 'childID', 'i32') }}};
    _emscripten_proxy_finish(ctx);
  },

  _wasmfs_opfs_get_entries__deps: [],
  _wasmfs_opfs_get_entries: async function(ctx, dirID, entriesPtr, errPtr) {
    let dirHandle = wasmfsOPFSDirectoryHandles.get(dirID);

    // TODO: Use 'for await' once Acorn supports that.
    try {
      let iter = dirHandle.entries();
      for (let entry; entry = await iter.next(), !entry.done;) {
        let [name, child] = entry.value;
        withStackSave(() => {
          let namePtr = allocateUTF8OnStack(name);
          let type = child.kind == "file" ?
              {{{ cDefine('File::DataFileKind') }}} :
          {{{ cDefine('File::DirectoryKind') }}};
          __wasmfs_opfs_record_entry(entriesPtr, namePtr, type)
        });
      }
    } catch {
      let err = -{{{ cDefine('EIO') }}};
      {{{ makeSetValue('errPtr', 0, 'err', 'i32') }}};
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
  _wasmfs_opfs_move: async function(ctx, fileID, newDirID, namePtr, errPtr) {
    let name = UTF8ToString(namePtr);
    let fileHandle = wasmfsOPFSFileHandles.get(fileID);
    let newDirHandle = wasmfsOPFSDirectoryHandles.get(newDirID);
    try {
      await fileHandle.move(newDirHandle, name);
    } catch {
      let err = -{{{ cDefine('EIO') }}};
      {{{ makeSetValue('errPtr', 0, 'err', 'i32') }}};
    }
    _emscripten_proxy_finish(ctx);
  },

  _wasmfs_opfs_remove_child__deps: ['$wasmfsOPFSDirectoryHandles'],
  _wasmfs_opfs_remove_child: async function(ctx, dirID, namePtr, errPtr) {
    let name = UTF8ToString(namePtr);
    let dirHandle = wasmfsOPFSDirectoryHandles.get(dirID);
    try {
      await dirHandle.removeEntry(name);
    } catch {
      let err = -{{{ cDefine('EIO') }}};
      {{{ makeSetValue('errPtr', 0, 'err', 'i32') }}};
    }
    _emscripten_proxy_finish(ctx);
  },

  _wasmfs_opfs_free_file__deps: ['$wasmfsOPFSFileHandles'],
  _wasmfs_opfs_free_file: function(fileID) {
    wasmfsOPFSFileHandles.free(fileID);
  },

  _wasmfs_opfs_free_directory__deps: ['$wasmfsOPFSDirectoryHandles'],
  _wasmfs_opfs_free_directory: function(dirID) {
    wasmfsOPFSDirectoryHandles.free(dirID);
  },

  _wasmfs_opfs_open_access__deps: ['$wasmfsOPFSFileHandles',
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
      accessID = wasmfsOPFSAccessHandles.allocate(accessHandle);
    } catch (e) {
      // TODO: Presumably only one of these will appear in the final API?
      if (e.name === "InvalidStateError" ||
          e.name === "NoModificationAllowedError") {
        accessID = -{{{ cDefine('EACCES') }}};
      } else {
#if ASSERTIONS
        err('unexpected error:', e, e.stack);
#endif
        accessID = -{{{ cDefine('EIO') }}};
      }
    }
    {{{ makeSetValue('accessIDPtr', 0, 'accessID', 'i32') }}};
    _emscripten_proxy_finish(ctx);
  },

  _wasmfs_opfs_open_blob__deps: ['$wasmfsOPFSFileHandles',
                                 '$wasmfsOPFSBlobs'],
  _wasmfs_opfs_open_blob: async function(ctx, fileID, blobIDPtr) {
    let fileHandle = wasmfsOPFSFileHandles.get(fileID);
    let blobID;
    try {
      let blob = await fileHandle.getFile();
      blobID = wasmfsOPFSBlobs.allocate(blob);
    } catch (e) {
      if (e.name === "NotAllowedError") {
        blobID = -{{{ cDefine('EACCES') }}};
      } else {
#if ASSERTIONS
        err('unexpected error:', e, e.stack);
#endif
        blobID = -{{{ cDefine('EIO') }}};
      }
    }
    {{{ makeSetValue('blobIDPtr', 0, 'blobID', 'i32') }}};
    _emscripten_proxy_finish(ctx);
  },

  _wasmfs_opfs_close_access__deps: ['$wasmfsOPFSAccessHandles'],
  _wasmfs_opfs_close_access: async function(ctx, accessID, errPtr) {
    let accessHandle = wasmfsOPFSAccessHandles.get(accessID);
    try {
      await accessHandle.close();
    } catch {
      let err = -{{{ cDefine('EIO') }}};
      {{{ makeSetValue('errPtr', 0, 'err', 'i32') }}};
    }
    wasmfsOPFSAccessHandles.free(accessID);
    _emscripten_proxy_finish(ctx);
  },

  _wasmfs_opfs_close_blob__deps: ['$wasmfsOPFSBlobs'],
  _wasmfs_opfs_close_blob: function(blobID) {
    wasmfsOPFSBlobs.free(blobID);
  },

  _wasmfs_opfs_read_access__deps: ['$wasmfsOPFSAccessHandles'],
  _wasmfs_opfs_read_access: function(accessID, bufPtr, len, pos) {
    let accessHandle = wasmfsOPFSAccessHandles.get(accessID);
    let data = HEAPU8.subarray(bufPtr, bufPtr + len);
    try {
      return accessHandle.read(data, {at: pos});
    } catch (e) {
      if (e.name == "TypeError") {
        return -{{{ cDefine('EINVAL') }}};
      }
#if ASSERTIONS
      err('unexpected error:', e, e.stack);
#endif
      return -{{{ cDefine('EIO') }}};
    }
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
        nread = -{{{ cDefine('EFAULT') }}};
      } else {
#if ASSERTIONS
        err('unexpected error:', e, e.stack);
#endif
        nread = -{{{ cDefine('EIO') }}};
      }
    }

    {{{ makeSetValue('nreadPtr', 0, 'nread', 'i32') }}};
    _emscripten_proxy_finish(ctx);
  },

  _wasmfs_opfs_write_access__deps: ['$wasmfsOPFSAccessHandles'],
  _wasmfs_opfs_write_access: function(accessID, bufPtr, len, pos) {
    let accessHandle = wasmfsOPFSAccessHandles.get(accessID);
    let data = HEAPU8.subarray(bufPtr, bufPtr + len);
    try {
      return accessHandle.write(data, {at: pos});
    } catch (e) {
      if (e.name == "TypeError") {
        return -{{{ cDefine('EINVAL') }}};
      }
#if ASSERTIONS
      err('unexpected error:', e, e.stack);
#endif
      return -{{{ cDefine('EIO') }}};
    }
  },

  _wasmfs_opfs_get_size_access__deps: ['$wasmfsOPFSAccessHandles'],
  _wasmfs_opfs_get_size_access: async function(ctx, accessID, sizePtr) {
    let accessHandle = wasmfsOPFSAccessHandles.get(accessID);
    let size;
    try {
      size = await accessHandle.getSize();
    } catch {
      size = -{{{ cDefine('EIO') }}};
    }
    {{{ makeSetValue('sizePtr', 0, 'size', 'i64') }}};
    _emscripten_proxy_finish(ctx);
  },

  _wasmfs_opfs_get_size_blob__deps: ['$wasmfsOPFSBlobs'],
  _wasmfs_opfs_get_size_blob: function(blobID) {
    // This cannot fail.
    return wasmfsOPFSBlobs.get(blobID).size;
  },

  _wasmfs_opfs_get_size_file__deps: ['$wasmfsOPFSFileHandles'],
  _wasmfs_opfs_get_size_file: async function(ctx, fileID, sizePtr) {
    let fileHandle = wasmfsOPFSFileHandles.get(fileID);
    let size;
    try {
      size = (await fileHandle.getFile()).size;
    } catch {
      size = -{{{ cDefine('EIO') }}};
    }
    {{{ makeSetValue('sizePtr', 0, 'size', 'i64') }}};
    _emscripten_proxy_finish(ctx);
  },

  _wasmfs_opfs_set_size_access__deps: ['$wasmfsOPFSAccessHandles'],
  _wasmfs_opfs_set_size_access: async function(ctx, accessID,
                                               {{{ defineI64Param('size') }}},
                                               errPtr) {
    {{{ receiveI64ParamAsDouble('size') }}};
    let accessHandle = wasmfsOPFSAccessHandles.get(accessID);
    try {
      await accessHandle.truncate(size);
    } catch {
      let err = -{{{ cDefine('EIO') }}};
      {{{ makeSetValue('errPtr', 0, 'err', 'i32') }}};
    }
    _emscripten_proxy_finish(ctx);
  },

  _wasmfs_opfs_set_size_file__deps: ['$wasmfsOPFSFileHandles'],
  _wasmfs_opfs_set_size_file: async function(ctx, fileID,
                                             {{{ defineI64Param('size') }}},
                                             errPtr) {
    {{{ receiveI64ParamAsDouble('size') }}};
    let fileHandle = wasmfsOPFSFileHandles.get(fileID);
    try {
      let writable = await fileHandle.createWritable({keepExistingData: true});
      await writable.truncate(size);
      await writable.close();
    } catch {
      let err = -{{{ cDefine('EIO') }}};
      {{{ makeSetValue('errPtr', 0, 'err', 'i32') }}};
    }
    _emscripten_proxy_finish(ctx);
  },

  _wasmfs_opfs_flush_access__deps: ['$wasmfsOPFSAccessHandles'],
  _wasmfs_opfs_flush_access: async function(ctx, accessID, errPtr) {
    let accessHandle = wasmfsOPFSAccessHandles.get(accessID);
    try {
      await accessHandle.flush();
    } catch {
      let err = -{{{ cDefine('EIO') }}};
      {{{ makeSetValue('errPtr', 0, 'err', 'i32') }}};
    }
    _emscripten_proxy_finish(ctx);
  }
});
