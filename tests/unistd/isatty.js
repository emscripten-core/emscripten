FS.createDevice('/', 'read', function() {}, null);
FS.createDevice('/', 'write', null, function() {});
FS.createDevice('/', 'all', function() {}, function() {});
FS.createFolder('/', 'folder', true, true);
FS.createDataFile('/', 'file', 'test', true, true);
