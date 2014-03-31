#include <stdio.h>
#include <emscripten.h>

void success() {
  int result = 1;
  REPORT_RESULT();
}

int main() {
  EM_ASM(
    FS.mkdir('/working');
    FS.mount(IDBFS, {}, '/working');
  );

#if FIRST
  // store local files to backing IDB. Note that we use the JS FS API for everything here, but we
  // could use normal libc fwrite etc. to do the writing. All we need the JS FS API for is to
  // mount the filesystem and do syncfs.
  EM_ASM_ARGS({
    FS.writeFile('/working/waka.txt', 'az');
    FS.writeFile('/working/moar.txt', $0);
    FS.syncfs(function (err) {
      assert(!err);

      ccall('success', 'v');
    });
  }, SECRET);
#else
  // load files from backing IDB
  EM_ASM_ARGS({
    FS.syncfs(true, function (err) {
      assert(!err);

      var contents = FS.readFile('/working/waka.txt', { encoding: 'utf8' });
      assert(contents === 'az', 'bad contents ' + contents);

      // note we convert to a number here (using +), since we used writeFile, which writes a
      // JS string.
      var secret = +FS.readFile('/working/moar.txt', { encoding: 'utf8' });
      assert(secret === $0, 'bad secret ' + [secret, $0, typeof secret, typeof $0]);

      ccall('success', 'v', '', []);
    });
  }, SECRET);
#endif

  emscripten_exit_with_live_runtime();

  return 0;
}
