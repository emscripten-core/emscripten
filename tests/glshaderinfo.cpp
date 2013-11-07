#define GL_GLEXT_PROTOTYPES
#define EGL_EGLEXT_PROTOTYPES

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <Glut/glut.h>
#else
#include <GL/gl.h>
#include <GL/glut.h>
#endif

#include <assert.h>
#include <emscripten.h>

int main(int argc, char *argv[])
{
   /* Initialize the window */
   glutInit(&argc, argv);
   glutInitWindowSize(300, 300);
   glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

   glutCreateWindow("es2gears");

   GLint shaderCompiler; 
   glGetIntegerv(GL_SHADER_COMPILER, &shaderCompiler);

   GLint numShaderBinaryFormats; 
   glGetIntegerv(GL_NUM_SHADER_BINARY_FORMATS, &numShaderBinaryFormats);

   printf("%d,%d\n", shaderCompiler, numShaderBinaryFormats);

   if (!shaderCompiler && numShaderBinaryFormats == 0) {
      printf("In current environment, the GLES2 implementation IS NOT standard conforming! "
           "GL_SHADER_COMPILER == GL_FALSE and GL_NUM_SHADER_BINARY_FORMATS == 0! "
           "In GLES2 spec, either compiling shaders or binary shaders must be supported! (Section 2.10 - Vertex Shaders)\n");
      int result = 0;
      REPORT_RESULT();
   } else {
      assert(numShaderBinaryFormats == 0);
      int result = 1;
      REPORT_RESULT();
   }
   return 0;
}
