//
// Book:      OpenGL(R) ES 2.0 Programming Guide
// Authors:   Aaftab Munshi, Dan Ginsburg, Dave Shreiner
// ISBN-10:   0321502795
// ISBN-13:   9780321502797
// Publisher: Addison-Wesley Professional
// URLs:      http://safari.informit.com/9780321563835
//            http://www.opengles-book.com
//

// Hello_Triangle_KD.c
//
//    This is a simple example that draws a single triangle with
//    a minimal vertex/fragment shader using OpenKODE.
//
#include <KD/kd.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>

typedef struct
{
   // Handle to a program object
   GLuint programObject;

   // EGL handles
   EGLDisplay eglDisplay;
   EGLContext eglContext;
   EGLSurface eglSurface;

} UserData;

///
// Create a shader object, load the shader source, and
// compile the shader.
//
GLuint LoadShader ( GLenum type, const char *shaderSrc )
{
   GLuint shader;
   GLint compiled;
   
   // Create the shader object
   shader = glCreateShader ( type );

   if ( shader == 0 )
   	return 0;

   // Load the shader source
   glShaderSource ( shader, 1, &shaderSrc, NULL );
   
   // Compile the shader
   glCompileShader ( shader );

   // Check the compile status
   glGetShaderiv ( shader, GL_COMPILE_STATUS, &compiled );

   if ( !compiled ) 
   {
      GLint infoLen = 0;

      glGetShaderiv ( shader, GL_INFO_LOG_LENGTH, &infoLen );
      
      if ( infoLen > 1 )
      {
         char* infoLog = kdMalloc (sizeof(char) * infoLen );

         glGetShaderInfoLog ( shader, infoLen, NULL, infoLog );
         kdLogMessage ( infoLog );            
         
         kdFree ( infoLog );
      }

      glDeleteShader ( shader );
      return 0;
   }

   return shader;

}

///
// Initialize the shader and program object
//
int Init ( UserData *userData )
{
   GLbyte vShaderStr[] =  
      "attribute vec4 vPosition;    \n"
      "void main()                  \n"
      "{                            \n"
      "   gl_Position = vPosition;  \n"
      "}                            \n";
   
   GLbyte fShaderStr[] =  
      "precision mediump float;\n"\
      "void main()                                  \n"
      "{                                            \n"
      "  gl_FragColor = vec4 ( 1.0, 0.0, 0.0, 1.0 );\n"
      "}                                            \n";

   GLuint vertexShader;
   GLuint fragmentShader;
   GLuint programObject;
   GLint linked;

   // Load the vertex/fragment shaders
   vertexShader = LoadShader ( GL_VERTEX_SHADER, vShaderStr );
   fragmentShader = LoadShader ( GL_FRAGMENT_SHADER, fShaderStr );

   // Create the program object
   programObject = glCreateProgram ( );
   
   if ( programObject == 0 )
      return 0;

   glAttachShader ( programObject, vertexShader );
   glAttachShader ( programObject, fragmentShader );

   // Bind vPosition to attribute 0   
   glBindAttribLocation ( programObject, 0, "vPosition" );

   // Link the program
   glLinkProgram ( programObject );

   // Check the link status
   glGetProgramiv ( programObject, GL_LINK_STATUS, &linked );

   if ( !linked ) 
   {
      GLint infoLen = 0;

      glGetProgramiv ( programObject, GL_INFO_LOG_LENGTH, &infoLen );
      
      if ( infoLen > 1 )
      {
         char* infoLog = kdMalloc (sizeof(char) * infoLen );

         glGetProgramInfoLog ( programObject, infoLen, NULL, infoLog );
         kdLogMessage ( infoLog );
         
         kdFree ( infoLog );
      }

      glDeleteProgram ( programObject );
      return FALSE;
   }

   // Store the program object
   userData->programObject = programObject;

   glClearColor ( 0.0f, 0.0f, 0.0f, 0.0f );
   return TRUE;
}

