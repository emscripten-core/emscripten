//========================================================================
// This is an example program for the GLFW library
//
// The program uses a "split window" view, rendering four views of the
// same scene in one window (e.g. uesful for 3D modelling software). This
// demo uses scissors to separete the four different rendering areas from
// each other.
//
// (If the code seems a little bit strange here and there, it may be
//  because I am not a friend of orthogonal projections)
//========================================================================

#include <GL/glfw.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


//========================================================================
// Global variables
//========================================================================

// Mouse position
static int xpos = 0, ypos = 0;

// Window size
static int width, height;

// Active view: 0 = none, 1 = upper left, 2 = upper right, 3 = lower left,
// 4 = lower right
static int active_view = 0;

// Rotation around each axis
static int rot_x = 0, rot_y = 0, rot_z = 0;

// Do redraw?
static int do_redraw = 1;


//========================================================================
// Draw a solid torus (use a display list for the model)
//========================================================================

#define TORUS_MAJOR     1.5
#define TORUS_MINOR     0.5
#define TORUS_MAJOR_RES 32
#define TORUS_MINOR_RES 32

static void drawTorus( void )
{
    static GLuint torus_list = 0;
    int    i, j, k;
    double s, t, x, y, z, nx, ny, nz, scale, twopi;

    if( !torus_list )
    {
        // Start recording displaylist
        torus_list = glGenLists( 1 );
        glNewList( torus_list, GL_COMPILE_AND_EXECUTE );

        // Draw torus
        twopi = 2.0 * M_PI;
        for( i = 0; i < TORUS_MINOR_RES; i++ )
        {
            glBegin( GL_QUAD_STRIP );
            for( j = 0; j <= TORUS_MAJOR_RES; j++ )
            {
                for( k = 1; k >= 0; k-- )
                {
                    s = (i + k) % TORUS_MINOR_RES + 0.5;
                    t = j % TORUS_MAJOR_RES;

                    // Calculate point on surface
                    x = (TORUS_MAJOR+TORUS_MINOR*cos(s*twopi/TORUS_MINOR_RES))*cos(t*twopi/TORUS_MAJOR_RES);
                    y = TORUS_MINOR * sin(s * twopi / TORUS_MINOR_RES);
                    z = (TORUS_MAJOR+TORUS_MINOR*cos(s*twopi/TORUS_MINOR_RES))*sin(t*twopi/TORUS_MAJOR_RES);

                    // Calculate surface normal
                    nx = x - TORUS_MAJOR*cos(t*twopi/TORUS_MAJOR_RES);
                    ny = y;
                    nz = z - TORUS_MAJOR*sin(t*twopi/TORUS_MAJOR_RES);
                    scale = 1.0 / sqrt( nx*nx + ny*ny + nz*nz );
                    nx *= scale;
                    ny *= scale;
                    nz *= scale;

                    glNormal3f( (float)nx, (float)ny, (float)nz );
                    glVertex3f( (float)x, (float)y, (float)z );
                }
            }
            glEnd();
        }

        // Stop recording displaylist
        glEndList();
    }
    else
    {
        // Playback displaylist
        glCallList( torus_list );
    }
}


//========================================================================
// Draw the scene (a rotating torus)
//========================================================================

static void drawScene( void )
{
    const GLfloat model_diffuse[4]  = {1.0f, 0.8f, 0.8f, 1.0f};
    const GLfloat model_specular[4] = {0.6f, 0.6f, 0.6f, 1.0f};
    const GLfloat model_shininess   = 20.0f;

    glPushMatrix();

    // Rotate the object
    glRotatef( (GLfloat)rot_x*0.5f, 1.0f, 0.0f, 0.0f );
    glRotatef( (GLfloat)rot_y*0.5f, 0.0f, 1.0f, 0.0f );
    glRotatef( (GLfloat)rot_z*0.5f, 0.0f, 0.0f, 1.0f );

    // Set model color (used for orthogonal views, lighting disabled)
    glColor4fv( model_diffuse );

    // Set model material (used for perspective view, lighting enabled)
    glMaterialfv( GL_FRONT, GL_DIFFUSE, model_diffuse );
    glMaterialfv( GL_FRONT, GL_SPECULAR, model_specular );
    glMaterialf(  GL_FRONT, GL_SHININESS, model_shininess );

    // Draw torus
    drawTorus();

    glPopMatrix();
}


//========================================================================
// Draw a 2D grid (used for orthogonal views)
//========================================================================

