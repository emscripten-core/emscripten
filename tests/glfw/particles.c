//========================================================================
// This is a simple, but cool particle engine (buzz-word meaning many
// small objects that are treated as points and drawn as textures
// projected on simple geometry).
//
// This demonstration generates a colorful fountain-like animation. It
// uses several advanced OpenGL teqhniques:
//
//  1) Lighting (per vertex)
//  2) Alpha blending
//  3) Fog
//  4) Texturing
//  5) Display lists (for drawing the static environment geometry)
//  6) Vertex arrays (for drawing the particles)
//  7) GL_EXT_separate_specular_color is used (if available)
//
// Even more so, this program uses multi threading. The program is
// essentialy divided into a main rendering thread and a particle physics
// calculation thread. My benchmarks under Windows 2000 on a single
// processor system show that running this program as two threads instead
// of a single thread means no difference (there may be a very marginal
// advantage for the multi threaded case). On dual processor systems I
// have had reports of 5-25% of speed increase when running this program
// as two threads instead of one thread.
//
// The default behaviour of this program is to use two threads. To force
// a single thread to be used, use the command line switch -s.
//
// To run a fixed length benchmark (60 s), use the command line switch -b.
//
// Benchmark results (640x480x16, best of three tests):
//
//  CPU               GFX                   1 thread      2 threads
//  Athlon XP 2700+   GeForce Ti4200 (oc)    757 FPS        759 FPS
//  P4 2.8 GHz (SMT)  GeForce FX5600         548 FPS        550 FPS
//
// One more thing: Press 'w' during the demo to toggle wireframe mode.
//========================================================================

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <GL/glfw.h>

// Define tokens for GL_EXT_separate_specular_color if not already defined
#ifndef GL_EXT_separate_specular_color
#define GL_LIGHT_MODEL_COLOR_CONTROL_EXT  0x81F8
#define GL_SINGLE_COLOR_EXT               0x81F9
#define GL_SEPARATE_SPECULAR_COLOR_EXT    0x81FA
#endif // GL_EXT_separate_specular_color

// Some <math.h>'s do not define M_PI
#ifndef M_PI
#define M_PI 3.141592654
#endif

// Desired fullscreen resolution
#define WIDTH  640
#define HEIGHT 480


//========================================================================
// Type definitions
//========================================================================

typedef struct { float x,y,z; } VEC;

// This structure is used for interleaved vertex arrays (see the
// DrawParticles function) - Note: This structure SHOULD be packed on most
// systems. It uses 32-bit fields on 32-bit boundaries, and is a multiple
// of 64 bits in total (6x32=3x64). If it does not work, try using pragmas
// or whatever to force the structure to be packed.
typedef struct {
    GLfloat s, t;         // Texture coordinates
    GLuint  rgba;         // Color (four ubytes packed into an uint)
    GLfloat x, y, z;      // Vertex coordinates
} VERTEX;


//========================================================================
// Program control global variables
//========================================================================

// "Running" flag (true if program shall continue to run)
int running;

// Window dimensions
int width, height;

// "wireframe" flag (true if we use wireframe view)
int wireframe;

// "multithreading" flag (true if we use multithreading)
int multithreading;

// Thread synchronization
struct {
    double    t;         // Time (s)
    float     dt;        // Time since last frame (s)
    int       p_frame;   // Particle physics frame number
    int       d_frame;   // Particle draw frame number
    GLFWcond  p_done;    // Condition: particle physics done
    GLFWcond  d_done;    // Condition: particle draw done
    GLFWmutex particles_lock; // Particles data sharing mutex
} thread_sync;


//========================================================================
// Texture declarations (we hard-code them into the source code, since
// they are so simple)
//========================================================================

#define P_TEX_WIDTH  8    // Particle texture dimensions
#define P_TEX_HEIGHT 8
#define F_TEX_WIDTH  16   // Floor texture dimensions
#define F_TEX_HEIGHT 16

// Texture object IDs
GLuint particle_tex_id, floor_tex_id;

// Particle texture (a simple spot)
const unsigned char particle_texture[ P_TEX_WIDTH * P_TEX_HEIGHT ] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x11, 0x22, 0x22, 0x11, 0x00, 0x00,
    0x00, 0x11, 0x33, 0x88, 0x77, 0x33, 0x11, 0x00,
    0x00, 0x22, 0x88, 0xff, 0xee, 0x77, 0x22, 0x00,
    0x00, 0x22, 0x77, 0xee, 0xff, 0x88, 0x22, 0x00,
    0x00, 0x11, 0x33, 0x77, 0x88, 0x33, 0x11, 0x00,
    0x00, 0x00, 0x11, 0x33, 0x22, 0x11, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// Floor texture (your basic checkered floor)
