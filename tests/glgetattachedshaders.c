#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <stdio.h>
#include <stdlib.h>

static void die(const char *msg)
{
   printf("%s\n", msg);
   abort();
}

static void create_context(void)
{
   EGLint num_config;
   EGLContext g_egl_ctx;
   EGLDisplay g_egl_dpy;
   EGLConfig g_config;

   static const EGLint attribute_list[] =
   {
      EGL_RED_SIZE, 8,
      EGL_GREEN_SIZE, 8,
      EGL_BLUE_SIZE, 8,
      EGL_ALPHA_SIZE, 8,
      EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
      EGL_NONE
   };

   static const EGLint context_attributes[] =
   {
      EGL_CONTEXT_CLIENT_VERSION, 2,
      EGL_NONE
   };

   g_egl_dpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);
   if (!g_egl_dpy)
      die("failed to create display");

   if (!eglInitialize(g_egl_dpy, NULL, NULL))
      die("failed to initialize egl");

   if (!eglChooseConfig(g_egl_dpy, attribute_list, &g_config, 1, &num_config))
      die("failed to choose config");

   g_egl_ctx = eglCreateContext(g_egl_dpy, g_config, EGL_NO_CONTEXT, context_attributes);
   if (!g_egl_ctx)
      die("failed to create context");
}

int main(int argc, char *argv[])
{
   unsigned i;

   create_context();

   GLuint prog = glCreateProgram();
   if (glGetError())
      die("failed to create program");

   GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
   if (glGetError())
      die("failed to create vertex shader");
   glAttachShader(prog, vertex);

   GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
   if (glGetError())
      die("failed to create fragment shader");
   glAttachShader(prog, fragment);

   GLuint shaders[2];
   GLsizei count;

   glGetAttachedShaders(prog, 2, &count, shaders);
   if (glGetError())
      die("failed to get attached shaders");
   if (count != 2)
      die("unknown number of shaders returned");
   if (shaders[0] == shaders[1])
      die("returned identical shaders");

   for (i = 0; i < count; i++)
   {
      if (shaders[i] == 0)
         die("returned 0");
      if (shaders[i] != vertex && shaders[i] != fragment)
         die("unknown shader returned");
   }

   int result = 1;
   REPORT_RESULT();

   return 0;
}
