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

        var ex;
        try {
            FS.close(file);
        } catch(err) {
            ex = err;
        }

#if WASMFS
        assert(ex instanceof FS.WasmFSError && ex.code === 8);
#else
        assert(ex instanceof FS.ErrnoError && ex.errno === 8 /* EBADF */)
#endif
    );

    puts("success");
}
