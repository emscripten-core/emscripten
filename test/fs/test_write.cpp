// Copyright 2016 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

// https://github.com/emscripten-core/emscripten/pull/4705: Test that FS.write() with canOwn=true works.

#include <fstream>
#include <iostream>
#include <string>

#include <emscripten/emscripten.h>

int main()
{
    EM_ASM(
        var stream = FS.open('testfile', 'w+');

        var data = new Uint8Array(128);
        var str = "Hello! ";
        stringToUTF8Array(str, data, 0, lengthBytesUTF8(str)+1);
        data = data.subarray(0, lengthBytesUTF8(str));
        FS.write(stream, data, 0, lengthBytesUTF8(str), 0, /*canOwn=*/true);
        var pos = lengthBytesUTF8(str);

        str = '1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890';
        data = new Uint8Array(str.length+1);
        stringToUTF8Array(str, data, 0, lengthBytesUTF8(str)+1);
        FS.write(stream, data, 0, lengthBytesUTF8(str)+1, pos, /*canOwn=*/false);

        FS.close(stream);

        var ex;
        try {
            FS.write(stream, data, 0, lengthBytesUTF8(str)+1, pos, /*canOwn=*/false);
        } catch (err) {
            ex = err;
        }
        assert(ex.name === 'ErrnoError' && ex.errno === 8 /* EBADF */)
    );

    std::ifstream file("testfile");
    std::string line;
    getline(file, line);
    std::cout << "read " << line << std::endl;

    return 0;
}
