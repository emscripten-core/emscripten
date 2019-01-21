// Copyright 2015 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#if LZ4
mergeInto(LibraryManager.library, {
  $LZ4__deps: ['$FS'],
  $LZ4: {
    DIR_MODE: {{{ cDefine('S_IFDIR') }}} | 511 /* 0777 */,
    FILE_MODE: {{{ cDefine('S_IFREG') }}} | 511 /* 0777 */,
    CHUNK_SIZE: -1,
    codec: null,
    init: function() {
      if (LZ4.codec) return;
      LZ4.codec = (function() {
        {{{ read('mini-lz4.js') }}};
        return MiniLZ4;
      })();
      LZ4.CHUNK_SIZE = LZ4.codec.CHUNK_SIZE;
    },
    loadPackage: function (pack) {
      LZ4.init();
      var compressedData = pack['compressedData'];
      if (!compressedData) compressedData = LZ4.codec.compressPackage(pack['data']);
      assert(compressedData.cachedIndexes.length === compressedData.cachedChunks.length);
      for (var i = 0; i < compressedData.cachedIndexes.length; i++) {
        compressedData.cachedIndexes[i] = -1;
        compressedData.cachedChunks[i] = compressedData.data.subarray(compressedData.cachedOffset + i*LZ4.CHUNK_SIZE,
                                                                      compressedData.cachedOffset + (i+1)*LZ4.CHUNK_SIZE);
        assert(compressedData.cachedChunks[i].length === LZ4.CHUNK_SIZE);
      }
      pack['metadata'].files.forEach(function(file) {
        var dir = PATH.dirname(file.filename);
        var name = PATH.basename(file.filename);
        FS.createPath('', dir, true, true);
        var parent = FS.analyzePath(dir).object;
        LZ4.createNode(parent, name, LZ4.FILE_MODE, 0, {
          compressedData: compressedData,
          start: file.start,
          end: file.end,
        });
      });
    },
    createNode: function (parent, name, mode, dev, contents, mtime) {
      var node = FS.createNode(parent, name, mode);
      node.mode = mode;
      node.node_ops = LZ4.node_ops;
      node.stream_ops = LZ4.stream_ops;
      node.timestamp = (mtime || new Date).getTime();
      assert(LZ4.FILE_MODE !== LZ4.DIR_MODE);
      if (mode === LZ4.FILE_MODE) {
        node.size = contents.end - contents.start;
        node.contents = contents;
      } else {
        node.size = 4096;
        node.contents = {};
      }
      if (parent) {
        parent.contents[name] = node;
      }
      return node;
    },
    node_ops: {
      getattr: function(node) {
        return {
          dev: 1,
          ino: undefined,
          mode: node.mode,
          nlink: 1,
          uid: 0,
          gid: 0,
          rdev: undefined,
          size: node.size,
          atime: new Date(node.timestamp),
          mtime: new Date(node.timestamp),
          ctime: new Date(node.timestamp),
          blksize: 4096,
          blocks: Math.ceil(node.size / 4096),
        };
      },
      setattr: function(node, attr) {
        if (attr.mode !== undefined) {
          node.mode = attr.mode;
        }
        if (attr.timestamp !== undefined) {
          node.timestamp = attr.timestamp;
        }
      },
      lookup: function(parent, name) {
        throw new FS.ErrnoError(ERRNO_CODES.ENOENT);
      },
      mknod: function (parent, name, mode, dev) {
        throw new FS.ErrnoError(ERRNO_CODES.EPERM);
      },
      rename: function (oldNode, newDir, newName) {
        throw new FS.ErrnoError(ERRNO_CODES.EPERM);
      },
      unlink: function(parent, name) {
        throw new FS.ErrnoError(ERRNO_CODES.EPERM);
      },
      rmdir: function(parent, name) {
        throw new FS.ErrnoError(ERRNO_CODES.EPERM);
      },
      readdir: function(node) {
        throw new FS.ErrnoError(ERRNO_CODES.EPERM);
      },
      symlink: function(parent, newName, oldPath) {
        throw new FS.ErrnoError(ERRNO_CODES.EPERM);
      },
      readlink: function(node) {
        throw new FS.ErrnoError(ERRNO_CODES.EPERM);
      },
    },
    stream_ops: {
      read: function (stream, buffer, offset, length, position) {
        //console.log('LZ4 read ' + [offset, length, position]);
        length = Math.min(length, stream.node.size - position);
        if (length <= 0) return 0;
        var contents = stream.node.contents;
        var compressedData = contents.compressedData;
        var written = 0;
        while (written < length) {
          var start = contents.start + position + written; // start index in uncompressed data
          var desired = length - written;
          //console.log('current read: ' + ['start', start, 'desired', desired]);
          var chunkIndex = Math.floor(start / LZ4.CHUNK_SIZE);
          var compressedStart = compressedData.offsets[chunkIndex];
          var compressedSize = compressedData.sizes[chunkIndex];
          var currChunk;
          if (compressedData.successes[chunkIndex]) {
            var found = compressedData.cachedIndexes.indexOf(chunkIndex);
            if (found >= 0) {
              currChunk = compressedData.cachedChunks[found];
            } else {
              // decompress the chunk
              compressedData.cachedIndexes.pop();
              compressedData.cachedIndexes.unshift(chunkIndex);
              currChunk = compressedData.cachedChunks.pop();
              compressedData.cachedChunks.unshift(currChunk);
              if (compressedData.debug) {
                console.log('decompressing chunk ' + chunkIndex);
                Module['decompressedChunks'] = (Module['decompressedChunks'] || 0) + 1;
              }
              var compressed = compressedData.data.subarray(compressedStart, compressedStart + compressedSize);
              //var t = Date.now();
              var originalSize = LZ4.codec.uncompress(compressed, currChunk);
              //console.log('decompress time: ' + (Date.now() - t));
              if (chunkIndex < compressedData.successes.length-1) assert(originalSize === LZ4.CHUNK_SIZE); // all but the last chunk must be full-size
            }
          } else {
            // uncompressed
            currChunk = compressedData.data.subarray(compressedStart, compressedStart + LZ4.CHUNK_SIZE);
          }
          var startInChunk = start % LZ4.CHUNK_SIZE;
          var endInChunk = Math.min(startInChunk + desired, LZ4.CHUNK_SIZE);
          buffer.set(currChunk.subarray(startInChunk, endInChunk), offset + written);
          var currWritten = endInChunk - startInChunk;
          written += currWritten;
        }
        return written;
      },
      write: function (stream, buffer, offset, length, position) {
        throw new FS.ErrnoError(ERRNO_CODES.EIO);
      },
      llseek: function (stream, offset, whence) {
        var position = offset;
        if (whence === 1) {  // SEEK_CUR.
          position += stream.position;
        } else if (whence === 2) {  // SEEK_END.
          if (FS.isFile(stream.node.mode)) {
            position += stream.node.size;
          }
        }
        if (position < 0) {
          throw new FS.ErrnoError(ERRNO_CODES.EINVAL);
        }
        return position;
      },
    },
  },
});
if (LibraryManager.library['$FS__deps']) {
  LibraryManager.library['$FS__deps'].push('$LZ4'); // LZ4=1, so auto-include us
} else {
  warn('FS does not seem to be in use (no preloaded files etc.), LZ4 will not do anything');
}
#endif

