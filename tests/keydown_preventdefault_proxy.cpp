#include <stdio.h>
#include <emscripten.h>
#include <html5.h>
static int result = 1;

// The event handler functions can return 1 to suppress the event and disable the default action. That calls event.preventDefault();
// Returning 0 signals that the event was not consumed by the code, and will allow the event to pass on and bubble up normally.
extern "C"
{
  EM_BOOL keydown_callback(int eventType, const EmscriptenKeyboardEvent *e, void *userData)
  {
    if ((e->keyCode == 65) || (e->keyCode == 8))
    {
      result *= 2;
    }
    else
    {
      REPORT_RESULT();
      emscripten_run_script("throw 'done'");
    }
    return 0;
  }
}

extern "C"
{
  EM_BOOL keypress_callback(int eventType, const EmscriptenKeyboardEvent *e, void *userData)
  {
    result *= 3;
    return 0;
  }
}

extern "C"
{
  EM_BOOL keyup_callback(int eventType, const EmscriptenKeyboardEvent *e, void *userData)
  {
    if ((e->keyCode == 65) || (e->keyCode == 8))
    {
      result *= 5;
    }
    return 0;
  }
}

int main(int argc, char **argv)
{
  printf("main argc:%d\n", argc);

  emscripten_set_keydown_callback("#document", 0, 1, keydown_callback);
  emscripten_set_keypress_callback("#document", 0, 1, keypress_callback);
  emscripten_set_keyup_callback("#document", 0, 1, keyup_callback);

  return 0;
}

