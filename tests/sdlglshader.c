/*
THIS WORK, INCLUDING THE SOURCE CODE, DOCUMENTATION
AND RELATED MEDIA AND DATA, IS PLACED INTO THE PUBLIC DOMAIN.

THE ORIGINAL AUTHOR IS KYLE FOLEY.

THIS SOFTWARE IS PROVIDED AS-IS WITHOUT WARRANTY
OF ANY KIND, NOT EVEN THE IMPLIED WARRANTY OF
MERCHANTABILITY. THE AUTHOR OF THIS SOFTWARE,
ASSUMES _NO_ RESPONSIBILITY FOR ANY CONSEQUENCE
RESULTING FROM THE USE, MODIFICATION, OR
REDISTRIBUTION OF THIS SOFTWARE.
*/

#include "SDL/SDL.h"
#include "SDL/SDL_opengl.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

// GL_ARB_shading_language_100, GL_ARB_shader_objects, GL_ARB_fragment_shader, GL_ARB_vertex_shader
PFNGLCREATEPROGRAMOBJECTARBPROC       glCreateProgramObject_      = NULL;
PFNGLDELETEOBJECTARBPROC              glDeleteObject_             = NULL;
PFNGLUSEPROGRAMOBJECTARBPROC          glUseProgramObject_         = NULL; 
PFNGLCREATESHADEROBJECTARBPROC        glCreateShaderObject_       = NULL;
PFNGLSHADERSOURCEARBPROC              glShaderSource_             = NULL;
PFNGLCOMPILESHADERARBPROC             glCompileShader_            = NULL;
PFNGLGETOBJECTPARAMETERIVARBPROC      glGetObjectParameteriv_     = NULL;
PFNGLATTACHOBJECTARBPROC              glAttachObject_             = NULL;
PFNGLGETINFOLOGARBPROC                glGetInfoLog_               = NULL;
PFNGLLINKPROGRAMARBPROC               glLinkProgram_              = NULL;
PFNGLGETUNIFORMLOCATIONARBPROC        glGetUniformLocation_       = NULL;
PFNGLUNIFORM1FARBPROC                 glUniform1f_                = NULL;
PFNGLUNIFORM2FARBPROC                 glUniform2f_                = NULL;
PFNGLUNIFORM3FARBPROC                 glUniform3f_                = NULL;
PFNGLUNIFORM4FARBPROC                 glUniform4f_                = NULL;
PFNGLUNIFORM1FVARBPROC                glUniform1fv_               = NULL;
PFNGLUNIFORM2FVARBPROC                glUniform2fv_               = NULL;
PFNGLUNIFORM3FVARBPROC                glUniform3fv_               = NULL;
PFNGLUNIFORM4FVARBPROC                glUniform4fv_               = NULL;
PFNGLUNIFORM1IARBPROC                 glUniform1i_                = NULL;
PFNGLBINDATTRIBLOCATIONARBPROC        glBindAttribLocation_       = NULL;
PFNGLGETACTIVEUNIFORMARBPROC          glGetActiveUniform_         = NULL;

void initARB() {
  glCreateProgramObject_ =        (PFNGLCREATEPROGRAMOBJECTARBPROC)     SDL_GL_GetProcAddress("glCreateProgramObjectARB");
  glDeleteObject_ =               (PFNGLDELETEOBJECTARBPROC)            SDL_GL_GetProcAddress("glDeleteObjectARB");
  glUseProgramObject_ =           (PFNGLUSEPROGRAMOBJECTARBPROC)        SDL_GL_GetProcAddress("glUseProgramObjectARB");
  glCreateShaderObject_ =         (PFNGLCREATESHADEROBJECTARBPROC)      SDL_GL_GetProcAddress("glCreateShaderObjectARB");
  glShaderSource_ =               (PFNGLSHADERSOURCEARBPROC)            SDL_GL_GetProcAddress("glShaderSourceARB");
  glCompileShader_ =              (PFNGLCOMPILESHADERARBPROC)           SDL_GL_GetProcAddress("glCompileShaderARB");
  glGetObjectParameteriv_ =       (PFNGLGETOBJECTPARAMETERIVARBPROC)    SDL_GL_GetProcAddress("glGetObjectParameterivARB");
  glAttachObject_ =               (PFNGLATTACHOBJECTARBPROC)            SDL_GL_GetProcAddress("glAttachObjectARB");
  glGetInfoLog_ =                 (PFNGLGETINFOLOGARBPROC)              SDL_GL_GetProcAddress("glGetInfoLogARB");
  glLinkProgram_ =                (PFNGLLINKPROGRAMARBPROC)             SDL_GL_GetProcAddress("glLinkProgramARB");
  glGetUniformLocation_ =         (PFNGLGETUNIFORMLOCATIONARBPROC)      SDL_GL_GetProcAddress("glGetUniformLocationARB");
  glUniform1f_ =                  (PFNGLUNIFORM1FARBPROC)               SDL_GL_GetProcAddress("glUniform1fARB");
  glUniform2f_ =                  (PFNGLUNIFORM2FARBPROC)               SDL_GL_GetProcAddress("glUniform2fARB");
  glUniform3f_ =                  (PFNGLUNIFORM3FARBPROC)               SDL_GL_GetProcAddress("glUniform3fARB");
  glUniform4f_ =                  (PFNGLUNIFORM4FARBPROC)               SDL_GL_GetProcAddress("glUniform4fARB");
  glUniform1fv_ =                 (PFNGLUNIFORM1FVARBPROC)              SDL_GL_GetProcAddress("glUniform1fvARB");
  glUniform2fv_ =                 (PFNGLUNIFORM2FVARBPROC)              SDL_GL_GetProcAddress("glUniform2fvARB");
  glUniform3fv_ =                 (PFNGLUNIFORM3FVARBPROC)              SDL_GL_GetProcAddress("glUniform3fvARB");
  glUniform4fv_ =                 (PFNGLUNIFORM4FVARBPROC)              SDL_GL_GetProcAddress("glUniform4fvARB");
  glUniform1i_ =                  (PFNGLUNIFORM1IARBPROC)               SDL_GL_GetProcAddress("glUniform1iARB");
  glBindAttribLocation_ =         (PFNGLBINDATTRIBLOCATIONARBPROC)      SDL_GL_GetProcAddress("glBindAttribLocationARB");
  glGetActiveUniform_ =           (PFNGLGETACTIVEUNIFORMARBPROC)        SDL_GL_GetProcAddress("glGetActiveUniformARB");
}

