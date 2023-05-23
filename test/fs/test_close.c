#include <emscripten/emscripten.h>
#include <stdio.h>

int main() {

    EM_ASM(
        FS.writeFile("testfile", 'a=1\nb=2\n');
        FS.mkdir("/testdir");
        var file = FS.open("testfile", "r");
        var error = FS.close(file);
        assert(!error);

        file = FS.open("/testdir", "r");
        error = FS.close(file);
        assert(!error);

#if WASMFS
        error = FS.close(file);
        console.log("Error: ", error);
        assert(error === 8 /* EBADF */);
#else
        var ex;
        try {
            FS.close(file);
        } catch(err) {
            ex = err;
        }
        assert(ex instanceof FS.ErrnoError && ex.errno === 8 /* EBADF */)
#endif
    );

    puts("success");
}
