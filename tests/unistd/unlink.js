FS.createDevice('/', 'device', function() {}, function() {});
FS.createDataFile('/', 'file', 'test', true, true);
FS.createDataFile('/', 'file-forbidden', 'test', true, false);
FS.createFolder('/', 'empty', true, true);
FS.createFolder('/', 'empty-forbidden', true, false);
FS.createFolder('/', 'full', true, true);
FS.createFolder('/full', 'junk', true, true);
