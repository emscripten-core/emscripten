#include <stdio.h>
#include <emscripten.h>
#include <assert.h>

int main()
{
	int ENVIRONMENT_IS_WORKER = EM_ASM_INT_V(return ENVIRONMENT_IS_WORKER);
	printf("ENVIRONMENT_IS_WORKER: %d\n", ENVIRONMENT_IS_WORKER);
	assert(ENVIRONMENT_IS_WORKER);
#ifdef REPORT_RESULT
	REPORT_RESULT(1);
#endif
}
