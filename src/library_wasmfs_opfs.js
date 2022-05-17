/**
 * @license
 * Copyright 2022 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

mergeInto(LibraryManager.library, {
  // TODO: Generate these ID pools from a common utility.
  $wasmfsOPFSDirectories: {
    allocated: [],
    free: [],
    get: function(i) {
      assert(this.allocated[i] !== undefined);
      return this.allocated[i];
    }
  },

  $wasmfsOPFSFiles: {
    allocated: [],
    free: [],
    get: function(i) {
      assert(this.allocated[i] !== undefined);
      return this.allocated[i];
    }
  },

  $wasmfsOPFSAccesses: {
    allocated: [],
    free: [],
    get: function(i) {
      assert(this.allocated[i] !== undefined);
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
    delete ids.allocated[id];
    ids.free.push(id);
  },

  _wasmfs_opfs_init_root_directory__deps: ['$wasmfsOPFSDirectories'],
  _wasmfs_opfs_init_root_directory: async function(ctx) {
    if (wasmfsOPFSDirectories.allocated.length == 0) {
      // Directory 0 is reserved as the root
      let root = await navigator.storage.getDirectory();
      wasmfsOPFSDirectories.allocated.push(root);
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
                                     '$wasmfsOPFSDirectories',
                                     '$wasmfsOPFSFiles'],
  $wasmfsOPFSGetOrCreateFile: async function(parent, name, create) {
    let parentHandle = wasmfsOPFSDirectories.get(parent);
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
    return wasmfsOPFSAllocate(wasmfsOPFSFiles, fileHandle);
  },

  // Return the file ID for the directory with `name` under `parent`, creating
  // it if it doesn't exist and `create` or otherwise one of the following error
  // codes:
  //
  // -1: directory does not exist.
  // -2: directory exists but is actually a data file.
  $wasmfsOPFSGetOrCreateDir__deps: ['$wasmfsOPFSAllocate',
                                    '$wasmfsOPFSDirectories'],
  $wasmfsOPFSGetOrCreateDir: async function(parent, name, create) {
    let parentHandle = wasmfsOPFSDirectories.get(parent);
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
    return wasmfsOPFSAllocate(wasmfsOPFSDirectories, childHandle);
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
    let dirHandle = wasmfsOPFSDirectories.get(dirID);
    for await (let [name, child] of dirHandle.entries()) {
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
  _wasmfs_opfs_insert_directory: async function(ctx, parent, namePtr, childIDPtr) {
    let name = UTF8ToString(namePtr);
    let childID = await wasmfsOPFSGetOrCreateDir(parent, name, true);
    {{{ makeSetValue('childIDPtr', 0, 'childID', 'i32') }}};
    _emscripten_proxy_finish(ctx);
  },

  _wasmfs_opfs_move__deps: ['$wasmfsOPFSFiles', '$wasmfsOPFSDirectories'],
  _wasmfs_opfs_move: async function(ctx, fileID, newDirID, namePtr) {
    let name = UTF8ToString(namePtr);
    let fileHandle = wasmfsOPFSFiles.get(fileID);
    let newDirHandle = wasmfsOPFSDirectories.get(newDirID);
    await fileHandle.move(newDirHandle, name);
    _emscripten_proxy_finish(ctx);
  },

  _wasmfs_opfs_remove_child__deps: ['$wasmfsOPFSFree', '$wasmfsOPFSDirectories'],
  _wasmfs_opfs_remove_child: async function(ctx, dirID, namePtr) {
    let name = UTF8ToString(namePtr);
    let dirHandle = wasmfsOPFSDirectories.get(dirID);
    await dirHandle.removeEntry(name);
    _emscripten_proxy_finish(ctx);
  },

  _wasmfs_opfs_free_file__deps: ['$wasmfsOPFSFree', '$wasmfsOPFSFiles'],
  _wasmfs_opfs_free_file: function(fileID) {
    wasmfsOPFSFree(wasmfsOPFSFiles, fileID);
  },

  _wasmfs_opfs_free_directory__deps: ['$wasmfsOPFSFree',
                                      '$wasmfsOPFSDirectories'],
  _wasmfs_opfs_free_directory: function(dirID) {
    wasmfsOPFSFree(wasmfsOPFSDirectories, dirID);
  },

  _wasmfs_opfs_open__deps: ['$wasmfsOPFSAllocate',
                            '$wasmfsOPFSFiles',
                            '$wasmfsOPFSAccesses'],
  _wasmfs_opfs_open: async function(ctx, fileID, accessIDPtr) {
    let fileHandle = wasmfsOPFSFiles.get(fileID);
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
      accessID = wasmfsOPFSAllocate(wasmfsOPFSAccesses, accessHandle);
    } catch (e) {
      if (e.name === "InvalidStateError") {
        accessID = -1;
      }
      throw e;
    }
    {{{ makeSetValue('accessIDPtr', 0, 'accessID', 'i32') }}};
    _emscripten_proxy_finish(ctx);
  },

  _wasmfs_opfs_close__deps: ['$wasmfsOPFSFree', '$wasmfsOPFSAccesses'],
  _wasmfs_opfs_close: async function(ctx, accessID) {
    let accessHandle = wasmfsOPFSAccesses.get(accessID);
    await accessHandle.close();
    wasmfsOPFSFree(wasmfsOPFSAccesses, accessID);
    _emscripten_proxy_finish(ctx);
  },

  _wasmfs_opfs_read__deps: ['$wasmfsOPFSAccesses'],
  _wasmfs_opfs_read: function(accessID, bufPtr, len, pos) {
    let accessHandle = wasmfsOPFSAccesses.get(accessID);
    let data = HEAPU8.subarray(bufPtr, bufPtr + len);
    return accessHandle.read(data, {at: pos});
  },

  _wasmfs_opfs_write__deps: ['$wasmfsOPFSAccesses'],
  _wasmfs_opfs_write: function(accessID, bufPtr, len, pos, nwrittenPtr) {
    let accessHandle = wasmfsOPFSAccesses.get(accessID);
    let data = HEAPU8.subarray(bufPtr, bufPtr + len);
    return accessHandle.write(data, {at: pos});
  },

  _wasmfs_opfs_get_size__deps: ['$wasmfsOPFSAccesses'],
  _wasmfs_opfs_get_size: async function(ctx, accessID, sizePtr) {
    let accessHandle = wasmfsOPFSAccesses.get(accessID);
    let size = await accessHandle.getSize();
    {{{ makeSetValue('sizePtr', 0, 'size', 'i32') }}};
    _emscripten_proxy_finish(ctx);
  },

  _wasmfs_opfs_set_size__deps: ['$wasmfsOPFSAccesses'],
  _wasmfs_opfs_set_size: async function(ctx, accessID, size) {
    let accessHandle = wasmfsOPFSAccesses.get(accessID);
    await accessHandle.truncate(size);
    _emscripten_proxy_finish(ctx);
  },

  _wasmfs_opfs_flush__deps: ['$wasmfsOPFSAccesses'],
  _wasmfs_opfs_flush: async function(ctx, accessID) {
    let accessHandle = wasmfsOPFSAccesses.get(accessID);
    await accessHandle.flush();
    _emscripten_proxy_finish(ctx);
  }
});
