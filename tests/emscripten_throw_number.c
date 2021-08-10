#include <emscripten/html5.h>

int main()
{
	emscripten_throw_number(42);
	__builtin_trap();
}
