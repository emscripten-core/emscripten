#include <stdio.h>
#include <string.h>
#include <emscripten.h>

void __attribute__((noinline)) InteropString()
{
	char *string = (char*)EM_ASM_INT_V({
		var str = "hello, this is a string! ";
		for(var i = 0; i < 16; ++i)
			str = str + str;
		var stringOnTheStack = allocate(intArrayFromString(str), 'i8', ALLOC_STACK);
		return stringOnTheStack;
	});

	int stringLength = strlen(string);
	printf("Got string: %s\n", string);
	printf("Received a string of length %d.\n", stringLength);
}

int main()
{
	InteropString();
}
