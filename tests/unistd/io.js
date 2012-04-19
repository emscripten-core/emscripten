(function() {
  var devicePayload = [65, 66, 67, 68];
  FS.createDevice('/', 'device', function() {
    if (devicePayload.length) {
      return devicePayload.shift();
    } else {
      return null;
    }
  }, function(arg) {
    Module.print("TO DEVICE: " + arg);
  });
  FS.createDevice('/', 'broken-device', function() {
    throw new Error('Broken device input.');
  }, function(arg) {
    throw new Error('Broken device output.');
  });
  FS.createDataFile('/', 'file', '1234567890', true, true);
  FS.createFolder('/', 'folder', true, true);
})();
