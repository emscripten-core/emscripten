//========================================================================
// This is a small test application for GLFW.
// The program prints "Hello world!", using two threads.
//========================================================================

#include <stdio.h>
#include <GL/glfw.h>


//========================================================================
// HelloFun() - Thread function
//========================================================================

void GLFWCALL HelloFun( void *arg )
{
    // Print the first part of the message
    printf( "Hello " );
}


//========================================================================
// main() - Main function (main thread)
//========================================================================

int main( void )
{
    GLFWthread thread;

    // Initialise GLFW
    if( !glfwInit() )
    {
        return 0;
    }

    // Create thread
    thread = glfwCreateThread( HelloFun, NULL );

    // Wait for thread to die
    glfwWaitThread( thread, GLFW_WAIT );

    // Print the rest of the message
    printf( "world!\n" );

    // Terminate GLFW
    glfwTerminate();

    return 0;
}