static void drawGrid( float scale, int steps )
{
    int   i;
    float x, y;

    glPushMatrix();

    // Set background to some dark bluish grey
    glClearColor( 0.05f, 0.05f, 0.2f, 0.0f);
    glClear( GL_COLOR_BUFFER_BIT );

    // Setup modelview matrix (flat XY view)
    glLoadIdentity();
    gluLookAt( 0.0, 0.0, 1.0,
               0.0, 0.0, 0.0,
               0.0, 1.0, 0.0 );

    // We don't want to update the Z-buffer
    glDepthMask( GL_FALSE );

    // Set grid color
    glColor3f( 0.0f, 0.5f, 0.5f );

    glBegin( GL_LINES );

    // Horizontal lines
    x = scale * 0.5f * (float)(steps-1);
    y = -scale * 0.5f * (float)(steps-1);
    for( i = 0; i < steps; i ++ )
    {
        glVertex3f( -x, y, 0.0f );
        glVertex3f( x, y, 0.0f );
        y += scale;
    }

    // Vertical lines
    x = -scale * 0.5f * (float)(steps-1);
    y = scale * 0.5f * (float)(steps-1);
    for( i = 0; i < steps; i ++ )
    {
        glVertex3f( x, -y, 0.0f );
        glVertex3f( x, y, 0.0f );
        x += scale;
    }

    glEnd();

    // Enable Z-buffer writing again
    glDepthMask( GL_TRUE );

    glPopMatrix();
}


//========================================================================
// Draw all views
//========================================================================

