FS.createDataFile('/', "file.txt", "foo");
FS.createDataFile('/', "fILe.txt", "foo2");
var fileContents = FS.readFile("/file.txt");
out('file.txt: ' + fileContents);
var ret = FS.analyzePath('/file.txt');
out('file.txt collison: ' + ret.object.name_next);
var errCode = 0;
try {
  FS.createDataFile('/', "FIlE.txt", "foo2");
} catch (e) {
  errCode = e.errno;
}
out('errorCode: ' + errCode);
