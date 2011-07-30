FS.createFolder('/', 'forbidden', false, false);
FS.createFolder('/forbidden', 'test', true, true);
FS.createPath('/', 'abc/123', true, true);
FS.createPath('/', 'abc/456', true, true);
FS.createPath('/', 'def/789', true, true);
FS.createDevice('/abc', 'deviceA', function() {}, function() {});
FS.createDevice('/def', 'deviceB', function() {}, function() {});
FS.createLink('/abc', 'localLink', '123', true, true);
FS.createLink('/abc', 'rootLink', '/', true, true);
FS.createLink('/abc', 'relativeLink', '../def', true, true);

function explore(path) {
  print(path);
  var ret = FS.analyzePath(path);
  print('  isRoot: ' + ret.isRoot);
  print('  exists: ' + ret.exists);
  print('  error: ' + ret.error);
  print('  path: ' + ret.path);
  print('  name: ' + ret.name);
  print('  object.contents: ' + (ret.object && JSON.stringify(Object.keys(ret.object.contents || {}))));
  print('  parentExists: ' + ret.parentExists);
  print('  parentPath: ' + ret.parentPath);
  print('  parentObject.contents: ' + (ret.parentObject && JSON.stringify(Object.keys(ret.parentObject.contents))));
  print('');
}

FS.currentPath = '/abc';
explore('');
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
