FS.createDataFile('/', "file.txt", "foo");
var errCode = 0;
try {
  FS.createDataFile('/', "fILe.txt", "foo2");
}
catch (e)
{
  errCode = e.errno;
}
var fileContents = FS.readFile("/file.txt");
out('file.txt: ' + fileContents);
var ret = FS.analyzePath('/file.txt');
out('file.txt collision: ' + ret.object.name_next);
out('errorCode: ' + errCode);
