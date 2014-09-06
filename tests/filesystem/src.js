var dummy_device = FS.makedev(64, 0);
FS.registerDevice(dummy_device, {});

FS.mkdir('/forbidden', 0000);
FS.mkdir('/forbidden/test');
FS.mkdir('/abc');
FS.mkdir('/abc/123');
FS.mkdir('/abc/456');
FS.mkdir('/def');
FS.mkdir('/def/789');
FS.mkdev('/abc/deviceA', dummy_device);
FS.mkdev('/def/deviceB', dummy_device);
FS.symlink('123', '/abc/localLink');
FS.symlink('/', '/abc/rootLink');
FS.symlink('../def', '/abc/relativeLink');
FS.ignorePermissions = false;

function explore(path) {
  Module.print(path);
  var ret = FS.analyzePath(path);
  Module.print('  isRoot: ' + ret.isRoot);
  Module.print('  exists: ' + ret.exists);
  Module.print('  error: ' + ret.error);
  Module.print('  path: ' + ret.path);
  Module.print('  name: ' + ret.name);
  Module.print('  object.contents: ' + (ret.object && JSON.stringify(Object.keys(ret.object.contents || {}))));
  Module.print('  parentExists: ' + ret.parentExists);
  Module.print('  parentPath: ' + ret.parentPath);
  Module.print('  parentObject.contents: ' + (ret.parentObject && JSON.stringify(Object.keys(ret.parentObject.contents))));
  Module.print('');
}

FS.currentPath = '/abc';
explore('/');
explore('.');
explore('..');
explore('../..');
explore('/abc');
explore('/abc/123');
explore('/abc/noexist');
explore('/abc/deviceA');
explore('/abc/localLink');
explore('/abc/rootLink');
explore('/abc/relativeLink');
explore('/abc/relativeLink/deviceB');
explore('/abc/rootLink/noexist');
explore('/abc/rootLink/abc/noexist');
explore('/forbidden');
explore('/forbidden/test');
explore('/forbidden/noexist');
explore('/noexist1/noexist2');
