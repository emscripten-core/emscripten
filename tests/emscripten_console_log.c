#include <emscripten.h>
#include <emscripten/html5.h>

int main()
{
	emscripten_console_log("Hello!");
	emscripten_console_warn("Hello!");
	emscripten_console_error("Hello!");
	if (EM_ASM_INT(return Module['testPassed']))
	{
#ifdef REPORT_RESULT
		REPORT_RESULT(0);
#endif
	}
}
