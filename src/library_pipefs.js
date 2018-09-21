// Copyright 2017 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

mergeInto(LibraryManager.library, {
  $PIPEFS__postset: '__ATINIT__.push(function() { PIPEFS.root = FS.mount(PIPEFS, {}, null); });',
  $PIPEFS__deps: ['$FS'],
  $PIPEFS: {
    BUCKET_BUFFER_SIZE: 1024 * 8, // 8KiB Buffer
    mount: function (mount) {
      // Do not pollute the real root directory or its child nodes with pipes
      // Looks like it is OK to create another pseudo-root node not linked to the FS.root hierarchy this way
      return FS.createNode(null, '/', {{{ cDefine('S_IFDIR') }}} | 511 /* 0777 */, 0);
    },
    createPipe: function () {
      var pipe = {
        buckets: []
      };

      pipe.buckets.push({
        buffer: new Uint8Array(PIPEFS.BUCKET_BUFFER_SIZE),
        offset: 0,
        roffset: 0
      });

      var rName = PIPEFS.nextname();
      var wName = PIPEFS.nextname();
      var rNode = FS.createNode(PIPEFS.root, rName, {{{ cDefine('S_IFIFO') }}}, 0);
      var wNode = FS.createNode(PIPEFS.root, wName, {{{ cDefine('S_IFIFO') }}}, 0);

      rNode.pipe = pipe;
      wNode.pipe = pipe;

      var readableStream = FS.createStream({
        path: rName,
        node: rNode,
        flags: FS.modeStringToFlags('r'),
        seekable: false,
        stream_ops: PIPEFS.stream_ops
      });
      rNode.stream = readableStream;

      var writableStream = FS.createStream({
        path: wName,
        node: wNode,
        flags: FS.modeStringToFlags('w'),
        seekable: false,
        stream_ops: PIPEFS.stream_ops
      });
      wNode.stream = writableStream;

      return {
        readable_fd: readableStream.fd,
        writable_fd: writableStream.fd
      };
    },
    stream_ops: {
      poll: function (stream) {
        var pipe = stream.node.pipe;

        if ((stream.flags & {{{ cDefine('O_ACCMODE') }}}) === {{{ cDefine('O_WRONLY') }}}) {
          return ({{{ cDefine('POLLWRNORM') }}} | {{{ cDefine('POLLOUT') }}});
        } else {
          if (pipe.buckets.length > 0) {
            for (var i = 0; i < pipe.buckets.length; i++) {
              var bucket = pipe.buckets[i];
              if (bucket.offset - bucket.roffset > 0) {
                return ({{{ cDefine('POLLRDNORM') }}} | {{{ cDefine('POLLIN') }}});
              }
            }
          }
        }

        return 0;
      },
      ioctl: function (stream, request, varargs) {
        return ERRNO_CODES.EINVAL;
      },
      read: function (stream, buffer, offset, length, position /* ignored */) {
        var pipe = stream.node.pipe;
        var currentLength = 0;

        for (var i = 0; i < pipe.buckets.length; i++) {
          var bucket = pipe.buckets[i];
          currentLength += bucket.offset - bucket.roffset;
        }

#if USE_PTHREADS
        assert(buffer instanceof ArrayBuffer || buffer instanceof SharedArrayBuffer || ArrayBuffer.isView(buffer));
#else
        assert(buffer instanceof ArrayBuffer || ArrayBuffer.isView(buffer));
#endif
        var data = buffer.subarray(offset, offset + length);

        if (length <= 0) {
          return 0;
        }
        if (currentLength == 0) {
          // Behave as if the read end is always non-blocking
          throw new FS.ErrnoError(ERRNO_CODES.EAGAIN);
        }
        var toRead = Math.min(currentLength, length);

        var totalRead = toRead;
        var toRemove = 0;

        for (var i = 0; i < pipe.buckets.length; i++) {
          var currBucket = pipe.buckets[i];
          var bucketSize = currBucket.offset - currBucket.roffset;

          if (toRead <= bucketSize) {
            var tmpSlice = currBucket.buffer.subarray(currBucket.roffset, currBucket.offset);
            if (toRead < bucketSize) {
              tmpSlice = tmpSlice.subarray(0, toRead);
              currBucket.roffset += toRead;
            } else {
              toRemove++;
            }
            data.set(tmpSlice);
            break;
          } else {
            var tmpSlice = currBucket.buffer.subarray(currBucket.roffset, currBucket.offset);
            data.set(tmpSlice);
            data = data.subarray(tmpSlice.byteLength);
            toRead -= tmpSlice.byteLength;
            toRemove++;
          }
        }

        if (toRemove && toRemove == pipe.buckets.length) {
          // Do not generate excessive garbage in use cases such as
          // write several bytes, read everything, write several bytes, read everything...
          toRemove--;
          pipe.buckets[toRemove].offset = 0;
          pipe.buckets[toRemove].roffset = 0;
        }

        pipe.buckets.splice(0, toRemove);

        return totalRead;
      },
      write: function (stream, buffer, offset, length, position /* ignored */) {
        var pipe = stream.node.pipe;

#if USE_PTHREADS
        assert(buffer instanceof ArrayBuffer || buffer instanceof SharedArrayBuffer || ArrayBuffer.isView(buffer));
#else
        assert(buffer instanceof ArrayBuffer || ArrayBuffer.isView(buffer));
#endif
        var data = buffer.subarray(offset, offset + length);

        var dataLen = data.byteLength;
        if (dataLen <= 0) {
          return 0;
        }

        var currBucket = null;

        if (pipe.buckets.length == 0) {
          currBucket = {
            buffer: new Uint8Array(PIPEFS.BUCKET_BUFFER_SIZE),
            offset: 0,
            roffset: 0
          };
          pipe.buckets.push(currBucket);
        } else {
          currBucket = pipe.buckets[pipe.buckets.length - 1];
        }

        assert(currBucket.offset <= PIPEFS.BUCKET_BUFFER_SIZE);

        var freeBytesInCurrBuffer = PIPEFS.BUCKET_BUFFER_SIZE - currBucket.offset;
        if (freeBytesInCurrBuffer >= dataLen) {
          currBucket.buffer.set(data, currBucket.offset);
          currBucket.offset += dataLen;
          return dataLen;
        } else if (freeBytesInCurrBuffer > 0) {
          currBucket.buffer.set(data.subarray(0, freeBytesInCurrBuffer), currBucket.offset);
          currBucket.offset += freeBytesInCurrBuffer;
          data = data.subarray(freeBytesInCurrBuffer, data.byteLength);
        }

        var numBuckets = (data.byteLength / PIPEFS.BUCKET_BUFFER_SIZE) | 0;
        var remElements = data.byteLength % PIPEFS.BUCKET_BUFFER_SIZE;

        for (var i = 0; i < numBuckets; i++) {
          var newBucket = {
            buffer: new Uint8Array(PIPEFS.BUCKET_BUFFER_SIZE),
            offset: PIPEFS.BUCKET_BUFFER_SIZE,
            roffset: 0
          };
          pipe.buckets.push(newBucket);
          newBucket.buffer.set(data.subarray(0, PIPEFS.BUCKET_BUFFER_SIZE));
          data = data.subarray(PIPEFS.BUCKET_BUFFER_SIZE, data.byteLength);
        }

        if (remElements > 0) {
          var newBucket = {
            buffer: new Uint8Array(PIPEFS.BUCKET_BUFFER_SIZE),
            offset: data.byteLength,
            roffset: 0
          };
          pipe.buckets.push(newBucket);
          newBucket.buffer.set(data);
        }

        return dataLen;
      },
      close: function (stream) {
        var pipe = stream.node.pipe;
        pipe.buckets = null;
      }
    },
    nextname: function () {
      if (!PIPEFS.nextname.current) {
        PIPEFS.nextname.current = 0;
      }
      return 'pipe[' + (PIPEFS.nextname.current++) + ']';
    },
  },
});
