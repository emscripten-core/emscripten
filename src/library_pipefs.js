mergeInto(LibraryManager.library, {
  $PIPEFS__postset: '__ATINIT__.push(function() { PIPEFS.root = FS.mount(PIPEFS, {}, null); });',
  $PIPEFS__deps: ['$FS'],
  $PIPEFS: {
    BUCKET_BUFFER_SIZE: 1024 * 8, // 8KiB Buffer
    mount: function (mount) {
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

      return [ readableStream.fd, writableStream.fd ];
    },
    stream_ops: {
      poll: function (stream) {
        var pipe = stream.node.pipe;
        var currentLength = 0;

        if (pipe.buckets.length > 0) {
          pipe.buckets.forEach(function (bucket) {
            currentLength += bucket.offset - bucket.roffset;
          });
        }

        return (currentLength > 0) ? ({{{ cDefine('POLLRDNORM') }}} | {{{ cDefine('POLLIN') }}}) : 0;
      },
      ioctl: function (stream, request, varargs) {
        return ERRNO_CODES.EINVAL;
      },
      read: function (stream, buffer, offset, length, position /* ignored */) {
        var pipe = stream.node.pipe;
        var currentLength = 0;

        pipe.buckets.forEach(function (bucket) {
          currentLength += bucket.offset - bucket.roffset;
        });

        assert(buffer instanceof ArrayBuffer || ArrayBuffer.isView(buffer));
        var data = new Uint8Array(buffer);
        data = buffer.subarray(offset, offset + length);

        var toRead = Math.min(currentLength, length);
        if (toRead == 0) {
          return 0;
        }

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

        while (toRemove--) {
          pipe.buckets.shift();
        }

        return totalRead;
      },
      write: function (stream, buffer, offset, length, position /* ignored */) {
        var pipe = stream.node.pipe;

        assert(buffer instanceof ArrayBuffer || ArrayBuffer.isView(buffer));
        var data = new Uint8Array(buffer);
        data = buffer.subarray(offset, offset + length);

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

        var numBuckets = ~~(data.byteLength / PIPEFS.BUCKET_BUFFER_SIZE);
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
