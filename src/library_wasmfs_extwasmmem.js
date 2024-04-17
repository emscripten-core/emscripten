/**
 * @license
 * Copyright 2023 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

// for cpp users: use fallocate() in advance to get better write performance.

// WARNING for library developer: do not use << for uint in js, use * 4 instead.

var LibraryExtWasmMemFS = {
  $extWasmMemFS_local: {
    /**@type Uint32Array*/
    dataFilesU32: null, 
    // empty_block: |block_size|next_empty_block_ptr|prev_empty_block_ptr|....|
    // file_block: |block_size|0xffffffff|.....| // 0xffffffff marks file block.
    // block size include itself, and should be allocated with 4 byte boundary, 12 byte minimal.

    /**@type Uint8Array */
    dataFilesU8: null,

    /**@type Int32Array */
    control: null, // | w_mutex | r_count | index_count | head_empty_ptr | unalloc_ptr | 

    /** @type Uint32Array*/
    index: null,  // |file_start_ptr|file_size| 
    //file_start_ptr can be 0 for the space that have not been allocated.

    
    /**************** function below require proper rwlock. *****************/
    get_block_size: function(ptr) {
      return extWasmMemFS_local.dataFilesU32[ptr >>> 2];
    },

    get_next_empty_ptr: function(ptr) {
      return extWasmMemFS_local.dataFilesU32[(ptr >>> 2) + 1];
    },

    set_next_empty_ptr: function(ptr, next_ptr) {
      #if ASSERTIONS
      assert(ptr > 0);
      assert(next_ptr >= 0);
      #endif
      extWasmMemFS_local.dataFilesU32[(ptr >>> 2) + 1] = next_ptr;
    },

    get_prev_empty_ptr: function(ptr) {
      return extWasmMemFS_local.dataFilesU32[(ptr >>> 2) + 2];
    },

    set_prev_empty_ptr: function(ptr, prev_ptr) {
      #if ASSERTIONS
      assert(ptr > 0);
      assert(prev_ptr >= 0);
      #endif
      extWasmMemFS_local.dataFilesU32[(ptr >>> 2) + 2] = prev_ptr;
    },

    get_adjacent_block_ptr: function(ptr) {
      return ptr + extWasmMemFS_local.get_block_size(ptr);
    },

    is_file_block: function(ptr) {
      return extWasmMemFS_local.get_next_empty_ptr(ptr) === 0xffffffff;
    },
    
    set_block_size: function(ptr, size) {
      #if ASSERTIONS
      assert(ptr > 0);
      assert(size > 0);
      #endif
      extWasmMemFS_local.dataFilesU32[ptr >>> 2] = size;
    },
    
    init_file_block: function(ptr, block_size) {
      #if ASSERTIONS
      assert(ptr > 0);
      assert(block_size > 0);
      #endif
      extWasmMemFS_local.set_block_size(ptr, block_size);
      extWasmMemFS_local.dataFilesU32[(ptr >>> 2) + 1] = 0xffffffff;
    },

    init_empty_block: function(ptr, block_size) {
      #if ASSERTIONS
      assert(ptr > 0);
      assert(block_size > 0);
      #endif
      extWasmMemFS_local.set_block_size(ptr, block_size);
      extWasmMemFS_local.set_prev_empty_ptr(ptr, 0);
      extWasmMemFS_local.set_next_empty_ptr(ptr, 0);
    },

    get_empty_head: function() {
      return extWasmMemFS_local.control[3];
    },

    get_empty_unalloc: function() {
      return extWasmMemFS_local.control[4];
    },

    set_empty_head: function(new_head) {
      extWasmMemFS_local.control[3] = new_head;
    },

    set_empty_unalloc: function(new_tail) {
      extWasmMemFS_local.control[4] = new_tail;
    },

    remove_block_from_empty_list: function(ptr) {
      #if ASSERTIONS
      assert(ptr > 0);
      #endif

      var next = extWasmMemFS_local.get_next_empty_ptr(ptr);
      var prev = extWasmMemFS_local.get_prev_empty_ptr(ptr);
      if (prev === 0) {
        extWasmMemFS_local.set_empty_head(next);
      }

      if (next !== 0) {
        extWasmMemFS_local.set_prev_empty_ptr(next, prev);
      }
      
      if (prev !== 0) {
        extWasmMemFS_local.set_next_empty_ptr(prev, next);
      }
      extWasmMemFS_local.set_next_empty_ptr(ptr, 0);
      extWasmMemFS_local.set_prev_empty_ptr(ptr, 0);
    },

    check_last_block_and_merge_into_unalloc: function(ptr) {
      #if ASSERTIONS
      assert(ptr > 0);
      //the block should not be in empty list.
      assert(extWasmMemFS_local.get_next_empty_ptr(ptr) === 0 || extWasmMemFS_local.get_next_empty_ptr(ptr) === 0xffffffff); 
      #endif
      if (extWasmMemFS_local.get_adjacent_block_ptr(ptr) === extWasmMemFS_local.get_empty_unalloc()) {
        extWasmMemFS_local.dataFilesU8.fill(0, ptr, ptr+12);
        extWasmMemFS_local.set_empty_unalloc(ptr);
        return true;
      }
      return false;
    },

    check_continuous_empty_block_and_merge: function (ptr) {
      #if ASSERTIONS
      assert(ptr > 0);
      #endif
      //check if we can combine more empty block together.
      var cptr = extWasmMemFS_local.get_adjacent_block_ptr(ptr);
      var csize = extWasmMemFS_local.get_block_size(ptr);
      var unalloc = extWasmMemFS_local.get_empty_unalloc();
      while (cptr < unalloc && (!extWasmMemFS_local.is_file_block(cptr))) {
        let bsize = extWasmMemFS_local.get_block_size(cptr);
        csize += bsize;
        if (bsize === 0) {
          throw new Error("bad fs data block");
        }
        extWasmMemFS_local.remove_block_from_empty_list(cptr);
        cptr = extWasmMemFS_local.get_adjacent_block_ptr(cptr);
      }
      extWasmMemFS_local.set_block_size(ptr, csize);
    },

    insert_empty_block_to_head: function (ptr) {
      #if ASSERTIONS
      assert(ptr > 0);
      assert(extWasmMemFS_local.get_block_size(ptr) !== 0);
      #endif

      extWasmMemFS_local.check_continuous_empty_block_and_merge(ptr);            

      //check if the block is last block in linear memory.
      if (extWasmMemFS_local.check_last_block_and_merge_into_unalloc(ptr)) {
        return;
      }

      var ohead = extWasmMemFS_local.get_empty_head();
      if (ohead !== 0) {
        extWasmMemFS_local.set_next_empty_ptr(ptr, ohead);
        extWasmMemFS_local.set_prev_empty_ptr(ohead, ptr);
        extWasmMemFS_local.set_prev_empty_ptr(ptr, 0);
      } else {
        extWasmMemFS_local.set_prev_empty_ptr(ptr, 0);
        extWasmMemFS_local.set_next_empty_ptr(ptr, 0);
      }

      extWasmMemFS_local.set_empty_head(ptr);
    },

    update_typed_arrays: function () {
      extWasmMemFS_local.dataFilesU32 = new Uint32Array(Module['extWasmMemFS']['dataFiles'].buffer); //data files may grow.
      extWasmMemFS_local.dataFilesU8 = new Uint8Array(Module['extWasmMemFS']['dataFiles'].buffer);
    },

    ensure_underlied_buffer_size: function (byteLength) {
      var buffer_size = extWasmMemFS_local.dataFilesU8.buffer.byteLength;
      var deltaInByte = byteLength - buffer_size;
      if (deltaInByte > 0) {
        var delta = Math.max(deltaInByte, 200 * 1024 * 1024);
        delta = (delta + 65535) >>> 16;
        try {
          Module['extWasmMemFS']['dataFiles'].grow(delta);
        } catch (e) {
          if (e instanceof RangeError) {
            throw "ENOBUFS";
          } else {
            throw e;
          }
        }
        extWasmMemFS_local.update_typed_arrays();
      }
    },

    alloc_new_file_block_on_unalloc_region: function (size) {
      var unalloc = extWasmMemFS_local.get_empty_unalloc();
      extWasmMemFS_local.ensure_underlied_buffer_size(unalloc + size + 1);
      extWasmMemFS_local.set_empty_unalloc(unalloc + size);
      extWasmMemFS_local.init_file_block(unalloc, size);
      return unalloc;
    },

    alloc_new_file_block_on_empty_block: function(empty_ptr, file_block_size) {
      #if ASSERTIONS
      assert(empty_ptr > 0);
      assert(extWasmMemFS_local.get_block_size(empty_ptr) > 8);
      #endif
      var empty_size = extWasmMemFS_local.get_block_size(empty_ptr);
      var real_alloc_size = (empty_size - file_block_size > 32)? file_block_size: empty_size;
      extWasmMemFS_local.remove_block_from_empty_list(empty_ptr);
      if (real_alloc_size < empty_size) {
        //cut one empty block into two.
        var new_empty_ptr = empty_ptr + real_alloc_size;
        var new_empty_size = empty_size - real_alloc_size;
        extWasmMemFS_local.init_empty_block(new_empty_ptr, new_empty_size);
        extWasmMemFS_local.insert_empty_block_to_head(new_empty_ptr);
      }
      extWasmMemFS_local.init_file_block(empty_ptr, real_alloc_size);
      return empty_ptr;
    },

    alloc_new_file_block: function (file_block_size) {           
      var eptr = extWasmMemFS_local.get_empty_head();
      while (eptr !== 0) {
        extWasmMemFS_local.check_continuous_empty_block_and_merge(eptr);
        if (extWasmMemFS_local.get_block_size(eptr) >= file_block_size) {
          //found an empty block.
          break;
        }
        eptr = extWasmMemFS_local.get_next_empty_ptr(eptr);
      }

      if (eptr !== 0) {
        // found an empty block.
        return extWasmMemFS_local.alloc_new_file_block_on_empty_block(eptr, file_block_size);
      } 

      return extWasmMemFS_local.alloc_new_file_block_on_unalloc_region(file_block_size);                        
    },

    try_expand_file_block: function (ptr, required_size, initalizeToZero) {
      var original_size = extWasmMemFS_local.get_block_size(ptr);
      var delta = required_size - original_size;
      if (delta <= 0) {
        return true; //large enough.
      }
      
      // 1. if current block is last block, get space from unalloc region, expand the dataFile buffer with grow if needed.
      var unalloc = extWasmMemFS_local.get_empty_unalloc();
      var adjacent_ptr = extWasmMemFS_local.get_adjacent_block_ptr(ptr);
      if (adjacent_ptr >= unalloc) {
        try {
          extWasmMemFS_local.ensure_underlied_buffer_size(ptr + required_size + 1);
        } catch (e) {
          if (e === "ENOBUFS") {
            return false;
          } else {
            throw e;
          }
        }
        extWasmMemFS_local.set_empty_unalloc(ptr + required_size);
        extWasmMemFS_local.set_block_size(ptr, required_size);
        return true;
      }

      // 2. if not last block, use get_adjacent_ptr to find empty blocks behind current file block.
      if (!extWasmMemFS_local.is_file_block(adjacent_ptr)) {
        var empty_block_size = extWasmMemFS_local.get_block_size(adjacent_ptr);
        if (empty_block_size + original_size >= required_size) {
          
          //for situation that empty block is too large and need split.
          extWasmMemFS_local.alloc_new_file_block_on_empty_block(adjacent_ptr, required_size - original_size);
          var new_block_size = extWasmMemFS_local.get_block_size(adjacent_ptr);
          extWasmMemFS_local.set_block_size(ptr, original_size + new_block_size);
          if (initalizeToZero) {
            extWasmMemFS_local.dataFilesU8.fill(0, adjacent_ptr, adjacent_ptr + new_block_size);
          }
          return true;
        }
      }

      return false;
    },

    shrink_file_block: function (ptr, shrink_size) {
      //not implemented.
    },

    delete_file_block: function (ptr) {
      extWasmMemFS_local.init_empty_block(ptr, extWasmMemFS_local.get_block_size(ptr))
      extWasmMemFS_local.insert_empty_block_to_head(ptr);
    },
    /*********** Function above require proper rwlock. ***********/

  },
  $extWasmMemFS_local__postset:
  `
extWasmMemFS_local.control = new Uint32Array(Module['extWasmMemFS']['control']);
extWasmMemFS_local.index = new Uint32Array(Module['extWasmMemFS']['index']);
  `,

  $extWasmMemFS_rwlock: {    
    r_lock: function () {
      while (1) {
        while (Atomics.load(extWasmMemFS_local.control, 0) === 1) {}
        Atomics.add(extWasmMemFS_local.control, 1, 1);
        if (Atomics.load(extWasmMemFS_local.control, 0) === 1) {
          Atomics.sub(extWasmMemFS_local.control, 1, 1);
        } else {
          break;
        }
      }
      extWasmMemFS_local.update_typed_arrays();
    },

    r_unlock: function () {
      Atomics.sub(extWasmMemFS_local.control, 1, 1);
    },

    w_lock: function() {
      while(Atomics.exchange(extWasmMemFS_local.control, 0, 1)) {}
      while(Atomics.load(extWasmMemFS_local.control, 1) !== 0) {}
      extWasmMemFS_local.update_typed_arrays();
    },
  
    w_unlock: function() {
      Atomics.store(extWasmMemFS_local.control, 0, 0);
    }
  },

  wasmfs_extwasmmem_file_handle_alloc__sig:'i',
  wasmfs_extwasmmem_file_handle_alloc: function() {
    let myindex = Atomics.add(extWasmMemFS_local.control, 2, 1);
    return myindex;
  },

  wasmfs_extwasmmem_file_open__sig:'iii',
  wasmfs_extwasmmem_file_open: function(handle, bCreate) {
    return 0;
  },

  wasmfs_extwasmmem_file_delete__sig:'vi',
  wasmfs_extwasmmem_file_delete: function(handle) {
    extWasmMemFS_rwlock.w_lock();
    var file_block_ptr = extWasmMemFS_local.index[handle * 2];
    if (file_block_ptr > 0) {
      //delete twice or empty file will cause file_block_ptr to be 0
      extWasmMemFS_local.index[handle * 2 + 1] = 0;
      extWasmMemFS_local.index[handle * 2] = 0;
      extWasmMemFS_local.delete_file_block(file_block_ptr);
    }
    extWasmMemFS_rwlock.w_unlock();
  },

  $wasmfs_extwasmmem_file_read_internal: function (handle, wasmBuf, len, offset) {
    var file_block_ptr = extWasmMemFS_local.index[handle * 2];
    var file_size = extWasmMemFS_local.index[handle * 2 + 1];
    if (file_block_ptr === 0) {
      return 0;
    }

    var read_start = file_block_ptr + 8 + offset;
    var read_end = Math.min(file_block_ptr + 8 + offset + len, file_block_ptr + 8 + file_size);
    if (read_start >= read_end) {
      return 0;
    }

    if (wasmBuf === null) {
      return extWasmMemFS_local.dataFilesU8.slice(read_start, read_end);
    } else {
      HEAPU8.set(extWasmMemFS_local.dataFilesU8.subarray(read_start, read_end), wasmBuf);
    }
    return read_end - read_start
  },

  wasmfs_extwasmmem_file_read__deps: ['$wasmfs_extwasmmem_file_read_internal'],
  wasmfs_extwasmmem_file_read__sig:'iipii',
  wasmfs_extwasmmem_file_read: function(handle, wasmBuf, len, offset) {
    extWasmMemFS_rwlock.r_lock();
    var ret = wasmfs_extwasmmem_file_read_internal(handle, wasmBuf, len, offset);
    extWasmMemFS_rwlock.r_unlock();
    return ret;
  },

  $wasmfs_extwasmmem_file_write_internal: function (handle, sourceArray, len, offset, isSetSize) {
    if (len > 2147483647) return -{{{cDefine('EFBIG')}}};
    var file_block_ptr = extWasmMemFS_local.index[handle * 2];
    var file_size = extWasmMemFS_local.index[(handle * 2) + 1];
    var file_exists = (file_block_ptr > 0);
    var file_block_size = 0;
    if (file_exists) {
      file_block_size = extWasmMemFS_local.get_block_size(file_block_ptr);
    }
    var new_file_size = Math.max(file_size, offset + len);
    var required_block_size = ((new_file_size + 8 + 3) >>> 2) * 4; // 3->4, 4->4, 5->8, 6->8, etc. 

    //1. block is not large enough, try to expand it.
    if (file_exists && file_block_size < required_block_size) {
      
      //allocate more at first try.
      var success = false;
      if (!isSetSize){ 
        var rec = ((Math.floor(required_block_size * 1.25) + 3) >>> 2) * 4;
        success = extWasmMemFS_local.try_expand_file_block(file_block_ptr, rec, false);
      }
      
      if (!success) {
        success = extWasmMemFS_local.try_expand_file_block(file_block_ptr, required_block_size, isSetSize);
      }
      
      if (success) {
        file_block_size = extWasmMemFS_local.get_block_size(file_block_ptr);
      }
    }
    
    //2. file exists and file block is large enough.
    if (file_exists && file_block_size >= required_block_size) {
      if (offset > file_size) {
        extWasmMemFS_local.dataFilesU8.fill(0, file_block_ptr + 8 + file_size, file_block_ptr + 8 + offset);
      }
      if (sourceArray === null) {
        extWasmMemFS_local.dataFilesU8.fill(0, file_block_ptr + 8 + offset, file_block_ptr + 8 + offset + len);
      } else {
        extWasmMemFS_local.dataFilesU8.set(sourceArray, file_block_ptr + 8 + offset);
      }
      extWasmMemFS_local.index[(handle * 2) + 1] = new_file_size;
      return len;
    }

    //3. file not exists or file block is not large enough, alloc a new block.
    var recommended_alloc_space = required_block_size;
    if (!isSetSize) {
      recommended_alloc_space = recommended_alloc_space > 64 * 1024 ? required_block_size * 1.2 : required_block_size * 2; 
      recommended_alloc_space = ((Math.ceil(recommended_alloc_space) + 3) >>> 2) * 4;
    } 
    var new_file_block_ptr = extWasmMemFS_local.alloc_new_file_block(recommended_alloc_space);
    
    //4. write to new place
    extWasmMemFS_local.dataFilesU8.copyWithin(new_file_block_ptr + 8, file_block_ptr + 8, file_block_ptr + 8 + file_size);
    if (offset > file_size) {
      extWasmMemFS_local.dataFilesU8.fill(0, new_file_block_ptr + 8 + file_size, new_file_block_ptr + 8 + offset);
    }
    if (sourceArray === null) {
      extWasmMemFS_local.dataFilesU8.fill(0, new_file_block_ptr + 8 + offset, new_file_block_ptr + 8 + offset + len);
    } else {
      extWasmMemFS_local.dataFilesU8.set(sourceArray, new_file_block_ptr + 8 + offset);
    }
    
    //5. update index.
    extWasmMemFS_local.index[handle * 2] = new_file_block_ptr;
    extWasmMemFS_local.index[handle * 2 + 1] = new_file_size;

    //6. if file exists originally, free the orignal space.
    if (file_exists) {
      extWasmMemFS_local.delete_file_block(file_block_ptr);
    }
    return len;
  },

  wasmfs_extwasmmem_file_write__deps: ['$wasmfs_extwasmmem_file_write_internal'],
  wasmfs_extwasmmem_file_write__sig:'iipii',
  wasmfs_extwasmmem_file_write: function(handle, buf, len, offset) {
    if (len === 0) {
      return 0;
    }
    extWasmMemFS_rwlock.w_lock();
    var ret;
    try {
      ret = wasmfs_extwasmmem_file_write_internal(handle, HEAPU8.subarray(buf, buf + len), len, offset, false);
    } catch (e) {
      if (e === "ENOBUFS") {
        ret = 0;
      } else {
        throw e;
      }
    } finally {
      extWasmMemFS_rwlock.w_unlock();
    }
    
    if (ret < 0) {
      return 0;
    }
    return ret;
  },

  wasmfs_extwasmmem_file_getSize__sig:'ii',
  wasmfs_extwasmmem_file_getSize: function(handle) {
    extWasmMemFS_rwlock.r_lock();
    var size = extWasmMemFS_local.index[(handle << 1) | 1];
    extWasmMemFS_rwlock.r_unlock();
    return size;
  },

  wasmfs_extwasmmem_file_setSize__deps: ['$wasmfs_extwasmmem_file_write_internal'],
  wasmfs_extwasmmem_file_setSize__sig:'iii',
  wasmfs_extwasmmem_file_setSize: function(handle, targetSize) {
    extWasmMemFS_rwlock.w_lock();
    
    var file_block_ptr = extWasmMemFS_local.index[handle * 2];
    var file_size = extWasmMemFS_local.index[(handle * 2) + 1];
    if (file_size >= targetSize && file_block_ptr > 0) {
      extWasmMemFS_local.shrink_file_block(file_block_ptr, targetSize);
      extWasmMemFS_local.index[(handle * 2) + 1] = targetSize;
    } else {
      wasmfs_extwasmmem_file_write_internal(handle, null, targetSize - file_size, file_size, true);
    }

    extWasmMemFS_rwlock.w_unlock();
    return 0;
  },

  // js api for fast r/w file, mainly for use in main thread.
  // File and its dir should be created with posix file api in advance.
  // For other file operation, use posix file api as well.
  $ExtWasmMemFS__deps: ['$withStackSave', '$allocateUTF8OnStack', '$wasmfs_extwasmmem_file_write_internal', '$wasmfs_extwasmmem_file_read_internal', 'wasmfs_extwasmmem_file_setSize', '_extwasmmem_get_file_handle_by_path'],
  $ExtWasmMemFS: {
    
    /**
     * 
     * @param {string} path 
     * @param {Uint8Array} data 
     */
    writeFile: function (path, data, entire_file_hint) {
      var u8array = data;
      var len = u8array.length;
      if (entire_file_hint === undefined) {
        entire_file_hint = true; //default to true.
      }

      var handle = withStackSave(() => {
        let __path = allocateUTF8OnStack(path);
        let handle = __extwasmmem_get_file_handle_by_path(__path, /*createIfNotExist = */1);
        return handle;
      });

      let ret = handle;
      if (handle >= 0) {
        if (len === 0) {
          return 0;
        }
        extWasmMemFS_rwlock.w_lock();
        try {
          ret = wasmfs_extwasmmem_file_write_internal(handle, u8array, len, 0, entire_file_hint);
        } catch (ex) {
          if (ex === 'ENOBUFS') {
            return -{{{cDefine('ENOBUFS')}}};
          } else {
            throw ex;
          }
        } finally {
          extWasmMemFS_rwlock.w_unlock();
        }
      }

      if (ret < 0) {
        err("extwasmmemFS bad write, errno: "+ ret);
      }
      
      return ret;
    },

    /**
     * 
     * @param {string} path 
     * @param {{len: number, offset: number}} opts
     * @return {Uint8Array | number} content read in a new Uint8Array or negative errno.
     */
    readFile: function (path, opts={}) {
      var len = opts.len = opts.len || 0xffffffff;
      var offset = opts.offset = opts.offset || 0;
      var handle = withStackSave(() => {
        let __path = allocateUTF8OnStack(path);
        let handle = __extwasmmem_get_file_handle_by_path(__path, /*createIfNotExist = */0);
        return handle;
      });
      let ret = handle;
      if (handle >= 0) {
        extWasmMemFS_rwlock.r_lock();
        ret = wasmfs_extwasmmem_file_read_internal(handle, null, len, offset);
        extWasmMemFS_rwlock.r_unlock();
      }

      if ((typeof ret) === 'number' && ret < 0) {
        err("extwasmmemFS bad read, errno: " + ret);
        return ret;
      } else if (ret === 0) {
        ret = new Uint8Array();
      }
      
      return ret;
    }
  }
};

autoAddDeps(LibraryExtWasmMemFS, "$extWasmMemFS_local");
autoAddDeps(LibraryExtWasmMemFS, "$extWasmMemFS_rwlock");
mergeInto(LibraryManager.library, LibraryExtWasmMemFS);
DEFAULT_LIBRARY_FUNCS_TO_INCLUDE.push('$ExtWasmMemFS');
