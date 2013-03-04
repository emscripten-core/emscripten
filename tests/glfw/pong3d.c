//========================================================================
// This is a small test application for GLFW.
// This is an OpenGL port of the famous "PONG" game (the first computer
// game ever?). It is very simple, and could be improved alot. It was
// created in order to show off the gaming capabilities of GLFW.
//========================================================================

#include <GL/glfw.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


//========================================================================
// Constants
//========================================================================

// Screen resolution
#define WIDTH        640
#define HEIGHT       480

// Player size (units)
#define PLAYER_XSIZE  0.05f
#define PLAYER_YSIZE  0.15f

// Ball size (units)
#define BALL_SIZE  0.02f

// Maximum player movement speed (units / second)
#define MAX_SPEED    1.5f

// Player movement acceleration (units / seconds^2)
#define ACCELERATION  4.0f

// Player movement deceleration (units / seconds^2)
#define DECELERATION  2.0f

// Ball movement speed (units / second)
#define BALL_SPEED    0.4f

// Menu options
#define MENU_NONE    0
#define MENU_PLAY    1
#define MENU_QUIT    2

// Game events
#define NOBODY_WINS  0
#define PLAYER1_WINS 1
#define PLAYER2_WINS 2

// Winner ID
#define NOBODY       0
#define PLAYER1      1
#define PLAYER2      2

// Camera positions
#define CAMERA_CLASSIC   0
#define CAMERA_ABOVE     1
#define CAMERA_SPECTATOR 2
#define CAMERA_DEFAULT   CAMERA_CLASSIC


//========================================================================
// Textures
//========================================================================

#define TEX_TITLE    0
#define TEX_MENU     1
#define TEX_INSTR    2
#define TEX_WINNER1  3
#define TEX_WINNER2  4
#define TEX_FIELD    5
#define NUM_TEXTURES 6

// Texture names
char * tex_name[ NUM_TEXTURES ] = {
    "pong3d_title.tga",
    "pong3d_menu.tga",
    "pong3d_instr.tga",
    "pong3d_winner1.tga",
    "pong3d_winner2.tga",
    "pong3d_field.tga"
};

// OpenGL texture object IDs
GLuint tex_id[ NUM_TEXTURES ];


//========================================================================
// Global variables
//========================================================================

// Display information
int width, height;

// Frame information
double thistime, oldtime, dt, starttime;

// Camera information
int camerapos;

// Player information
struct {
    double ypos;     // -1.0 to +1.0
    double yspeed;   // -MAX_SPEED to +MAX_SPEED
} player1, player2;

// Ball information
struct {
    double xpos, ypos;
    double xspeed, yspeed;
} ball;

// And the winner is...
int winner;

// Lighting configuration
const GLfloat env_ambient[4]     = {1.0f,1.0f,1.0f,1.0f};
const GLfloat light1_position[4] = {-3.0f,3.0f,2.0f,1.0f};
const GLfloat light1_diffuse[4]  = {1.0f,1.0f,1.0f,0.0f};
const GLfloat light1_ambient[4]  = {0.0f,0.0f,0.0f,0.0f};

// Object material properties
const GLfloat player1_diffuse[4] = {1.0f,0.3f,0.3f,1.0f};
const GLfloat player1_ambient[4] = {0.3f,0.1f,0.0f,1.0f};
const GLfloat player2_diffuse[4] = {0.3f,1.0f,0.3f,1.0f};
const GLfloat player2_ambient[4] = {0.1f,0.3f,0.1f,1.0f};
const GLfloat ball_diffuse[4]    = {1.0f,1.0f,0.5f,1.0f};
const GLfloat ball_ambient[4]    = {0.3f,0.3f,0.1f,1.0f};
const GLfloat border_diffuse[4]  = {0.3f,0.3f,1.0f,1.0f};
const GLfloat border_ambient[4]  = {0.1f,0.1f,0.3f,1.0f};
const GLfloat floor_diffuse[4]   = {1.0f,1.0f,1.0f,1.0f};
const GLfloat floor_ambient[4]   = {0.3f,0.3f,0.3f,1.0f};


//========================================================================
// LoadTextures() - Load textures from disk and upload to OpenGL card
//========================================================================

