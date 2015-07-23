
var inputFile = Module['arguments'][0];
var outputFile = Module['arguments'][1];

Module.printErr('loading ' + inputFile)
FS.createDataFile('/', 'input.js', Module['readBinary'](inputFile), true, false);

__ATEXIT__.push(function() {
  Module.printErr('dumping binary as JS array from ' + outputFile)
  Module.print(JSON.stringify(MEMFS.getFileDataAsRegularArray(FS.root.contents[outputFile]).map(function(x) { return unSign(x, 8) })));
});

Module['callMain'](['input.js', outputFile]);

