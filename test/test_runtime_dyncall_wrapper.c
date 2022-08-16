#include <emscripten.h>
#include "stdint.h"
#include "stdio.h"

uint64_t f1(uint64_t x){
    return x;
}

void f2(int i, uint64_t j, float f, double d){
    printf("i: %d j: %lld f: %f d: %lf\n", i, j, f, d);
}


int main(){
    EM_ASM({
        var w = createDyncallWrapper("jj");
        console.log(w($0, 2, 7), tempRet0);
    }, f1);

    EM_ASM({
        var w = createDyncallWrapper("vijfd");
        w($0, 2, 7, 2, 3.12, 77.12);
    }, f2);
}