GLboolean LoadTextures( void )
{
    int  i;

    // Generate texture objects
    glGenTextures( NUM_TEXTURES, tex_id );

    // Load textures
    for( i = 0; i < NUM_TEXTURES; i ++ )
    {
        // Select texture object
        glBindTexture( GL_TEXTURE_2D, tex_id[ i ] );

        // Set texture parameters
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

        // Upload texture from file to texture memory
        if( !glfwLoadTexture2D( tex_name[ i ], 0 ) )
        {
            fprintf( stderr, "Failed to load texture %s\n", tex_name[ i ] );
            return GL_FALSE;
        }
    }

    return GL_TRUE;
}


//========================================================================
// DrawImage() - Draw a 2D image as a texture
//========================================================================

void DrawImage( int texnum, float x1, float x2, float y1, float y2 )
{
    glEnable( GL_TEXTURE_2D );
    glBindTexture( GL_TEXTURE_2D, tex_id[ texnum ] );
    glBegin( GL_QUADS );
      glTexCoord2f( 0.0f, 1.0f );
      glVertex2f( x1, y1 );
      glTexCoord2f( 1.0f, 1.0f );
      glVertex2f( x2, y1 );
      glTexCoord2f( 1.0f, 0.0f );
      glVertex2f( x2, y2 );
      glTexCoord2f( 0.0f, 0.0f );
      glVertex2f( x1, y2 );
    glEnd();
    glDisable( GL_TEXTURE_2D );
}


//========================================================================
// GameMenu() - Game menu (returns menu option)
//========================================================================

int GameMenu( void )
{
    int option;

    // Enable sticky keys
    glfwEnable( GLFW_STICKY_KEYS );

    // Wait for a game menu key to be pressed
    do
    {
        // Get window size
        glfwGetWindowSize( &width, &height );

        // Set viewport
        glViewport( 0, 0, width, height );

        // Clear display
        glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
        glClear( GL_COLOR_BUFFER_BIT );

        // Setup projection matrix
        glMatrixMode( GL_PROJECTION );
        glLoadIdentity();
        glOrtho( 0.0f, 1.0f, 1.0f, 0.0f, -1.0f, 1.0f );

        // Setup modelview matrix
        glMatrixMode( GL_MODELVIEW );
        glLoadIdentity();

        // Display title
        glColor3f( 1.0f, 1.0f, 1.0f );
        DrawImage( TEX_TITLE, 0.1f, 0.9f, 0.0f, 0.3f );

        // Display menu
        glColor3f( 1.0f, 1.0f, 0.0f );
        DrawImage( TEX_MENU, 0.38f, 0.62f, 0.35f, 0.5f );

        // Display instructions
        glColor3f( 0.0f, 1.0f, 1.0f );
        DrawImage( TEX_INSTR, 0.32f, 0.68f, 0.65f, 0.85f );

        // Swap buffers
        glfwSwapBuffers();

        // Check for keys
        if( glfwGetKey( 'Q' ) || !glfwGetWindowParam( GLFW_OPENED ) )
        {
            option = MENU_QUIT;
        }
        else if( glfwGetKey( GLFW_KEY_F1 ) )
        {
            option = MENU_PLAY;
        }
        else
        {
            option = MENU_NONE;
        }

        // To avoid horrible busy waiting, sleep for at least 20 ms
        glfwSleep( 0.02 );
    }
    while( option == MENU_NONE );

    // Disable sticky keys
    glfwDisable( GLFW_STICKY_KEYS );

    return option;
}


//========================================================================
// NewGame() - Initialize a new game
//========================================================================

void NewGame( void )
{
    // Frame information
    starttime = thistime = glfwGetTime();

    // Camera information
    camerapos = CAMERA_DEFAULT;

    // Player 1 information
    player1.ypos   = 0.0;
    player1.yspeed = 0.0;

    // Player 2 information
    player2.ypos   = 0.0;
    player2.yspeed = 0.0;

    // Ball information
    ball.xpos = -1.0 + PLAYER_XSIZE;
    ball.ypos = player1.ypos;
    ball.xspeed = 1.0;
    ball.yspeed = 1.0;
}


//========================================================================
// PlayerControl() - Player control
//========================================================================

