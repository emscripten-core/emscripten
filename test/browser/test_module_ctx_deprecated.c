/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

// Module.ctx stays usable while deprecated: it is the live context, user code
// can still delete it to reset the context, and only reads from user code warn.

#include <assert.h>
#include <emscripten.h>
#include <GLFW/glfw3.h>

EM_JS(int, warning_count, (), {
  return Module['warnings'].filter((text) => text.includes('`Module.ctx` is deprecated')).length;
});

GLFWwindow* create_window() {
  GLFWwindow* window = glfwCreateWindow(32, 32, "ctx", NULL, NULL);
  assert(window);
  glfwMakeContextCurrent(window);
  return window;
}

int main() {
  assert(glfwInit());
  create_window();
  // The runtime read Module['ctx'] above, in glfwCreateWindow.
  assert(warning_count() == 0);

  EM_ASM({
    assert(Module['ctx']);
    assert(Module['ctx'] === GLctx);
    assert(Module['ctx'] instanceof WebGLRenderingContext);
    Module['ctx'].clearColor(0, 0, 0, 1);
    Module['ctx'].clear(Module['ctx'].COLOR_BUFFER_BIT);
    assert(!Module['ctx'].getError());
  });
  assert(warning_count() == 1);

  // Defold assigns the context itself, and deletes it to make the next
  // Browser.createContext create a new one instead of reusing it.
  int reset = EM_ASM_INT({
    const canvas = Browser.getCanvas();
    Module['ctx'] = Browser.createContext(canvas, true, true);
    const handle = GL.currentContext.handle;
    const reused = Browser.createContext(canvas, true, true) === Module['ctx'] && GL.currentContext.handle === handle;
    delete Module['ctx'];
    Browser.createContext(canvas, true, true);
    return reused && GL.currentContext.handle !== handle;
  });
  assert(reset);

  glfwTerminate();
  return 0;
}
