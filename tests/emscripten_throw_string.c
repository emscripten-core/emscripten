#include <emscripten/html5.h>

int main()
{
	emscripten_throw_string("Hello!");
	__builtin_trap();
}