void PlayerControl( void )
{
    float joy1pos[ 2 ], joy2pos[ 2 ];

    // Get joystick X & Y axis positions
    glfwGetJoystickPos( GLFW_JOYSTICK_1, joy1pos, 2 );
    glfwGetJoystickPos( GLFW_JOYSTICK_2, joy2pos, 2 );

    // Player 1 control
    if( glfwGetKey( 'A' ) || joy1pos[ 1 ] > 0.2f )
    {
        player1.yspeed += dt * ACCELERATION;
        if( player1.yspeed > MAX_SPEED )
        {
            player1.yspeed = MAX_SPEED;
        }
    }
    else if( glfwGetKey( 'Z' ) || joy1pos[ 1 ] < -0.2f )
    {
        player1.yspeed -= dt * ACCELERATION;
        if( player1.yspeed < -MAX_SPEED )
        {
            player1.yspeed = -MAX_SPEED;
        }
    }
    else
    {
        player1.yspeed /= exp( DECELERATION * dt );
    }

    // Player 2 control
    if( glfwGetKey( 'K' ) || joy2pos[ 1 ] > 0.2f )
    {
        player2.yspeed += dt * ACCELERATION;
        if( player2.yspeed > MAX_SPEED )
        {
            player2.yspeed = MAX_SPEED;
        }
    }
    else if( glfwGetKey( 'M' ) || joy2pos[ 1 ] < -0.2f )
    {
        player2.yspeed -= dt * ACCELERATION;
        if( player2.yspeed < -MAX_SPEED )
        {
            player2.yspeed = -MAX_SPEED;
        }
    }
    else
    {
        player2.yspeed /= exp( DECELERATION * dt );
    }

    // Update player 1 position
    player1.ypos += dt * player1.yspeed;
    if( player1.ypos > 1.0 - PLAYER_YSIZE )
    {
        player1.ypos = 1.0 - PLAYER_YSIZE;
        player1.yspeed = 0.0;
    }
    else if( player1.ypos < -1.0 + PLAYER_YSIZE )
    {
        player1.ypos = -1.0 + PLAYER_YSIZE;
        player1.yspeed = 0.0;
    }

    // Update player 2 position
    player2.ypos += dt * player2.yspeed;
    if( player2.ypos > 1.0 - PLAYER_YSIZE )
    {
        player2.ypos = 1.0 - PLAYER_YSIZE;
        player2.yspeed = 0.0;
    }
    else if( player2.ypos < -1.0 + PLAYER_YSIZE )
    {
        player2.ypos = -1.0 + PLAYER_YSIZE;
        player2.yspeed = 0.0;
    }
}


//========================================================================
// BallControl() - Ball control
//========================================================================

int BallControl( void )
{
    int event;
    double ballspeed;

    // Calculate new ball speed
    ballspeed = BALL_SPEED * (1.0 + 0.02*(thistime-starttime));
    ball.xspeed = ball.xspeed > 0 ? ballspeed : -ballspeed;
    ball.yspeed = ball.yspeed > 0 ? ballspeed : -ballspeed;
    ball.yspeed *= 0.74321;

    // Update ball position
    ball.xpos += dt * ball.xspeed;
    ball.ypos += dt * ball.yspeed;

    // Did the ball hit a top/bottom wall?
    if( ball.ypos >= 1.0 )
    {
        ball.ypos = 2.0 - ball.ypos;
        ball.yspeed = -ball.yspeed;
    }
    else if( ball.ypos <= -1.0 )
    {
        ball.ypos = -2.0 - ball.ypos;
        ball.yspeed = -ball.yspeed;
    }

    // Did the ball hit/miss a player?
    event = NOBODY_WINS;

    // Is the ball entering the player 1 goal?
    if( ball.xpos < -1.0 + PLAYER_XSIZE )
    {
        // Did player 1 catch the ball?
        if( ball.ypos > (player1.ypos-PLAYER_YSIZE) &&
            ball.ypos < (player1.ypos+PLAYER_YSIZE) )
        {
            ball.xpos = -2.0 + 2.0*PLAYER_XSIZE - ball.xpos;
            ball.xspeed = -ball.xspeed;
        }
        else
        {
            event = PLAYER2_WINS;
        }
    }

    // Is the ball entering the player 2 goal?
    if( ball.xpos > 1.0 - PLAYER_XSIZE )
    {
        // Did player 2 catch the ball?
        if( ball.ypos > (player2.ypos-PLAYER_YSIZE) &&
            ball.ypos < (player2.ypos+PLAYER_YSIZE) )
        {
            ball.xpos = 2.0 - 2.0*PLAYER_XSIZE - ball.xpos;
            ball.xspeed = -ball.xspeed;
        }
        else
        {
            event = PLAYER1_WINS;
        }
    }

    return event;
}


//========================================================================
// DrawBox() - Draw a 3D box
//========================================================================

#define TEX_SCALE 4.0f


