mergeInto(LibraryManager.library, {
  $MEMFS__deps: ['$FS'],
  $MEMFS: {
    mount: function(mount) {
      return MEMFS.create_node(null, '/', {{{ cDefine('S_IFDIR') }}} | 0777, 0);
    },
    create_node: function(parent, name, mode, dev) {
      if (FS.isBlkdev(mode) || FS.isFIFO(mode)) {
        // no supported
        throw new FS.ErrnoError(ERRNO_CODES.EPERM);
      }
      var node = FS.createNode(parent, name, mode, dev);
      node.node_ops = MEMFS.node_ops;
      if (FS.isDir(node.mode)) {
        node.stream_ops = MEMFS.stream_ops;
        node.contents = {};
      } else if (FS.isFile(node.mode)) {
        node.stream_ops = MEMFS.stream_ops;
        node.contents = [];
      } else if (FS.isLink(node.mode)) {
        node.stream_ops = MEMFS.stream_ops;
      } else if (FS.isChrdev(node.mode)) {
        node.stream_ops = FS.chrdev_stream_ops;
      }
      node.timestamp = Date.now();
      // add the new node to the parent
      if (parent) {
        parent.contents[name] = node;
      }
      return node;
    },
    node_ops: {
      getattr: function(node) {
        var attr = {};
        // device numbers reuse inode numbers.
        attr.dev = FS.isChrdev(node.mode) ? node.id : 1;
        attr.ino = node.id;
        attr.mode = node.mode;
        attr.nlink = 1;
        attr.uid = 0;
        attr.gid = 0;
        attr.rdev = node.rdev;
        if (FS.isDir(node.mode)) {
          attr.size = 4096;
        } else if (FS.isFile(node.mode)) {
          attr.size = node.contents.length;
        } else if (FS.isLink(node.mode)) {
          attr.size = node.link.length;
        } else {
          attr.size = 0;
        }
        attr.atime = new Date(node.timestamp);
        attr.mtime = new Date(node.timestamp);
        attr.ctime = new Date(node.timestamp);
        // NOTE: In our implementation, st_blocks = Math.ceil(st_size/st_blksize),
        //       but this is not required by the standard.
        attr.blksize = 4096;
        attr.blocks = Math.ceil(attr.size / attr.blksize);
        return attr;
      },
      setattr: function(node, attr) {
        if (attr.mode !== undefined) {
          node.mode = attr.mode;
        }
        if (attr.timestamp !== undefined) {
          node.timestamp = attr.timestamp;
        }
        if (attr.size !== undefined) {
          var contents = node.contents;
          if (attr.size < contents.length) contents.length = attr.size;
          else while (attr.size > contents.length) contents.push(0);
        }
      },
      lookup: function(parent, name) {
        throw new FS.ErrnoError(ERRNO_CODES.ENOENT);
      },
      mknod: function(parent, name, mode, dev) {
        return MEMFS.create_node(parent, name, mode, dev);
      },
      rename: function(old_node, new_dir, new_name) {
        // if we're overwriting a directory at new_name, make sure it's empty.
        if (FS.isDir(old_node.mode)) {
          var new_node;
          try {
            new_node = FS.lookupNode(new_dir, new_name);
          } catch (e) {
          }
          if (new_node) {
            for (var i in new_node.contents) {
              throw new FS.ErrnoError(ERRNO_CODES.ENOTEMPTY);
            }
          }
        }
        // do the internal rewiring
        delete old_node.parent.contents[old_node.name];
        old_node.name = new_name;
        new_dir.contents[new_name] = old_node;
      },
      unlink: function(parent, name) {
        delete parent.contents[name];
      },
      rmdir: function(parent, name) {
        var node = FS.lookupNode(parent, name);
        for (var i in node.contents) {
          throw new FS.ErrnoError(ERRNO_CODES.ENOTEMPTY);
        }
        delete parent.contents[name];
      },
      symlink: function(parent, newname, oldpath) {
        var node = MEMFS.create_node(parent, newname, 0777 | {{{ cDefine('S_IFLNK') }}}, 0);
        node.link = oldpath;
        return node;
      },
      readlink: function(node) {
        if (!FS.isLink(node.mode)) {
          throw new FS.ErrnoError(ERRNO_CODES.EINVAL);
        }
        return node.link;
      },
    },
    stream_ops: {
      open: function(stream) {
        if (FS.isDir(stream.node.mode)) {
          // cache off the directory entries when open'd
          var entries = ['.', '..']
          for (var key in stream.node.contents) {
            if (!stream.node.contents.hasOwnProperty(key)) {
              continue;
            }
            entries.push(key);
          }
          stream.entries = entries;
        }
      },
      read: function(stream, buffer, offset, length, position) {
        var contents = stream.node.contents;
        var size = Math.min(contents.length - position, length);
#if USE_TYPED_ARRAYS == 2
        if (contents.subarray) { // typed array
          buffer.set(contents.subarray(position, position + size), offset);
        } else
#endif
        {
          for (var i = 0; i < size; i++) {
            buffer[offset + i] = contents[position + i];
          }
        }
        return size;
      },
      write: function(stream, buffer, offset, length, position) {
        var contents = stream.node.contents;
        while (contents.length < position) contents.push(0);
        for (var i = 0; i < length; i++) {
          contents[position + i] = buffer[offset + i];
        }
        stream.node.timestamp = Date.now();
        return length;
      },
      llseek: function(stream, offset, whence) {
        var position = offset;
        if (whence === 1) {  // SEEK_CUR.
          position += stream.position;
        } else if (whence === 2) {  // SEEK_END.
          if (FS.isFile(stream.node.mode)) {
            position += stream.node.contents.length;
          }
        }
        if (position < 0) {
          throw new FS.ErrnoError(ERRNO_CODES.EINVAL);
        }
        stream.ungotten = [];
        stream.position = position;
        return position;
      },
      readdir: function(stream) {
        return stream.entries;
      },
      allocate: function(stream, offset, length) {
        var contents = stream.node.contents;
        var limit = offset + length;
        while (limit > contents.length) contents.push(0);
      },
      mmap: function(stream, buffer, offset, length, position, prot, flags) {
        if (!FS.isFile(stream.node.mode)) {
          throw new FS.ErrnoError(ERRNO_CODES.ENODEV);
        }
        var ptr;
        var allocated;
        var contents = stream.node.contents;
        // Only make a new copy when MAP_PRIVATE is specified.
        if (!(flags & {{{ cDefine('MAP_PRIVATE') }}})) {
          // We can't emulate MAP_SHARED when the file is not backed by the buffer
          // we're mapping to (e.g. the HEAP buffer).
          assert(contents.buffer === buffer || contents.buffer === buffer.buffer);
          allocated = false;
          ptr = contents.byteOffset;
        } else {
          // Try to avoid unnecessary slices.
          if (position > 0 || position + length < contents.length) {
            if (contents.subarray) {
              contents = contents.subarray(position, position + length);
            } else {
              contents = Array.prototype.slice.call(contents, position, position + length);
            }
          }
          allocated = true;
          ptr = _malloc(length);
          if (!ptr) {
            throw new FS.ErrnoError(ERRNO_CODES.ENOMEM);
          }
          buffer.set(contents, ptr);
        }
        return { ptr: ptr, allocated: allocated };
      },
    }
  }
});