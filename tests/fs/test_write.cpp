// https://github.com/kripken/emscripten/pull/4705: Test that FS.write() with canOwn=true works.

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
        data = new Uint8Array(100);
        stringToUTF8Array(str, data, 0, lengthBytesUTF8(str)+1);
        FS.write(stream, data, 0, lengthBytesUTF8(str)+1, pos, /*canOwn=*/false);

        FS.close(stream);
    );

    std::ifstream file("testfile");
    std::string line;
    getline(file, line);
    std::cout << "read " << line << std::endl;

    return 0;
}