void DrawBox( float x1, float y1, float z1, float x2, float y2, float z2 )
{
    // Draw six sides of a cube
    glBegin( GL_QUADS );
      // Side 1 (down)
      glNormal3f( 0.0f, 0.0f, -1.0f );
      glTexCoord2f( 0.0f, 0.0f );
      glVertex3f( x1,y2,z1 );
      glTexCoord2f( TEX_SCALE, 0.0f );
      glVertex3f( x2,y2,z1 );
      glTexCoord2f( TEX_SCALE, TEX_SCALE );
      glVertex3f( x2,y1,z1 );
      glTexCoord2f( 0.0f, TEX_SCALE );
      glVertex3f( x1,y1,z1 );
      // Side 2 (up)
      glNormal3f( 0.0f, 0.0f, 1.0f );
      glTexCoord2f( 0.0f, 0.0f );
      glVertex3f( x1,y1,z2 );
      glTexCoord2f( TEX_SCALE, 0.0f );
      glVertex3f( x2,y1,z2 );
      glTexCoord2f( TEX_SCALE, TEX_SCALE );
      glVertex3f( x2,y2,z2 );
      glTexCoord2f( 0.0f, TEX_SCALE );
      glVertex3f( x1,y2,z2 );
      // Side 3 (backward)
      glNormal3f( 0.0f, -1.0f, 0.0f );
      glTexCoord2f( 0.0f, 0.0f );
      glVertex3f( x1,y1,z1 );
      glTexCoord2f( TEX_SCALE, 0.0f );
      glVertex3f( x2,y1,z1 );
      glTexCoord2f( TEX_SCALE, TEX_SCALE );
      glVertex3f( x2,y1,z2 );
      glTexCoord2f( 0.0f, TEX_SCALE );
      glVertex3f( x1,y1,z2 );
      // Side 4 (forward)
      glNormal3f( 0.0f, 1.0f, 0.0f );
      glTexCoord2f( 0.0f, 0.0f );
      glVertex3f( x1,y2,z2 );
      glTexCoord2f( TEX_SCALE, 0.0f );
      glVertex3f( x2,y2,z2 );
      glTexCoord2f( TEX_SCALE, TEX_SCALE );
      glVertex3f( x2,y2,z1 );
      glTexCoord2f( 0.0f, TEX_SCALE );
      glVertex3f( x1,y2,z1 );
      // Side 5 (left)
      glNormal3f( -1.0f, 0.0f, 0.0f );
      glTexCoord2f( 0.0f, 0.0f );
      glVertex3f( x1,y1,z2 );
      glTexCoord2f( TEX_SCALE, 0.0f );
      glVertex3f( x1,y2,z2 );
      glTexCoord2f( TEX_SCALE, TEX_SCALE );
      glVertex3f( x1,y2,z1 );
      glTexCoord2f( 0.0f, TEX_SCALE );
      glVertex3f( x1,y1,z1 );
      // Side 6 (right)
      glNormal3f( 1.0f, 0.0f, 0.0f );
      glTexCoord2f( 0.0f, 0.0f );
      glVertex3f( x2,y1,z1 );
      glTexCoord2f( TEX_SCALE, 0.0f );
      glVertex3f( x2,y2,z1 );
      glTexCoord2f( TEX_SCALE, TEX_SCALE );
      glVertex3f( x2,y2,z2 );
      glTexCoord2f( 0.0f, TEX_SCALE );
      glVertex3f( x2,y1,z2 );
    glEnd();
}


//========================================================================
// UpdateDisplay() - Draw graphics (all game related OpenGL stuff goes
// here)
//========================================================================