const unsigned char floor_texture[ F_TEX_WIDTH * F_TEX_HEIGHT ] = {
    0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
    0xff, 0xf0, 0xcc, 0xf0, 0xf0, 0xf0, 0xff, 0xf0, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
    0xf0, 0xcc, 0xee, 0xff, 0xf0, 0xf0, 0xf0, 0xf0, 0x30, 0x66, 0x30, 0x30, 0x30, 0x20, 0x30, 0x30,
    0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xee, 0xf0, 0xf0, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
    0xf0, 0xf0, 0xf0, 0xf0, 0xcc, 0xf0, 0xf0, 0xf0, 0x30, 0x30, 0x55, 0x30, 0x30, 0x44, 0x30, 0x30,
    0xf0, 0xdd, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0x33, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
    0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xff, 0xf0, 0xf0, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x60, 0x30,
    0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0x33, 0x33, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
    0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x33, 0x30, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0,
    0x30, 0x30, 0x30, 0x30, 0x30, 0x20, 0x30, 0x30, 0xf0, 0xff, 0xf0, 0xf0, 0xdd, 0xf0, 0xf0, 0xff,
    0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x55, 0x33, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xff, 0xf0, 0xf0,
    0x30, 0x44, 0x66, 0x30, 0x30, 0x30, 0x30, 0x30, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0,
    0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0xf0, 0xf0, 0xf0, 0xaa, 0xf0, 0xf0, 0xcc, 0xf0,
    0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0xff, 0xf0, 0xf0, 0xf0, 0xff, 0xf0, 0xdd, 0xf0,
    0x30, 0x30, 0x30, 0x77, 0x30, 0x30, 0x30, 0x30, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0,
    0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0,
};


//========================================================================
// These are fixed constants that control the particle engine. In a
// modular world, these values should be variables...
//========================================================================

// Maximum number of particles
#define MAX_PARTICLES   3000

// Life span of a particle (in seconds)
#define LIFE_SPAN       8.0f

// A new particle is born every [BIRTH_INTERVAL] second
#define BIRTH_INTERVAL (LIFE_SPAN/(float)MAX_PARTICLES)

// Particle size (meters)
#define PARTICLE_SIZE   0.7f

// Gravitational constant (m/s^2)
#define GRAVITY         9.8f

// Base initial velocity (m/s)
#define VELOCITY        8.0f

// Bounce friction (1.0 = no friction, 0.0 = maximum friction)
#define FRICTION        0.75f

// "Fountain" height (m)
#define FOUNTAIN_HEIGHT 3.0f

// Fountain radius (m)
#define FOUNTAIN_RADIUS 1.6f

// Minimum delta-time for particle phisics (s)
#define MIN_DELTA_T     (BIRTH_INTERVAL * 0.5f)


//========================================================================
// Particle system global variables
//========================================================================

// This structure holds all state for a single particle
typedef struct {
    float x,y,z;     // Position in space
    float vx,vy,vz;  // Velocity vector
    float r,g,b;     // Color of particle
    float life;      // Life of particle (1.0 = newborn, < 0.0 = dead)
    int   active;    // Tells if this particle is active
} PARTICLE;

// Global vectors holding all particles. We use two vectors for double
// buffering.
static PARTICLE particles[ MAX_PARTICLES ];

// Global variable holding the age of the youngest particle
static float min_age;

// Color of latest born particle (used for fountain lighting)
static float glow_color[4];

// Position of latest born particle (used for fountain lighting)
static float glow_pos[4];


//========================================================================
// Object material and fog configuration constants
//========================================================================

const GLfloat fountain_diffuse[4]  = {0.7f,1.0f,1.0f,1.0f};
const GLfloat fountain_specular[4] = {1.0f,1.0f,1.0f,1.0f};
const GLfloat fountain_shininess   = 12.0f;
const GLfloat floor_diffuse[4]     = {1.0f,0.6f,0.6f,1.0f};
const GLfloat floor_specular[4]    = {0.6f,0.6f,0.6f,1.0f};
const GLfloat floor_shininess      = 18.0f;
const GLfloat fog_color[4] = {0.1f, 0.1f, 0.1f, 1.0f};


//========================================================================
// InitParticle() - Initialize a new particle
//========================================================================

void InitParticle( PARTICLE *p, double t )
{
    float   xy_angle, velocity;

    // Start position of particle is at the fountain blow-out
    p->x = 0.0f;
    p->y = 0.0f;
    p->z = FOUNTAIN_HEIGHT;

    // Start velocity is up (Z)...
    p->vz = 0.7f + (0.3f/4096.f) * (float) (rand() & 4095);

    // ...and a randomly chosen X/Y direction
    xy_angle = (2.f * (float)M_PI / 4096.f) * (float) (rand() & 4095);
    p->vx = 0.4f * (float) cos( xy_angle );
    p->vy = 0.4f * (float) sin( xy_angle );

    // Scale velocity vector according to a time-varying velocity
    velocity = VELOCITY*(0.8f + 0.1f*(float)(sin( 0.5*t )+sin( 1.31*t )));
    p->vx *= velocity;
    p->vy *= velocity;
    p->vz *= velocity;

    // Color is time-varying
    p->r = 0.7f + 0.3f * (float) sin( 0.34*t + 0.1 );
    p->g = 0.6f + 0.4f * (float) sin( 0.63*t + 1.1 );
    p->b = 0.6f + 0.4f * (float) sin( 0.91*t + 2.1 );

    // Store settings for fountain glow lighting
    glow_pos[0] = 0.4f * (float) sin( 1.34*t );
    glow_pos[1] = 0.4f * (float) sin( 3.11*t );
    glow_pos[2] = FOUNTAIN_HEIGHT + 1.0f;
    glow_pos[3] = 1.0f;
    glow_color[0] = p->r;
    glow_color[1] = p->g;
    glow_color[2] = p->b;
    glow_color[3] = 1.0f;

    // The particle is new-born and active
    p->life = 1.0f;
    p->active = 1;
}


