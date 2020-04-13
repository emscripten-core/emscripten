#include <emscripten/html5.h>
#include <stdio.h>

void timeout(void *userData)
{
	printf("Got timeout handler\n");
#ifdef REPORT_RESULT
	// Test passed
	REPORT_RESULT(1);
#endif
}

int main()
{
	emscripten_set_timeout(timeout, 2000, 0);
	emscripten_unwind_to_js_event_loop();
	printf("This should not be called!\n");
#ifdef REPORT_RESULT
	// Should not reach here
	REPORT_RESULT(-1);
#endif
}
