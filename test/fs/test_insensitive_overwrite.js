FS.createDataFile('/', "file.txt", "foo");
FS.createDataFile('/', "fILe.txt", "foo2");
var fileContents = FS.readFile("/file.txt");
out('file.txt: ' + fileContents);
var ret = FS.analyzePath('/file.txt');
out('file.txt collison: ' + ret.object.name_next);