//========================================================================
// UpdateParticle() - Update a particle
//========================================================================

#define FOUNTAIN_R2 (FOUNTAIN_RADIUS+PARTICLE_SIZE/2)*(FOUNTAIN_RADIUS+PARTICLE_SIZE/2)

void UpdateParticle( PARTICLE *p, float dt )
{
    // If the particle is not active, we need not do anything
    if( !p->active )
    {
        return;
    }

    // The particle is getting older...
    p->life = p->life - dt * (1.0f / LIFE_SPAN);

    // Did the particle die?
    if( p->life <= 0.0f )
    {
        p->active = 0;
        return;
    }

    // Update particle velocity (apply gravity)
    p->vz = p->vz - GRAVITY * dt;

    // Update particle position
    p->x = p->x + p->vx * dt;
    p->y = p->y + p->vy * dt;
    p->z = p->z + p->vz * dt;

    // Simple collision detection + response
    if( p->vz < 0.0f )
    {
        // Particles should bounce on the fountain (with friction)
        if( (p->x*p->x + p->y*p->y) < FOUNTAIN_R2 &&
            p->z < (FOUNTAIN_HEIGHT + PARTICLE_SIZE/2) )
        {
            p->vz = -FRICTION * p->vz;
            p->z  = FOUNTAIN_HEIGHT + PARTICLE_SIZE/2 +
                        FRICTION * (FOUNTAIN_HEIGHT +
                        PARTICLE_SIZE/2 - p->z);
        }

        // Particles should bounce on the floor (with friction)
        else if( p->z < PARTICLE_SIZE/2 )
        {
            p->vz = -FRICTION * p->vz;
            p->z  = PARTICLE_SIZE/2 +
                        FRICTION * (PARTICLE_SIZE/2 - p->z);
        }

    }
}


//========================================================================
// ParticleEngine() - The main frame for the particle engine. Called once
// per frame.
//========================================================================

void ParticleEngine( double t, float dt )
{
    int      i;
    float    dt2;

    // Update particles (iterated several times per frame if dt is too
    // large)
    while( dt > 0.0f )
    {
        // Calculate delta time for this iteration
        dt2 = dt < MIN_DELTA_T ? dt : MIN_DELTA_T;

        // Update particles
        for( i = 0; i < MAX_PARTICLES; i ++ )
        {
            UpdateParticle( &particles[ i ], dt2 );
        }

        // Increase minimum age
        min_age += dt2;

        // Should we create any new particle(s)?
        while( min_age >= BIRTH_INTERVAL )
        {
            min_age -= BIRTH_INTERVAL;

            // Find a dead particle to replace with a new one
            for( i = 0; i < MAX_PARTICLES; i ++ )
            {
                if( !particles[ i ].active )
                {
                    InitParticle( &particles[ i ], t + min_age );
                    UpdateParticle( &particles[ i ], min_age );
                    break;
                }
            }
        }

        // Decrease frame delta time
        dt -= dt2;
    }
}


//========================================================================
// DrawParticles() - Draw all active particles. We use OpenGL 1.1 vertex
// arrays for this in order to accelerate the drawing.
//========================================================================

#define BATCH_PARTICLES 70  // Number of particles to draw in each batch
                            // (70 corresponds to 7.5 KB = will not blow
                            // the L1 data cache on most CPUs)
#define PARTICLE_VERTS  4   // Number of vertices per particle

