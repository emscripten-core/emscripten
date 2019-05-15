#include <cassert>
#include <cstdio>

extern "C" void *emscripten_wasm_return_address(int level);

void func(void) {
	assert(emscripten_wasm_return_address(0) != 0);
	assert(emscripten_wasm_return_address(1) == 0);
}

// We need to take these two arguments or clang can potentially generate
// a main function that takes two arguments and calls our main, messing up
// the stack trace and breaking this test.
int main(int argc, char **argv) {
	// main is called by the actual entry point
	assert(emscripten_wasm_return_address(0) == 0);
	func();
	std::puts("passed");
}
