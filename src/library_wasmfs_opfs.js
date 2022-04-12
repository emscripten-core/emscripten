/**
 * @license
 * Copyright 2022 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

mergeInto(LibraryManager.library, {
  $wasmfsOPFSDirectories: {
    allocated: [],
    free: [],
    get: function(i) {
      return this.allocated[i];
    }
  },

  $wasmfsOPFSFiles: {
    allocated: [],
    free: [],
    get: function(i) {
      return this.allocated[i];
    }
  },

  $wasmfsOPFSAccesses: {
    allocated: [],
    free: [],
    get: function(i) {
      return this.allocated[i];
    }
  },

  $wasmfsOPFSAllocate: function(ids, handle) {
    let id = ids.allocated.length;
    if (ids.free.length > 0) {
      id = ids.free.pop();
    }
    assert(ids.allocated[id] === undefined);
    ids.allocated[id] = handle;
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
    let parent_handle = wasmfsOPFSDirectories.get(parent);
    assert(parent_handle !== undefined);
    let file_handle;
    try {
      file_handle = await parent_handle.getFileHandle(name, {create: create});
    } catch (err) {
      if (err.name === "NotFoundError") {
        return -1;
      }
      if (err.name === "TypeMismatchError") {
        return -2;
      }
      abort("Unknown exception " + err.name);
    }
    return wasmfsOPFSAllocate(wasmfsOPFSFiles, file_handle);
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
    let parent_handle = wasmfsOPFSDirectories.get(parent);
    assert(parent_handle !== undefined);
    let child_handle;
    try {
      child_handle =
          await parent_handle.getDirectoryHandle(name, {create: create});
    } catch (err) {
      if (err.name === "NotFoundError") {
        return -1;
      }
      if (err.name === "TypeMismatchError") {
        return -2;
      }
      abort("Unknown exception " + err.name);
    }
    return wasmfsOPFSAllocate(wasmfsOPFSDirectories, child_handle);
  },

  _wasmfs_opfs_get_child__deps: ['$wasmfsOPFSGetOrCreateFile',
                                 '$wasmfsOPFSGetOrCreateDir'],
  _wasmfs_opfs_get_child:
      async function(ctx, parent, name_p, child_type_p, child_id_p) {
    let name = UTF8ToString(name_p);
    let child_type = 1;
    let child_id = await wasmfsOPFSGetOrCreateFile(parent, name, false);
    if (child_id == -2) {
      child_type = 2;
      child_id = await wasmfsOPFSGetOrCreateDir(parent, name, false);
    }
    {{{ makeSetValue('child_type_p', 0, 'child_type', 'i32') }}};
    {{{ makeSetValue('child_id_p', 0, 'child_id', 'i32') }}};
    _emscripten_proxy_finish(ctx);
  },

  _wasmfs_opfs_get_entries__deps: [],
  _wasmfs_opfs_get_entries: async function(ctx, dir_id, entries) {
    let dir_handle = wasmfsOPFSDirectories.get(dir_id);
    for await (const [name, child] of dir_handle.entries()) {
      withStackSave(() => {
        let name_p = allocateUTF8OnStack(name);
        // TODO: Figure out how to use `cDefine` here
        let type = child.kind == "file" ? 1 : 2;
        __wasmfs_opfs_record_entry(entries, name_p, type)
      });
    }
    _emscripten_proxy_finish(ctx);
  },

  _wasmfs_opfs_insert_file__deps: ['$wasmfsOPFSGetOrCreateFile'],
  _wasmfs_opfs_insert_file: async function(ctx, parent, name_p, child_id_p) {
    let name = UTF8ToString(name_p);
    let child_id = await wasmfsOPFSGetOrCreateFile(parent, name, true);
    {{{ makeSetValue('child_id_p', 0, 'child_id', 'i32') }}};
    _emscripten_proxy_finish(ctx);
  },

  _wasmfs_opfs_insert_directory__deps: ['$wasmfsOPFSGetOrCreateDir'],
  _wasmfs_opfs_insert_directory: async function(ctx, parent, name_p, child_id_p) {
    let name = UTF8ToString(name_p);
    let child_id = await wasmfsOPFSGetOrCreateDir(parent, name, true);
    {{{ makeSetValue('child_id_p', 0, 'child_id', 'i32') }}};
    _emscripten_proxy_finish(ctx);
  },

  _wasmfs_opfs_remove_child__deps: ['$wasmfsOPFSFree', '$wasmfsOPFSDirectories'],
  _wasmfs_opfs_remove_child: async function(ctx, dir_id, name_p) {
    let name = UTF8ToString(name_p);
    let dir_handle = wasmfsOPFSDirectories.get(dir_id);
    await dir_handle.removeEntry(name);
    _emscripten_proxy_finish(ctx);
  },

  _wasmfs_opfs_free_file__deps: ['$wasmfsOPFSFree', '$wasmfsOPFSFiles'],
  _wasmfs_opfs_free_file: function(file_id) {
    wasmfsOPFSFree(wasmfsOPFSFiles, file_id);
  },

  _wasmfs_opfs_free_directory__deps: ['$wasmfsOPFSFree',
                                      '$wasmfsOPFSDirectories'],
  _wasmfs_opfs_free_directory: function(dir_id) {
    wasmfsOPFSFree(wasmfsOPFSDirectories, dir_id);
  },

  _wasmfs_opfs_open__deps: ['$wasmfsOPFSAllocate',
                            '$wasmfsOPFSFiles',
                            '$wasmfsOPFSAccesses'],
  _wasmfs_opfs_open: async function(ctx, file_id, access_id_p) {
    let file_handle = wasmfsOPFSFiles.get(file_id);
    let access_id;
    try {
      let access_handle = await file_handle.createSyncAccessHandle();
      access_id = wasmfsOPFSAllocate(wasmfsOPFSAccesses, access_handle);
    } catch (err) {
      if (err.name === "InvalidStateError") {
        access_id = -1;
      }
      abort("Unknown error opening file");
    }
    {{{ makeSetValue('access_id_p', 0, 'access_id', 'i32') }}};
    _emscripten_proxy_finish(ctx);
  },

  _wasmfs_opfs_close__deps: ['$wasmfsOPFSFree', '$wasmfsOPFSAccesses'],
  _wasmfs_opfs_close: async function(ctx, access_id) {
    let access_handle = wasmfsOPFSAccesses.get(access_id);
    await access_handle.close();
    wasmfsOPFSFree(wasmfsOPFSAccesses, access_id);
    _emscripten_proxy_finish(ctx);
  },

  _wasmfs_opfs_read__deps: ['$wasmfsOPFSAccesses'],
  _wasmfs_opfs_read: function(access_id, buf_p, len, pos) {
    let access_handle = wasmfsOPFSAccesses.get(access_id);
    let data = HEAPU8.subarray(buf_p, buf_p + len);
    return access_handle.read(data, {at: pos});
  },

  _wasmfs_opfs_write__deps: ['$wasmfsOPFSAccesses'],
  _wasmfs_opfs_write: function(access_id, buf_p, len, pos, nwritten_p) {
    let access_handle = wasmfsOPFSAccesses.get(access_id);
    let data = HEAPU8.subarray(buf_p, buf_p + len);
    return access_handle.write(data, {at: pos});
  },

  _wasmfs_opfs_get_size__deps: ['$wasmfsOPFSAccesses'],
  _wasmfs_opfs_get_size: async function(ctx, access_id, size_p) {
    let access_handle = wasmfsOPFSAccesses.get(access_id);
    let size = await access_handle.getSize();
    {{{ makeSetValue('size_p', 0, 'size', 'i32') }}};
    _emscripten_proxy_finish(ctx);
  },

  _wasmfs_opfs_set_size__deps: ['$wasmfsOPFSAccesses'],
  _wasmfs_opfs_set_size: async function(ctx, access_id, size) {
    let access_handle = wasmfsOPFSAccesses.get(access_id);
    await access_handle.truncate(size);
    _emscripten_proxy_finish(ctx);
  },

  _wasmfs_opfs_flush__deps: ['$wasmfsOPFSAccesses'],
  _wasmfs_opfs_flush: async function(ctx, access_id) {
    let access_handle = wasmfsOPFSAccesses.get(access_id);
    await access_handle.flush();
    _emscripten_proxy_finish(ctx);
  }
});
