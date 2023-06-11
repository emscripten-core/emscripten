#include <emscripten/emscripten.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <assert.h>
#include <fcntl.h>

void test_fs_mmap() {
    EM_ASM(
        FS.writeFile('mmaptest', 'a=1_b=2_');

        var stream = FS.open('mmaptest', 'r+');
        assert(stream);

        var mapped = FS.mmap(stream, 12, 0, 1 | 2 /* PROT_READ | PROT_WRITE */, 1 /* MAP_SHARED */);
        var ret = new Uint8Array(Module.HEAPU8.subarray(mapped.ptr, mapped.ptr + 12));
        var fileContents = "";
        for (var i = 0; i < 12; i++) {
            fileContents += String.fromCharCode(ret[i]);
        }
        assert(fileContents === 'a=1_b=2_\0\0\0\0');

        ret[8] = ':'.charCodeAt(0);
        ret[9] = 'x'.charCodeAt(0);
        ret[10] = 'y'.charCodeAt(0);
        ret[11] = 'z'.charCodeAt(0);
        Module.HEAPU8.set(ret, mapped.ptr); // We must update the array in Wasm memory before WasmFS msync.

        // The WasmFS msync syscall requires a pointer to the mapped memory, while the legacy JS API takes in any buffer
        // to write as a Uint8Array to write to a file.
#if WASMFS
        FS.msync(stream, mapped.ptr, 0, 12, 1 /* MAP_SHARED */);
        FS.munmap(mapped.ptr, 12);
#else
        FS.msync(stream, new Uint8Array(ret), 0, 12, 1 /* MAP_SHARED */);
        FS.munmap(stream);
#endif
    );

    FILE *fptr = fopen("mmaptest", "r");
    char res[13];
    fgets(res, 13, fptr);
    assert(strcmp(res, "a=1_b=2_:xyz") == 0);

    remove("mmaptest");
}

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

    /********** test FS.rmdir() **********/
    EM_ASM(
        // Create multiple directories
        FS.mkdir('/dir1');
        FS.mkdir('/dir2');
    );

    struct stat s;
    stat("/dir1", &s);
    assert(S_ISDIR(s.st_mode));
    stat("/dir2", &s);
    assert(S_ISDIR(s.st_mode));


    EM_ASM(
        // Remove the multiple directories
        FS.rmdir('/dir1');
        FS.rmdir('/dir2');
    );

    int err = open("/dir1", O_RDWR);
    assert(err);
    err = open("/dir2", O_RDWR);
    assert(err);

    EM_ASM(    
        // Create a directory with a file inside it
        FS.mkdir('/test_dir');
        FS.writeFile('/test_dir/file.txt', 'Hello World!');

        // Attempt to remove the directory (should fail)
        var ex;
        try {
        FS.rmdir('/test_dir');
        } catch (err) {
        ex = err;
        }
        assert(ex.name === "ErrnoError" && ex.errno === 55 /* ENOTEMPTY */);

        // Remove the file and then the directory
        FS.unlink('/test_dir/file.txt');
        FS.rmdir('/test_dir');

        // Attempt to remove a non-existent directory (should fail)
        try {
        FS.rmdir('/non_existent_dir');
        } catch (err) {
        ex = err;
        }
        assert(ex.name === "ErrnoError" && ex.errno === 44 /* ENOEN */);
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

        assert(ex.name === "ErrnoError" && ex.errno === 8 /* EBADF */);
    );

    /********** test FS.mknod() **********/
    EM_ASM(
        FS.mknod("mknodtest", 0100000 | 0777); /* S_IFREG | S_RWXU | S_RWXG | S_RWXO */

        FS.create("createtest", 0400); /* S_IRUSR */
    );
    struct stat stats;
    stat("mknodtest", &stats);

    assert(S_ISREG(stats.st_mode));
    assert(stats.st_mode & 0777);

    stat("createtest", &stats);
    assert(S_ISREG(stats.st_mode));
    assert(stats.st_mode & 0400);

    test_fs_mmap();

    remove("mknodtest");
    remove("createtest");
    remove("testfile");
    remove("renametestfile");
    remove("readtestfile");
    remove("closetestfile");

    puts("success");
}
