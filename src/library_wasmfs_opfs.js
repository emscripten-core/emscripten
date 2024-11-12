/**
 * @license
 * Copyright 2022 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

addToLibrary({
  $wasmfsOPFSDirectoryHandles__deps: ['$HandleAllocator'],
  $wasmfsOPFSDirectoryHandles: "new HandleAllocator()",
  $wasmfsOPFSFileHandles__deps: ['$HandleAllocator'],
  $wasmfsOPFSFileHandles: "new HandleAllocator()",
  $wasmfsOPFSAccessHandles__deps: ['$HandleAllocator'],
  $wasmfsOPFSAccessHandles: "new HandleAllocator()",
  $wasmfsOPFSBlobs__deps: ["$HandleAllocator"],
  $wasmfsOPFSBlobs: "new HandleAllocator()",

#if !PTHREADS
  // OPFS will only be used on modern browsers that supports JS classes.
  $FileSystemAsyncAccessHandle: class {
    // This class implements the same interface as the sync version, but has
    // async reads and writes. Hopefully this will one day be implemented by the
    // platform so we can remove it.
    constructor(handle) {
      this.handle = handle;
    }
    async close() {}
    async flush() {}
    async getSize() {
      let file = await this.handle.getFile();
      return file.size;
    }
    async read(buffer, options = { at: 0 }) {
      let file = await this.handle.getFile();
      // The end position may be past the end of the file, but slice truncates
      // it.
      let slice = await file.slice(options.at, options.at + buffer.length);
      let fileBuffer = await slice.arrayBuffer();
      let array = new Uint8Array(fileBuffer);
      buffer.set(array);
      return array.length;
    }
    async write(buffer, options = { at: 0 }) {
      let writable = await this.handle.createWritable({keepExistingData: true});
      await writable.write({ type: 'write', position: options.at, data: buffer });
      await writable.close();
      return buffer.length;
    }
    async truncate(size) {
      let writable = await this.handle.createWritable({keepExistingData: true});
      await writable.truncate(size);
      await writable.close();
    }
  },

  $wasmfsOPFSCreateAsyncAccessHandle__deps: ['$FileSystemAsyncAccessHandle'],
  $wasmfsOPFSCreateAsyncAccessHandle: (fileHandle) => new FileSystemAsyncAccessHandle(fileHandle),
#endif

#if PTHREADS
  $wasmfsOPFSProxyFinish__deps: ['emscripten_proxy_finish'],
#endif
  $wasmfsOPFSProxyFinish: (ctx) => {
    // When using pthreads the proxy needs to know when the work is finished.
    // When used with JSPI the work will be executed in an async block so there
    // is no need to notify when done.
#if PTHREADS
    _emscripten_proxy_finish(ctx);
#endif
  },

  _wasmfs_opfs_init_root_directory__deps: ['$wasmfsOPFSDirectoryHandles', '$wasmfsOPFSProxyFinish'],
  _wasmfs_opfs_init_root_directory: async function(ctx) {
    // allocated.length starts off as 1 since 0 is a reserved handle
    if (wasmfsOPFSDirectoryHandles.allocated.length == 1) {
      // Closure compiler errors on this as it does not recognize the OPFS
      // API yet, it seems. Unfortunately an existing annotation for this is in
      // the closure compiler codebase, and cannot be overridden in user code
      // (it complains on a duplicate type annotation), so just suppress it.
      /** @suppress {checkTypes} */
      let root = await navigator.storage.getDirectory();
      wasmfsOPFSDirectoryHandles.allocated.push(root);
    }
    wasmfsOPFSProxyFinish(ctx);
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
        return -{{{ cDefs.EEXIST }}};
      }
      if (e.name === "TypeMismatchError") {
        return -{{{ cDefs.EISDIR }}};
      }
#if ASSERTIONS
      err('unexpected error:', e, e.stack);
#endif
      return -{{{ cDefs.EIO }}};
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
        return -{{{ cDefs.EEXIST }}};
      }
      if (e.name === "TypeMismatchError") {
        return -{{{ cDefs.ENOTDIR }}};
      }
#if ASSERTIONS
      err('unexpected error:', e, e.stack);
