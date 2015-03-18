#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <emscripten.h>
#include <html5.h>

int result = 0;
void report_result()
{
  printf("Test finished with result %d\n", result);
#ifdef REPORT_RESULT
  REPORT_RESULT();
#endif
}

void finish(void*)
{
  report_result();  
}

EM_BOOL context_lost(int eventType, const void *reserved, void *userData)
{
  printf("C code received a signal for WebGL context lost! This should not happen!\n");
  result = 1;
  report_result();
  return 0;
}

EM_BOOL context_restored(int eventType, const void *reserved, void *userData)
{
  printf("C code received a signal for WebGL context restored! This should not happen!\n");
  result = 1;
  report_result();
  return 0;
}

int main()
{
  EmscriptenWebGLContextAttributes attrs;
  emscripten_webgl_init_context_attributes(&attrs);
  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context = emscripten_webgl_create_context(0, &attrs);
  emscripten_set_webglcontextlost_callback(0, 0, 0, context_lost);
  emscripten_set_webglcontextrestored_callback(0, 0, 0, context_restored);
  // When we force a context loss, we should get an event, i.e. context_lost_desired() should get called.
  EM_ASM_INT({
      // The GL object is accessed here in a closure unsafe manner, so this test should not be run with closure enabled.
      Module['firstGLContextExt'] = GL.contexts[$0].GLctx.getExtension('WEBGL_lose_context');
    }, context);

  emscripten_webgl_destroy_context(context);

  EM_ASM_INT({
      Module['firstGLContextExt'].loseContext();
    }, context);

  emscripten_async_call(finish, 0, 3000);
}
