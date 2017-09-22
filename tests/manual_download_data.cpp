#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <emscripten/emscripten.h>

int main()
{
	FILE *handle = fopen("file.txt", "r");
	char str[128] = {};
	fread(str, 1, sizeof(str), handle);
	printf("str: %s\n", str);
	assert(!strcmp(str, "Hello!"));
	printf("OK\n");
#ifdef REPORT_RESULT
	int result = EM_ASM_INT({return Module.manuallyDownloadedData;});
	REPORT_RESULT(result);
#endif
}
