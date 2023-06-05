#include <emscripten/emscripten.h>
#include <stdio.h>
#include <string.h>

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

    /********** test FS.mmap() **********/
    EM_ASM(
        FS.writeFile('mmaptest', 'a=1_b=2_');

        var stream = FS.open('mmaptest', 'r');
        assert(stream);

        var mapped = FS.mmap(stream, 12, 0, 1 /* PROT_READ */, 1 /* MAP_SHARED */);
        console.log(mapped);
        var ret = new Uint8Array(Module.HEAP8.subarray(mapped.ptr, mapped.ptr + 12));
        console.log(ret);
        for (var i = 0; i < 12; i++) {
            console.log("Char: ", String.fromCharCode(ret[i]));
        }

        ret[8] = ':'.charCodeAt(0);
        ret[9] = 'x'.charCodeAt(0);
        ret[10] = 'y'.charCodeAt(0);
        ret[11] = 'z'.charCodeAt(0);

        FS.msync(stream, ret, 0, 12, 0);

        var out = FS.readFile('mmaptest', { encoding: 'utf8'});
        console.log("Written: " + out);

        // FS.munmap(stream);
    );

    FILE *fptr = fopen("mmaptest", "r");
    char res[13];
    fgets(res, 13, fptr);

    printf("Res: %s, %d\n", res, strcmp(res, "a=1_b=2_:xyz"));

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

    puts("success");
}
