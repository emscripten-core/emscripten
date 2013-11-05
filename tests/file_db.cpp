#include <stdio.h>
#include <emscripten.h>

void later(void *) {}

int main() {
#if FIRST
  FILE *f = fopen("waka.txt", "w");
  fputc('a', f);
  fputc('z', f);
  fclose(f);

  EM_ASM(
    FS.saveFilesToDB(['waka.txt', 'moar.txt'], function() {
      Module.print('save ok');
      var xhr = new XMLHttpRequest();
      xhr.open('GET', 'http://localhost:8888/report_result?1');
      xhr.send();
      setTimeout(function() { window.close() }, 1000);
    }, function(e) {
      abort('saving should succeed ' + e);
    });
  );
#else
  EM_ASM(
    FS.loadFilesFromDB(['waka.txt', 'moar.txt'], function() {
      function stringy(arr) {
        return Array.prototype.map.call(arr, function(x) { return String.fromCharCode(x) }).join('');
      }
      assert(stringy(FS.analyzePath('waka.txt').object.contents) == 'az');
      var secret = stringy(FS.analyzePath('moar.txt').object.contents);
      Module.print('load: ' + secret);
      var xhr = new XMLHttpRequest();
      xhr.open('GET', 'http://localhost:8888/report_result?' + secret);
      xhr.send();
      setTimeout(function() { window.close() }, 1000);
    }, function() {
      abort('loading should succeed');
    });
  );
#endif

  emscripten_async_call(later, NULL, 100); // keep runtime alive

  return 0;
}

