FS.createDataFile('/', 'file', '', true, true);
FS.createFolder('/', 'folder', true, true);
FS.createDevice('/', 'device', function() {}, function() {});
FS.createLink('/', 'link', 'folder', true, true);
