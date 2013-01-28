
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>

#include <string.h>
#include <sys/time.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES/gl.h>

typedef struct {
   EGLBoolean verbose;

   EGLDisplay display;
   EGLConfig conf;

   EGLint width, height;

   EGLContext context;
   EGLSurface surface;
   
   GLuint defaultFramebuffer;
} EMSScreen;

static EMSScreen gScreen;


EGLBoolean initOpenGL(EMSScreen*screen)
{
    if (screen == NULL)
        return EGL_FALSE;

    screen->display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    printf("eglGetDisplay returrned %p\n", screen->display);

    if (screen->display == EGL_NO_DISPLAY)
    {
        printf("eglGetDisplay could not find a default context\n");
        return EGL_FALSE;
    }
    
    EGLint maj, min;

    if (!eglInitialize(screen->display, &maj, &min))
    {
        printf("eglInitialize failed\n");
        return EGL_FALSE;
    }
    
    printf("eglInitialize returned version %d.%d\n", maj, min);
    
    if (!eglBindAPI(EGL_OPENGL_ES_API))
    {
        printf("eglBindAPI failed\n");
        return EGL_FALSE;
    }
    
    const EGLint attribs[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES_BIT,
        EGL_NONE
    };

    EGLint num_configs;

    if (!eglChooseConfig(screen->display, attribs, &screen->conf, 1, &num_configs) || num_configs == 0)
    {
        printf("eglChooseConfig failed\n");
        return EGL_FALSE;
    }

    screen->surface = eglCreateWindowSurface(screen->display, screen->conf, (EGLNativeWindowType)NULL, NULL);
    
    if (EGL_NO_SURFACE == screen->surface)
    {
        printf("eglCreateWindowSurface returned EGL_NO_SURFACE\n");
        return EGL_FALSE;
    }
    
    const EGLint context_attribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 1,
        EGL_NONE
    };

    screen->context = eglCreateContext(screen->display, screen->conf, EGL_NO_CONTEXT, context_attribs);

    if (EGL_NO_CONTEXT == screen->context)
    {
        printf("eglCreateContext returned EGL_NO_CONTEXT\n");
        return EGL_FALSE;
    }
    
    return EGL_TRUE;
}


EGLBoolean endOpenGL(EMSScreen*screen)
{
    eglDestroyContext(screen->display, screen->context);
    eglDestroySurface(screen->display, screen->surface);

    if (!eglTerminate(screen->display))
    {
        printf("eglTerminate failed\n");
        return EGL_FALSE;
    }
    return EGL_TRUE;
}

int
main(int argc, char *argv[])
{
    gScreen.width  = 320;
    gScreen.height = 200;
    
    if (!initOpenGL(&gScreen))
    {
        printf("Initializing OpenGLES failed.\n");
        return 1;
    }
    
    printf("Initializing OpenGLES succeeded.\n");

    eglMakeCurrent(gScreen.display, gScreen.surface, gScreen.surface, gScreen.context);

    // Create default framebuffer object.
    glGenFramebuffersOES(1, &gScreen.defaultFramebuffer);


    glDeleteFramebuffers(1, &gScreen.defaultFramebuffer);

    if (!endOpenGL(&gScreen))
    {
        printf("endOpenGL failed\n");
        return 1;
    }
   return 0;
}


