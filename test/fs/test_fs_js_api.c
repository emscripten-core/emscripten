#include <emscripten/emscripten.h>
#include <stdio.h>
#include <sys/stat.h>
#include <assert.h>

int main() {
    /********** test FS.open() **********/
    EM_ASM(
        FS.writeFile('testfile', 'a=1\nb=2\n');
        var readStream = FS.open('testfile', 'r');
        var writeStream = FS.open('testfile', 'w');
        var writePlusStream = FS.open('testfile', 'w+');
        var appendStream = FS.open('testfile', 'a');

        assert(readStream && readStream.fd >= 0);
        assert(writeStream && writeStream.fd >= 0);
        assert(writePlusStream && writePlusStream.fd >= 0);
        assert(appendStream && appendStream.fd >= 0);

        var ex;
        try {
            FS.open('filenothere', 'r');
        } catch(err) {
            ex = err;
        }
        assert(ex.name === "ErrnoError" && ex.errno === 44 /* ENOENT */);

        var createFileNotHere = FS.open('filenothere', 'w+');

        assert(createFileNotHere && createFileNotHere.fd >= 0);
    );

    /********** test FS.truncate() **********/
    EM_ASM(
        FS.writeFile('truncatetest', 'a=1\nb=2\n');
    );

    struct stat s;
    stat("truncatetest", &s);
    assert(s.st_size == 8);

    EM_ASM(
        FS.truncate('truncatetest', 2);
    );
    stat("truncatetest", &s);
    assert(s.st_size == 2);

    EM_ASM(
        FS.truncate('truncatetest', 10);
    );
    stat("truncatetest", &s);
    assert(s.st_size == 10);

    EM_ASM(
        var truncateStream = FS.open('truncatetest', 'w');
        FS.ftruncate(truncateStream.fd, 4);
    );
    stat("truncatetest", &s);
    assert(s.st_size == 4);

    EM_ASM(
        var ex;
        try {
            FS.truncate('truncatetest', -10);
        } catch(err) {
            ex = err;
            console.log(ex);
        }

        // assert(ex.name === "ErrnoError" && ex.errno === 28 /* EINVAL */);
    );

//     EM_ASM(
//         var ex;
//         try {
//             var truncateStream = FS.open('truncatetest', 'w');
// #if WASMFS
//             FS.ftruncate(truncateStream, -10);
// #else
//             FS.ftruncate(truncateStream.fd, -10);
// #endif
//         } catch(err) {
//             ex = err;
//         }

//         assert(ex.name === "ErrnoError" && ex.errno === 28 /* EINVAL */);
//     );

    // EM_ASM(
    //     var ex;
    //     try {
    //         FS.truncate('nonexistent', 10);
    //     } catch(err) {
    //         ex = err;
    //     }
    //     assert(ex.name === "ErrnoError" && ex.errno === 44 /* ENOENT */);

    //     var ex;
    //     try {
    //         FS.ftruncate(99, 10);
    //     } catch(err) {
    //         ex = err;
    //     }

    //     assert(ex.name === "ErrnoError" && ex.errno === 8 /* EBADF */);
    // );
    
    /********** test FS.rename() **********/
    EM_ASM(
        FS.mkdir('renamedir');
        FS.writeFile('renamedir/renametestfile', "");

        FS.rename('renamedir/renametestfile', 'renamedir/newname');
        var newnameStream = FS.open('renamedir/newname', 'r');
        assert(newnameStream);

        var ex;
        try {
            FS.open('renamedir/renametestfile', 'r');
        } catch (err) {
            ex = err;
        }
        assert(ex.name === "ErrnoError" && ex.errno === 44 /* ENOENT */);

        
        try {
            FS.rename('renamedir', 'renamedir/newdirname');
        } catch (err) {
            ex = err;
        }
        // The old path should not be an ancestor of the new path.
        assert(ex.name === "ErrnoError" && ex.errno === 28 /* EINVAL */);

        FS.writeFile('toplevelfile', "");
        try {
            FS.rename('renamedir', 'toplevelfile');
        } catch (err) {
            ex = err;
        }
        assert(ex.name === "ErrnoError" && ex.errno === 54 /* ENOTDIR */);
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

    remove("truncatetest");
    remove("testfile");
    remove("renametestfile");
    remove("readtestfile");
    remove("closetestfile");

    puts("success");
}
