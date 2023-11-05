/*
 * Copyright 2023 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <GLFW/glfw3.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <emscripten/html5.h>

int main()
{

    GLFWwindow *window;

    assert(glfwInit() == GL_TRUE);

    // installing a onFullscreen callback so that we can check that
    // Browser.requestFullscreen gets called properly
    EM_ASM(
      Module.fsCount = 0;
      Module.notFsCount = 0;
      Module.onFullscreen = (isFullscreen) => {
        console.log("from callback " + isFullscreen);
        if (isFullscreen)
          Module.fsCount++;
        else
          Module.notFsCount++;
    };
   );

    // use case where fullscreen is denied (not user initiated): glfw catches
    // the issue and still sets the canvas size to the size of the screen
    // See details about this issue: https://github.com/emscripten-core/emscripten/issues/20600
    {
      window = glfwCreateWindow(640, 480, "test_glfw3_fullscreen.c", NULL, NULL);
      assert(window != NULL);
      int w, h;
      glfwGetWindowSize(window, &w, &h);
      assert(w == 640 && h == 480);
      assert(EM_ASM_INT(return Module.fsCount;) == 0);
      assert(EM_ASM_INT(return Module.notFsCount;) == 0);
      int screenWidth = EM_ASM_INT(return screen.width;);
      int screenHeight = EM_ASM_INT(return screen.height;);
      glfwSetWindowSize(window, screenWidth, screenHeight);
      printf("after glfwSetWindowSize %dx%d\n", screenWidth, screenHeight);
      // asynchronous call => wait (not more than 5s) for condition to become true
      for (int i = 0; i < 50; i++)
      {
        emscripten_sleep(100);
        glfwGetWindowSize(window, &w, &h);
        printf("after glfwGetWindowSize %dx%d\n", w, h);
        if(w == screenWidth && h == screenHeight)
          break;
      }
      printf("end of loop glfwGetWindowSize %dx%d\n", w, h);
      assert(w == screenWidth && h == screenHeight);
      assert(EM_ASM_INT(return Module.fsCount;) == 0);
      assert(EM_ASM_INT(return Module.notFsCount;) == 1);

      glfwDestroyWindow(window);
    }

    glfwTerminate();

#ifdef REPORT_RESULT
    REPORT_RESULT(1);
#endif
    return 0;
}