void DrawParticles( double t, float dt )
{
    int       i, particle_count;
    VERTEX    vertex_array[ BATCH_PARTICLES * PARTICLE_VERTS ], *vptr;
    float     alpha;
    GLuint    rgba;
    VEC       quad_lower_left, quad_lower_right;
    GLfloat   mat[ 16 ];
    PARTICLE  *pptr;

    // Here comes the real trick with flat single primitive objects (s.c.
    // "billboards"): We must rotate the textured primitive so that it
    // always faces the viewer (is coplanar with the view-plane).
    // We:
    //   1) Create the primitive around origo (0,0,0)
    //   2) Rotate it so that it is coplanar with the view plane
    //   3) Translate it according to the particle position
    // Note that 1) and 2) is the same for all particles (done only once).

    // Get modelview matrix. We will only use the upper left 3x3 part of
    // the matrix, which represents the rotation.
    glGetFloatv( GL_MODELVIEW_MATRIX, mat );

    // 1) & 2) We do it in one swift step:
    // Although not obvious, the following six lines represent two matrix/
    // vector multiplications. The matrix is the inverse 3x3 rotation
    // matrix (i.e. the transpose of the same matrix), and the two vectors
    // represent the lower left corner of the quad, PARTICLE_SIZE/2 *
    // (-1,-1,0), and the lower right corner, PARTICLE_SIZE/2 * (1,-1,0).
    // The upper left/right corners of the quad is always the negative of
    // the opposite corners (regardless of rotation).
    quad_lower_left.x = (-PARTICLE_SIZE/2) * (mat[0] + mat[1]);
    quad_lower_left.y = (-PARTICLE_SIZE/2) * (mat[4] + mat[5]);
    quad_lower_left.z = (-PARTICLE_SIZE/2) * (mat[8] + mat[9]);
    quad_lower_right.x = (PARTICLE_SIZE/2) * (mat[0] - mat[1]);
    quad_lower_right.y = (PARTICLE_SIZE/2) * (mat[4] - mat[5]);
    quad_lower_right.z = (PARTICLE_SIZE/2) * (mat[8] - mat[9]);

    // Don't update z-buffer, since all particles are transparent!
    glDepthMask( GL_FALSE );

    // Enable blending
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE );

    // Select particle texture
    if( !wireframe )
    {
        glEnable( GL_TEXTURE_2D );
        glBindTexture( GL_TEXTURE_2D, particle_tex_id );
    }

    // Set up vertex arrays. We use interleaved arrays, which is easier to
    // handle (in most situations) and it gives a linear memeory access
    // access pattern (which may give better performance in some
    // situations). GL_T2F_C4UB_V3F means: 2 floats for texture coords,
    // 4 ubytes for color and 3 floats for vertex coord (in that order).
    // Most OpenGL cards / drivers are optimized for this format.
    glInterleavedArrays( GL_T2F_C4UB_V3F, 0, vertex_array );

    // Is particle physics carried out in a separate thread?
    if( multithreading )
    {
        // Wait for particle physics thread to be done
        glfwLockMutex( thread_sync.particles_lock );
        while( running && thread_sync.p_frame <= thread_sync.d_frame )
        {
            glfwWaitCond( thread_sync.p_done, thread_sync.particles_lock,
                          0.1 );
        }

        // Store the frame time and delta time for the physics thread
        thread_sync.t  = t;
        thread_sync.dt = dt;

        // Update frame counter
        thread_sync.d_frame ++;
    }
    else
    {
        // Perform particle physics in this thread
        ParticleEngine( t, dt );
    }

    // Loop through all particles and build vertex arrays.
    particle_count = 0;
    vptr = vertex_array;
    pptr = particles;
    for( i = 0; i < MAX_PARTICLES; i ++ )
    {
        if( pptr->active )
        {
            // Calculate particle intensity (we set it to max during 75%
            // of its life, then it fades out)
            alpha =  4.0f * pptr->life;
            if( alpha > 1.0f )
            {
                alpha = 1.0f;
            }

            // Convert color from float to 8-bit (store it in a 32-bit
            // integer using endian independent type casting)
            ((GLubyte *)&rgba)[0] = (GLubyte)(pptr->r * 255.0f);
            ((GLubyte *)&rgba)[1] = (GLubyte)(pptr->g * 255.0f);
            ((GLubyte *)&rgba)[2] = (GLubyte)(pptr->b * 255.0f);
            ((GLubyte *)&rgba)[3] = (GLubyte)(alpha * 255.0f);

            // 3) Translate the quad to the correct position in modelview
            // space and store its parameters in vertex arrays (we also
            // store texture coord and color information for each vertex).

            // Lower left corner
            vptr->s    = 0.0f;
            vptr->t    = 0.0f;
            vptr->rgba = rgba;
            vptr->x    = pptr->x + quad_lower_left.x;
            vptr->y    = pptr->y + quad_lower_left.y;
            vptr->z    = pptr->z + quad_lower_left.z;
            vptr ++;

            // Lower right corner
            vptr->s    = 1.0f;
            vptr->t    = 0.0f;
            vptr->rgba = rgba;
            vptr->x    = pptr->x + quad_lower_right.x;
            vptr->y    = pptr->y + quad_lower_right.y;
            vptr->z    = pptr->z + quad_lower_right.z;
            vptr ++;

            // Upper right corner
            vptr->s    = 1.0f;
            vptr->t    = 1.0f;
            vptr->rgba = rgba;
            vptr->x    = pptr->x - quad_lower_left.x;
            vptr->y    = pptr->y - quad_lower_left.y;
            vptr->z    = pptr->z - quad_lower_left.z;
            vptr ++;

            // Upper left corner
            vptr->s    = 0.0f;
            vptr->t    = 1.0f;
            vptr->rgba = rgba;
            vptr->x    = pptr->x - quad_lower_right.x;
            vptr->y    = pptr->y - quad_lower_right.y;
            vptr->z    = pptr->z - quad_lower_right.z;
            vptr ++;

            // Increase count of drawable particles
            particle_count ++;
        }

        // If we have filled up one batch of particles, draw it as a set
        // of quads using glDrawArrays.
        if( particle_count >= BATCH_PARTICLES )
        {
            // The first argument tells which primitive type we use (QUAD)
            // The second argument tells the index of the first vertex (0)
            // The last argument is the vertex count
            glDrawArrays( GL_QUADS, 0, PARTICLE_VERTS * particle_count );
            particle_count = 0;
            vptr = vertex_array;
        }

        // Next particle
        pptr ++;
    }

    // We are done with the particle data: Unlock mutex and signal physics
    // thread
    if( multithreading )
    {
        glfwUnlockMutex( thread_sync.particles_lock );
        glfwSignalCond( thread_sync.d_done );
    }

    // Draw final batch of particles (if any)
    glDrawArrays( GL_QUADS, 0, PARTICLE_VERTS * particle_count );

    // Disable vertex arrays (Note: glInterleavedArrays implicitly called
    // glEnableClientState for vertex, texture coord and color arrays)
    glDisableClientState( GL_VERTEX_ARRAY );
    glDisableClientState( GL_TEXTURE_COORD_ARRAY );
    glDisableClientState( GL_COLOR_ARRAY );

    // Disable texturing and blending
    glDisable( GL_TEXTURE_2D );
    glDisable( GL_BLEND );

    // Allow Z-buffer updates again
    glDepthMask( GL_TRUE );
}


