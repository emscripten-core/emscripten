#include <string.h>
#include <stdio.h>
#include <math.h>
#include <emscripten/fetch.h>

int main()
{
  emscripten_fetch_attr_t attr;
  emscripten_fetch_attr_init(&attr);
  strcpy(attr.requestMethod, "GET");
  attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY | EMSCRIPTEN_FETCH_SYNCHRONOUS;
  emscripten_fetch_t *fetch = emscripten_fetch(&attr, "gears.png");
  
  if (result == 0) {
    result = 2;
    printf("emscripten_fetch() failed to run synchronously!\n");
  }
#ifdef REPORT_RESULT
    REPORT_RESULT(result);
#endif
}
