#include <stdio.h>
#include <string>
#include <emscripten.h>
#include <cassert>
#include <wchar.h>

// This code tests that utf32-encoded std::wstrings can be marshalled between C++ and JS.
int main() {
	std::wstring wstr = L"abc\u2603\u20AC123"; // U+2603 is snowman, U+20AC is the Euro sign.
	const int len = (wstr.length()+1)*4;
	char *memory = new char[len];

	asm("var str = Module.UTF32ToString(%0);"
		"Module.print(str);"
		"Module.stringToUTF32(str, %1);"
	:
	: "r"(wstr.c_str()), "r"(memory));

	// Compare memory to confirm that the string is intact after taking a route through JS side.
	const char *srcPtr = reinterpret_cast<const char *>(wstr.c_str());
	for(int i = 0; i < len; ++i) {
		assert(memory[i] == srcPtr[i]);
	}
	printf("OK.\n");
	delete[] memory;
}
