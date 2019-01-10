#include <emscripten.h>

EM_JS(void, console_log, (char* str), {
  console.log(UTF8ToString(str));
});

int main()
{
  console_log("minimal hello!");
#ifdef REPORT_RESULT
  REPORT_RESULT(0);
#endif
}