void UpdateDisplay( void )
{
    // Get window size
    glfwGetWindowSize( &width, &height );

    // Set viewport
    glViewport( 0, 0, width, height );

    // Clear display
    glClearColor( 0.02f, 0.02f, 0.02f, 0.0f );
    glClearDepth( 1.0f );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    // Setup projection matrix
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    gluPerspective(
        55.0f,                            // Angle of view
        (GLfloat)width/(GLfloat)height,   // Aspect
        1.0f,                             // Near Z
        100.0f                            // Far Z
    );

    // Setup modelview matrix
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    switch( camerapos )
    {
    default:
    case CAMERA_CLASSIC:
        gluLookAt(
            0.0f, 0.0f, 2.5f,
            0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f
        );
        break;
    case CAMERA_ABOVE:
        gluLookAt(
            0.0f, 0.0f, 2.5f,
            (float)ball.xpos, (float)ball.ypos, 0.0f,
            0.0f, 1.0f, 0.0f
        );
        break;
    case CAMERA_SPECTATOR:
        gluLookAt(
            0.0f, -2.0, 1.2f,
            (float)ball.xpos, (float)ball.ypos, 0.0f,
            0.0f, 0.0f, 1.0f
        );
        break;
    }

    // Enable depth testing
    glEnable( GL_DEPTH_TEST );
    glDepthFunc( GL_LEQUAL );

    // Enable lighting
    glEnable( GL_LIGHTING );
    glLightModelfv( GL_LIGHT_MODEL_AMBIENT, env_ambient );
    glLightModeli( GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE );
    glLightModeli( GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE );
    glLightfv( GL_LIGHT1, GL_POSITION, light1_position );
    glLightfv( GL_LIGHT1, GL_DIFFUSE,  light1_diffuse );
    glLightfv( GL_LIGHT1, GL_AMBIENT,  light1_ambient );
    glEnable( GL_LIGHT1 );

    // Front face is counter-clock-wise
    glFrontFace( GL_CCW );

    // Enable face culling (not necessary, but speeds up rendering)
    glCullFace( GL_BACK );
    glEnable( GL_CULL_FACE );

    // Draw Player 1
    glMaterialfv( GL_FRONT, GL_DIFFUSE, player1_diffuse );
    glMaterialfv( GL_FRONT, GL_AMBIENT, player1_ambient );
    DrawBox( -1.f,              (GLfloat)player1.ypos-PLAYER_YSIZE, 0.f,
             -1.f+PLAYER_XSIZE, (GLfloat)player1.ypos+PLAYER_YSIZE, 0.1f );

    // Draw Player 2
    glMaterialfv( GL_FRONT, GL_DIFFUSE, player2_diffuse );
    glMaterialfv( GL_FRONT, GL_AMBIENT, player2_ambient );
    DrawBox( 1.f-PLAYER_XSIZE, (GLfloat)player2.ypos-PLAYER_YSIZE, 0.f,
             1.f,              (GLfloat)player2.ypos+PLAYER_YSIZE, 0.1f );

    // Draw Ball
    glMaterialfv( GL_FRONT, GL_DIFFUSE, ball_diffuse );
    glMaterialfv( GL_FRONT, GL_AMBIENT, ball_ambient );
    DrawBox( (GLfloat)ball.xpos-BALL_SIZE, (GLfloat)ball.ypos-BALL_SIZE, 0.f,
             (GLfloat)ball.xpos+BALL_SIZE, (GLfloat)ball.ypos+BALL_SIZE, BALL_SIZE*2 );

    // Top game field border
    glMaterialfv( GL_FRONT, GL_DIFFUSE, border_diffuse );
    glMaterialfv( GL_FRONT, GL_AMBIENT, border_ambient );
    DrawBox( -1.1f, 1.0f, 0.0f,  1.1f, 1.1f, 0.1f );
    // Bottom game field border
    glColor3f( 0.0f, 0.0f, 0.7f );
    DrawBox( -1.1f, -1.1f, 0.0f,  1.1f, -1.0f, 0.1f );
    // Left game field border
    DrawBox( -1.1f, -1.0f, 0.0f,  -1.0f, 1.0f, 0.1f );
    // Left game field border
    DrawBox( 1.0f, -1.0f, 0.0f,  1.1f, 1.0f, 0.1f );

    // Enable texturing
    glEnable( GL_TEXTURE_2D );
    glBindTexture( GL_TEXTURE_2D, tex_id[ TEX_FIELD ] );

    // Game field floor
    glMaterialfv( GL_FRONT, GL_DIFFUSE, floor_diffuse );
    glMaterialfv( GL_FRONT, GL_AMBIENT, floor_ambient );
    DrawBox( -1.01f, -1.01f, -0.01f,  1.01f, 1.01f, 0.0f );

    // Disable texturing
    glDisable( GL_TEXTURE_2D );

    // Disable face culling
    glDisable( GL_CULL_FACE );

    // Disable lighting
    glDisable( GL_LIGHTING );

    // Disable depth testing
    glDisable( GL_DEPTH_TEST );
}


//========================================================================
// GameOver()
//========================================================================