static void drawAllViews( void )
{
    const GLfloat light_position[4] = {0.0f, 8.0f, 8.0f, 1.0f};
    const GLfloat light_diffuse[4]  = {1.0f, 1.0f, 1.0f, 1.0f};
    const GLfloat light_specular[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    const GLfloat light_ambient[4]  = {0.2f, 0.2f, 0.3f, 1.0f};
    double aspect;

    // Calculate aspect of window
    if( height > 0 )
    {
        aspect = (double)width / (double)height;
    }
    else
    {
        aspect = 1.0;
    }

    // Clear screen
    glClearColor( 0.0f, 0.0f, 0.0f, 0.0f);
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    // Enable scissor test
    glEnable( GL_SCISSOR_TEST );

    // Enable depth test
    glEnable( GL_DEPTH_TEST );
    glDepthFunc( GL_LEQUAL );


    // ** ORTHOGONAL VIEWS **

    // For orthogonal views, use wireframe rendering
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

    // Enable line anti-aliasing
    glEnable( GL_LINE_SMOOTH );
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    // Setup orthogonal projection matrix
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glOrtho( -3.0*aspect, 3.0*aspect, -3.0, 3.0, 1.0, 50.0 );

    // Upper left view (TOP VIEW)
    glViewport( 0, height/2, width/2, height/2 );
    glScissor( 0, height/2, width/2, height/2 );
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    gluLookAt( 0.0f, 10.0f, 1e-3f,   // Eye-position (above)
               0.0f, 0.0f, 0.0f,     // View-point
               0.0f, 1.0f, 0.0f );   // Up-vector
    drawGrid( 0.5, 12 );
    drawScene();

    // Lower left view (FRONT VIEW)
    glViewport( 0, 0, width/2, height/2 );
    glScissor( 0, 0, width/2, height/2 );
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    gluLookAt( 0.0f, 0.0f, 10.0f,    // Eye-position (in front of)
               0.0f, 0.0f, 0.0f,     // View-point
               0.0f, 1.0f, 0.0f );   // Up-vector
    drawGrid( 0.5, 12 );
    drawScene();

    // Lower right view (SIDE VIEW)
    glViewport( width/2, 0, width/2, height/2 );
    glScissor( width/2, 0, width/2, height/2 );
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    gluLookAt( 10.0f, 0.0f, 0.0f,    // Eye-position (to the right)
               0.0f, 0.0f, 0.0f,     // View-point
               0.0f, 1.0f, 0.0f );   // Up-vector
    drawGrid( 0.5, 12 );
    drawScene();

    // Disable line anti-aliasing
    glDisable( GL_LINE_SMOOTH );
    glDisable( GL_BLEND );


    // ** PERSPECTIVE VIEW **

    // For perspective view, use solid rendering
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

    // Enable face culling (faster rendering)
    glEnable( GL_CULL_FACE );
    glCullFace( GL_BACK );
    glFrontFace( GL_CW );

    // Setup perspective projection matrix
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    gluPerspective( 65.0f, aspect, 1.0f, 50.0f );

    // Upper right view (PERSPECTIVE VIEW)
    glViewport( width/2, height/2, width/2, height/2 );
    glScissor( width/2, height/2, width/2, height/2 );
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    gluLookAt( 3.0f, 1.5f, 3.0f,     // Eye-position
               0.0f, 0.0f, 0.0f,     // View-point
               0.0f, 1.0f, 0.0f );   // Up-vector

    // Configure and enable light source 1
    glLightfv( GL_LIGHT1, GL_POSITION, light_position );
    glLightfv( GL_LIGHT1, GL_AMBIENT, light_ambient );
    glLightfv( GL_LIGHT1, GL_DIFFUSE, light_diffuse );
    glLightfv( GL_LIGHT1, GL_SPECULAR, light_specular );
    glEnable( GL_LIGHT1 );
    glEnable( GL_LIGHTING );

    // Draw scene
    drawScene();

    // Disable lighting
    glDisable( GL_LIGHTING );

    // Disable face culling
    glDisable( GL_CULL_FACE );

    // Disable depth test
    glDisable( GL_DEPTH_TEST );

    // Disable scissor test
    glDisable( GL_SCISSOR_TEST );


    // Draw a border around the active view
    if( active_view > 0 && active_view != 2 )
    {
        glViewport( 0, 0, width, height );
        glMatrixMode( GL_PROJECTION );
        glLoadIdentity();
        glOrtho( 0.0, 2.0, 0.0, 2.0, 0.0, 1.0 );
        glMatrixMode( GL_MODELVIEW );
        glLoadIdentity();
        glColor3f( 1.0f, 1.0f, 0.6f );
        glTranslatef( (GLfloat) ((active_view - 1) & 1), (GLfloat) (1 - (active_view - 1) / 2), 0.0f );
        glBegin( GL_LINE_STRIP );
          glVertex2i( 0, 0 );
          glVertex2i( 1, 0 );
          glVertex2i( 1, 1 );
          glVertex2i( 0, 1 );
          glVertex2i( 0, 0 );
        glEnd();
    }
}


//========================================================================
// Window size callback function
//========================================================================

static void GLFWCALL windowSizeFun( int w, int h )
{
    width  = w;
    height = h > 0 ? h : 1;
    do_redraw = 1;
}


//========================================================================
// Window refresh callback function
//========================================================================

static void GLFWCALL windowRefreshFun( void )
{
    do_redraw = 1;
}


//========================================================================
// Mouse position callback function
//========================================================================

static void GLFWCALL mousePosFun( int x, int y )
{
    // Depending on which view was selected, rotate around different axes
    switch( active_view )
    {
        case 1:
            rot_x += y - ypos;
            rot_z += x - xpos;
            do_redraw = 1;
            break;
        case 3:
            rot_x += y - ypos;
            rot_y += x - xpos;
            do_redraw = 1;
            break;
        case 4:
            rot_y += x - xpos;
            rot_z += y - ypos;
            do_redraw = 1;
            break;
        default:
            // Do nothing for perspective view, or if no view is selected
            break;
    }

    // Remember mouse position
    xpos = x;
    ypos = y;
}


//========================================================================
// Mouse button callback function
//========================================================================

static void GLFWCALL mouseButtonFun( int button, int action )
{
    // Button clicked?
    if( ( button == GLFW_MOUSE_BUTTON_LEFT ) && action == GLFW_PRESS )
    {
        // Detect which of the four views was clicked
        active_view = 1;
        if( xpos >= width/2 )
        {
            active_view += 1;
        }
        if( ypos >= height/2 )
        {
            active_view += 2;
        }
    }

    // Button released?
    else if( button == GLFW_MOUSE_BUTTON_LEFT )
    {
        // Deselect any previously selected view
        active_view = 0;
    }

    do_redraw = 1;
}


//========================================================================
// main()
//========================================================================

int main( void )
{
    // Initialise GLFW
    if( !glfwInit() )
    {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        exit( EXIT_FAILURE );
    }

    // Open OpenGL window
    if( !glfwOpenWindow( 500, 500, 0,0,0,0, 16,0, GLFW_WINDOW ) )
    {
        fprintf( stderr, "Failed to open GLFW window\n" );
        glfwTerminate();
        exit( EXIT_FAILURE );
    }

    // Enable vsync
    glfwSwapInterval( 1 );

    // Set window title
    glfwSetWindowTitle( "Split view demo" );

    // Enable sticky keys
    glfwEnable( GLFW_STICKY_KEYS );

    // Enable mouse cursor (only needed for fullscreen mode)
    glfwEnable( GLFW_MOUSE_CURSOR );

    // Disable automatic event polling
    glfwDisable( GLFW_AUTO_POLL_EVENTS );

    // Set callback functions
    glfwSetWindowSizeCallback( windowSizeFun );
    glfwSetWindowRefreshCallback( windowRefreshFun );
    glfwSetMousePosCallback( mousePosFun );
    glfwSetMouseButtonCallback( mouseButtonFun );

    // Main loop
    do
    {
        // Only redraw if we need to
        if( do_redraw )
        {
            // Draw all views
            drawAllViews();

            // Swap buffers
            glfwSwapBuffers();

            do_redraw = 0;
        }

        // Wait for new events
        glfwWaitEvents();

    } // Check if the ESC key was pressed or the window was closed
    while( glfwGetKey( GLFW_KEY_ESC ) != GLFW_PRESS &&
           glfwGetWindowParam( GLFW_OPENED ) );

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    exit( EXIT_SUCCESS );
}

