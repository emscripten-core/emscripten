// TODO: put behind a flag
mergeInto(LibraryManager.library, {
  $LZ4FS__deps: ['$FS'],
  $LZ4FS: {
    DIR_MODE: {{{ cDefine('S_IFDIR') }}} | 511 /* 0777 */,
    FILE_MODE: {{{ cDefine('S_IFREG') }}} | 511 /* 0777 */,
    CHUNK_SIZE: -1,
    LZ4: null,
    mount: function (mount) {
      if (!LZ4FS.LZ4) {
        LZ4FS.LZ4 = (function() {
          {{{ read('mini-lz4.js') }}};
          return MiniLZ4;
        })();
        LZ4FS.CHUNK_SIZE = LZ4FS.LZ4.CHUNK_SIZE;
      }
      var root = LZ4FS.createNode(null, '/', LZ4FS.DIR_MODE, 0);
      var createdParents = {};
      function ensureParent(path) {
        // return the parent node, creating subdirs as necessary
        var parts = path.split('/');
        var parent = root;
        for (var i = 0; i < parts.length-1; i++) {
          var curr = parts.slice(0, i+1).join('/');
          if (!createdParents[curr]) {
            createdParents[curr] = LZ4FS.createNode(parent, curr, LZ4FS.DIR_MODE, 0);
          }
          parent = createdParents[curr];
        }
        return parent;
      }
      function base(path) {
        var parts = path.split('/');
        return parts[parts.length-1];
      }
      mount.opts["packages"].forEach(function(pack) {
        var compressedData = pack['compressedData'];
        if (!compressedData) compressedData = LZ4FS.LZ4.compressPackage(pack['data']);
        assert(compressedData.cachedIndexes.length === compressedData.cachedChunks.length);
        for (var i = 0; i < compressedData.cachedIndexes.length; i++) {
          compressedData.cachedIndexes[i] = -1;
          compressedData.cachedChunks[i] = compressedData.data.subarray(compressedData.cachedOffset + i*LZ4FS.CHUNK_SIZE,
                                                                        compressedData.cachedOffset + (i+1)*LZ4FS.CHUNK_SIZE);
          assert(compressedData.cachedChunks[i].length === LZ4FS.CHUNK_SIZE);
        }
        console.log('mounting package');
        pack['metadata'].files.forEach(function(file) {
          var name = file.filename.substr(1); // remove initial slash
          LZ4FS.createNode(ensureParent(name), base(name), LZ4FS.FILE_MODE, 0, {
            compressedData: compressedData,
            start: file.start,
            end: file.end,
          });
        });
      });
      return root;
    },
    createNode: function (parent, name, mode, dev, contents, mtime) {
      var node = FS.createNode(parent, name, mode);
      node.mode = mode;
      node.node_ops = LZ4FS.node_ops;
      node.stream_ops = LZ4FS.stream_ops;
      node.timestamp = (mtime || new Date).getTime();
      assert(LZ4FS.FILE_MODE !== LZ4FS.DIR_MODE);
      if (mode === LZ4FS.FILE_MODE) {
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
        //console.log('LZ4FS read ' + [offset, length, position]);
        length = Math.min(length, stream.node.size - position);
        if (length <= 0) return 0;
        var contents = stream.node.contents;
        var compressedData = contents.compressedData;
        var written = 0;
        while (written < length) {
          var start = contents.start + position + written; // start index in uncompressed data
          var desired = length - written;
          //console.log('current read: ' + ['start', start, 'desired', desired]);
          var chunkIndex = Math.floor(start / LZ4FS.CHUNK_SIZE);
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
              var originalSize = LZ4FS.LZ4.uncompress(compressed, currChunk);
              //console.log('decompress time: ' + (Date.now() - t));
              assert(originalSize === LZ4FS.CHUNK_SIZE);
            }
          } else {
            // uncompressed
            currChunk = compressedData.data.subarray(compressedStart, compressedStart + LZ4FS.CHUNK_SIZE);
          }
          var startInChunk = start % LZ4FS.CHUNK_SIZE;
          var endInChunk = Math.min(startInChunk + desired, LZ4FS.CHUNK_SIZE);
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