void GameOver( void )
{
    // Enable sticky keys
    glfwEnable( GLFW_STICKY_KEYS );

    // Until the user presses ESC or SPACE
    while( !glfwGetKey( GLFW_KEY_ESC ) && !glfwGetKey( ' ' ) &&
           glfwGetWindowParam( GLFW_OPENED ) )
    {
        // Draw display
        UpdateDisplay();

        // Setup projection matrix
        glMatrixMode( GL_PROJECTION );
        glLoadIdentity();
        glOrtho( 0.0f, 1.0f, 1.0f, 0.0f, -1.0f, 1.0f );

        // Setup modelview matrix
        glMatrixMode( GL_MODELVIEW );
        glLoadIdentity();

        // Enable blending
        glEnable( GL_BLEND );

        // Dim background
        glBlendFunc( GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA );
        glColor4f( 0.3f, 0.3f, 0.3f, 0.3f );
        glBegin( GL_QUADS );
          glVertex2f( 0.0f, 0.0f );
          glVertex2f( 1.0f, 0.0f );
          glVertex2f( 1.0f, 1.0f );
          glVertex2f( 0.0f, 1.0f );
        glEnd();

        // Display winner text
        glBlendFunc( GL_ONE, GL_ONE_MINUS_SRC_COLOR );
        if( winner == PLAYER1 )
        {
            glColor4f( 1.0f, 0.5f, 0.5f, 1.0f );
            DrawImage( TEX_WINNER1, 0.35f, 0.65f, 0.46f, 0.54f );
        }
        else if( winner == PLAYER2 )
        {
            glColor4f( 0.5f, 1.0f, 0.5f, 1.0f );
            DrawImage( TEX_WINNER2, 0.35f, 0.65f, 0.46f, 0.54f );
        }

        // Disable blending
        glDisable( GL_BLEND );

        // Swap buffers
        glfwSwapBuffers();
    }

    // Disable sticky keys
    glfwDisable( GLFW_STICKY_KEYS );
}


//========================================================================
// GameLoop() - Game loop
//========================================================================

void GameLoop( void )
{
    int playing, event;

    // Initialize a new game
    NewGame();

    // Enable sticky keys
    glfwEnable( GLFW_STICKY_KEYS );

    // Loop until the game ends
    playing = GL_TRUE;
    while( playing && glfwGetWindowParam( GLFW_OPENED ) )
    {
        // Frame timer
        oldtime = thistime;
        thistime = glfwGetTime();
        dt = thistime - oldtime;

        // Get user input and update player positions
        PlayerControl();

        // Move the ball, and check if a player hits/misses the ball
        event = BallControl();

        // Did we have a winner?
        switch( event )
        {
        case PLAYER1_WINS:
            winner = PLAYER1;
            playing = GL_FALSE;
            break;
        case PLAYER2_WINS:
            winner = PLAYER2;
            playing = GL_FALSE;
            break;
        default:
            break;
        }

        // Did the user press ESC?
        if( glfwGetKey( GLFW_KEY_ESC ) )
        {
            playing = GL_FALSE;
        }

        // Did the user change camera view?
        if( glfwGetKey( '1' ) )
        {
            camerapos = CAMERA_CLASSIC;
        }
        else if( glfwGetKey( '2' ) )
        {
            camerapos = CAMERA_ABOVE;
        }
        else if( glfwGetKey( '3' ) )
        {
            camerapos = CAMERA_SPECTATOR;
        }

        // Draw display
        UpdateDisplay();

        // Swap buffers
        glfwSwapBuffers();
    }

    // Disable sticky keys
    glfwDisable( GLFW_STICKY_KEYS );

    // Show winner
    GameOver();
}


//========================================================================
// main() - Program entry point
//========================================================================

int main( void )
{
    int menuoption;

    // Initialize GLFW
    if( !glfwInit() )
    {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        exit( EXIT_FAILURE );
    }

    // Open OpenGL window
    if( !glfwOpenWindow( WIDTH, HEIGHT, 0,0,0,0, 16,0, GLFW_FULLSCREEN ) )
    {
        fprintf( stderr, "Failed to open GLFW window\n" );
        glfwTerminate();
        exit( EXIT_FAILURE );
    }

    glfwSwapInterval( 1 );

    // Load all textures
    if( !LoadTextures() )
    {
        glfwTerminate();
        exit( EXIT_FAILURE );
    }

    // Main loop
    do
    {
        // Get menu option
        menuoption = GameMenu();

        // If the user wants to play, let him...
        if( menuoption == MENU_PLAY )
        {
            GameLoop();
        }
    }
    while( menuoption != MENU_QUIT );

    // Unload all textures
    if( glfwGetWindowParam( GLFW_OPENED ) )
    {
        glDeleteTextures( NUM_TEXTURES, tex_id );
    }

    // Terminate GLFW
    glfwTerminate();

    exit( EXIT_SUCCESS );
}

