#include <emscripten/emscripten.h>
#include <stdio.h>

int main() {
    /********** test FS.open() **********/
    EM_ASM(
        FS.writeFile('testfile', 'a=1\nb=2\n');
        var readStream = FS.open('testfile', 'r');
        var writeStream = FS.open('testfile', 'w');
        var writePlusStream = FS.open('testfile', 'w+');
        var appendStream = FS.open('testfile', 'a');
#if WASMFS
        assert(readStream >= 0);
        assert(writeStream >= 0);
        assert(writePlusStream >= 0);
        assert(appendStream >= 0);
#else
        assert(readStream && readStream.fd >= 0);
        assert(writeStream && writeStream.fd >= 0);
        assert(writePlusStream && writePlusStream.fd >= 0);
        assert(appendStream && appendStream.fd >= 0);
#endif

        var ex;
        try {
            FS.open('filenothere', 'r');
        } catch(err) {
            ex = err;
        }
        assert(ex.name === "ErrnoError" && ex.errno === 44 /* ENOENT */);

        var createFileNotHere = FS.open('filenothere', 'w+');
#if WASMFS
        assert(createFileNotHere >= 0);
#else
        assert(createFileNotHere && createFileNotHere.fd >= 0);
#endif
    );

    /********** test FS.read() **********/
    EM_ASM(
        FS.writeFile("readtestfile", 'a=1_b=2_');

        var buf = new Uint8Array(8);
        var stream = FS.open("readtestfile", "r");
        var numRead = FS.read(stream, buf, 0, 4); // read first 4 bytes into buf[0]
        assert(numRead == 4);
        // console.log("First four");
        var decodedString = new TextDecoder().decode(buf.subarray(0, 4));
        // console.log(decodedString);
        assert(decodedString.toString() === "a=1_");
        numRead = FS.read(stream, buf, 4, 4); // read next 4 bytes into buf[4]
        assert(numRead == 4);
        // console.log("With next four (No pos arg means offset is moving)");
        // console.log("Off: ", stream);
        // console.log(new TextDecoder().decode(buf));
        decodedString = new TextDecoder().decode(buf);
        assert(decodedString == 'a=1_b=2_');

        stream = FS.open("readtestfile", "r"); // reopen stream to reset offset
        var extraBuf = new Uint8Array(8);
        numRead = FS.read(stream, extraBuf, 0, 4, 0); // pread variant
        assert(numRead == 4);
        // console.log("pread from pos 0");
        // console.log("Off: ", stream);
        // console.log(new TextDecoder().decode(extraBuf));
        decodedString = new TextDecoder().decode(extraBuf.subarray(0, 4));
        assert(decodedString == 'a=1_');

        var doubleBuf = new Uint8Array(8);
        var firstNumRead = FS.read(stream, doubleBuf, 0, 4);
        var secondNumRead = FS.read(stream, doubleBuf, 4, 4);
        assert(firstNumRead == 4 && secondNumRead == 4);
        // console.log("read again without pos (Above should not have moved offset)");
        // console.log("Off: ", stream);
        // console.log(new TextDecoder().decode(doubleBuf));
        decodedString = new TextDecoder().decode(doubleBuf);
        assert(decodedString == 'a=1_b=2_');

        stream = FS.open("readtestfile", "r"); // reopen stream to reset offset (Commenting out results in nothing read to buf)
        var fullBuf = new Uint8Array(8);
        numRead = FS.read(stream, fullBuf, 0, 8);
        assert(numRead == 8);
        // console.log("Full Buf: ");
        // console.log("Off: ", stream);
        // console.log(new TextDecoder().decode(fullBuf));
        decodedString = new TextDecoder().decode(fullBuf);
        assert(decodedString == 'a=1_b=2_');

        FS.close(stream);
        var ex;
        try {
            FS.read(stream, buf, 4, 4);
        } catch (err) {
            ex = err;
        }
        assert(ex.name === 'ErrnoError' && ex.errno == 8 /* EBADF */);
    );

    /********** test FS.close() **********/
    EM_ASM(
        FS.writeFile("closetestfile", 'a=1\nb=2\n');
        FS.mkdir("/testdir");
        var file = FS.open("closetestfile", "r");
        var error = FS.close(file);
        assert(!error);

        file = FS.open("/testdir", "r");
        error = FS.close(file);
        assert(!error);

        var ex;
        try {
            FS.close(file);
        } catch(err) {
            ex = err;
        }

        assert(ex.name === "ErrnoError" && ex.errno === 8 /* EBADF */)
    );

    puts("success");
}
