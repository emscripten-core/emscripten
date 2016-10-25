#include <stdio.h>
#include <string.h>
#include <emscripten.h>

void __attribute__((noinline)) InteropString(char *staticBuffer)
{
	char *string = (char*)EM_ASM_INT_V({
		var str = "hello, this is a string! ";
		for(var i = 0; i < 15; ++i)
			str = str + str;
		var stringOnTheStack = allocate(intArrayFromString(str), 'i8', ALLOC_STACK);
		return stringOnTheStack;
	});

	int stringLength = strlen(string);
	printf("Got string: %s\n", string);
	printf("Received a string of length %d.\n", stringLength);
	strcpy(staticBuffer, string);
}

int main()
{
	char staticBuffer[512288] = {}; // Make asm.js side consume a large portion of the stack, before bumping the rest with C++<->JS interop.
	InteropString(staticBuffer);
	int stringLength = strlen(staticBuffer);
	printf("Got string: %s\n", staticBuffer);
	printf("Received a string of length %d.\n", stringLength);
}
