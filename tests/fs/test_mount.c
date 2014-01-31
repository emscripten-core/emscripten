#include <assert.h>
#include <stdio.h>
#include <emscripten.h>

int main() {
  EM_ASM(
    var ex;

    // write a file that should be unaffected by this process
    FS.writeFile('/safe.txt', 'abc');

    // mount it the first time
    FS.mkdir('/working');
    FS.mount(MEMFS, {}, '/working');
    FS.writeFile('/working/waka.txt', 'az');

    // mount to a missing directory
    try {
      FS.mount(MEMFS, {}, '/missing');
    } catch (e) {
      ex = e;
    }
    assert(ex instanceof FS.ErrnoError && ex.errno === ERRNO_CODES.ENOENT);

    // mount to an existing mountpoint
    try {
      FS.mount(MEMFS, {}, '/working');
    } catch (e) {
      ex = e;
    }
    assert(ex instanceof FS.ErrnoError && ex.errno === ERRNO_CODES.EBUSY);

    // unmount
    FS.unmount('/working');

    // unmount something that's not mounted
    try {
      FS.unmount('/working');
    } catch (e) {
      ex = e;
    }
    assert(ex instanceof FS.ErrnoError && ex.errno === ERRNO_CODES.EINVAL);

    // try to read the file from the old mount
    try {
      FS.readFile('/working/waka.txt', { encoding: 'utf8' });
    } catch (e) {
      ex = e;
    }
    assert(ex instanceof FS.ErrnoError && ex.errno === ERRNO_CODES.ENOENT);

    // check the safe file
    var contents = FS.readFile('/safe.txt', { encoding: 'utf8' });
    assert(contents === 'abc');
  );

  puts("success");

  return 0;
}
