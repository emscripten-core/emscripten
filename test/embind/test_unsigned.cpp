// Copyright 2017 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <emscripten/bind.h>
#include <emscripten/emscripten.h>
#include <cstdio>
static void set_bind_f64(emscripten::val val) {
    printf("set_bind_f64: %x\n", (uint32_t)val.as<double>());
}
static void set_bind_u64(emscripten::val val) {
    printf("set_bind_u64: %x\n", (uint32_t)val.as<uint64_t>());
}
static void set_bind_u32(emscripten::val val) {
    printf("set_bind_u32: %x\n", val.as<uint32_t>());
}
extern "C" {
    EMSCRIPTEN_KEEPALIVE void set_c_u64(uint64_t v) {
        printf("set_c_u64: %x\n", (uint32_t)v);
    }
    EMSCRIPTEN_KEEPALIVE void set_c_u32(uint32_t v) {
        printf("set_c_u32: %x\n", v);
    }
}
EMSCRIPTEN_BINDINGS(TEST) {
    emscripten::function("set_bind_f64", &set_bind_f64);
    emscripten::function("set_bind_u64", &set_bind_u64);
    emscripten::function("set_bind_u32", &set_bind_u32);
}

int main()
{
    EM_ASM(
        Module['set_bind_f64'](2147483648);
//        Module['set_bind_u64'](2147483648); // todo: embind does not currently support 64-bit integers.
        Module['set_bind_u32'](2147483648);
//        Module['_set_c_u64'](2147483648); // todo: embind does not currently support 64-bit integers.
        Module['_set_c_u32'](2147483648);
    );
}
