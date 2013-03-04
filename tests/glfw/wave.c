/*****************************************************************************
 * Wave Simulation in OpenGL
 * (C) 2002 Jakob Thomsen
 * http://home.in.tum.de/~thomsen
 * Modified for GLFW by Sylvain Hellegouarch - sh@programmationworld.com
 * Modified for variable frame rate by Marcus Geelnard
 * 2003-Jan-31: Minor cleanups and speedups / MG
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glfw.h>

#ifndef M_PI
 #define M_PI 3.1415926535897932384626433832795
#endif

/* Maximum delta T to allow for differential calculations */
#define MAX_DELTA_T 0.01

/* Animation speed (10.0 looks good) */
#define ANIMATION_SPEED 10.0


GLfloat alpha = 210.0f, beta = -70.0f;
GLfloat zoom = 2.0f;

int running = 1;

struct Vertex
{
  GLfloat x,y,z;
  GLfloat r,g,b;
};

#define GRIDW 50
#define GRIDH 50
#define VERTEXNUM (GRIDW*GRIDH)

#define QUADW (GRIDW-1)
#define QUADH (GRIDH-1)
#define QUADNUM (QUADW*QUADH)

GLuint quad[4*QUADNUM];
struct Vertex vertex[VERTEXNUM];

/* The grid will look like this:
 *
 *      3   4   5
 *      *---*---*
 *      |   |   |
 *      | 0 | 1 |
 *      |   |   |
 *      *---*---*
 *      0   1   2
 */

void initVertices( void )
{
  int x,y,p;

  /* place the vertices in a grid */
  for(y=0;y<GRIDH;y++)
    for(x=0;x<GRIDW;x++)
    {
      p = y*GRIDW + x;

      //vertex[p].x = (-GRIDW/2)+x+sin(2.0*M_PI*(double)y/(double)GRIDH);
      //vertex[p].y = (-GRIDH/2)+y+cos(2.0*M_PI*(double)x/(double)GRIDW);
      vertex[p].x = (GLfloat)(x-GRIDW/2)/(GLfloat)(GRIDW/2);
      vertex[p].y = (GLfloat)(y-GRIDH/2)/(GLfloat)(GRIDH/2);
      vertex[p].z = 0;//sin(d*M_PI);
      //vertex[p].r = (GLfloat)x/(GLfloat)GRIDW;
      //vertex[p].g = (GLfloat)y/(GLfloat)GRIDH;
      //vertex[p].b = 1.0-((GLfloat)x/(GLfloat)GRIDW+(GLfloat)y/(GLfloat)GRIDH)/2.0;
      if((x%4<2)^(y%4<2))
      {
        vertex[p].r = 0.0;
      }
      else
      {
        vertex[p].r=1.0;
      }

      vertex[p].g = (GLfloat)y/(GLfloat)GRIDH;
      vertex[p].b = 1.f-((GLfloat)x/(GLfloat)GRIDW+(GLfloat)y/(GLfloat)GRIDH)/2.f;
    }

  for(y=0;y<QUADH;y++)
    for(x=0;x<QUADW;x++)
    {
      p = 4*(y*QUADW + x);

      /* first quad */
      quad[p+0] = y    *GRIDW+x;    /* some point */
      quad[p+1] = y    *GRIDW+x+1;  /* neighbor at the right side */
      quad[p+2] = (y+1)*GRIDW+x+1;  /* upper right neighbor */
      quad[p+3] = (y+1)*GRIDW+x;    /* upper neighbor */
    }
}

double dt;
double p[GRIDW][GRIDH];
double vx[GRIDW][GRIDH], vy[GRIDW][GRIDH];
double ax[GRIDW][GRIDH], ay[GRIDW][GRIDH];



void initSurface( void )
{
  int    x, y;
  double dx, dy, d;

  for(y = 0; y<GRIDH; y++)
  {
    for(x = 0; x<GRIDW; x++)
    {
      dx = (double)(x-GRIDW/2);
      dy = (double)(y-GRIDH/2);
      d = sqrt( dx*dx + dy*dy );
      if(d < 0.1 * (double)(GRIDW/2))
      {
        d = d * 10.0;
        p[x][y] = -cos(d * (M_PI / (double)(GRIDW * 4))) * 100.0;
      }
      else
      {
        p[x][y] = 0.0;
      }
      vx[x][y] = 0.0;
      vy[x][y] = 0.0;
    }
  }
}


/* Draw view */
void draw_screen( void )
{
  /* Clear the color and depth buffers. */
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  /* We don't want to modify the projection matrix. */
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  /* Move back. */
  glTranslatef(0.0, 0.0, -zoom);
  /* Rotate the view */
  glRotatef(beta, 1.0, 0.0, 0.0);
  glRotatef(alpha, 0.0, 0.0, 1.0);

  //glDrawArrays(GL_POINTS,0,VERTEXNUM); /* Points only */
  glDrawElements(GL_QUADS, 4*QUADNUM, GL_UNSIGNED_INT, quad);
  //glDrawElements(GL_LINES, QUADNUM, GL_UNSIGNED_INT, quad);

  glfwSwapBuffers();
}