//========================================================================
// Fountain geometry specification
//========================================================================

#define FOUNTAIN_SIDE_POINTS 14
#define FOUNTAIN_SWEEP_STEPS 32

static const float fountain_side[ FOUNTAIN_SIDE_POINTS*2 ] = {
    1.2f, 0.0f,  1.0f, 0.2f,  0.41f, 0.3f, 0.4f, 0.35f,
    0.4f, 1.95f, 0.41f, 2.0f, 0.8f, 2.2f,  1.2f, 2.4f,
    1.5f, 2.7f,  1.55f,2.95f, 1.6f, 3.0f,  1.0f, 3.0f,
    0.5f, 3.0f,  0.0f, 3.0f
};

static const float fountain_normal[ FOUNTAIN_SIDE_POINTS*2 ] = {
    1.0000f, 0.0000f,  0.6428f, 0.7660f,  0.3420f, 0.9397f,  1.0000f, 0.0000f,
    1.0000f, 0.0000f,  0.3420f,-0.9397f,  0.4226f,-0.9063f,  0.5000f,-0.8660f,
    0.7660f,-0.6428f,  0.9063f,-0.4226f,  0.0000f,1.00000f,  0.0000f,1.00000f,
    0.0000f,1.00000f,  0.0000f,1.00000f
};


//========================================================================
// DrawFountain() - Draw a fountain
//========================================================================

void DrawFountain( void )
{
    static GLuint fountain_list = 0;
    double angle;
    float  x, y;
    int m, n;

    // The first time, we build the fountain display list
    if( !fountain_list )
    {
        // Start recording of a new display list
        fountain_list = glGenLists( 1 );
        glNewList( fountain_list, GL_COMPILE_AND_EXECUTE );

        // Set fountain material
        glMaterialfv( GL_FRONT, GL_DIFFUSE,   fountain_diffuse );
        glMaterialfv( GL_FRONT, GL_SPECULAR,  fountain_specular );
        glMaterialf(  GL_FRONT, GL_SHININESS, fountain_shininess );

        // Build fountain using triangle strips
        for( n = 0; n < FOUNTAIN_SIDE_POINTS-1; n ++ )
        {
            glBegin( GL_TRIANGLE_STRIP );
            for( m = 0; m <= FOUNTAIN_SWEEP_STEPS; m ++ )
            {
                angle = (double) m * (2.0*M_PI/(double)FOUNTAIN_SWEEP_STEPS);
                x = (float) cos( angle );
                y = (float) sin( angle );

                // Draw triangle strip
                glNormal3f( x * fountain_normal[ n*2+2 ],
                            y * fountain_normal[ n*2+2 ],
                            fountain_normal[ n*2+3 ] );
                glVertex3f( x * fountain_side[ n*2+2 ],
                            y * fountain_side[ n*2+2 ],
                            fountain_side[ n*2+3 ] );
                glNormal3f( x * fountain_normal[ n*2 ],
                            y * fountain_normal[ n*2 ],
                            fountain_normal[ n*2+1 ] );
                glVertex3f( x * fountain_side[ n*2 ],
                            y * fountain_side[ n*2 ],
                            fountain_side[ n*2+1 ] );
            }
            glEnd();
        }

        // End recording of display list
        glEndList();
    }
    else
    {
        // Playback display list
        glCallList( fountain_list );
    }
}


//========================================================================
// TesselateFloor() - Recursive function for building variable tesselated
// floor
//========================================================================

