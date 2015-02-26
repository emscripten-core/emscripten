#include <stdio.h>
#include <string>
#include <emscripten.h>
#include <cassert>
#include <wchar.h>

typedef unsigned int utf32;
typedef unsigned short utf16;

// This code tests that Unicode std::wstrings can be marshalled between C++ and JS.
int main() {
	std::wstring wstr = L"abc\u2603\u20AC\U0002007C123 --- abc\u2603\u20AC\U0002007C123"; // U+2603 is snowman, U+20AC is the Euro sign, U+2007C is a Chinese Han character that looks like three raindrops.

	printf("sizeof(wchar_t): %d.\n", (int)sizeof(wchar_t));

	if (sizeof(wchar_t) == 4) {
		utf32 *memory = new utf32[wstr.length()+1];

		EM_ASM_INT({
			var str = Module.UTF32ToString($0);
			Module.print(str);
			var numBytesWritten = Module.stringToUTF32(str, $1, $2);
			if (numBytesWritten != 23*4) throw 'stringToUTF32 wrote an invalid length ' + numBytesWritten;
		}, wstr.c_str(), memory, (wstr.length()+1)*sizeof(utf32));

		// Compare memory to confirm that the string is intact after taking a route through JS side.
		const utf32 *srcPtr = reinterpret_cast<const utf32 *>(wstr.c_str());
		for(int i = 0;; ++i) {
			assert(memory[i] == srcPtr[i]);
			if (srcPtr[i] == 0)
				break;
		}

		EM_ASM_INT({
			var str = Module.UTF32ToString($0);
			Module.print(str);
			var numBytesWritten = Module.stringToUTF32(str, $1, $2);
			if (numBytesWritten != 5*4) throw 'stringToUTF32 wrote an invalid length ' + numBytesWritten;
		}, wstr.c_str(), memory, 6*sizeof(utf32));
		assert(memory[5] == 0);

		delete[] memory;
	} else { // sizeof(wchar_t) == 2, and we're building with -fshort-wchar.
		utf16 *memory = new utf16[2*wstr.length()+1];

		EM_ASM_INT({
			var str = Module.UTF16ToString($0);
			Module.print(str);
			var numBytesWritten = Module.stringToUTF16(str, $1, $2);
			if (numBytesWritten != 25*2) throw 'stringToUTF16 wrote an invalid length ' + numBytesWritten;
		}, wstr.c_str(), memory, (2*wstr.length()+1)*sizeof(utf16));

		// Compare memory to confirm that the string is intact after taking a route through JS side.
		const utf16 *srcPtr = reinterpret_cast<const utf16 *>(wstr.c_str());
		for(int i = 0;; ++i) {
			assert(memory[i] == srcPtr[i]);
			if (srcPtr[i] == 0)
				break;
		}

		EM_ASM_INT({
			var str = Module.UTF16ToString($0);
			Module.print(str);
			var numBytesWritten = Module.stringToUTF16(str, $1, $2);
			if (numBytesWritten != 5*2) throw 'stringToUTF16 wrote an invalid length ' + numBytesWritten;
		}, wstr.c_str(), memory, 6*sizeof(utf16));
		assert(memory[5] == 0);
		
		delete[] memory;
	}

	printf("OK.\n");
}
