#include <emscripten/threading.h>
#include <stdio.h>

int main()
{
	printf("emscripten_num_logical_cores returns %d.\n", (int)emscripten_num_logical_cores());
#ifdef REPORT_RESULT
	int result = 0;
	REPORT_RESULT();
#endif
}
