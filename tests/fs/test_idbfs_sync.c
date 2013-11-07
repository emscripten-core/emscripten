#include <stdio.h>
#include <emscripten.h>

#define EM_ASM_REEXPAND(x) EM_ASM(x)

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
  // store local files to backing IDB
  EM_ASM_REEXPAND(
    FS.writeFile('/working/waka.txt', 'az');
    FS.writeFile('/working/moar.txt', SECRET);
    FS.syncfs(function (err) {
      assert(!err);

      ccall('success', 'v', '', []);
    });
  );
#else
  // load files from backing IDB
  EM_ASM_REEXPAND(
    FS.syncfs(true, function (err) {
      assert(!err);

      var contents = FS.readFile('/working/waka.txt', { encoding: 'utf8' });
      assert(contents === 'az');

      var secret = FS.readFile('/working/moar.txt', { encoding: 'utf8' });
      assert(secret === SECRET);

      ccall('success', 'v', '', []);
    });
  );
#endif

  emscripten_exit_with_live_runtime();

  return 0;
}
