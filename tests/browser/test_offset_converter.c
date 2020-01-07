#include <stdio.h>
#include <emscripten.h>

void *get_pc(void) { return __builtin_return_address(0); }

void magic_test_function(void) {
  EM_ASM({
    console.log($0);
    var xhr = new XMLHttpRequest();
    xhr.open('GET', encodeURI('http://localhost:8888?stderr=' + $0));
    xhr.send();

    console.log(wasmOffsetConverter.getName($0));
    var xhr = new XMLHttpRequest();
    xhr.open('GET', encodeURI('http://localhost:8888?stderr=' + wasmOffsetConverter.getName($0)));
    xhr.send();

    var result = wasmOffsetConverter.getName($0) == 'magic_test_function';

    reportResultToServer(+result, true);
  }, get_pc());
}

int main(void) { magic_test_function(); }

