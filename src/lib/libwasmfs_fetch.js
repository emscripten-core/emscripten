/**
 * @license
 * Copyright 2022 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

addToLibrary({
  $wasmFS$JSMemoryRanges: {},

  // Fetch backend: On first access of the file (either a read or a getSize), it
  // will fetch() the data from the network asynchronously. Otherwise, after
  // that fetch it behaves just like JSFile (and it reuses the code from there).

  _wasmfs_create_fetch_backend_js__deps: [
    '$wasmFS$backends',
    '$wasmFS$JSMemoryRanges',
    '_wasmfs_fetch_get_file_url',
    '_wasmfs_fetch_get_chunk_size',
  ],
  _wasmfs_create_fetch_backend_js: async function(backend) {
    // Get a promise that fetches the data and stores it in JS memory (if it has
    // not already been fetched).
    async function getFileRange(file, offset, len) {
      var url = '';
      var fileUrl_p = __wasmfs_fetch_get_file_url(file);
      var fileUrl = UTF8ToString(fileUrl_p);
      var isAbs = fileUrl.indexOf('://') !== -1;
      if (isAbs) {
        url = fileUrl;
      } else {
        try {
          var u = new URL(fileUrl, self.location.origin);
          url = u.toString();
        } catch (_e) {
          throw {status: 404};
        }
      }
      var chunkSize = __wasmfs_fetch_get_chunk_size(file);
      offset ??= 0;
      len = len || chunkSize;
      var firstChunk = (offset / chunkSize) | 0;
      var lastChunk = ((offset+len) / chunkSize) | 0;
      if (!(file in wasmFS$JSMemoryRanges)) {
        var fileInfo = await fetch(url,{method:"HEAD", headers:{"Range": "bytes=0-"}});
        if (fileInfo.ok &&
           fileInfo.headers.has("Content-Length") &&
           fileInfo.headers.get("Accept-Ranges") == "bytes" &&
           (parseInt(fileInfo.headers.get("Content-Length")) > chunkSize*2)) {
          wasmFS$JSMemoryRanges[file] = {size:parseInt(fileInfo.headers.get("Content-Length")), chunks:[], chunkSize:chunkSize};
        } else {
          // may as well/forced to download the whole file
          var wholeFileReq = await fetch(url);
          if (!wholeFileReq.ok) {
            throw wholeFileReq;
          }
          var wholeFileData = new Uint8Array(await wholeFileReq.arrayBuffer());
          var text = new TextDecoder().decode(wholeFileData);
          wasmFS$JSMemoryRanges[file] = {size:wholeFileData.byteLength, chunks:[wholeFileData], chunkSize:wholeFileData.byteLength};
          return Promise.resolve();
        }
      }
      var allPresent = true;
      var i;
      if (lastChunk * chunkSize < offset+len) {
        lastChunk += 1;
      }
      for (i = firstChunk; i < lastChunk; i++) {
        if (!wasmFS$JSMemoryRanges[file].chunks[i]) {
          allPresent = false;
          break;
        }
      }
      if (allPresent) {
        // The data is already here, so nothing to do before we continue on to
        // the actual read.
        return Promise.resolve();
      }
      // This is the first time we want the chunk's data.
      var start = firstChunk*chunkSize;
      var end = lastChunk*chunkSize;
      var response = await fetch(url, {headers:{"Range": `bytes=${start}-${end-1}`}});
      if (!response.ok) {
        throw response;
      }
      var bytes = new Uint8Array(await response['arrayBuffer']());
      for (i = firstChunk; i < lastChunk; i++) {
        wasmFS$JSMemoryRanges[file].chunks[i] = bytes.slice(i*chunkSize-start,(i+1)*chunkSize-start);
      }
      return Promise.resolve();
    }

    wasmFS$backends[backend] = {
      // alloc/free operations are not actually async. Just forward to the
      // parent class, but we must return a Promise as the caller expects.
      allocFile: async (file) => {
        // nop
        return Promise.resolve();
      },
      freeFile: async (file) => {
        // free memory
        wasmFS$JSMemoryRanges[file] = undefined;
        return Promise.resolve();
      },

      write: async (file, buffer, length, offset) => {
        abort("TODO: file writing in fetch backend? read-only for now");
      },

      // read/getSize fetch the data, then forward to the parent class.
      read: async (file, buffer, length, offset) => {
        try {
          await getFileRange(file, offset || 0, length);
        } catch (failedResponse) {
          return failedResponse.status === 404 ? -{{{ cDefs.ENOENT }}} : -{{{ cDefs.EBADF }}};
        }
        var fileInfo = wasmFS$JSMemoryRanges[file];
        var fileData = fileInfo.chunks;
        var chunkSize = fileInfo.chunkSize;
        var firstChunk = (offset / chunkSize) | 0;
        var lastChunk = ((offset+length) / chunkSize) | 0;
        if (offset + length > lastChunk * chunkSize) {
          lastChunk += 1;
        }
        var readLength = 0;
        for (var i = firstChunk; i < lastChunk; i++) {
          var chunk = fileData[i];
          var start = Math.max(i*chunkSize, offset);
          var chunkStart = i*chunkSize;
          var end = Math.min(chunkStart+chunkSize, offset+length);
          HEAPU8.set(chunk.subarray(start-chunkStart, end-chunkStart), buffer+(start-offset));
          readLength = end - offset;
        }
        return readLength;
      },
      getSize: async (file) => {
        try {
          await getFileRange(file, 0, 0);
        } catch (failedResponse) {
          return 0;
        }
        return wasmFS$JSMemoryRanges[file].size;
      },
    };
  },

});
