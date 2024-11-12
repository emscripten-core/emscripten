/**
 * @license
 * Copyright 2015 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

#if LZ4
addToLibrary({
  $LZ4__deps: ['$FS', '$preloadPlugins'],
  $LZ4: {
    DIR_MODE: {{{ cDefs.S_IFDIR }}} | 511 /* 0777 */,
    FILE_MODE: {{{ cDefs.S_IFREG }}} | 511 /* 0777 */,
    CHUNK_SIZE: -1,
    codec: null,
    init() {
      if (LZ4.codec) return;
      LZ4.codec = (() => {
        {{{ read('../third_party/mini-lz4.js') }}};
        return MiniLZ4;
      })();
      LZ4.CHUNK_SIZE = LZ4.codec.CHUNK_SIZE;
    },
    loadPackage(pack, preloadPlugin) {
      LZ4.init();
      var compressedData = pack['compressedData'] || LZ4.codec.compressPackage(pack['data']);
      assert(compressedData['cachedIndexes'].length === compressedData['cachedChunks'].length);
      for (var i = 0; i < compressedData['cachedIndexes'].length; i++) {
        compressedData['cachedIndexes'][i] = -1;
        compressedData['cachedChunks'][i] = compressedData['data'].subarray(compressedData['cachedOffset'] + i*LZ4.CHUNK_SIZE,
                                                                      compressedData['cachedOffset'] + (i+1)*LZ4.CHUNK_SIZE);
        assert(compressedData['cachedChunks'][i].length === LZ4.CHUNK_SIZE);
      }
      pack['metadata'].files.forEach((file) => {
        var dir = PATH.dirname(file.filename);
        var name = PATH.basename(file.filename);
        FS.createPath('', dir, true, true);
        var parent = FS.analyzePath(dir).object;
        LZ4.createNode(parent, name, LZ4.FILE_MODE, 0, {
          compressedData,
          start: file.start,
          end: file.end,
        });
      });
      // Preload files if necessary. This code is largely similar to
      // createPreloadedFile in library_fs.js. However, a main difference here
      // is that we only decompress the file if it can be preloaded.
      // Abstracting out the common parts seems to be more effort than it is
      // worth.
      if (preloadPlugin) {
        Browser.init();
        pack['metadata'].files.forEach((file) => {
          var handled = false;
          var fullname = file.filename;
          preloadPlugins.forEach((plugin) => {
            if (handled) return;
            if (plugin['canHandle'](fullname)) {
              var dep = getUniqueRunDependency('fp ' + fullname);
              addRunDependency(dep);
              var finish = () => removeRunDependency(dep);
              var byteArray = FS.readFile(fullname);
              plugin['handle'](byteArray, fullname, finish, finish);
              handled = true;
            }
          });
        });
      }
    },
    createNode(parent, name, mode, dev, contents, mtime) {
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
      getattr(node) {
        return {
          dev: 1,
          ino: node.id,
          mode: node.mode,
          nlink: 1,
          uid: 0,
          gid: 0,
          rdev: 0,
          size: node.size,
          atime: new Date(node.timestamp),
          mtime: new Date(node.timestamp),
          ctime: new Date(node.timestamp),
          blksize: 4096,
          blocks: Math.ceil(node.size / 4096),
        };
      },
      setattr(node, attr) {
        if (attr.mode !== undefined) {
          node.mode = attr.mode;
        }
        if (attr.timestamp !== undefined) {
          node.timestamp = attr.timestamp;
        }
      },
      lookup(parent, name) {
        throw new FS.ErrnoError({{{ cDefs.ENOENT }}});
      },
      mknod(parent, name, mode, dev) {
        throw new FS.ErrnoError({{{ cDefs.EPERM }}});
      },
      rename(oldNode, newDir, newName) {
        throw new FS.ErrnoError({{{ cDefs.EPERM }}});
      },
      unlink(parent, name) {
        throw new FS.ErrnoError({{{ cDefs.EPERM }}});
      },
      rmdir(parent, name) {
        throw new FS.ErrnoError({{{ cDefs.EPERM }}});
      },
      readdir(node) {
        throw new FS.ErrnoError({{{ cDefs.EPERM }}});
      },
      symlink(parent, newName, oldPath) {
        throw new FS.ErrnoError({{{ cDefs.EPERM }}});
      },
    },
    stream_ops: {
      read(stream, buffer, offset, length, position) {
        //out('LZ4 read ' + [offset, length, position]);
        length = Math.min(length, stream.node.size - position);
        if (length <= 0) return 0;
        var contents = stream.node.contents;
        var compressedData = contents.compressedData;
        var written = 0;
        while (written < length) {
          var start = contents.start + position + written; // start index in uncompressed data
          var desired = length - written;
          //out('current read: ' + ['start', start, 'desired', desired]);
          var chunkIndex = Math.floor(start / LZ4.CHUNK_SIZE);
          var compressedStart = compressedData['offsets'][chunkIndex];
          var compressedSize = compressedData['sizes'][chunkIndex];
          var currChunk;
          if (compressedData['successes'][chunkIndex]) {
            var found = compressedData['cachedIndexes'].indexOf(chunkIndex);
            if (found >= 0) {
              currChunk = compressedData['cachedChunks'][found];
            } else {
              // decompress the chunk
              compressedData['cachedIndexes'].pop();
              compressedData['cachedIndexes'].unshift(chunkIndex);
              currChunk = compressedData['cachedChunks'].pop();
              compressedData['cachedChunks'].unshift(currChunk);
              if (compressedData['debug']) {
                out('decompressing chunk ' + chunkIndex);
                Module['decompressedChunks'] = (Module['decompressedChunks'] || 0) + 1;
              }
              var compressed = compressedData['data'].subarray(compressedStart, compressedStart + compressedSize);
              //var t = Date.now();
              var originalSize = LZ4.codec.uncompress(compressed, currChunk);
              //out('decompress time: ' + (Date.now() - t));
              if (chunkIndex < compressedData['successes'].length-1) assert(originalSize === LZ4.CHUNK_SIZE); // all but the last chunk must be full-size
            }
          } else {
            // uncompressed
            currChunk = compressedData['data'].subarray(compressedStart, compressedStart + LZ4.CHUNK_SIZE);
          }
          var startInChunk = start % LZ4.CHUNK_SIZE;
          var endInChunk = Math.min(startInChunk + desired, LZ4.CHUNK_SIZE);
          buffer.set(currChunk.subarray(startInChunk, endInChunk), offset + written);
          var currWritten = endInChunk - startInChunk;
          written += currWritten;
        }
        return written;
      },
      write(stream, buffer, offset, length, position) {
        throw new FS.ErrnoError({{{ cDefs.EIO }}});
      },
      llseek(stream, offset, whence) {
        var position = offset;
        if (whence === {{{ cDefs.SEEK_CUR }}}) {
          position += stream.position;
        } else if (whence === {{{ cDefs.SEEK_END }}}) {
          if (FS.isFile(stream.node.mode)) {
            position += stream.node.size;
          }
        }
        if (position < 0) {
          throw new FS.ErrnoError({{{ cDefs.EINVAL }}});
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

