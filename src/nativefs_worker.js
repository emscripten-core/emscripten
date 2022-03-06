var files = [];
var fd = 2;
var sharedMemory;
var fs_lock = -1;
var nativefs_handle;

this.onmessage = function (msg) {
    if (msg.data.ops === "push_file") {
        if (msg.data.file === undefined) {
            console.error("invalid operation: called push_file operation with invalid" +
                          " file: " + msg.data.file + " } !");
            return ;
        }
        fd += 1;
        files.push({
            handle: fd,
            offset: 0,
            file: msg.data.file
        });

        var index = fd - 2;
        var file_state = new Int32Array(sharedMemory, nativefs_handle + (5*4) + (index*16), 4);

        Atomics.store(file_state, 1, 0);
        Atomics.notify(file_state, 1);
        Atomics.store(file_state, 0, fd);
        Atomics.notify(file_state, 0);
    }
    else if (msg.data.ops === "close_file") {
        if (msg.data.fd === undefined) {
            console.error("invalid operation: called close_file operation with invalid" +
                          " fd: " + msg.data.fd + " } !");
            return ;
        }
        
    }
    else if (msg.data.ops === "close_file") {
        if (msg.data.fs === undefined) {
            console.error("invalid operation: called close_file operation with invalid" +
                          " fd : " + msg.data.fd + " } !");
            return ;
        }
        files[fd - 3].handle = 0;
        files[fd - 3].offset = 0;
        files[fd - 3].file = 0;
    }
    /* not implemented, only for debug */
    else if (msg.data.ops === "cleanup_nativefs") {
        for (let file of files)
            console.log(file);
        files = [];
    }
    else if (msg.data.ops === "init_nativefs") {
        /*
          worker is ready to receive the runtime
        */
        this.postMessage( { ops: "init_nativefs" } );
    }
    else if (msg.data.ops === "init_nativefs_runtime") {
        if (msg.data.moduleWasmMemory === undefined) {
            console.error("invalid operation: called init_nativefs_runtime operation with invalid" +
                          " { moduleWasmMemory: " +
                          msg.data.moduleWasmMemory +
                          " nativefs_handle: " +
                          msg.data.handle + " } !");
            return ;
        }
        sharedMemory = msg.data.moduleWasmMemory;
        nativefs_handle = msg.data.handle;
        this.postMessage( { ops: "init_fs_lock" } );
    }
    else if (msg.data.ops === "read") {
        if (msg.data.addr === undefined
            || msg.data.handle === undefined
            || msg.data.size === undefined) {
            console.error("invalid operation: called read operation with invalid { addr: " +
                          msg.data.addr + " handle: " + msg.data.handle +
                          " size: " + msg.data.size + " } !");
            return ;
        }
        let p_buffer = msg.data.addr;
        let index = msg.data.handle - 3;
        let size = msg.data.size;
        let offset = files[index].offset;
        let file = files[index].file;
        let handle_index = msg.data.handle - 2;

        var file_state = new Int32Array(sharedMemory, nativefs_handle + (5*4) + (handle_index*16), 4);

        // wait for read() call to acquire lock
        Atomics.wait(file_state, 1, 0);
        let blob = new Blob(
            [file.slice(offset, offset + size)],
            { type: 'application/octet-stream' });
        let reader = new FileReaderSync();
        let result = reader.readAsArrayBuffer(blob);
        let dataView = new Uint8Array(result);
        let wasmMemoryView = new Uint8Array(sharedMemory, p_buffer, dataView.length);
        let wasmMemoryItems = wasmMemoryView.entries();
        for ( let item of dataView.entries() ) {
            //item is [index, value]
            let ind = wasmMemoryItems.next().value[0];
            wasmMemoryView[ind] = item[1];
        }
        files[index].offset += dataView.length;
        Atomics.store(file_state, 3, files[index].offset);
        // finished reading:
        // 1. udpate size
        Atomics.store(file_state, 2, dataView.length);
        Atomics.notify(file_state, 2);
        // 2. notify read()
        Atomics.store(file_state, 1, 0);
        Atomics.notify(file_state, 1);
    }
    else if (msg.data.ops === "seek") {
        if (msg.data.offset === undefined
            || msg.data.whence === undefined
            || msg.data.handle === undefined) {
            console.error("invalid operation: called seek operation with invalid { offset: " + msg.data.offset
                          + " whence: " + msg.data.whence
                          + " handle: " + msg.data.handle
                          + " } !");
            return ;
        }
        let handle_index = msg.data.handle - 2;
        var file_state = new Int32Array(sharedMemory, nativefs_handle + (5*4) + (handle_index*16), 4);
        let index = msg.data.handle - 3;
        
        // wait for seek() call to acquire lock
        Atomics.wait(file_state, 1, 0);
        let new_offset = msg.data.offset;
        if (msg.data.whence === {{{ cDefine('NATIVEFS_SEEK_CUR') }}} ) {
            new_offset = files[index].offset + msg.data.offset; 
        }
        else if (msg.data.whence === {{{ cDefine('NATIVEFS_SEEK_END') }}} ) {
            new_offset = files[index].file.size + msg.data.offset;
        }
        files[index].offset = new_offset;

        // notify seek()
        Atomics.store(file_state, 1, 0);
        Atomics.notify(file_state, 1);
        Atomics.store(file_state, 3, new_offset);
        Atomics.notify(file_state, 3);
    }
}
