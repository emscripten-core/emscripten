//========================================================================
// This is an example program for the GLFW library
//
// It shows texture loading with mipmap generation and rendering with
// trilienar texture filtering
//========================================================================

#include <stdio.h>
#include <stdlib.h>

#include <GL/glfw.h>

int main( void )
{
    int width, height, x;
    double time;
    GLboolean running;
    GLuint textureID;
    char* texturePath = "mipmaps.tga";

    // Initialise GLFW
    if( !glfwInit() )
    {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        exit( EXIT_FAILURE );
    }

    // Open OpenGL window
    if( !glfwOpenWindow( 640, 480, 0,0,0,0, 0,0, GLFW_WINDOW ) )
    {
        fprintf( stderr, "Failed to open GLFW window\n" );
        glfwTerminate();
        exit( EXIT_FAILURE );
    }

    glfwSetWindowTitle( "Trilinear interpolation" );

    // Enable sticky keys
    glfwEnable( GLFW_STICKY_KEYS );

    // Enable vertical sync (on cards that support it)
    glfwSwapInterval( 1 );

    // Generate and bind our texture ID
    glGenTextures( 1, &textureID );
    glBindTexture( GL_TEXTURE_2D, textureID );

    // Load texture from file into video memory, including mipmap levels
    if( !glfwLoadTexture2D( texturePath, GLFW_BUILD_MIPMAPS_BIT ) )
    {
        fprintf( stderr, "Failed to load texture %s\n", texturePath );
        glfwTerminate();
        exit( EXIT_FAILURE );
    }

    // Use trilinear interpolation (GL_LINEAR_MIPMAP_LINEAR)
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                     GL_LINEAR_MIPMAP_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                     GL_LINEAR );

    // Enable plain 2D texturing
    glEnable( GL_TEXTURE_2D );

    running = GL_TRUE;
    while( running )
    {
        // Get time and mouse position
        time = glfwGetTime();
        glfwGetMousePos( &x, NULL );

        // Get window size (may be different than the requested size)
        glfwGetWindowSize( &width, &height );
        height = height > 0 ? height : 1;

        // Set viewport
        glViewport( 0, 0, width, height );

        // Clear color buffer
        glClearColor( 0.0f, 0.0f, 0.0f, 0.0f);
        glClear( GL_COLOR_BUFFER_BIT );

        // Select and setup the projection matrix
        glMatrixMode( GL_PROJECTION );
        glLoadIdentity();
        gluPerspective( 65.0f, (GLfloat)width / (GLfloat)height, 1.0f,
            50.0f );

        // Select and setup the modelview matrix
        glMatrixMode( GL_MODELVIEW );
        glLoadIdentity();
        gluLookAt( 0.0f,  3.0f, -20.0f,    // Eye-position
                   0.0f, -4.0f, -11.0f,    // View-point
                   0.0f,  1.0f,   0.0f );  // Up-vector

        // Draw a textured quad
        glRotatef( 0.05f * (GLfloat)x + (GLfloat)time * 5.0f, 0.0f, 1.0f, 0.0f );
        glBegin( GL_QUADS );
          glTexCoord2f( -20.0f,  20.0f );
          glVertex3f( -50.0f, 0.0f, -50.0f );
          glTexCoord2f(  20.0f,  20.0f );
          glVertex3f(  50.0f, 0.0f, -50.0f );
          glTexCoord2f(  20.0f, -20.0f );
          glVertex3f(  50.0f, 0.0f,  50.0f );
          glTexCoord2f( -20.0f, -20.0f );
          glVertex3f( -50.0f, 0.0f,  50.0f );
        glEnd();

        // Swap buffers
        glfwSwapBuffers();

        // Check if the ESC key was pressed or the window was closed
        running = !glfwGetKey( GLFW_KEY_ESC ) &&
                  glfwGetWindowParam( GLFW_OPENED );
    }

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    exit( EXIT_SUCCESS );
}

