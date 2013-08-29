(function() {
  var major = 80;

  var device = FS.makedev(major++, 0);
  var device_ops = {
    open: function(stream) {
      stream.payload = [65, 66, 67, 68];
    },
    read: function(stream, buffer, offset, length, pos) {
      var bytesRead = 0;
      for (var i = 0; i < length; i++) {
        if (stream.payload.length) {
          bytesRead++;
          buffer[offset+i] = stream.payload.shift();
        } else {
          break;
        }
      }
      return bytesRead;
    },
    write: function(stream, buffer, offset, length, pos) {
      for (var i = 0; i < length; i++) {
        Module.print("TO DEVICE: " + buffer[offset+i]);
      }
      return i;
    }
  };
  FS.registerDevice(device, device_ops);

  FS.mkdev('/device', 0666, device);

  var broken_device = FS.makedev(major++, 0);
  var broken_device_ops = {
    read: function(stream, buffer, offset, length, pos) {
      throw new FS.ErrnoError(ERRNO_CODES.EIO);
    },
    write: function(stream, buffer, offset, length, pos) {
      throw new FS.ErrnoError(ERRNO_CODES.EIO);
    }
  };
  FS.registerDevice(broken_device, broken_device_ops);

  FS.mkdev('/broken-device', 0666, broken_device);

  // NB: These are meant to test FS.createDevice specifically,
  //     and as such do not use registerDevice/mkdev
  FS.createDevice('/', 'createDevice-read-only', function() {});
  FS.createDevice('/', 'createDevice-write-only', null, function() {});

  FS.createDataFile('/', 'file', '1234567890', true, true);
  FS.createFolder('/', 'folder', true, true);
})();
