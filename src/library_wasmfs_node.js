/**
 * @license
 * Copyright 2022 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

addToLibrary({
  $wasmfsNodeIsWindows: !!process.platform.match(/^win/),

  $wasmfsNodeConvertNodeCode__deps: ['$ERRNO_CODES'],
  $wasmfsNodeConvertNodeCode: (e) => {
    var code = e.code;
#if ASSERTIONS
    assert(code in ERRNO_CODES, 'unexpected node error code: ' + code + ' (' + e + ')');
#endif
    return ERRNO_CODES[code];
  },

  $wasmfsTry__deps: ['$wasmfsNodeConvertNodeCode'],
  $wasmfsTry: (f) => {
    try {
      return f();
    } catch (e) {
      if (!e.code) throw e;
      return wasmfsNodeConvertNodeCode(e);
    }
  },

  $wasmfsNodeFixStat__deps: ['$wasmfsNodeIsWindows'],
  $wasmfsNodeFixStat: (stat) => {
    if (wasmfsNodeIsWindows) {
      // Node.js on Windows never represents permission bit 'x', so
      // propagate read bits to execute bits
      stat.mode |= (stat.mode & {{{ cDefs.S_IRUSR | cDefs.S_IRGRP | cDefs.S_IROTH }}}) >> 2;
    }
    return stat;
  },

  $wasmfsNodeLstat__deps: ['$wasmfsNodeFixStat'],
  $wasmfsNodeLstat: (path) => {
    let stat;
    try {
      stat = fs.lstatSync(path);
    } catch (e) {
      if (!e.code) throw e;
      return undefined;
    }
    return wasmfsNodeFixStat(stat);
  },

  $wasmfsNodeFstat__deps: ['$wasmfsNodeFixStat'],
  $wasmfsNodeFstat: (fd) => {
    let stat;
    try {
      stat = fs.fstatSync(fd);
    } catch (e) {
      if (!e.code) throw e;
      return undefined;
    }
    return wasmfsNodeFixStat(stat);
  },

  // Ignore closure type errors due to outdated readdirSync annotations, see
  // https://github.com/google/closure-compiler/pull/4093
  _wasmfs_node_readdir__docs: '/** @suppress {checkTypes} */',
  _wasmfs_node_readdir__deps: [
    '$wasmfsTry',
    '$stackSave',
    '$stackRestore',
    '$stringToUTF8OnStack',
    '_wasmfs_node_record_dirent',
  ],
  _wasmfs_node_readdir: (path_p, vec) => {
    let path = UTF8ToString(path_p);
    return wasmfsTry(() => {
      let entries = fs.readdirSync(path, { withFileTypes: true });
      entries.forEach((entry) => {
        let sp = stackSave();
        let name = stringToUTF8OnStack(entry.name);
        let type;
        if (entry.isFile()) {
          type = {{{ cDefine('File::DataFileKind') }}};
        } else if (entry.isDirectory()) {
          type = {{{ cDefine('File::DirectoryKind') }}};
        } else if (entry.isSymbolicLink()) {
          type = {{{ cDefine('File::SymlinkKind') }}};
        } else {
          type = {{{ cDefine('File::UnknownKind') }}};
        }
        __wasmfs_node_record_dirent(vec, name, type);
        stackRestore(sp);
        // implicitly return 0
      });
    });
  },

  _wasmfs_node_get_mode__deps: ['$wasmfsNodeLstat'],
  _wasmfs_node_get_mode: (path_p, mode_p) => {
    let stat = wasmfsNodeLstat(UTF8ToString(path_p));
    if (stat === undefined) {
      return 1;
    }
    {{{ makeSetValue('mode_p', 0, 'stat.mode', 'i32') }}};
    // implicitly return 0
  },

  _wasmfs_node_stat_size__deps: ['$wasmfsNodeLstat'],
  _wasmfs_node_stat_size: (path_p, size_p) => {
    let stat = wasmfsNodeLstat(UTF8ToString(path_p));
    if (stat === undefined) {
      return 1;
    }
    {{{ makeSetValue('size_p', 0, 'stat.size', 'i32') }}};
    // implicitly return 0
  },

  _wasmfs_node_fstat_size__deps: ['$wasmfsNodeFstat'],
  _wasmfs_node_fstat_size: (fd, size_p) => {
    let stat = wasmfsNodeFstat(fd);
    if (stat === undefined) {
      return 1;
    }
    {{{ makeSetValue('size_p', 0, 'stat.size', 'i32') }}};
    // implicitly return 0
  },

  _wasmfs_node_insert_file__deps: ['$wasmfsTry'],
  _wasmfs_node_insert_file: (path_p, mode) => {
    return wasmfsTry(() => {
      fs.closeSync(fs.openSync(UTF8ToString(path_p), 'ax', mode));
      // implicitly return 0
    });
  },

  _wasmfs_node_insert_directory__deps: ['$wasmfsTry'],
  _wasmfs_node_insert_directory: (path_p, mode) => {
    return wasmfsTry(() => {
      fs.mkdirSync(UTF8ToString(path_p), mode)
      // implicitly return 0
    });
  },

  _wasmfs_node_unlink__deps: ['$wasmfsTry'],
  _wasmfs_node_unlink: (path_p) => {
    return wasmfsTry(() => {
      fs.unlinkSync(UTF8ToString(path_p))
      // implicitly return 0
    });
  },

  _wasmfs_node_rmdir__deps: ['$wasmfsTry'],
  _wasmfs_node_rmdir: (path_p) => {
    return wasmfsTry(() => {
      fs.rmdirSync(UTF8ToString(path_p))
      // implicitly return 0
    });
  },

  _wasmfs_node_open__deps: ['$wasmfsTry'],
  _wasmfs_node_open: (path_p, mode_p) => {
    return wasmfsTry(() =>
      fs.openSync(UTF8ToString(path_p), UTF8ToString(mode_p))
    );
  },

  _wasmfs_node_close__deps: [],
  _wasmfs_node_close: (fd) => {
    return wasmfsTry(() => {
      fs.closeSync(fd);
      // implicitly return 0
    });
  },

  _wasmfs_node_read__deps: ['$wasmfsTry'],
  _wasmfs_node_read: (fd, buf_p, len, pos, nread_p) => {
    return wasmfsTry(() => {
      // TODO: Cache open file descriptors to guarantee that opened files will
      // still exist when we try to access them.
      let nread = fs.readSync(fd, new Int8Array(HEAPU8.buffer, buf_p, len), 0, len, pos);
      {{{ makeSetValue('nread_p', 0, 'nread', 'i32') }}};
      // implicitly return 0
    });
  },

  _wasmfs_node_write__deps : ['$wasmfsTry'],
  _wasmfs_node_write : (fd, buf_p, len, pos, nwritten_p) => {
    return wasmfsTry(() => {
      // TODO: Cache open file descriptors to guarantee that opened files will
      // still exist when we try to access them.
      let nwritten = fs.writeSync(fd, new Int8Array(HEAPU8.buffer, buf_p, len), 0, len, pos);
      {{{ makeSetValue('nwritten_p', 0, 'nwritten', 'i32') }}};
      // implicitly return 0
    });
  },
});
