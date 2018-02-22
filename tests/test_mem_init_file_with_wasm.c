#include <stdio.h>
#include <emscripten.h>

int global_data = 123;
int main() {
	int success = (global_data == 123) && MAIN_THREAD_EM_ASM_INT(return memoryInitializer.indexOf('.mem')) != -1;
	if (success) puts("success"); // This depends on global FILE* stdout being initialized to non-null, otherwise will crash in musl __overflow() function.
	else puts("fail");
#ifdef REPORT_RESULT
	REPORT_RESULT(success);
#endif
}
