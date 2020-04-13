#include <emscripten.h>
#include <emscripten/exports.h>

extern "C" EMSCRIPTEN_KEEPALIVE int foo()
{
	return 1;
}

extern "C" EMSCRIPTEN_KEEPALIVE int bar()
{
	return 2;
}

typedef int (*intfunc)();

int main()
{
	intfunc f = (intfunc)emscripten_get_exported_function("_foo");
	intfunc b = (intfunc)emscripten_get_exported_function("_bar");
	EM_ASM(console.log($0 + ' ' + $1), f(), b());

	// Obtaining the same function pointer twice should return the
	// same address.
	intfunc b2 = (intfunc)emscripten_get_exported_function("_bar");
	EM_ASM(console.log($0), b == b2);
}
