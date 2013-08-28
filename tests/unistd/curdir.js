var dummy_device = FS.makedev(64, 0);
FS.registerDevice(dummy_device, {});

FS.createDataFile('/', 'file', '', true, true);
FS.createFolder('/', 'folder', true, true);
FS.mkdev('/device', 0666, dummy_device);
FS.createLink('/', 'link', 'folder', true, true);