///
// Draw a triangle using the shader pair created in Init()
//
void Draw ( UserData *userData )
{
   GLfloat vVertices[] = {  0.0f,  0.5f, 0.0f, 
                           -0.5f, -0.5f, 0.0f,
                            0.5f, -0.5f, 0.0f };
      
   // Set the viewport
   glViewport ( 0, 0, 320, 240 );
   
   // Clear the color buffer
   glClear ( GL_COLOR_BUFFER_BIT );

   // Use the program object
   glUseProgram ( userData->programObject );

   // Load the vertex data
   glVertexAttribPointer ( 0, 3, GL_FLOAT, GL_FALSE, 0, vVertices );
   glEnableVertexAttribArray ( 0 );

   glDrawArrays ( GL_TRIANGLES, 0, 3 );

   eglSwapBuffers ( userData->eglDisplay, userData->eglSurface );
}


///
// InitEGLContext()
//
//    Initialize an EGL rendering context and all associated elements
//
EGLBoolean InitEGLContext ( UserData *userData,
                            KDWindow *window,                             
                            EGLConfig config )
{
   EGLContext context;
   EGLSurface surface;   
   EGLint contextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE, EGL_NONE };

   // Get native window handle
   EGLNativeWindowType hWnd;
   if(kdRealizeWindow(window, &hWnd) != 0)
   {
      return EGL_FALSE;
   }
   surface = eglCreateWindowSurface(userData->eglDisplay, config, hWnd, NULL);
   if ( surface == EGL_NO_SURFACE )
   {
      return EGL_FALSE;
   }

   // Create a GL context
   context = eglCreateContext(userData->eglDisplay, config, EGL_NO_CONTEXT, contextAttribs );
   if ( context == EGL_NO_CONTEXT )
   {
      return EGL_FALSE;
   }   

   // Make the context current
   if ( !eglMakeCurrent(userData->eglDisplay, surface, surface, context) )
   {
      return EGL_FALSE;
   }

   userData->eglContext = context;
   userData->eglSurface = surface;

   return EGL_TRUE;
} 

/// 
// kdMain() 
//
//    Main function for OpenKODE application
//
KDint kdMain ( KDint argc, const KDchar *const *argv )
{
   EGLint attribList[] =
   {
       EGL_RED_SIZE,       8,
       EGL_GREEN_SIZE,     8,
       EGL_BLUE_SIZE,      8,
       EGL_ALPHA_SIZE,     EGL_DONT_CARE,
       EGL_DEPTH_SIZE,     EGL_DONT_CARE,
       EGL_STENCIL_SIZE,   EGL_DONT_CARE,
       EGL_NONE
   };
   EGLint majorVersion, 
         minorVersion;
   UserData userData;
   EGLint numConfigs;
   EGLConfig config;
   KDWindow *window = KD_NULL;

   userData.eglDisplay = eglGetDisplay( EGL_DEFAULT_DISPLAY );

   // Initialize EGL
   if ( !eglInitialize(userData.eglDisplay, &majorVersion, &minorVersion) )
   {
      return EGL_FALSE;
   }

   // Get configs
   if ( !eglGetConfigs(userData.eglDisplay, NULL, 0, &numConfigs) )
   {
      return EGL_FALSE;
   }

   // Choose config
   if ( !eglChooseConfig(userData.eglDisplay, attribList, &config, 1, &numConfigs) )
   {
      return EGL_FALSE;
   }


   // Use OpenKODE to create a Window
   window = kdCreateWindow ( userData.eglDisplay, config, KD_NULL );
   if( !window )
      kdExit ( 0 );

   if ( !InitEGLContext ( &userData, window, config ) )
      kdExit ( 0 );

   if ( !Init ( &userData ) )
      kdExit ( 0 );

   // Main Loop
   while ( 1 )
   {
      // Wait for an event
      const KDEvent *evt = kdWaitEvent ( 0 );
      if ( evt )
      {
         // Exit app
         if ( evt->type == KD_EVENT_WINDOW_CLOSE)
            break;
      }

      // Draw frame
      Draw ( &userData );
   }

   // EGL clean up 
   eglMakeCurrent ( 0, 0, 0, 0 );
   eglDestroySurface ( userData.eglDisplay, userData.eglSurface );
   eglDestroyContext ( userData.eglDisplay, userData.eglContext );

   // Destroy the window
   kdDestroyWindow(window);

   return 0;
}