void TesselateFloor( float x1, float y1, float x2, float y2,
    int recursion )
{
    float delta, x, y;

    // Last recursion?
    if( recursion >= 5 )
    {
        delta = 999999.0f;
    }
    else
    {
        x = (float) (fabs(x1) < fabs(x2) ? fabs(x1) : fabs(x2));
        y = (float) (fabs(y1) < fabs(y2) ? fabs(y1) : fabs(y2));
        delta = x*x + y*y;
    }

    // Recurse further?
    if( delta < 0.1f )
    {
        x = (x1+x2) * 0.5f;
        y = (y1+y2) * 0.5f;
        TesselateFloor( x1,y1,  x, y, recursion + 1 );
        TesselateFloor(  x,y1, x2, y, recursion + 1 );
        TesselateFloor( x1, y,  x,y2, recursion + 1 );
        TesselateFloor(  x, y, x2,y2, recursion + 1 );
    }
    else
    {
        glTexCoord2f( x1*30.0f, y1*30.0f );
        glVertex3f( x1*80.0f, y1*80.0f , 0.0f );
        glTexCoord2f( x2*30.0f, y1*30.0f );
        glVertex3f( x2*80.0f, y1*80.0f , 0.0f );
        glTexCoord2f( x2*30.0f, y2*30.0f );
        glVertex3f( x2*80.0f, y2*80.0f , 0.0f );
        glTexCoord2f( x1*30.0f, y2*30.0f );
        glVertex3f( x1*80.0f, y2*80.0f , 0.0f );
    }
}


//========================================================================
// DrawFloor() - Draw floor. We builde the floor recursively, and let the
// tesselation in the centre (near x,y=0,0) be high, while the selleation
// around the edges be low.
//========================================================================

void DrawFloor( void )
{
    static GLuint floor_list = 0;

    // Select floor texture
    if( !wireframe )
    {
        glEnable( GL_TEXTURE_2D );
        glBindTexture( GL_TEXTURE_2D, floor_tex_id );
    }

    // The first time, we build the floor display list
    if( !floor_list )
    {
        // Start recording of a new display list
        floor_list = glGenLists( 1 );
        glNewList( floor_list, GL_COMPILE_AND_EXECUTE );

        // Set floor material
        glMaterialfv( GL_FRONT, GL_DIFFUSE, floor_diffuse );
        glMaterialfv( GL_FRONT, GL_SPECULAR, floor_specular );
        glMaterialf(  GL_FRONT, GL_SHININESS, floor_shininess );

        // Draw floor as a bunch of triangle strips (high tesselation
        // improves lighting)
        glNormal3f( 0.0f, 0.0f, 1.0f );
        glBegin( GL_QUADS );
        TesselateFloor( -1.0f,-1.0f, 0.0f,0.0f, 0 );
        TesselateFloor(  0.0f,-1.0f, 1.0f,0.0f, 0 );
        TesselateFloor(  0.0f, 0.0f, 1.0f,1.0f, 0 );
        TesselateFloor( -1.0f, 0.0f, 0.0f,1.0f, 0 );
        glEnd();

        // End recording of display list
        glEndList();
    }
    else
    {
        // Playback display list
        glCallList( floor_list );
    }

    glDisable( GL_TEXTURE_2D );

}


//========================================================================
// SetupLights() - Position and configure light sources
//========================================================================

void SetupLights( void )
{
    float l1pos[4], l1amb[4], l1dif[4], l1spec[4];
    float l2pos[4], l2amb[4], l2dif[4], l2spec[4];

    // Set light source 1 parameters
    l1pos[0] = 0.0f; l1pos[1] = -9.0f; l1pos[2] = 8.0f; l1pos[3] = 1.0f;
    l1amb[0] = 0.2f; l1amb[1] = 0.2f; l1amb[2] = 0.2f; l1amb[3] = 1.0f;
    l1dif[0] = 0.8f; l1dif[1] = 0.4f; l1dif[2] = 0.2f; l1dif[3] = 1.0f;
    l1spec[0] = 1.0f; l1spec[1] = 0.6f; l1spec[2] = 0.2f; l1spec[3] = 0.0f;

    // Set light source 2 parameters
    l2pos[0] = -15.0f; l2pos[1] = 12.0f; l2pos[2] = 1.5f; l2pos[3] = 1.0f;
    l2amb[0] =   0.0f; l2amb[1] =  0.0f; l2amb[2] = 0.0f; l2amb[3] = 1.0f;
    l2dif[0] =   0.2f; l2dif[1] =  0.4f; l2dif[2] = 0.8f; l2dif[3] = 1.0f;
    l2spec[0] = 0.2f; l2spec[1] = 0.6f; l2spec[2] = 1.0f; l2spec[3] = 0.0f;

    // Configure light sources in OpenGL
    glLightfv( GL_LIGHT1, GL_POSITION, l1pos );
    glLightfv( GL_LIGHT1, GL_AMBIENT, l1amb );
    glLightfv( GL_LIGHT1, GL_DIFFUSE, l1dif );
    glLightfv( GL_LIGHT1, GL_SPECULAR, l1spec );
    glLightfv( GL_LIGHT2, GL_POSITION, l2pos );
    glLightfv( GL_LIGHT2, GL_AMBIENT, l2amb );
    glLightfv( GL_LIGHT2, GL_DIFFUSE, l2dif );
    glLightfv( GL_LIGHT2, GL_SPECULAR, l2spec );
    glLightfv( GL_LIGHT3, GL_POSITION, glow_pos );
    glLightfv( GL_LIGHT3, GL_DIFFUSE, glow_color );
    glLightfv( GL_LIGHT3, GL_SPECULAR, glow_color );

    // Enable light sources
    glEnable( GL_LIGHT1 );
    glEnable( GL_LIGHT2 );
    glEnable( GL_LIGHT3 );
}


