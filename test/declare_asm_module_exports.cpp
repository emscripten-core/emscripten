#include <emscripten.h>
#include <stdio.h>

extern "C" int EMSCRIPTEN_KEEPALIVE cFunction(void) { return 1; }

EM_JS(int, jsFunction, (), {
  return _cFunction();
});

int main()
{
	printf("cFunction: %d\n", cFunction());
	printf("jsFunction: %d\n", jsFunction());
#ifdef REPORT_RESULT
	REPORT_RESULT(jsFunction());
#endif
}
