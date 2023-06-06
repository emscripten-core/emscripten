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

        // Test read variant, checking that the file offset moves correctly.
        var buf = new Uint8Array(8);
        var stream = FS.open("readtestfile", "r");
        var numRead = FS.read(stream, buf, 0, 4);
        assert(numRead == 4);
        assert((new TextDecoder().decode(buf.subarray(0, 4))) === "a=1_");
        numRead = FS.read(stream, buf, 4, 4);
        assert(numRead == 4);
        assert((new TextDecoder().decode(buf)) == 'a=1_b=2_');

        // Test pread variant.
        stream = FS.open("readtestfile", "r");
        var extraBuf = new Uint8Array(8);
        numRead = FS.read(stream, extraBuf, 0, 4, 0);
        assert(numRead == 4);
        assert((new TextDecoder().decode(extraBuf.subarray(0, 4))) == 'a=1_');

        // Check that pread did not move the file offset (Offset begins at 0).
        var doubleBuf = new Uint8Array(8);
        var firstNumRead = FS.read(stream, doubleBuf, 0, 4);
        var secondNumRead = FS.read(stream, doubleBuf, 4, 4);
        assert(firstNumRead == 4 && secondNumRead == 4);
        assert((new TextDecoder().decode(doubleBuf)) == 'a=1_b=2_');

        // Check that full read works.
        stream = FS.open("readtestfile", "r");
        var fullBuf = new Uint8Array(8);
        numRead = FS.read(stream, fullBuf, 0, 8);
        assert(numRead == 8);
        assert((new TextDecoder().decode(fullBuf)) == 'a=1_b=2_');

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