void setShaders() {
  GLuint v, f, p;
  GLint ok;

	const char *vv = "void main()                   \n"
                   "{                             \n"
	                 "  gl_Position = ftransform() + vec4(0.1, -0.25, 0, 0); \n"
                   "}";
  const char *ff = "void main()                \n"
                   "{	                         \n"
	                 "  gl_FragColor = vec4(gl_FragCoord.y/480.0, gl_FragCoord.x/640.0, 0.66, 1.0); \n"
                   "}";

	v = glCreateShaderObject_(GL_VERTEX_SHADER);
	f = glCreateShaderObject_(GL_FRAGMENT_SHADER);

	glShaderSource_(v, 1, &vv,NULL);
	glShaderSource_(f, 1, &ff,NULL);

	glCompileShader_(v);
  glGetObjectParameteriv_(v, GL_OBJECT_COMPILE_STATUS_ARB, &ok);
  if (!ok) {
    char msg[512];
    glGetShaderInfoLog(v, sizeof msg, NULL, msg);
    printf("shader compilation issue: %s\n", msg);
  }
  assert(ok);

	glCompileShader_(f);
  glGetObjectParameteriv_(f, GL_OBJECT_COMPILE_STATUS_ARB, &ok);
  assert(ok);

	p = glCreateProgramObject_();
	glAttachObject_(p,f);
	glAttachObject_(p,v);

	glLinkProgram_(p);
  glGetObjectParameteriv_(p, GL_OBJECT_LINK_STATUS_ARB, &ok);
  assert(ok);

	glUseProgramObject_(p);
}

int main(int argc, char *argv[])
{
    SDL_Surface *screen;

    assert(SDL_Init(SDL_INIT_VIDEO) == 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    screen = SDL_SetVideoMode( 640, 480, 16, SDL_OPENGL );
    assert(screen);
    
    glClearColor(0, 0, 0, 0);
    glViewport(0, 0, 640, 480);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 640, 480, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glClear(GL_COLOR_BUFFER_BIT);

    initARB();
    setShaders();

    glColor3f(0, 1, 1); // is overridden by the shader, useful for debugging native builds
    glBegin( GL_TRIANGLES );
      glTexCoord2i(0, 0); glVertex3f( 10,  10,  0);
      glTexCoord2i(1, 0); glVertex3f( 300, 10,  0);
      glTexCoord2i(1, 1); glVertex3f( 300, 328, 0);
    glEnd();

    glColor3f(1, 1, 0); // is overridden by the shader, useful for debugging native builds
    glBegin( GL_TRIANGLES );
        glTexCoord2f(0, 0.5); glVertex3f(410, 10,  0);
        glTexCoord2f(1, 0.5); glVertex3f(600, 10,  0);
        glTexCoord2f(1, 1  ); glVertex3f(630, 400, 0);
    glEnd();

    // Test https://github.com/kripken/emscripten/issues/3693
    glBegin( GL_TRIANGLES );
    glEnd();

    SDL_GL_SwapBuffers();
    
#ifndef __EMSCRIPTEN__
    SDL_Delay(3000);
#endif

    SDL_Quit();
    return 0;
}

