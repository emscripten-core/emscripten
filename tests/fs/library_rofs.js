
/*
  example of a read-only filesystem.  Call FS.mount(ROFS, {root_addr: root}, path)
  to cause 'path' to be a read-only file system containing all of the "files" described
  by 'root'.  'root' must be the address of a data structure which in C is the root ro_dir below:
    
  // struct used when a directory entry is a file (not another directory)
  // and so has ro_dir_ent.is_dir = false.  In this case, ro_dir_ent.ptr.file
  // points to one of these
  struct ro_file_ent
  {
    const char*  file_data;    // ptr to the data of the file
    size_t       file_data_sz; // size of the data pointed to by file_data
  };

  // struct used when a directory entry is another directory (not a file)
  // and so has the owning ro_dir_ent.is_dir = true.  In this case
  // ro_dir_ent.ptr.dir points to a ro_dir (below)
  struct ro_dir_ent
  {
    const char* d_name;                   // the name of the file or directory
    union {
      const struct ro_file_ent*   file;   // if this is a file, then the pointer to the ro_file_ent
      const struct ro_dir*        dir;    // otherwise if this another directory, the pointer to that ro_dir
    } ptr;
    int is_dir;                           // true if this ro_dir_ent is a directory, otherwise false (if it is a file)
  };

  // struct used to represent a directory
  struct ro_dir {
    size_t                    num_ents;   // number of entries (files or other directories) in this directory
    const struct ro_dir_ent*  ents;       // pointer to the list of entries in this directory
  };
  
  This is only an _example_ of a way to do this.  The js code below uses several
  'makeGetValue' statements to read the fields in the C data structures described above.
  Were you to use this code as a starting point and modify these data structures, you
  would also have to change the offsets passed to the makeGetValue statements to
  match your modifications.  Search for 'makeGetValue' in this file.  Each represents
  a place where it is offsetting an address to a field in one of these data structures.

*/
mergeInto(LibraryManager.library, {
  $ROFS__deps: ['$ERRNO_CODES', '$FS', '$MEMFS'],
  $ROFS: {
  
    ops_table: null,
    
    mount: function(mount) {
      if (!ROFS.ops_table) {
        ROFS.ops_table = {
          dir: {
            node: {
              getattr: ROFS.node_ops.getattr,
              lookup: MEMFS.node_ops.lookup,
              readdir: ROFS.node_ops.readdir,
              mknod: ROFS.node_ops.mknod,
            },
            stream: {
            },
          },
          file: {
            node: {
              getattr: ROFS.node_ops.getattr,
              setattr: ROFS.node_ops.setattr,
            },
            stream: {
              llseek: MEMFS.stream_ops.llseek,
              read: ROFS.stream_ops.read,
            }
          },
        };
      }
      
      var node = FS.createNode(null,'/',{{{ cDefine('S_IFDIR') }}} | 511 /* 0777 */, 0);
    
      node.node_ops = ROFS.ops_table.dir.node;
      node.stream_ops = ROFS.ops_table.dir.stream; // currently empty (see dir.stream above), but FS needs a non-null stream_ops.
      node.contents = ['.', '..'];
      
      ROFS.recursive_mount_dir(mount.opts.root_addr, node);
      
      return node;
    },
    
    node_ops: {
    
      getattr: function(node) {
        var attr = {};
        attr.dev = 1;
        attr.ino = node.id;
        attr.mode = node.mode;
        attr.nlink = 1;
        attr.uid = 0;
        attr.gid = 0;
        attr.rdev = node.rdev;
        if (FS.isDir(node.mode)) {
          attr.size = 4096;
        } else /*if (FS.isFile(node.mode))*/ {
          attr.size = {{{ makeGetValue('node.contents', '4', 'i32') }}};  // <- this line is why we need a custom getattr
        }
        attr.atime = new Date(node.timestamp);
        attr.mtime = new Date(node.timestamp);
        attr.ctime = new Date(node.timestamp);
        attr.blksize = 4096;
        attr.blocks = Math.ceil(attr.size / attr.blksize);
        return attr;
      },
      readdir: function(node) {
        return node.contents;
      },
      setattr: function(node, attr) {
        throw new FS.ErrnoError(ERRNO_CODES.EROFS);
      },
      mknod: function(parent, name, mode, dev) {
        throw new FS.ErrnoError(ERRNO_CODES.EROFS);
      }
    
    },
    
    stream_ops: {
      
      read: function(stream, buffer, offset, length, position) {
        var contents = stream.node.contents;
        var ptr = {{{ makeGetValue('contents', '0', 'i32') }}};
        var len = {{{ makeGetValue('contents', '4', 'i32') }}};
        if (position >= len)
          return 0;
        var size = length;
        if (position + size > len)
          size = len - position;
#if USE_TYPED_ARRAYS == 2
        if (size > 8)
          buffer.set(HEAP8.subarray(ptr+position, ptr+position+size), offset);
        else
#endif
        {
          for (var i = 0; i < size; i++)
            buffer[offset + i] = HEAP8[ptr + position + i];
        }
        return size;
      },
    
    },
    
    recursive_mount_dir: function (dir_addr, parent)  {
      var num_ents = {{{ makeGetValue('dir_addr', '0', 'i32') }}};
      var ents_addr = {{{ makeGetValue('dir_addr', '4', 'i32') }}};
      
      for (var i = 0; i < num_ents; i++) {
        var d_name_addr = {{{ makeGetValue('ents_addr', 'i*12', 'i32') }}};
        var d_name = Pointer_stringify(d_name_addr);
        var ptr = {{{ makeGetValue('ents_addr', 'i*12+4', 'i32') }}};
        var is_dir = {{{ makeGetValue('ents_addr', 'i*12+8', 'i32') }}};
        
        var mode;
        if (is_dir != 0)
          mode = {{{ cDefine('S_IFDIR') }}} | 511/*0777*/;
        else
          mode = {{{ cDefine('S_IFREG') }}} | 292/*0444*/;
        var node = FS.createNode(parent, d_name, mode, 0);
        if (is_dir != 0) {
          node.node_ops = ROFS.ops_table.dir.node;
          node.stream_ops = ROFS.ops_table.dir.stream;
          node.contents = ['.', '..'];
          ROFS.recursive_mount_dir(ptr, node);
        } else {
          node.node_ops = ROFS.ops_table.file.node;
          node.stream_ops = ROFS.ops_table.file.stream;
          node.contents = ptr;
        }
        parent.contents.push(d_name);
        
      }
      MEMFS.node_ops.setattr(parent, {mode: {{{ cDefine('S_IFDIR') }}} | 365/*0555*/});
      parent.is_readonly_fs = true;
    }
  
  },
  
  rofs_mount__sig: 'vii',
  rofs_mount__deps: ['$ROFS', '$FS'],
  rofs_mount: function (pathAddr, root_addr) {
    FS.mount(ROFS, {root_addr: root_addr}, Pointer_stringify(pathAddr));
  }
  
});
