#include <GL/glew.h>
#include <GL/glfw.h>
#include <emscripten.h>

#include "test_webgl_context_attributes_common.c"

int nbSamples = 0;
int nbDepthBits = 0;
int nbStencilBits = 0;
int nbAlphaBits = 0;

int main() {
  
    checkContextAttributesSupport();
  
    glfwInit();
  
#ifdef AA_ACTIVATED
    antiAliasingActivated = true;
    nbSamples = 4;
#endif
  
#ifdef DEPTH_ACTIVATED
    depthActivated = true;
    nbDepthBits = 16;
#endif  
  
#ifdef STENCIL_ACTIVATED
    stencilActivated = true;  
    nbStencilBits = 8;
#endif
  
#ifdef ALPHA_ACTIVATED
    alphaActivated = true;  
    nbAlphaBits = 8;
#endif

    glfwOpenWindowHint(GLFW_FSAA_SAMPLES, nbSamples);
    glfwOpenWindow(WINDOWS_SIZE, WINDOWS_SIZE, 8, 8, 8, nbAlphaBits, nbDepthBits, nbStencilBits, GLFW_WINDOW);
  
    glewInit();
    initGlObjects();

    draw();
  
    glfwTerminate();
  
    REPORT_RESULT();
  
    return 0;

}
 