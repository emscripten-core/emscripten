#include <emscripten/emscripten.h>
#include <stdio.h>
#include <unistd.h>

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
        } catch (err) {
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

    /********** test FS.readlink() **********/
    EM_ASM(
        FS.writeFile('/readlinktestfile', "");
        FS.symlink('readlinktestfile', 'readlinksymlink');

        var symlinkString = FS.readlink('readlinksymlink');
        console.log("sym: ", symlinkString);
        // assert(symlinkString === '/readlinktestfile');

        var ex;
        try {
            FS.readlink('nonexistent');
        } catch (err) {
            ex = err;
        }
        assert(ex.name === "ErrnoError" && ex.errno === 44 /* ENOENT */);

        try {
            FS.readlink('readlinktestfile');
        } catch (err) {
            ex = err;
        }
        assert(ex.name === "ErrnoError" && ex.errno === 28 /* EINVAL */)
    );

    char* buf = malloc(4096);
    int nBytes = readlink("readlinksymlink", buf, 4096);
    printf("'%s' points to '%.*s'\n", "C readlink", (int) nBytes, buf);

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
        } catch (err) {
            ex = err;
        }

        assert(ex.name === "ErrnoError" && ex.errno === 8 /* EBADF */);
    );

    remove("testfile");
    remove("closetestfile");
    remove("/testdir");

    puts("success");
}