#endif
      return -{{{ cDefs.EIO }}};
    }
    return wasmfsOPFSDirectoryHandles.allocate(childHandle);
  },

  _wasmfs_opfs_get_child__deps: ['$wasmfsOPFSGetOrCreateFile',
                                 '$wasmfsOPFSGetOrCreateDir', '$wasmfsOPFSProxyFinish'],
  _wasmfs_opfs_get_child:
      async function(ctx, parent, namePtr, childTypePtr, childIDPtr) {
    let name = UTF8ToString(namePtr);
    let childType = 1;
    let childID = await wasmfsOPFSGetOrCreateFile(parent, name, false);
    if (childID == -{{{ cDefs.EISDIR }}}) {
      childType = 2;
      childID = await wasmfsOPFSGetOrCreateDir(parent, name, false);
    }
    {{{ makeSetValue('childTypePtr', 0, 'childType', 'i32') }}};
    {{{ makeSetValue('childIDPtr', 0, 'childID', 'i32') }}};
    wasmfsOPFSProxyFinish(ctx);
  },

  _wasmfs_opfs_get_entries__deps: [
    '$wasmfsOPFSProxyFinish',
    '$stackSave',
    '$stackRestore',
    '_wasmfs_opfs_record_entry',
  ],
  _wasmfs_opfs_get_entries: async function(ctx, dirID, entriesPtr, errPtr) {
    let dirHandle = wasmfsOPFSDirectoryHandles.get(dirID);

    // TODO: Use 'for await' once Acorn supports that.
    try {
      let iter = dirHandle.entries();
      for (let entry; entry = await iter.next(), !entry.done;) {
        let [name, child] = entry.value;
        let sp = stackSave();
        let namePtr = stringToUTF8OnStack(name);
        let type = child.kind == "file" ?
            {{{ cDefine('File::DataFileKind') }}} :
        {{{ cDefine('File::DirectoryKind') }}};
          __wasmfs_opfs_record_entry(entriesPtr, namePtr, type)
        stackRestore(sp);
      }
    } catch {
      let err = -{{{ cDefs.EIO }}};
      {{{ makeSetValue('errPtr', 0, 'err', 'i32') }}};
    }
    wasmfsOPFSProxyFinish(ctx);
  },

  _wasmfs_opfs_insert_file__deps: ['$wasmfsOPFSGetOrCreateFile', '$wasmfsOPFSProxyFinish'],
  _wasmfs_opfs_insert_file: async function(ctx, parent, namePtr, childIDPtr) {
    let name = UTF8ToString(namePtr);
    let childID = await wasmfsOPFSGetOrCreateFile(parent, name, true);
    {{{ makeSetValue('childIDPtr', 0, 'childID', 'i32') }}};
    wasmfsOPFSProxyFinish(ctx);
  },

  _wasmfs_opfs_insert_directory__deps: ['$wasmfsOPFSGetOrCreateDir', '$wasmfsOPFSProxyFinish'],
  _wasmfs_opfs_insert_directory:
      async function(ctx, parent, namePtr, childIDPtr) {
    let name = UTF8ToString(namePtr);
    let childID = await wasmfsOPFSGetOrCreateDir(parent, name, true);
    {{{ makeSetValue('childIDPtr', 0, 'childID', 'i32') }}};
    wasmfsOPFSProxyFinish(ctx);
  },

  _wasmfs_opfs_move_file__deps: ['$wasmfsOPFSFileHandles',
                                 '$wasmfsOPFSDirectoryHandles',
                                 '$wasmfsOPFSProxyFinish'],
  _wasmfs_opfs_move_file: async function(ctx, fileID, newParentID, namePtr, errPtr) {
    let name = UTF8ToString(namePtr);
    let fileHandle = wasmfsOPFSFileHandles.get(fileID);
    let newDirHandle = wasmfsOPFSDirectoryHandles.get(newParentID);
    try {
      await fileHandle.move(newDirHandle, name);
    } catch {
      let err = -{{{ cDefs.EIO }}};
      {{{ makeSetValue('errPtr', 0, 'err', 'i32') }}};
    }
    wasmfsOPFSProxyFinish(ctx);
  },

  _wasmfs_opfs_remove_child__deps: ['$wasmfsOPFSDirectoryHandles', '$wasmfsOPFSProxyFinish'],
  _wasmfs_opfs_remove_child: async function(ctx, dirID, namePtr, errPtr) {
    let name = UTF8ToString(namePtr);
    let dirHandle = wasmfsOPFSDirectoryHandles.get(dirID);
    try {
      await dirHandle.removeEntry(name);
    } catch {
      let err = -{{{ cDefs.EIO }}};
      {{{ makeSetValue('errPtr', 0, 'err', 'i32') }}};
    }
    wasmfsOPFSProxyFinish(ctx);
  },

  _wasmfs_opfs_free_file__deps: ['$wasmfsOPFSFileHandles'],
  _wasmfs_opfs_free_file: (fileID) => {
    wasmfsOPFSFileHandles.free(fileID);
  },

  _wasmfs_opfs_free_directory__deps: ['$wasmfsOPFSDirectoryHandles'],
  _wasmfs_opfs_free_directory: (dirID) => {
    wasmfsOPFSDirectoryHandles.free(dirID);
  },

  _wasmfs_opfs_open_access__deps: ['$wasmfsOPFSFileHandles',
                                   '$wasmfsOPFSAccessHandles', '$wasmfsOPFSProxyFinish',
#if !PTHREADS
                                   '$wasmfsOPFSCreateAsyncAccessHandle'
#endif
                                  ],
  _wasmfs_opfs_open_access: async function(ctx, fileID, accessIDPtr) {
    let fileHandle = wasmfsOPFSFileHandles.get(fileID);
    let accessID;
    try {
      let accessHandle;
#if PTHREADS
      // TODO: Remove this once the Access Handles API has settled.
      // TODO: Closure is confused by this code that supports two versions of
      //       the same API, so suppress type checking on it.
      /** @suppress {checkTypes} */
      var len = FileSystemFileHandle.prototype.createSyncAccessHandle.length;
      if (len == 0) {
        accessHandle = await fileHandle.createSyncAccessHandle();
      } else {
        accessHandle = await fileHandle.createSyncAccessHandle(
            {mode: "in-place"});
      }
#else
      accessHandle = await wasmfsOPFSCreateAsyncAccessHandle(fileHandle);
#endif
      accessID = wasmfsOPFSAccessHandles.allocate(accessHandle);
    } catch (e) {
      // TODO: Presumably only one of these will appear in the final API?
      if (e.name === "InvalidStateError" ||
          e.name === "NoModificationAllowedError") {
        accessID = -{{{ cDefs.EACCES }}};
      } else {
#if ASSERTIONS
        err('unexpected error:', e, e.stack);
#endif
        accessID = -{{{ cDefs.EIO }}};
      }
    }
    {{{ makeSetValue('accessIDPtr', 0, 'accessID', 'i32') }}};
    wasmfsOPFSProxyFinish(ctx);
  },

  _wasmfs_opfs_open_blob__deps: ['$wasmfsOPFSFileHandles',
                                 '$wasmfsOPFSBlobs', '$wasmfsOPFSProxyFinish'],
  _wasmfs_opfs_open_blob: async function(ctx, fileID, blobIDPtr) {
    let fileHandle = wasmfsOPFSFileHandles.get(fileID);
    let blobID;
    try {
      let blob = await fileHandle.getFile();
      blobID = wasmfsOPFSBlobs.allocate(blob);
    } catch (e) {
      if (e.name === "NotAllowedError") {
        blobID = -{{{ cDefs.EACCES }}};
      } else {
#if ASSERTIONS
        err('unexpected error:', e, e.stack);
#endif
        blobID = -{{{ cDefs.EIO }}};
      }
    }
    {{{ makeSetValue('blobIDPtr', 0, 'blobID', 'i32') }}};
    wasmfsOPFSProxyFinish(ctx);
  },

  _wasmfs_opfs_close_access__deps: ['$wasmfsOPFSAccessHandles', '$wasmfsOPFSProxyFinish'],
  _wasmfs_opfs_close_access: async function(ctx, accessID, errPtr) {
    let accessHandle = wasmfsOPFSAccessHandles.get(accessID);
    try {
      await accessHandle.close();
    } catch {
      let err = -{{{ cDefs.EIO }}};
      {{{ makeSetValue('errPtr', 0, 'err', 'i32') }}};
    }
    wasmfsOPFSAccessHandles.free(accessID);
    wasmfsOPFSProxyFinish(ctx);
  },

  _wasmfs_opfs_close_blob__deps: ['$wasmfsOPFSBlobs'],
  _wasmfs_opfs_close_blob: (blobID) => {
    wasmfsOPFSBlobs.free(blobID);
  },

  _wasmfs_opfs_read_access__i53abi: true,
  _wasmfs_opfs_read_access__deps: ['$wasmfsOPFSAccessHandles'],
  _wasmfs_opfs_read_access: {{{ asyncIf(!PTHREADS) }}}  function(accessID, bufPtr, len, pos) {
    let accessHandle = wasmfsOPFSAccessHandles.get(accessID);
    let data = HEAPU8.subarray(bufPtr, bufPtr + len);
    try {
      return {{{ awaitIf(!PTHREADS) }}} accessHandle.read(data, {at: pos});
    } catch (e) {
      if (e.name == "TypeError") {
        return -{{{ cDefs.EINVAL }}};
      }
#if ASSERTIONS
      err('unexpected error:', e, e.stack);
#endif
      return -{{{ cDefs.EIO }}};
    }
  },

  _wasmfs_opfs_read_blob__i53abi: true,
  _wasmfs_opfs_read_blob__deps: ['$wasmfsOPFSBlobs', '$wasmfsOPFSProxyFinish'],
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
        nread = -{{{ cDefs.EFAULT }}};
      } else {
#if ASSERTIONS
        err('unexpected error:', e, e.stack);
#endif
        nread = -{{{ cDefs.EIO }}};
      }
    }

    {{{ makeSetValue('nreadPtr', 0, 'nread', 'i32') }}};
    wasmfsOPFSProxyFinish(ctx);
  },

  _wasmfs_opfs_write_access__i53abi: true,
  _wasmfs_opfs_write_access__deps: ['$wasmfsOPFSAccessHandles'],
  _wasmfs_opfs_write_access: {{{ asyncIf(!PTHREADS) }}} function(accessID, bufPtr, len, pos) {
    let accessHandle = wasmfsOPFSAccessHandles.get(accessID);
    let data = HEAPU8.subarray(bufPtr, bufPtr + len);
    try {
      return {{{ awaitIf(!PTHREADS) }}} accessHandle.write(data, {at: pos});
    } catch (e) {
      if (e.name == "TypeError") {
        return -{{{ cDefs.EINVAL }}};
      }
#if ASSERTIONS
      err('unexpected error:', e, e.stack);
#endif
      return -{{{ cDefs.EIO }}};
    }
  },

  _wasmfs_opfs_get_size_access__deps: ['$wasmfsOPFSAccessHandles', '$wasmfsOPFSProxyFinish'],
  _wasmfs_opfs_get_size_access: async function(ctx, accessID, sizePtr) {
    let accessHandle = wasmfsOPFSAccessHandles.get(accessID);
    let size;
    try {
      size = await accessHandle.getSize();
    } catch {
      size = -{{{ cDefs.EIO }}};
    }
    {{{ makeSetValue('sizePtr', 0, 'size', 'i64') }}};
    wasmfsOPFSProxyFinish(ctx);
  },

  _wasmfs_opfs_get_size_blob__i53abi: true,
  _wasmfs_opfs_get_size_blob__deps: ['$wasmfsOPFSBlobs'],
  _wasmfs_opfs_get_size_blob: (blobID) => {
    // This cannot fail.
	  return wasmfsOPFSBlobs.get(blobID).size;
  },

  _wasmfs_opfs_get_size_file__deps: ['$wasmfsOPFSFileHandles', '$wasmfsOPFSProxyFinish'],
  _wasmfs_opfs_get_size_file: async function(ctx, fileID, sizePtr) {
    let fileHandle = wasmfsOPFSFileHandles.get(fileID);
    let size;
    try {
      size = (await fileHandle.getFile()).size;
    } catch {
      size = -{{{ cDefs.EIO }}};
    }
    {{{ makeSetValue('sizePtr', 0, 'size', 'i64') }}};
    wasmfsOPFSProxyFinish(ctx);
  },

  _wasmfs_opfs_set_size_access__i53abi: true,
  _wasmfs_opfs_set_size_access__deps: ['$wasmfsOPFSAccessHandles', '$wasmfsOPFSProxyFinish'],
  _wasmfs_opfs_set_size_access: async function(ctx, accessID, size, errPtr) {
    let accessHandle = wasmfsOPFSAccessHandles.get(accessID);
    try {
      await accessHandle.truncate(size);
    } catch {
      let err = -{{{ cDefs.EIO }}};
      {{{ makeSetValue('errPtr', 0, 'err', 'i32') }}};
    }
    wasmfsOPFSProxyFinish(ctx);
  },

  _wasmfs_opfs_set_size_file__i53abi: true,
  _wasmfs_opfs_set_size_file__deps: ['$wasmfsOPFSFileHandles', '$wasmfsOPFSProxyFinish'],
  _wasmfs_opfs_set_size_file: async function(ctx, fileID, size, errPtr) {
    let fileHandle = wasmfsOPFSFileHandles.get(fileID);
    try {
      let writable = await fileHandle.createWritable({keepExistingData: true});
      await writable.truncate(size);
      await writable.close();
    } catch {
      let err = -{{{ cDefs.EIO }}};
      {{{ makeSetValue('errPtr', 0, 'err', 'i32') }}};
    }
    wasmfsOPFSProxyFinish(ctx);
  },

  _wasmfs_opfs_flush_access__deps: ['$wasmfsOPFSAccessHandles', '$wasmfsOPFSProxyFinish'],
  _wasmfs_opfs_flush_access: async function(ctx, accessID, errPtr) {
    let accessHandle = wasmfsOPFSAccessHandles.get(accessID);
    try {
      await accessHandle.flush();
    } catch {
      let err = -{{{ cDefs.EIO }}};
      {{{ makeSetValue('errPtr', 0, 'err', 'i32') }}};
    }
    wasmfsOPFSProxyFinish(ctx);
  }
});
