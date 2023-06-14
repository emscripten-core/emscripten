#include <emscripten/emscripten.h>
#include <stdio.h>
#include <sys/stat.h>
#include <assert.h>

void test_fs_utime() {
    EM_ASM(
        FS.writeFile('utimetest', 'a=1\nb=2\n');
    );
    struct stat utimeStats;
    stat("utimetest", &utimeStats);

    EM_ASM(
        FS.utime('utimetest', 10500, 8000);
    );
    stat("utimetest", &utimeStats);
#if WASMFS
    assert(utimeStats.st_atime == 10);
    assert(utimeStats.st_mtime == 8);
#else
    assert(utimeStats.st_atime == 10);
    assert(utimeStats.st_mtime == 10);
#endif
    remove("utimetest");
}

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

    test_fs_utime();

    puts("success");
}
