/**
 * @license
 * Copyright 2022 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

mergeInto(LibraryManager.library, {
  $wasmfsNodeIsWindows: !!process.platform.match(/^win/),

  $wasmfsNodeConvertNodeCode__deps: ['$ERRNO_CODES'],
  $wasmfsNodeConvertNodeCode: function(e) {
    var code = e.code;
#if ASSERTIONS
    assert(code in ERRNO_CODES, 'unexpected node error code: ' + code + ' (' + e + ')');
#endif
    return ERRNO_CODES[code];
  },

  $wasmfsNodeLstat__deps: ['$wasmfsNodeIsWindows'],
  $wasmfsNodeLstat: function(path) {
    let stat;
    try {
      stat = fs.lstatSync(path);
      if (wasmfsNodeIsWindows) {
        // Node.js on Windows never represents permission bit 'x', so
        // propagate read bits to execute bits
        stat.mode = stat.mode | ((stat.mode & 292) >> 2);
      }
      return stat;
    } catch (e) {
      if (!e.code) throw e;
      return undefined;
    }
  },

  _wasmfs_node_readdir__deps: ['$wasmfsNodeConvertNodeCode'],
  _wasmfs_node_readdir: function(path_p, vec) {
    let path = UTF8ToString(path_p);
    let entries;
    try {
      entries = fs.readdirSync(path, { withFileTypes: true });
    } catch (e) {
      if (!e.code) throw e;
      return wasmfsNodeConvertNodeCode(e);
    }
    for (let entry of entries) {
      withStackSave(() => {
        let name = allocateUTF8OnStack(entry.name);
        let type;
        // TODO: Figure out how to use `cDefine` here.
        if (entry.isFile()) {
          type = 1;
        } else if (entry.isDirectory()) {
          type = 2;
        } else if (entry.isSymbolicLink()) {
          type = 3;
        } else {
          type = 0;
        }
        __wasmfs_node_record_dirent(vec, name, type);
      });
    }
    // implicitly return 0
  },

  _wasmfs_node_get_mode__deps: ['$wasmfsNodeLstat'],
  _wasmfs_node_get_mode: function(path_p, mode_p) {
    let stat = wasmfsNodeLstat(UTF8ToString(path_p));
    if (stat === undefined) {
      return 1;
    }
    {{{ makeSetValue('mode_p', 0, 'stat.mode', 'i32') }}};
    // implicitly return 0
  },

  _wasmfs_node_insert_file__deps: ['$wasmfsNodeConvertNodeCode'],
  _wasmfs_node_insert_file: function(path_p, mode) {
    try {
      fs.closeSync(fs.openSync(UTF8ToString(path_p), 'ax', mode));
    } catch (e) {
      if (!e.code) throw e;
      return wasmfsNodeConvertNodeCode(e);
    }
    // implicitly return 0
  },

  _wasmfs_node_insert_directory__deps: ['$wasmfsNodeConvertNodeCode'],
  _wasmfs_node_insert_directory: function(path_p, mode) {
    try {
      fs.mkdirSync(UTF8ToString(path_p), { mode: mode });
    } catch (e) {
      if (!e.code) throw e;
      return wasmfsNodeConvertNodeCode(e);
    }
    // implicitly return 0
  },

  _wasmfs_node_get_file_size__deps: ['$wasmfsNodeLstat'],
  _wasmfs_node_get_file_size: function(path_p, size_p) {
    let stat = wasmfsNodeLstat(UTF8ToString(path_p));
    if (stat === undefined) {
      return 1;
    }
    {{{ makeSetValue('size_p', 0, 'stat.size', 'i32') }}};
    // implicitly return 0
  },

  _wasmfs_node_read__deps: ['$wasmfsNodeConvertNodeCode'],
  _wasmfs_node_read: function(path_p, buf_p, len, pos, nread_p) {
    let fd;
    try {
      // TODO: Cache open file descriptors to guarantee that opened files will
      // still exist when we try to access them.
      fd = fs.openSync(UTF8ToString(path_p));
      let nread = fs.readSync(fd, HEAPU8, buf_p, len, pos);
      {{{ makeSetValue('nread_p', 0, 'nread', 'i32') }}};
    } catch (e) {
      if (!e.code) throw e;
      return wasmfsNodeConvertNodeCode(e);
    } finally {
      if (fd !== undefined) {
        fs.closeSync(fd);
      }
    }
    // implicitly return 0
  },

  _wasmfs_node_write__deps : ['$wasmfsNodeConvertNodeCode'],
  _wasmfs_node_write : function(path_p, buf_p, len, pos, nwritten_p) {
    let fd;
    try {
      // TODO: Cache open file descriptors to guarantee that opened files will
      // still exist when we try to access them.
      fd = fs.openSync(UTF8ToString(path_p), 'w');
      let nwritten = fs.writeSync(fd, HEAPU8, buf_p, len, pos);
      {{{ makeSetValue('nwritten_p', 0, 'nwritten', 'i32') }}};
    } catch (e) {
      if (!e.code) throw e;
      return wasmfsNodeConvertNodeCode(e);
    } finally {
      if (fd !== undefined) {
        fs.closeSync(fd);
      }
    }
    // implicitly return 0
  },
});