//========================================================================
// Draw() - Main rendering function
//========================================================================

void Draw( double t )
{
    double xpos, ypos, zpos, angle_x, angle_y, angle_z;
    static double t_old = 0.0;
    float  dt;

    // Calculate frame-to-frame delta time
    dt = (float)(t-t_old);
    t_old = t;

    // Setup viewport
    glViewport( 0, 0, width, height );

    // Clear color and Z-buffer
    glClearColor( 0.1f, 0.1f, 0.1f, 1.0f );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    // Setup projection
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    gluPerspective( 65.0, (double)width/(double)height, 1.0, 60.0 );

    // Setup camera
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    // Rotate camera
    angle_x = 90.0 - 10.0;
    angle_y = 10.0 * sin( 0.3 * t );
    angle_z = 10.0 * t;
    glRotated( -angle_x, 1.0, 0.0, 0.0 );
    glRotated( -angle_y, 0.0, 1.0, 0.0 );
    glRotated( -angle_z, 0.0, 0.0, 1.0 );

    // Translate camera
    xpos =  15.0 * sin( (M_PI/180.0) * angle_z ) +
             2.0 * sin( (M_PI/180.0) * 3.1 * t );
    ypos = -15.0 * cos( (M_PI/180.0) * angle_z ) +
             2.0 * cos( (M_PI/180.0) * 2.9 * t );
    zpos = 4.0 + 2.0 * cos( (M_PI/180.0) * 4.9 * t );
    glTranslated( -xpos, -ypos, -zpos );

    // Enable face culling
    glFrontFace( GL_CCW );
    glCullFace( GL_BACK );
    glEnable( GL_CULL_FACE );

    // Enable lighting
    SetupLights();
    glEnable( GL_LIGHTING );

    // Enable fog (dim details far away)
    glEnable( GL_FOG );
    glFogi( GL_FOG_MODE, GL_EXP );
    glFogf( GL_FOG_DENSITY, 0.05f );
    glFogfv( GL_FOG_COLOR, fog_color );

    // Draw floor
    DrawFloor();

    // Enable Z-buffering
    glEnable( GL_DEPTH_TEST );
    glDepthFunc( GL_LEQUAL );
    glDepthMask( GL_TRUE );

    // Draw fountain
    DrawFountain();

    // Disable fog & lighting
    glDisable( GL_LIGHTING );
    glDisable( GL_FOG );

    // Draw all particles (must be drawn after all solid objects have been
    // drawn!)
    DrawParticles( t, dt );

    // Z-buffer not needed anymore
    glDisable( GL_DEPTH_TEST );
}


//========================================================================
// Resize() - GLFW window resize callback function
//========================================================================

void GLFWCALL Resize( int x, int y )
{
    width = x;
    height = y > 0 ? y : 1;   // Prevent division by zero in aspect calc.
}


//========================================================================
// Input callback functions
//========================================================================

void GLFWCALL KeyFun( int key, int action )
{
    if( action == GLFW_PRESS )
    {
        switch( key )
        {
        case GLFW_KEY_ESC:
            running = 0;
            break;
        case 'W':
            wireframe = !wireframe;
            glPolygonMode( GL_FRONT_AND_BACK,
                           wireframe ? GL_LINE : GL_FILL );
            break;
        default:
            break;
        }
    }
}


//========================================================================
// PhysicsThreadFun() - Thread for updating particle physics
//========================================================================

void GLFWCALL PhysicsThreadFun( void *arg )
{
    while( running )
    {
        // Lock mutex
        glfwLockMutex( thread_sync.particles_lock );

        // Wait for particle drawing to be done
        while( running && thread_sync.p_frame > thread_sync.d_frame )
        {
            glfwWaitCond( thread_sync.d_done, thread_sync.particles_lock,
                          0.1 );
        }

        // No longer running?
        if( !running )
        {
            break;
        }

        // Update particles
        ParticleEngine( thread_sync.t, thread_sync.dt );

        // Update frame counter
        thread_sync.p_frame ++;

        // Unlock mutex and signal drawing thread
        glfwUnlockMutex( thread_sync.particles_lock );
        glfwSignalCond( thread_sync.p_done );
    }
}


//========================================================================
// main()
//========================================================================

