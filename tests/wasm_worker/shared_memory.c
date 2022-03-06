#include <emscripten.h>
#include <stdio.h>

int main()
{
	printf("%d\n", EM_ASM_INT(return buffer instanceof SharedArrayBuffer));
}
