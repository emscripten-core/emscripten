var LibraryNATIVEFS = {
    /*
      Documentation of functions is in include/emscripten/nativefs.h
    */
    $NATIVEFS: {
        /*
          nativefs_worker: a worker spawned by the runtime, that will 
          allow synchronous reads on files selected by the user.

          spawned in preamble.js, then the runtime will wait for
          the init_nativefs message to remove the runtimeDependency.
         */
        worker: 0,
        /*
          Allocated memory region mapped like this :
          {
            fs_lock: int,
            dispatch_index: int,
            dispatch_param1: int,
            dispatch_param2: int,
            dispatch_param3: int,
            // struct per file:
            {
              fileIndex: int, // Starts at one
              fileLock: int, // protects from concurrent read/seek (same file)
              readSize: int, // new read size 
              seekOffset: int // new offset size
            }
          }
        */
        handle: 0,
        /*
          Keep track of files in the fs
          nativefs_init will push to this array each time a file is selected.
          { fd: , fileName: , fileSize: }
        */
        files: [],
        /* 
           dispatcher:
           using shared memory to set the index of the function that will be called

           Negative codes are for EM_ASM, and positive codes are
           for proxiedFunctionTable indexes. If the code is 0, the 
           function will be null and we will be able to handle this special
           case. (in src/library_pthread.js).
           sigPtr and arg are ignored.
        */        
        MAIN_THREAD_CONSTS: [
            /* dispatcher */
            function () {
                var view = new Int32Array(Module.wasmMemory.buffer, NATIVEFS.handle, 5);
                // get dispatcher index
                var index = Atomics.load(view, 1);
                // TODO: catch invalid indexes
                // when this function fails, it is rarely
                // with a nice 0 index (value is garbage)
                if (index == 0) {
                    console.error("nativefs error: could not dispatch functionindex is : " + index );
		            return;
                }
                NATIVEFS.MAIN_THREAD_CONSTS[index]();
            },
            function () {
                var view = new Int32Array(Module.wasmMemory.buffer, NATIVEFS.handle, 5);
                // FIXME: reassign or remove?
            },
            /* nativefs_read */
            function () {
                var view = new Int32Array(Module.wasmMemory.buffer, NATIVEFS.handle, 5);
                NATIVEFS.worker.postMessage({
                    ops: "read",
                    addr: Atomics.load(view, 2),
                    handle: Atomics.load(view, 3),
                    size: Atomics.load(view, 4)
                });
            },
            /* nativefs_seek */
            function () {
                var view = new Int32Array(Module.wasmMemory.buffer, NATIVEFS.handle, 5);
                NATIVEFS.worker.postMessage({
                    ops: "seek",
                    handle: Atomics.load(view, 2),
                    offset: Atomics.load(view, 3),
                    whence: Atomics.load(view, 4)
                });
            },
            /* get file size */
            function() {
                var view = new Int32Array(Module.wasmMemory.buffer, NATIVEFS.handle, 5);
                var handle = Atomics.load(view, 2);
                Atomics.store(view, 3, NATIVEFS.files[handle - 3].fileSize);
            },
            /* close file */
            function() {
                var view = new Int32Array(Module.wasmMemory.buffer, NATIVEFS.handle, 5);
                var fd = Atomics.load(view, 2);
                // remove file from files entry
                NATIVEFS.files[fd - 3] = {};
                // remove from files in the worker
                NATIVEFS.worker.postMessage({ ops: "close_file", fd: fd });
                Atomics.store(view, 3, 1);
            }
        ],
    },
    /*
      If we want to break free from the emscripten demo page, we will need
      to attach the event listener that connects the file with NATIVEFS to a
      specific element.      
    */
    nativefs_init__deps: ['$findEventTarget'],
    nativefs_init: function(element) {
        if (ENVIRONMENT_IS_PTHREAD) {
            err("nativefs: cannot init nativefs in a pthread!");
            return -1;
        }
        let fileHandle;
        let fileCount = 0;
        element = document.getElementById("NATIVEFS_button");
        if (!element) {
            element = findEventTarget(element);
            // FIXME: what happens if findEventTarget does not work?
        }
        element.addEventListener('click', async function() {
            element.innerHTML = "Selecting File";
            // TODO: handle compat mode for firefox
            // showOpenFilePicker() is a chrome/safari only api for now
            [fileHandle] = await window.showOpenFilePicker();
            fileHandle.getFile().then((blob) => {
                // Send the file to the nativefs_worker
                NATIVEFS.worker.postMessage( {
                    ops: "push_file",
                    file: blob,
                });
                // keep the record in the main thread
                // (the same will be done in the worker)
                NATIVEFS.files.push({
                    fileName: blob.name,
                    fileSize: blob.size
                });
                fileCount += 1;
                element.innerHTML = "Opened "+ fileCount + " files";
            });
        });
        return 0;
    },
    /*
      FIXME: helper function to get a value from the main thread,
      via shared memory
    */
    nativefs_get_prop_from_main_thread__deps: ['$mainThreadEM_ASM'],
    nativefs_get_prop_from_main_thread: function() {
        //FIXME: not implemented, not sure I need it anymore
        if (!ENVIRONMENT_IS_PTHREAD) {
            err("nativefs: operation not available in the main thread!");
            return -1;
        }
        var nativefs_handle = Module.PThread.nativeFSHandle;
        var view = new Int32Array(Module.wasmMemory.buffer, nativefs_handle, 5);
        Atomics.store(view,1, 1);
        mainThreadEM_ASM(-1, 0, 0, 1);
    },
    nativefs_read__deps: ['$mainThreadEM_ASM'],
    nativefs_read: function(fd, buffer, size) {
        if (!ENVIRONMENT_IS_PTHREAD) {
            err("nativefs: operation not available in the main thread!");
            return -1;
        }
        if ((fd == -1) || (buffer == 0) || (fd > {{{ NATIVEFS_MAX_FD }}}))
            return -1;
        if (size == 0)
            return 0;

        var nativefs_handle = Module.PThread.nativeFSHandle;
        var view = new Int32Array(Module.wasmMemory.buffer, nativefs_handle, 5);
        Atomics.store(view, 1, 4);
        Atomics.store(view, 2, fd);
        mainThreadEM_ASM(-1, 0, 0, 1);
        var fileSize = Atomics.load(view, 3);

        if (size > fileSize)
            size = fileSize;
        
        var index = fd - 2;
        var file_state = new Int32Array(Module.wasmMemory.buffer, Module.PThread.nativeFSHandle + (5*4) + (index*16), 4);
        // check if the file was pushed                             
        Atomics.wait(file_state, 0, 0);
        
        // Init read size: (don't touch read_size if the lock is still acquired)
        Atomics.wait(file_state, 1, 1);
        Atomics.store(file_state, 2, -1);
        
        // Acquire lock: (unblock atomic.wait() in read())
        Atomics.store(file_state, 1, 1);
        Atomics.notify(file_state, 1);

        // setup read call in mainThread
        Atomics.store(view, 1, 2);
        // set read ops parameters
        Atomics.store(view, 2, buffer);
        Atomics.store(view, 3, fd);
        Atomics.store(view, 4, size);

        mainThreadEM_ASM(-1, 0, 0, 1);

        // Wait for the size notification
        Atomics.wait(file_state, 2, -1);
        // release the lock:
        Atomics.wait(file_state, 1, 1);

        // return number of bytes read
        return Atomics.load(file_state, 2);
    },
    nativefs_seek: function(fd, offset, whence) {
        if (!ENVIRONMENT_IS_PTHREAD) {
            err("nativefs: operation not available in the main thread!");
            return -1;
        }
        if ((fd < 3) || (fd > {{{ NATIVEFS_MAX_FD }}}))
            return -1;
        var nativefs_handle = Module.PThread.nativeFSHandle;
        var view = new Int32Array(Module.wasmMemory.buffer, nativefs_handle, 5);
        var index = fd - 2;
        var file_state = new Int32Array(Module.wasmMemory.buffer, Module.PThread.nativeFSHandle + (5*4) + (index*16), 4);

        /*
          Setup get_filesize call in mainThread
          it will set the fileSize in the sharedMemory
          to allow us to read it in Atomics.load() call below
        */
        Atomics.store(view, 1, 4);
        Atomics.store(view, 2, fd);
        mainThreadEM_ASM(-1, 0, 0, 1);

        var fileSize = Atomics.load(view, 3);
        
        var currentOffset = Atomics.load(file_state, 3);
        if ( whence === {{{ cDefine('NATIVEFS_SEEK_SET') }}} ) {
            if ((offset < 0) || (offset > fileSize)) {
                return -1;
            }
        }
        else if ( whence === {{{ cDefine('NATIVEFS_SEEK_CUR') }}} ) {
            if ((currentOffset + offset < 0) || (currentOffset + offset > fileSize)) {
                return -1;
            }
        }
        else if ( whence === {{{ cDefine('NATIVEFS_SEEK_END') }}} ) {            
            if ((fileSize + offset < 0) || (fileSize + offset > fileSize)) {
                return -1;
            }
        }
        else {
            return -1;
        }            

        // check if the file was pushed
        Atomics.wait(file_state, 0, 0);

        // Acquire lock: (unblock atomic.wait() in seek())
        Atomics.store(file_state, 1, 1);
        Atomics.notify(file_state, 1);

        // setup seek call in mainThread
        Atomics.store(view, 1, 3);
        // set seek ops parameters
        Atomics.store(view,2, fd);
        Atomics.store(view,3, offset);
        Atomics.store(view,4, whence);

        mainThreadEM_ASM(-1, 0, 0, 1);

        // release the lock here
        Atomics.wait(file_state, 1, 1);
        if (file_state[1] == -1) {
            // the call failed
            return -1;
        }
        // return the new file offset
        return file_state[3];
    },
    nativefs_get_fd_from_filename: function(encodedName) {
        if (ENVIRONMENT_IS_PTHREAD) {
            err("nativefs: operation not available in the pthread!");
            return -1;
        }
        if (!encodedName)
            return -1;
        var textdecoder = new TextDecoder();
        var filename_view = textdecoder.decode(encodedName);
        var flag = 0;
        var j = 0;
        for (file in NATIVEFS.files) {
            var i = 0;
            for (let c of filename_view.entries()) {
                // item is [index, value]
                if (item[1] !== file.fileName[i++]) {
                    flag = 1;
                    break;
                }
            }
            // reached here without setting the flag to 1
            if (flag === 0)
                return (j + 3);
            j++;
        }
        // reached end without setting the flag to 1
        return -1;
    },
    /*
      helper function to mark the specified files item for gc.
    */
    nativefs_close_file: function(fd) {
        if (!ENVIRONMENT_IS_PTHREAD) {
            err("nativefs: operation not available in the main thread!");
            return -1;
        }
        if ((fd < 3) || (fd > {{{ NATIVEFS_MAX_FD }}}))
            return -1;        
        var nativefs_handle = Module.PThread.nativeFSHandle;
        var view = new Int32Array(Module.wasmMemory.buffer, nativefs_handle, 5);
        /* Setup close_file call in the main Thread */
        Atomics.store(view, 1, 5);
        Atomics.store(view, 2, fd);
        mainThreadEM_ASM(-1, 0, 0, 1);

        // reset file state to 0
        var index = fd - 2;
        var file_state = new Int32Array(Module.wasmMemory.buffer, Module.PThread.nativeFSHandle + (5*4) + (index*16), 4);
        Atomics.store(file_state, 0, 0);
        Atomics.store(file_state, 1, 0);
        Atomics.store(file_state, 2, -1);
        Atomics.store(file_state, 3, -1);

        // return success/error
        var ret = Atomics.load(view, 3);
        if (ret == 1)
            return 0;
        else            
            return -1;
    },
    /*
      This function should be called when EXIT_RUNTIME is marked in effect.
      It will invalidate the nativefs handle in the pthreads.
      No function can be called after that.
    */
    nativefs_cleanup: function() {
        if (ENVIRONMENT_IS_PTHREAD) {
            err("nativefs: operation not available in a pthread!");
            return -1;
        }
        if ((!NATIVEFS.handle) || (!NATIVEFS.worker)) {
            return -1;
        }
        if (NATIVEFS.handle) {
            Module._free(NATIVEFS.handle);
            delete NATIVEFS.files;
            delete NATIVEFS.worker;
            delete NATIVEFS.handle;
        }
        return 0;
    },
}

autoAddDeps(LibraryNATIVEFS, '$JSEvents');
mergeInto(LibraryManager.library, LibraryNATIVEFS);
if (EMSCRIPTEN_NATIVE_FS) {
    DEFAULT_LIBRARY_FUNCS_TO_INCLUDE.push('$NATIVEFS');
}
