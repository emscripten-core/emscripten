#include <emscripten/emscripten.h>
#include <emscripten/console.h>
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
	emscripten_outf("%d %d", f(), b());

	// Obtaining the same function pointer twice should return the
	// same address.
	intfunc b2 = (intfunc)emscripten_get_exported_function("_bar");
	emscripten_outf("%d", b == b2);
}