/* Initialize OpenGL */
void setup_opengl( void )
{
  /* Our shading model--Gouraud (smooth). */
  glShadeModel(GL_SMOOTH);

  /* Culling. */
  //glCullFace(GL_BACK);
  //glFrontFace(GL_CCW);
  //glEnable(GL_CULL_FACE);

  /* Switch on the z-buffer. */
  glEnable(GL_DEPTH_TEST);

  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_COLOR_ARRAY);
  glVertexPointer(3/*3 components per vertex (x,y,z)*/, GL_FLOAT, sizeof(struct Vertex), vertex);
  glColorPointer(3/*3 components per vertex (r,g,b)*/, GL_FLOAT, sizeof(struct Vertex), &vertex[0].r);  //Pointer to the first color
  glPointSize(2.0);

  /* Background color is black. */
  glClearColor(0, 0, 0, 0);
}


/* Modify the height of each vertex according to the pressure. */
void adjustGrid( void )
{
  int pos;
  int x, y;

  for(y = 0; y<GRIDH; y++)
  {
    for(x = 0; x<GRIDW; x++)
    {
      pos = y*GRIDW + x;
      vertex[pos].z = (float) (p[x][y]*(1.0/50.0));
    }
  }
}


/* Calculate wave propagation */
void calc( void )
{
  int    x, y, x2, y2;
  double time_step = dt * ANIMATION_SPEED;

  /* compute accelerations */
  for(x = 0; x < GRIDW; x++)
  {
    x2 = (x + 1) % GRIDW;
    for(y = 0; y < GRIDH; y++)
    {
      ax[x][y] = p[x][y] - p[x2][y];
    }
  }

  for(y = 0; y < GRIDH;y++)
  {
    y2 = (y + 1) % GRIDH;
    for(x = 0; x < GRIDW; x++)
    {
      ay[x][y] = p[x][y] - p[x][y2];
    }
  }

  /* compute speeds */
  for(x = 0; x < GRIDW; x++)
  {
    for(y = 0; y < GRIDH; y++)
    {
      vx[x][y] = vx[x][y] + ax[x][y] * time_step;
      vy[x][y] = vy[x][y] + ay[x][y] * time_step;
    }
  }

  /* compute pressure */
  for(x = 1; x < GRIDW; x++)
  {
    x2 = x - 1;
    for(y = 1; y < GRIDH; y++)
    {
      y2 = y - 1;
      p[x][y] = p[x][y] + (vx[x2][y] - vx[x][y] + vy[x][y2] - vy[x][y]) * time_step;
    }
  }
}


/* Handle key strokes */
void GLFWCALL handle_key_down(int key, int action)
{
  if( action != GLFW_PRESS )
  {
    return;
  }

  switch(key) {
    case GLFW_KEY_ESC:
      running = 0;
      break;
    case GLFW_KEY_SPACE:
      initSurface();
      break;
    case GLFW_KEY_LEFT:
      alpha+=5;
      break;
    case GLFW_KEY_RIGHT:
      alpha-=5;
      break;
    case GLFW_KEY_UP:
      beta-=5;
      break;
    case GLFW_KEY_DOWN:
      beta+=5;
      break;
    case GLFW_KEY_PAGEUP:
      if(zoom>1) zoom-=1;
      break;
    case GLFW_KEY_PAGEDOWN:
      zoom+=1;
      break;
    default:
      break;
  }
}


/* Callback function for window resize events */
void GLFWCALL handle_resize( int width, int height )
{
  float ratio = 1.0f;

  if( height > 0 )
  {
      ratio = (float) width / (float) height;
  }

  /* Setup viewport (Place where the stuff will appear in the main window). */
  glViewport(0, 0, width, height);

  /*
   * Change to the projection matrix and set
   * our viewing volume.
   */
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(60.0, ratio, 1.0, 1024.0);
}


/* Program entry point */
int main(int argc, char* argv[])
{
  /* Dimensions of our window. */
  int width, height;
  /* Style of our window. */
  int mode;
  /* Frame time */
  double t, t_old, dt_total;

  /* Initialize GLFW */
  if(glfwInit() == GL_FALSE)
  {
    fprintf(stderr, "GLFW initialization failed\n");
    exit(-1);
  }

  /* Desired window properties */
  width  = 640;
  height = 480;
  mode   = GLFW_WINDOW;

  /* Open window */
  if( glfwOpenWindow(width,height,0,0,0,0,16,0,mode) == GL_FALSE )
  {
    fprintf(stderr, "Could not open window\n");
    glfwTerminate();
    exit(-1);
  }

  /* Set title */
  glfwSetWindowTitle( "Wave Simulation" );

  glfwSwapInterval( 1 );

  /* Keyboard handler */
  glfwSetKeyCallback( handle_key_down );
  glfwEnable( GLFW_KEY_REPEAT );

  /* Window resize handler */
  glfwSetWindowSizeCallback( handle_resize );

  /* Initialize OpenGL */
  setup_opengl();

  /* Initialize simulation */
  initVertices();
  initSurface();
  adjustGrid();

  /* Initialize timer */
  t_old = glfwGetTime() - 0.01;

  /* Main loop */
  while(running)
  {
    /* Timing */
    t = glfwGetTime();
    dt_total = t - t_old;
    t_old = t;

    /* Safety - iterate if dt_total is too large */
    while( dt_total > 0.0f )
    {
        /* Select iteration time step */
        dt = dt_total > MAX_DELTA_T ? MAX_DELTA_T : dt_total;
        dt_total -= dt;

        /* Calculate wave propagation */
        calc();
    }

    /* Compute height of each vertex */
    adjustGrid();

    /* Draw wave grid to OpenGL display */
    draw_screen();

    /* Still running? */
    running = running && glfwGetWindowParam( GLFW_OPENED );
  }

  glfwTerminate();

  return 0;
}