int main( int argc, char **argv )
{
    int        i, frames, benchmark;
    double     t0, t;
    GLFWthread physics_thread = 0;

    // Use multithreading by default, but don't benchmark
    multithreading = 1;
    benchmark = 0;

    // Check command line arguments
    for( i = 1; i < argc; i ++ )
    {
        // Use benchmarking?
        if( strcmp( argv[i], "-b" ) == 0 )
        {
            benchmark = 1;
        }

        // Force multithreading off?
        else if( strcmp( argv[i], "-s" ) == 0 )
        {
            multithreading = 0;
        }

        // With a Finder launch on Mac OS X we get a bogus -psn_0_46268417
        // kind of argument (actual numbers vary). Ignore it.
        else if( strncmp( argv[i], "-psn_", 5) == 0 );

        // Usage
        else
        {
            if( strcmp( argv[i], "-?" ) != 0 )
            {
                printf( "Unknonwn option %s\n\n", argv[ i ] );
            }
            printf( "Usage: %s [options]\n", argv[ 0 ] );
            printf( "\n");
            printf( "Options:\n" );
            printf( " -b   Benchmark (run program for 60 s)\n" );
            printf( " -s   Run program as single thread (default is to use two threads)\n" );
            printf( " -?   Display this text\n" );
            printf( "\n");
            printf( "Program runtime controls:\n" );
            printf( " w    Toggle wireframe mode\n" );
            printf( " ESC  Exit program\n" );
            exit( 0 );
        }
    }

    // Initialize GLFW
    if( !glfwInit() )
    {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        exit( EXIT_FAILURE );
    }

    // Open OpenGL fullscreen window
    if( !glfwOpenWindow( WIDTH, HEIGHT, 0,0,0,0, 16,0, GLFW_FULLSCREEN ) )
    {
        fprintf( stderr, "Failed to open GLFW window\n" );
        glfwTerminate();
        exit( EXIT_FAILURE );
    }

    // Set window title
    glfwSetWindowTitle( "Particle engine" );

    // Disable VSync (we want to get as high FPS as possible!)
    glfwSwapInterval( 0 );

    // Window resize callback function
    glfwSetWindowSizeCallback( Resize );

    // Set keyboard input callback function
    glfwSetKeyCallback( KeyFun );

    // Upload particle texture
    glGenTextures( 1, &particle_tex_id );
    glBindTexture( GL_TEXTURE_2D, particle_tex_id );
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_LUMINANCE, P_TEX_WIDTH, P_TEX_HEIGHT,
                  0, GL_LUMINANCE, GL_UNSIGNED_BYTE, particle_texture );

    // Upload floor texture
    glGenTextures( 1, &floor_tex_id );
    glBindTexture( GL_TEXTURE_2D, floor_tex_id );
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_LUMINANCE, F_TEX_WIDTH, F_TEX_HEIGHT,
                  0, GL_LUMINANCE, GL_UNSIGNED_BYTE, floor_texture );

    // Check if we have GL_EXT_separate_specular_color, and if so use it
    if( glfwExtensionSupported( "GL_EXT_separate_specular_color" ) )
    {
        glLightModeli( GL_LIGHT_MODEL_COLOR_CONTROL_EXT,
                       GL_SEPARATE_SPECULAR_COLOR_EXT );
    }

    // Set filled polygon mode as default (not wireframe)
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    wireframe = 0;

    // Clear particle system
    for( i = 0; i < MAX_PARTICLES; i ++ )
    {
        particles[ i ].active = 0;
    }
    min_age = 0.0f;

    // Set "running" flag
    running = 1;

    // Set initial times
    thread_sync.t  = 0.0;
    thread_sync.dt = 0.001f;

    // Init threading
    if( multithreading )
    {
        thread_sync.p_frame = 0;
        thread_sync.d_frame = 0;
        thread_sync.particles_lock = glfwCreateMutex();
        thread_sync.p_done = glfwCreateCond();
        thread_sync.d_done = glfwCreateCond();
        physics_thread = glfwCreateThread( PhysicsThreadFun, NULL );
    }

    // Main loop
    t0 = glfwGetTime();
    frames = 0;
    while( running )
    {
        // Get frame time
        t = glfwGetTime() - t0;

        // Draw...
        Draw( t );

        // Swap buffers
        glfwSwapBuffers();

        // Check if window was closed
        running = running && glfwGetWindowParam( GLFW_OPENED );

        // Increase frame count
        frames ++;

        // End of benchmark?
        if( benchmark && t >= 60.0 )
        {
            running = 0;
        }
    }
    t = glfwGetTime() - t0;

    // Wait for particle physics thread to die
    if( multithreading )
    {
        glfwWaitThread( physics_thread, GLFW_WAIT );
    }

    // Display profiling information
    printf( "%d frames in %.2f seconds = %.1f FPS", frames, t,
            (double)frames / t );
    printf( " (multithreading %s)\n", multithreading ? "on" : "off" );

    // Terminate OpenGL
    glfwTerminate();

    exit( EXIT_SUCCESS );
}

