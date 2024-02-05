#include <emscripten.h>
#include <stdio.h>

int main()
{
	printf("%d\n", EM_ASM_INT(return wasmMemory.buffer instanceof SharedArrayBuffer));
}
