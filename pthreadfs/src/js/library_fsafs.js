/**
 * @license
 * Copyright 2021 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

mergeInto(LibraryManager.library, {
  $FSAFS__deps: ['$PThreadFS'],
  $FSAFS: {

    /* Debugging */

    debug: function(...args) {
      // Uncomment to print debug information.
      //
      // console.log(args);
    },

    /* Filesystem implementation (public interface) */

    createNode: function (parent, name, mode, dev) {
      FSAFS.debug('createNode', arguments);
      if (!PThreadFS.isDir(mode) && !PThreadFS.isFile(mode)) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EINVAL') }}});
      }
      var node = PThreadFS.createNode(parent, name, mode);
      node.node_ops = FSAFS.node_ops;
      node.stream_ops = FSAFS.stream_ops;
      if (PThreadFS.isDir(mode)) {
        node.contents = {};
      }
      node.timestamp = Date.now();
      return node;
    },

    mount: async function (mount) {
      FSAFS.debug('mount', arguments);
      let node = FSAFS.createNode(null, '/', {{{ cDefine('S_IFDIR') }}} | 511 /* 0777 */, 0);
      FSAFS.root = await navigator.storage.getDirectory();
      node.localReference = FSAFS.root;
      return node;
    },

    /* Operations on the nodes of the filesystem tree */

    node_ops: {
      getattr: async function(node) {
        FSAFS.debug('getattr', arguments);
        var attr = {};
        // device numbers reuse inode numbers.
        attr.dev = PThreadFS.isChrdev(node.mode) ? node.id : 1;
        attr.ino = node.id;
        attr.mode = node.mode;
        attr.nlink = 1;
        attr.uid = 0;
        attr.gid = 0;
        attr.rdev = node.rdev;
        if (PThreadFS.isDir(node.mode)) {
          attr.size = 4096;
        } else if (PThreadFS.isFile(node.mode)) {
          if (node.handle) {
            attr.size = await node.handle.getSize();
          } 
          else {
            let fileHandle = await node.localReference.createSyncAccessHandle();
            attr.size = await fileHandle.getSize();
            await fileHandle.close();
          }
        } else if (PThreadFS.isLink(node.mode)) {
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

      setattr: async function(node, attr) {
        FSAFS.debug('setattr', arguments);
        if (attr.mode !== undefined) {
          node.mode = attr.mode;
        }
        if (attr.timestamp !== undefined) {
          node.timestamp = attr.timestamp;
        }
        if (attr.size !== undefined) {
          let useOpen = false;
          let fileHandle = node.handle;
          try {
            if (!fileHandle) {
              // Open a handle that is closed later.
              useOpen = true;
              fileHandle = await node.localReference.createSyncAccessHandle();
            }
            await fileHandle.truncate(attr.size);
            
          } catch (e) {
            if (!('code' in e)) throw e;
            throw new PThreadFS.ErrnoError(-e.errno);
          } finally {
            if (useOpen) {
              await fileHandle.close();
            }
          }
        }
      },

      lookup: async function (parent, name) {
        FSAFS.debug('lookup', arguments);
        let childLocalReference = null;
        let mode = null;
        try {
          childLocalReference = await parent.localReference.getDirectoryHandle(name, {create: false});
          mode = {{{ cDefine('S_IFDIR') }}} | 511 /* 0777 */
        } catch (e) {
          try {
            childLocalReference = await parent.localReference.getFileHandle(name, {create: false});
            mode = {{{ cDefine('S_IFREG') }}} | 511 /* 0777 */
          } catch (e) {
            throw PThreadFS.genericErrors[{{{ cDefine('ENOENT') }}}];
          }
        }
        var node = PThreadFS.createNode(parent, name, mode);
        node.node_ops = FSAFS.node_ops;
        node.stream_ops = FSAFS.stream_ops;
        node.localReference = childLocalReference;
        return node;
      },

      mknod: async function (parent, name, mode, dev) {
        FSAFS.debug('mknod', arguments);
        let node = FSAFS.createNode(parent, name, mode, dev);
        try {
          if (PThreadFS.isDir(mode)) {
            node.localReference = await parent.localReference.getDirectoryHandle(name, {create: true});
          } else if (PThreadFS.isFile(mode)) {
            node.localReference = await parent.localReference.getFileHandle(name, {create: true});
          }
        } catch (e) {
          if (!('code' in e)) throw e;
          throw new PThreadFS.ErrnoError(-e.errno);
        }

        node.handle = null;
        node.refcount = 0;
        return node;
      },

      rename: function (oldNode, newParentNode, newName) {
        FSAFS.debug('rename', arguments);
        console.log('FSAFS error: rename is not implemented')
        throw new PThreadFS.ErrnoError({{{ cDefine('ENOSYS') }}});
      },

      unlink: async function(parent, name) {
        FSAFS.debug('unlink', arguments);
        delete parent.contents[name];
        return await parent.localReference.removeEntry(name);
      },

      rmdir: async function(parent, name) {
        FSAFS.debug('rmdir', arguments);
        for (var i in node.contents) {
          throw new FS.ErrnoError({{{ cDefine('ENOTEMPTY') }}});
        }
        delete parent.contents[name];
        return await parent.localReference.removeEntry(name);
      },

      readdir: async function(node) {
        FSAFS.debug('readdir', arguments);
        let entries = ['.', '..'];
        for await (let [name, handle] of node.localReference) {
          entries.push(name);
        }
        return entries;
      },

      symlink: function(parent, newName, oldPath) {
        console.log('FSAFS error: symlink is not implemented')
        throw new PThreadFS.ErrnoError({{{ cDefine('ENOSYS') }}});
      },

      readlink: function(node) {
        console.log('FSAFS error: readlink is not implemented')
        throw new PThreadFS.ErrnoError({{{ cDefine('ENOSYS') }}});
      },
    },

    /* Operations on file streams (i.e., file handles) */

    stream_ops: {
      open: async function (stream) {
        FSAFS.debug('open', arguments);
        if (PThreadFS.isDir(stream.node.mode)) {
          console.log('FSAFS error: open for directories is not fully implemented')
          throw new PThreadFS.ErrnoError({{{ cDefine('EISDIR') }}});
        }
        if (!PThreadFS.isFile(stream.node.mode)) {
          console.log('FSAFS error: open is only implemented for files')
          throw new PThreadFS.ErrnoError({{{ cDefine('ENOSYS') }}});
        }

        if (stream.node.handle) {
          stream.handle = stream.node.handle;
          ++stream.node.refcount;
        } else {
          stream.handle = await stream.node.localReference.createSyncAccessHandle();
          stream.node.handle = stream.handle;
          stream.node.refcount = 1;
        }
      },

      close: async function (stream) {
        FSAFS.debug('close', arguments);
        if (!PThreadFS.isFile(stream.node.mode)) {
          console.log('FSAFS error: close is only implemented for files');
          throw new PThreadFS.ErrnoError({{{ cDefine('ENOSYS') }}});
        }

        stream.handle = null;
        --stream.node.refcount;
        if (stream.node.refcount <= 0) {
          await stream.node.handle.close();
          stream.node.handle = null;
        }
      },

      fsync: async function(stream) {
        FSAFS.debug('fsync', arguments);
        if (stream.handle == null) {
          throw new PThreadFS.ErrnoError({{{ cDefine('EBADF') }}});
        }
        await stream.handle.flush();
        return 0;
      },

      read: async function (stream, buffer, offset, length, position) {
        FSAFS.debug('read', arguments);
        let data = buffer.subarray(offset, offset+length);
        let readBytes = await stream.handle.read(data, {at: position});
        return readBytes;
      },

      write: async function (stream, buffer, offset, length, position) {
        FSAFS.debug('write', arguments);
        stream.node.timestamp = Date.now();
        let data = buffer.subarray(offset, offset+length);
        let writtenBytes = await stream.handle.write(data, {at: position});
        return writtenBytes;
      },

      llseek: async function (stream, offset, whence) {
        FSAFS.debug('llseek', arguments);
        let position = offset;
        if (whence === {{{ cDefine('SEEK_CUR') }}}) {
          position += stream.position;
        } else if (whence === {{{ cDefine('SEEK_END') }}}) {
          if (PThreadFS.isFile(stream.node.mode)) {
            position += await stream.handle.getSize();
          }
        } 

        if (position < 0) {
          throw new PThreadFS.ErrnoError({{{ cDefine('EINVAL') }}});
        }
        return position;
      },

      mmap: function(stream, buffer, offset, length, position, prot, flags) {
        FSAFS.debug('mmap', arguments);
        throw new PThreadFS.ErrnoError({{{ cDefine('EOPNOTSUPP') }}});
      },

      msync: function(stream, buffer, offset, length, mmapFlags) {
        FSAFS.debug('msync', arguments);
        throw new PThreadFS.ErrnoError({{{ cDefine('EOPNOTSUPP') }}});
      },

      munmap: function(stream) {
        FSAFS.debug('munmap', arguments);
        throw new PThreadFS.ErrnoError({{{ cDefine('EOPNOTSUPP') }}});
      },
    }
  }
});