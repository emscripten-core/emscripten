#include <emscripten.h>

EM_JS(void, console_log, (char* str), {
  console.log(UTF8ToString(str));
});

int main()
{
  console_log("minimal hello!");
  return 0;
}
