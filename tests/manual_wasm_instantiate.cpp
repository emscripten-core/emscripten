#include <stdio.h>
#include <emscripten/emscripten.h>

int main()
{
	printf("OK\n");
#ifdef REPORT_RESULT
	int result = EM_ASM_INT({return Module.testWasmInstantiationSucceeded;});
	REPORT_RESULT(result);
#endif
}
