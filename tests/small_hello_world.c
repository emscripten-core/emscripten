#include <emscripten/html5.h>

int main()
{
	emscripten_console_log("hello!");
#ifdef REPORT_RESULT
	REPORT_RESULT(0);
#endif
}
