// The following results in 'hello [MANGLED]'
//
// Filed as https://github.com/ry/node/issues/issue/402

var sys = require("sys"),
    buf = new Buffer(1024), len,
    str1 = "aGVsbG8g",  // 'hello '
    str2 = "d29ybGQ=",  // 'world'

len = buf.write(str1, 0, 'base64');
len += buf.write(str2, len, 'base64');
sys.log("decoded result: " + buf.toString('binary', 0, len));
