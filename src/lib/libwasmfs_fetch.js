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
      len ??= chunkSize;
      // In which chunk does the seeked range start?  E.g., 5-14 with chunksize 8 will start in chunk 0.
      if (!(file in wasmFS$JSMemoryRanges)) {
        var fileInfo = await fetch(url, {method:'HEAD', headers:{'Range': 'bytes=0-'}});
        if (fileInfo.ok &&
            fileInfo.headers.has('Content-Length') &&
            fileInfo.headers.get('Accept-Ranges') == 'bytes' &&
            (parseInt(fileInfo.headers.get('Content-Length'), 10) > chunkSize*2)) {
          var size = parseInt(fileInfo.headers.get('Content-Length'), 10);
          wasmFS$JSMemoryRanges[file] = {
            size,
            chunks: [],
            chunkSize: chunkSize
          };
          len = Math.min(len, size-offset);
        } else {
          // may as well/forced to download the whole file
          var wholeFileReq = await fetch(url);
          if (!wholeFileReq.ok) {
            throw wholeFileReq;
          }
          var wholeFileData = new Uint8Array(await wholeFileReq.arrayBuffer());
          wasmFS$JSMemoryRanges[file] = {
            size: wholeFileData.byteLength,
            chunks: [wholeFileData],
            chunkSize: wholeFileData.byteLength
          };
          return;
        }
      }
      var firstChunk = (offset / chunkSize) | 0;
      // In which chunk does the seeked range end?  E.g., 5-14 with chunksize 8 will end in chunk 1, as will 5-16 (since byte 16 isn't requested).
      // This will always give us a chunk >= firstChunk since len > 0.
      var lastChunk = ((offset+len-1) / chunkSize) | 0;
      var allPresent = true;
      var i;
      // Do we have all the chunks already?  If so, we don't need to do any fetches.
      for (i = firstChunk; i <= lastChunk; i++) {
        if (!wasmFS$JSMemoryRanges[file].chunks[i]) {
          allPresent = false;
          break;
        }
      }
      if (allPresent) {
        // The data is already here, so nothing to do before we continue on to
        // the actual read.
        return;
      }
      // This is the first time we want the chunks' data.  We'll make
      // one request for all the chunks we need, rather than one
      // request per chunk.
      var start = firstChunk * chunkSize;
      // We must fetch *up to* the last byte of the last chunk.
      var end = (lastChunk+1) * chunkSize;
      var response = await fetch(url, {headers:{'Range': `bytes=${start}-${end-1}`}});
      if (!response.ok) {
        throw response;
      }
      var bytes = await response['bytes']();
      for (i = firstChunk; i <= lastChunk; i++) {
        wasmFS$JSMemoryRanges[file].chunks[i] = bytes.slice(i*chunkSize-start,(i+1)*chunkSize-start);
      }
    }

    wasmFS$backends[backend] = {
      // alloc/free operations are not actually async. Just forward to the
      // parent class, but we must return a Promise as the caller expects.
      allocFile: async (file) => { /* nop */ },
      freeFile: async (file) => {
        // free memory
        wasmFS$JSMemoryRanges[file] = undefined;
      },

      write: async (file, buffer, length, offset) => {
        console.error('TODO: file writing in fetch backend? read-only for now');
      },

      // read/getSize fetch the data, then forward to the parent class.
      read: async (file, buffer, length, offset) => {
        // This function assumes that offset is non-negative and length is positive.
        // C read() doesn't take an offset and so doesn't have to deal with the former situation,
        // and if the length is 0 or the offset is negative there's no reasonable read we can make.
        if (offset < 0 || length <= 0) {
          return 0;
        }
        try {
          await getFileRange(file, offset || 0, length);
        } catch (failedResponse) {
          return failedResponse.status === 404 ? -{{{ cDefs.ENOENT }}} : -{{{ cDefs.EBADF }}};
        }
        var fileInfo = wasmFS$JSMemoryRanges[file];
        length = Math.min(length, fileInfo.size-offset);
        // As above, we check the length just in case offset was beyond size and length is now negative.
        if (length <= 0) {
          return 0;
        }
        var chunks = fileInfo.chunks;
        var chunkSize = fileInfo.chunkSize;
        var firstChunk = (offset / chunkSize) | 0;
        // See comments in getFileRange.
        var lastChunk = ((offset+length-1) / chunkSize) | 0;
        var readLength = 0;
        for (var i = firstChunk; i <= lastChunk; i++) {
          var chunk = chunks[i];
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
