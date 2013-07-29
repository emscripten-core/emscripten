//
// File:       qjulia.c
//
// Abstract:   This example shows how to use OpenCL to raytrace a 4d Quaternion Julia-Set 
//             Fractal and intermix the results of a compute kernel with OpenGL for rendering.
//
//             For theory and information regarding 4d quaternion julia-sets consult the following:
//
//             http://local.wasp.uwa.edu.au/~pbourke/fractals/quatjulia/
//             http://www.omegafield.net/library/dynamical/quaternion_julia_sets.pdf
//             http://www.evl.uic.edu/files/pdf/Sandin.RayTracerJuliaSetsbw.pdf
//             http://www.cs.caltech.edu/~keenan/project_qjulia.html
//
// Version:    <1.0>
//
// Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple Inc. ("Apple")
//             in consideration of your agreement to the following terms, and your use,
//             installation, modification or redistribution of this Apple software
//             constitutes acceptance of these terms.  If you do not agree with these
//             terms, please do not use, install, modify or redistribute this Apple
//             software.
//
//             In consideration of your agreement to abide by the following terms, and
//             subject to these terms, Apple grants you a personal, non - exclusive
//             license, under Apple's copyrights in this original Apple software ( the
//             "Apple Software" ), to use, reproduce, modify and redistribute the Apple
//             Software, with or without modifications, in source and / or binary forms;
//             provided that if you redistribute the Apple Software in its entirety and
//             without modifications, you must retain this notice and the following text
//             and disclaimers in all such redistributions of the Apple Software. Neither
//             the name, trademarks, service marks or logos of Apple Inc. may be used to
//             endorse or promote products derived from the Apple Software without specific
//             prior written permission from Apple.  Except as expressly stated in this
//             notice, no other rights or licenses, express or implied, are granted by
//             Apple herein, including but not limited to any patent rights that may be
//             infringed by your derivative works or by other works in which the Apple
//             Software may be incorporated.
//
//             The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
//             WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
//             WARRANTIES OF NON - INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A
//             PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION
//             ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
//
//             IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
//             CONSEQUENTIAL DAMAGES ( INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
//             SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
//             INTERRUPTION ) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION
//             AND / OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER
//             UNDER THEORY OF CONTRACT, TORT ( INCLUDING NEGLIGENCE ), STRICT LIABILITY OR
//             OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Copyright ( C ) 2008 Apple Inc. All Rights Reserved.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef __EMSCRIPTEN__
    #include <GL/gl.h>
    #include <GL/glut.h>
    #include <CL/opencl.h>
#else
    #include <OpenGL/OpenGL.h>
    #include <OpenGL/gl.h>
    #include <OpenGL/CGLDevice.h>
    #include <GLUT/glut.h>
    #include <OpenCL/opencl.h>
#endif

#ifdef __APPLE__
    #include <mach/mach_time.h>
#endif


////////////////////////////////////////////////////////////////////////////////

#define USE_GL_ATTACHMENTS              (0)  // enable OpenGL attachments for Compute results
#define DEBUG_INFO                      (1)     
#define COMPUTE_KERNEL_FILENAME         ("qjulia_kernel.cl")
#define COMPUTE_KERNEL_METHOD_NAME      ("QJuliaKernel")
#define SEPARATOR                       ("----------------------------------------------------------------------\n")
#define WIDTH                           (128)
#define HEIGHT                          (128)

////////////////////////////////////////////////////////////////////////////////

static cl_context                       ComputeContext;
static cl_command_queue                 ComputeCommands;
static cl_kernel                        ComputeKernel;
static cl_program                       ComputeProgram;
static cl_device_id                     ComputeDeviceId;
static cl_device_type                   ComputeDeviceType;
static cl_mem                           ComputeResult;
static cl_mem                           ComputeImage;
static size_t                           MaxWorkGroupSize;
static int                              WorkGroupSize[2];
static int                              WorkGroupItems = 32;

////////////////////////////////////////////////////////////////////////////////

static int Width                        = WIDTH;
static int Height                       = HEIGHT;

static int Animated                     = 0;
static int Update                       = 1;

static float Epsilon                    = 0.003f;

static float ColorT                     = 0.0f;
static float ColorA[4]                  = { 0.25f, 0.45f, 1.0f, 1.0f };
static float ColorB[4]                  = { 0.25f, 0.45f, 1.0f, 1.0f };
static float ColorC[4]                  = { 0.25f, 0.45f, 1.0f, 1.0f };

static float MuT                        = 0.0f;
static float MuA[4]                     = { -.278f, -.479f, 0.0f, 0.0f };
static float MuB[4]                     = { 0.278f, 0.479f, 0.0f, 0.0f };
static float MuC[4]                     = { -.278f, -.479f, -.231f, .235f };

////////////////////////////////////////////////////////////////////////////////

static uint TextureId                   = 0;
static uint TextureTarget               = GL_TEXTURE_2D;
static uint TextureInternal             = GL_RGBA;
static uint TextureFormat               = GL_RGBA;
static uint TextureType                 = GL_UNSIGNED_BYTE;
static uint TextureWidth                = WIDTH;
static uint TextureHeight               = HEIGHT;
static size_t TextureTypeSize           = sizeof(char);
static uint ActiveTextureUnit           = GL_TEXTURE1_ARB;
static void* HostImageBuffer            = 0;

static double TimeElapsed               = 0;
static int FrameCount                   = 0;
static uint ReportStatsInterval         = 30;

static float ShadowTextColor[4]         = { 0.0f, 0.0f, 0.0f, 1.0f };
static float HighlightTextColor[4]      = { 0.9f, 0.9f, 0.9f, 1.0f };
static uint TextOffset[2]               = { 25, 25 };

static uint ShowStats                   = 1;
static char StatsString[512]            = "\0";
static uint ShowInfo                    = 1;
static char InfoString[512]             = "\0";

static float VertexPos[4][2]            = { { -1.0f, -1.0f },
                                            { +1.0f, -1.0f },
                                            { +1.0f, +1.0f },
                                            { -1.0f, +1.0f } };
static float TexCoords[4][2];

////////////////////////////////////////////////////////////////////////////////

static int 
DivideUp(int a, int b) 
{
    return ((a % b) != 0) ? (a / b + 1) : (a / b);
}

static uint64_t
GetCurrentTime()
{
    #ifdef __EMSCRIPTEN__
        return (emscripten_get_now() * 1000000);
    #else
        return mach_absolute_time();
    #endif
}

static double
SubtractTime( uint64_t uiEndTime, uint64_t uiStartTime )
{
    #ifdef __EMSCRIPTEN__
        return 1e-9 * (uiEndTime - uiStartTime);
    #else
        static double s_dConversion = 0.0;
        uint64_t uiDifference = uiEndTime - uiStartTime;
        if( 0.0 == s_dConversion )
        {
            mach_timebase_info_data_t kTimebase;
            kern_return_t kError = mach_timebase_info( &kTimebase );
            if( kError == 0  )
                s_dConversion = 1e-9 * (double) kTimebase.numer / (double) kTimebase.denom;
        }
            
        return s_dConversion * (double) uiDifference; 
    #endif
}

////////////////////////////////////////////////////////////////////////////////

static int LoadTextFromFile(
    const char *file_name, char **result_string, size_t *string_len)
{
    int fd;
    unsigned file_len;
    struct stat file_status;
    int ret;

    *string_len = 0;
    fd = open(file_name, O_RDONLY);
    if (fd == -1)
    {
        printf("Error opening file %s\n", file_name);
        return -1;
    }
    ret = fstat(fd, &file_status);
    if (ret)
    {
        printf("Error reading status for file %s\n", file_name);
        return -1;
    }
    file_len = file_status.st_size;

    *result_string = (char*)calloc(file_len + 1, sizeof(char));
    ret = read(fd, *result_string, file_len);
    if (!ret)
    {
        printf("Error reading from file %s\n", file_name);
        return -1;
    }

    close(fd);

    *string_len = file_len;
    return 0;
}

static void DrawString(float x, float y, float color[4], char *buffer)
{
#ifndef __EMSCRIPTEN__
    unsigned int uiLen, i;

    glPushAttrib(GL_LIGHTING_BIT);
    glDisable(GL_LIGHTING);

    glRasterPos2f(x, y);
    glColor3f(color[0], color[1], color[2]);
    uiLen = (unsigned int) strlen(buffer);
    for (i = 0; i < uiLen; i++)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, buffer[i]);
    }
    glPopAttrib();
#endif
}

static void DrawText(float x, float y, int light, char *format, ...)
{
#ifndef __EMSCRIPTEN__
    va_list args;
    char buffer[256];
    GLint iVP[4];
    GLint iMatrixMode;

    va_start(args, format);
    vsprintf(buffer, format, args);
    va_end(args);

    glPushAttrib(GL_LIGHTING_BIT);
    glDisable(GL_LIGHTING);
    glDisable(GL_BLEND);

    glGetIntegerv(GL_VIEWPORT, iVP);
    glViewport(0, 0, Width, Height);
    glGetIntegerv(GL_MATRIX_MODE, &iMatrixMode);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glScalef(2.0f / Width, -2.0f / Height, 1.0f);
    glTranslatef(-Width / 2.0f, -Height / 2.0f, 0.0f);

    if(light)
    {
        glColor4fv(ShadowTextColor);
        DrawString(x-0, y-0, ShadowTextColor, buffer);

        glColor4fv(HighlightTextColor);
        DrawString(x-2, y-2, HighlightTextColor, buffer);
    }
    else
    {
        glColor4fv(HighlightTextColor);
        DrawString(x-0, y-0, HighlightTextColor, buffer);

        glColor4fv(ShadowTextColor);
        DrawString(x-2, y-2, ShadowTextColor, buffer);   
    }
    
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);

    glPopMatrix();
    glMatrixMode(iMatrixMode);

    glPopAttrib();
    glViewport(iVP[0], iVP[1], iVP[2], iVP[3]);
#endif
}

static void 
CreateTexture(uint width, uint height)
{    
    if(TextureId)
        glDeleteTextures(1, &TextureId);
    TextureId = 0;
    
    printf("Creating Texture %d x %d...\n", width, height);

    TextureWidth = width;
    TextureHeight = height;
    
#ifndef __EMSCRIPTEN__
    glActiveTextureARB(ActiveTextureUnit);
#else
    glActiveTexture(ActiveTextureUnit);
#endif
    
    glGenTextures(1, &TextureId);
    glBindTexture(TextureTarget, TextureId);

#ifndef __EMSCRIPTEN__
    glTexParameteri(TextureTarget, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(TextureTarget, GL_TEXTURE_WRAP_T, GL_CLAMP);
#endif
    glTexParameteri(TextureTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(TextureTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(TextureTarget, 0, TextureInternal, TextureWidth, TextureHeight, 0, 
                 TextureFormat, TextureType, 0);
    glBindTexture(TextureTarget, 0);
}

static void 
RenderTexture( void *pvData )
{
    glDisable( GL_LIGHTING );

    glViewport( 0, 0, Width, Height );

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    gluOrtho2D( -1.0, 1.0, -1.0, 1.0 );

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    glMatrixMode( GL_TEXTURE );
    glLoadIdentity();
    
    glEnable( TextureTarget );
    glBindTexture( TextureTarget, TextureId );

    
    if(pvData) {
        glTexSubImage2D(TextureTarget, 0, 0, 0, TextureWidth, TextureHeight, 
                        TextureFormat, TextureType, pvData);
    }
    
#ifdef __EMSCRIPTEN__
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);

    glBegin( GL_QUADS );
    {
        glColor3f(1.0f, 1.0f, 1.0f);
        glTexCoord2f( 0.0f, 0.0f );
        glVertex3f( -1.0f, -1.0f, 0.0f );
        
        glColor3f(1.0f, 1.0f, 1.0f);
        glTexCoord2f( 0.0f, 1.0f );
        glVertex3f( -1.0f, 1.0f, 0.0f );
        
        glColor3f(1.0f, 1.0f, 1.0f);
        glTexCoord2f( 1.0f, 1.0f );
        glVertex3f( 1.0f, 1.0f, 0.0f );
        
        glColor3f(1.0f, 1.0f, 1.0f);
        glTexCoord2f( 1.0f, 0.0f );
        glVertex3f( 1.0f, -1.0f, 0.0f );
    }
#else
    glTexParameteri(TextureTarget, GL_TEXTURE_COMPARE_MODE_ARB, GL_NONE);

    glBegin( GL_QUADS );
    {
        glColor3f(1.0f, 1.0f, 1.0f);
        glTexCoord2f( 0.0f, 0.0f );
        glVertex3f( -1.0f, -1.0f, 0.0f );

        glTexCoord2f( 0.0f, 1.0f );
        glVertex3f( -1.0f, 1.0f, 0.0f );

        glTexCoord2f( 1.0f, 1.0f );
        glVertex3f( 1.0f, 1.0f, 0.0f );

        glTexCoord2f( 1.0f, 0.0f );
        glVertex3f( 1.0f, -1.0f, 0.0f );
    }
#endif
    glEnd();
    glBindTexture( TextureTarget, 0 );
    glDisable( TextureTarget );
}

static void 
Interpolate( float m[4], float t, float a[4], float b[4] )
{
    int i;
    for ( i = 0; i < 4; i++ )
        m[ i ] = ( 1.0f - t ) * a[ i ] + t * b[ i ];
}

static void 
UpdateMu( float t[4], float a[4], float b[4] )
{
    *t += 0.01f;

    uint seed = (uint)GetCurrentTime();
    
    if ( *t >= 1.0f )
    {
        *t = 0.0f;

        a[ 0 ] = b[ 0 ];
        a[ 1 ] = b[ 1 ];
        a[ 2 ] = b[ 2 ];
        a[ 3 ] = b[ 3 ];

        b[ 0 ] = 2.0f * rand_r(&seed) / (float) RAND_MAX - 1.0f;
        b[ 1 ] = 2.0f * rand_r(&seed) / (float) RAND_MAX - 1.0f;
        b[ 2 ] = 2.0f * rand_r(&seed) / (float) RAND_MAX - 1.0f;
        b[ 3 ] = 2.0f * rand_r(&seed) / (float) RAND_MAX - 1.0f;
    }
}

static void
RandomColor( float v[4] )
{
    uint seed = (uint)GetCurrentTime();
    v[ 0 ] = 2.0f * rand_r(&seed) / (float) RAND_MAX - 1.0f;
    v[ 1 ] = 2.0f * rand_r(&seed) / (float) RAND_MAX - 1.0f;
    v[ 2 ] = 2.0f * rand_r(&seed) / (float) RAND_MAX - 1.0f;
    v[ 3 ] = 1.0f;
}

static void 
UpdateColor( float t[4], float a[4], float b[4] )
{
    *t += 0.01f;
   
    if ( *t >= 1.0f )
    {
        *t = 0.0f;

        a[ 0 ] = b[ 0 ];
        a[ 1 ] = b[ 1 ];
        a[ 2 ] = b[ 2 ];
        a[ 3 ] = b[ 3 ];

        RandomColor(b);
    }
}

static int
Recompute(void)
{
    if(!ComputeKernel || !ComputeResult)
        return CL_SUCCESS;
        
    void *values[10];
    size_t sizes[10];
    size_t global[2];
    size_t local[2];

    int err = 0;
    unsigned int v = 0, s = 0, a = 0;
    values[v++] = &ComputeResult;
    values[v++] = MuC;
    values[v++] = ColorC;
    values[v++] = &Epsilon;

    sizes[s++] = sizeof(cl_mem);
    sizes[s++] = 4 * (sizeof(float));
    sizes[s++] = 4 * (sizeof(float));
    sizes[s++] = sizeof(float);

    if(Animated || Update)
    {
        Update = 0;
        err = CL_SUCCESS;
        for (a = 0; a < s; a++)
            err |= clSetKernelArg(ComputeKernel, a, sizes[a], values[a]);
    
        if (err)
            return -10;
    }
    
    int size_x = WorkGroupSize[0];
    int size_y = WorkGroupSize[1];
    
    global[0] = DivideUp(TextureWidth, size_x) * size_x; 
    global[1] = DivideUp(TextureHeight, size_y) * size_y;
    
    local[0] = size_x;
    local[1] = size_y;

#if (DEBUG_INFO)
    if(FrameCount <= 1)
        printf("Global[%4d %4d] Local[%4d %4d]\n", 
            (int)global[0], (int)global[1],
            (int)local[0], (int)local[1]);
#endif

    err = clEnqueueNDRangeKernel(ComputeCommands, ComputeKernel, 2, NULL, global, local, 0, NULL, NULL);
    if (err)
    {
        printf("Failed to enqueue kernel! %d\n", err);
        return err;
    }

#if (USE_GL_ATTACHMENTS)

    err = clEnqueueAcquireGLObjects(ComputeCommands, 1, &ComputeImage, 0, 0, 0);
    if (err != CL_SUCCESS)
    {
        printf("Failed to acquire GL object! %d\n", err);
        return EXIT_FAILURE;
    }

    size_t origin[] = { 0, 0, 0 };
    size_t region[] = { TextureWidth, TextureHeight, 1 };
    err = clEnqueueCopyBufferToImage(ComputeCommands, ComputeResult, ComputeImage, 
                                     0, origin, region, 0, NULL, 0);
    
    if(err != CL_SUCCESS)
    {
        printf("Failed to copy buffer to image! %d\n", err);
        return EXIT_FAILURE;
    }
    
    err = clEnqueueReleaseGLObjects(ComputeCommands, 1, &ComputeImage, 0, 0, 0);
    if (err != CL_SUCCESS)
    {
        printf("Failed to release GL object! %d\n", err);
        return EXIT_FAILURE;
    }

#else

    err = clEnqueueReadBuffer( ComputeCommands, ComputeResult, CL_TRUE, 0, Width * Height * TextureTypeSize * 4, HostImageBuffer, 0, NULL, NULL );      
    if (err != CL_SUCCESS)
    {
        printf("Failed to read buffer! %d\n", err);
        return EXIT_FAILURE;
    }

#endif

    return CL_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////

static int 
CreateComputeResult(void)
{
    int err = 0;
        
#if (USE_GL_ATTACHMENTS)

    if(ComputeImage)
        clReleaseMemObject(ComputeImage);
    ComputeImage = 0;
    
    printf("Allocating compute result image in device memory...\n");
    ComputeImage = clCreateFromGLTexture2D(ComputeContext, CL_MEM_WRITE_ONLY, TextureTarget, 0, TextureId, &err);
    if (!ComputeImage || err != CL_SUCCESS)
    {
        printf("Failed to create OpenGL texture reference! %d\n", err);
        return -1;
    }

#else

    if (HostImageBuffer)
        free(HostImageBuffer);

    printf("Allocating compute result image in host memory...\n");
    HostImageBuffer = malloc(TextureWidth * TextureHeight * TextureTypeSize * 4);
    if(!HostImageBuffer)
    {
        printf("Failed to create host image buffer!\n");
        return -1;
    }
     
    memset(HostImageBuffer, 0, TextureWidth * TextureHeight * TextureTypeSize * 4);

#endif

    if(ComputeResult)
        clReleaseMemObject(ComputeResult);
    ComputeResult = 0;
    
    ComputeResult = clCreateBuffer(ComputeContext, CL_MEM_WRITE_ONLY, TextureTypeSize * 4 * TextureWidth * TextureHeight, NULL, NULL);
    if (!ComputeResult)
    {
        printf("Failed to create OpenCL array!\n");
        return -1;
    }

    return CL_SUCCESS;
}

static int 
SetupComputeDevices(int gpu)
{
    int err;
	size_t returned_size;
    ComputeDeviceType = gpu ? CL_DEVICE_TYPE_GPU : CL_DEVICE_TYPE_CPU;

#if (USE_GL_ATTACHMENTS)

    printf(SEPARATOR);
    printf("Using active OpenGL context...\n");

    CGLContextObj kCGLContext = CGLGetCurrentContext();              
    CGLShareGroupObj kCGLShareGroup = CGLGetShareGroup(kCGLContext);
    
    cl_context_properties properties[] = { 
        CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE, 
        (cl_context_properties)kCGLShareGroup, 0 
    };
        
    // Create a context from a CGL share group
    //
#ifdef __EMSCRIPTEN__
    ComputeContext = clCreateContext(properties, 0, 0, NULL, 0, 0);
#else
    ComputeContext = clCreateContext(properties, 0, 0, clLogMessagesToStdoutAPPLE, 0, 0);
#endif
    if (!ComputeContext)
    {
        printf("Error: Failed to create a compute context!\n");
        return EXIT_FAILURE;
    }

#else

    // Locate a compute device
    //
    err = clGetDeviceIDs(NULL, ComputeDeviceType, 1, &ComputeDeviceId, NULL);
    if (err != CL_SUCCESS)
    {
        printf("Error: Failed to locate compute device!\n");
        return EXIT_FAILURE;
    }
  
    // Create a context containing the compute device(s)
    //
#ifdef __EMSCRIPTEN__
    ComputeContext = clCreateContext(0, 1, &ComputeDeviceId, NULL, NULL, &err);
#else
    ComputeContext = clCreateContext(0, 1, &ComputeDeviceId, clLogMessagesToStdoutAPPLE, NULL, &err);
#endif
    if (!ComputeContext)
    {
        printf("Error: Failed to create a compute context!\n");
        return EXIT_FAILURE;
    }

#endif

    unsigned int device_count;
    cl_device_id device_ids[16];

    err = clGetContextInfo(ComputeContext, CL_CONTEXT_DEVICES, sizeof(device_ids), device_ids, &returned_size);
    if(err)
    {
        printf("Error: Failed to retrieve compute devices for context!\n");
        return EXIT_FAILURE;
    }
    
    device_count = returned_size / sizeof(cl_device_id);
    
    int i = 0;
    int device_found = 0;
    cl_device_type device_type;	
    for(i = 0; i < device_count; i++) 
    {
        clGetDeviceInfo(device_ids[i], CL_DEVICE_TYPE, sizeof(cl_device_type), &device_type, NULL);
        if(device_type == ComputeDeviceType) 
        {
            ComputeDeviceId = device_ids[i];
            device_found = 1;
            break;
        }	
    }
    
    if(!device_found)
    {
        printf("Error: Failed to locate compute device!\n");
        return EXIT_FAILURE;
    }
        
    // Create a command queue
    //
    ComputeCommands = clCreateCommandQueue(ComputeContext, ComputeDeviceId, 0, &err);
    if (!ComputeCommands)
    {
        printf("Error: Failed to create a command queue!\n");
        return EXIT_FAILURE;
    }

    // Report the device vendor and device name
    // 
    cl_char vendor_name[1024] = {0};
    cl_char device_name[1024] = {0};
    err = clGetDeviceInfo(ComputeDeviceId, CL_DEVICE_VENDOR, sizeof(vendor_name), vendor_name, &returned_size);
    err|= clGetDeviceInfo(ComputeDeviceId, CL_DEVICE_NAME, sizeof(device_name), device_name, &returned_size);
    if (err != CL_SUCCESS)
    {
        printf("Error: Failed to retrieve device info!\n");
        return EXIT_FAILURE;
    }

    printf(SEPARATOR);
    printf("Connecting to %s %s...\n", vendor_name, device_name);

    return CL_SUCCESS;
}

static int 
SetupComputeKernel(void)
{
    int err = 0;
    char *source = 0;
    size_t length = 0;

    if(ComputeKernel)
        clReleaseKernel(ComputeKernel);    
    ComputeKernel = 0;

    if(ComputeProgram)
        clReleaseProgram(ComputeProgram);
    ComputeProgram = 0;
    
    printf(SEPARATOR);
    printf("Loading kernel source from file '%s'...\n", COMPUTE_KERNEL_FILENAME);    
    err = LoadTextFromFile(COMPUTE_KERNEL_FILENAME, &source, &length);
    if (!source || err)
    {
        printf("Error: Failed to load kernel source!\n");
        return EXIT_FAILURE;
    }

    const char* width_macro = "#define WIDTH";
    const char* height_macro = "#define HEIGHT";
    
    char* preprocess = malloc(strlen(source) + 1024);
    sprintf(preprocess, "\n%s (%d)\n%s (%d)\n%s", width_macro, Width, height_macro, Height, source); 
#if (DEBUG_INFO)
    printf("%s", preprocess);
#endif

    // Create the compute program from the source buffer
    //
    ComputeProgram = clCreateProgramWithSource(ComputeContext, 1, (const char **) & preprocess, NULL, &err);
    if (!ComputeProgram || err != CL_SUCCESS)
    {
        printf("Error: Failed to create compute program!\n");
        return EXIT_FAILURE;
    }
    free(source);
    free(preprocess);

    // Build the program executable
    //
    err = clBuildProgram(ComputeProgram, 0, NULL, NULL, NULL, NULL);
    if (err != CL_SUCCESS)
    {
        size_t len;
        char buffer[2048];

        printf("Error: Failed to build program executable!\n");
        clGetProgramBuildInfo(ComputeProgram, ComputeDeviceId, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
        printf("%s\n", buffer);
        return EXIT_FAILURE;
    }

    // Create the compute kernel from within the program
    //
    printf("Creating kernel '%s'...\n", COMPUTE_KERNEL_METHOD_NAME);    
    ComputeKernel = clCreateKernel(ComputeProgram, COMPUTE_KERNEL_METHOD_NAME, &err);
    if (!ComputeKernel || err != CL_SUCCESS)
    {
        printf("Error: Failed to create compute kernel!\n");
        return EXIT_FAILURE;
    }

    // Get the maximum work group size for executing the kernel on the device
    //
    err = clGetKernelWorkGroupInfo(ComputeKernel, ComputeDeviceId, CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &MaxWorkGroupSize, NULL);
    if (err != CL_SUCCESS)
    {
        printf("Error: Failed to retrieve kernel work group info! %d\n", err);
        exit(1);
    }

#if (DEBUG_INFO)
    printf("MaxWorkGroupSize: %d\n", MaxWorkGroupSize);
    printf("WorkGroupItems: %d\n", WorkGroupItems);
#endif

    WorkGroupSize[0] = (MaxWorkGroupSize > 1) ? (MaxWorkGroupSize / WorkGroupItems) : MaxWorkGroupSize;
    WorkGroupSize[1] = MaxWorkGroupSize / WorkGroupSize[0];

    printf(SEPARATOR);

    return CL_SUCCESS;

}

static void
Cleanup(void)
{
    clFinish(ComputeCommands);
    clReleaseKernel(ComputeKernel);
    clReleaseProgram(ComputeProgram);
    clReleaseCommandQueue(ComputeCommands);
    clReleaseMemObject(ComputeResult);
#if (USE_GL_ATTACHMENTS)
    clReleaseMemObject(ComputeImage);
#endif
    clReleaseContext(ComputeContext);
    
    ComputeCommands = 0;
    ComputeKernel = 0;
    ComputeProgram = 0;    
    ComputeResult = 0;
    ComputeImage = 0;
    ComputeContext = 0;
}

static void
Shutdown(void)
{
    printf(SEPARATOR);
    printf("Shutting down...\n");
    Cleanup();
    exit(0);
}

////////////////////////////////////////////////////////////////////////////////

static int 
SetupGraphics(void)
{
    CreateTexture(Width, Height);

    glClearColor (0.0, 0.0, 0.0, 0.0);

    glDisable(GL_DEPTH_TEST);
    glActiveTexture(GL_TEXTURE0);
    glViewport(0, 0, Width, Height);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    TexCoords[3][0] = 0.0f;
    TexCoords[3][1] = 0.0f;
    TexCoords[2][0] = Width;
    TexCoords[2][1] = 0.0f;
    TexCoords[1][0] = Width;
    TexCoords[1][1] = Height;
    TexCoords[0][0] = 0.0f;
    TexCoords[0][1] = Height;

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, VertexPos);
    glClientActiveTexture(GL_TEXTURE0);
    glTexCoordPointer(2, GL_FLOAT, 0, TexCoords);
    return GL_NO_ERROR;
}

static int 
Initialize(int gpu)
{
    int err;
    err = SetupGraphics();
    if (err != GL_NO_ERROR)
    {
        printf ("Failed to setup OpenGL state!");
        exit (err);
    }

    err = SetupComputeDevices(gpu);
    if(err != CL_SUCCESS)
    {
        printf ("Failed to connect to compute device! Error %d\n", err);
        exit (err);
    }

    cl_bool image_support;
    err = clGetDeviceInfo(ComputeDeviceId, CL_DEVICE_IMAGE_SUPPORT,
                          sizeof(image_support), &image_support, NULL);
    if (err != CL_SUCCESS) {
        printf("Unable to query device for image support");
        exit(err);
    }
    if (image_support == CL_FALSE) {
        printf("Qjulia requires images: Images not supported on this device.");
        return CL_IMAGE_FORMAT_NOT_SUPPORTED;
    }
    
    err = SetupComputeKernel();
    if (err != CL_SUCCESS)
    {
        printf ("Failed to setup compute kernel! Error %d\n", err);
        exit (err);
    }
    
    err = CreateComputeResult();
    if(err != CL_SUCCESS)
    {
        printf ("Failed to create compute result! Error %d\n", err);
        exit (err);
    }
    
    RandomColor(ColorA);
    RandomColor(ColorB);
    RandomColor(ColorC);
    
    return CL_SUCCESS;
}


static void
ReportInfo(void)
{
	if(ShowStats)
	{
	    int iX = 20;
	    int iY = 20;
	    
        DrawText(iX - 1, Height - iY - 1, 0, StatsString);
        DrawText(iX - 2, Height - iY - 2, 0, StatsString);
        DrawText(iX, Height - iY, 1, StatsString);
     }

    if(ShowInfo)
    {
        int iX = TextOffset[0];
        int iY = Height - TextOffset[1];
    
        DrawText(Width - iX - 1 - strlen(InfoString) * 10, Height - iY - 1, 0, InfoString);
        DrawText(Width - iX - 2 - strlen(InfoString) * 10, Height - iY - 2, 0, InfoString);
        DrawText(Width - iX - strlen(InfoString) * 10, Height - iY, 1, InfoString);
    
        ShowInfo = (ShowInfo > 200) ? 0 : ShowInfo + 1;
    }
}

static void 
ReportStats(
    uint64_t uiStartTime, uint64_t uiEndTime)
{
    TimeElapsed += SubtractTime(uiEndTime, uiStartTime);

    if(TimeElapsed && FrameCount && FrameCount > ReportStatsInterval) 
	{
        double fMs = (TimeElapsed * 1000.0 / (double) FrameCount);
        double fFps = 1.0 / (fMs / 1000.0);
#ifdef __EMSCRIPTEN__
        /*printf("[%s] Compute: %3.2f ms  Display: %3.2f fps (%s)\n",
                (ComputeDeviceType == CL_DEVICE_TYPE_GPU) ? "GPU" : "CPU",
                fMs, fFps, USE_GL_ATTACHMENTS ? "attached" : "copying");*/
#else
        sprintf(StatsString, "[%s] Compute: %3.2f ms  Display: %3.2f fps (%s)\n", 
                (ComputeDeviceType == CL_DEVICE_TYPE_GPU) ? "GPU" : "CPU", 
                fMs, fFps, USE_GL_ATTACHMENTS ? "attached" : "copying");
		glutSetWindowTitle(StatsString);
#endif

		FrameCount = 0;
        TimeElapsed = 0;
	}    
}

static void
Display(void)
{
    FrameCount++;
    uint64_t uiStartTime = GetCurrentTime();
    
    glClearColor (0.0, 0.0, 0.0, 0.0);
    glClear (GL_COLOR_BUFFER_BIT);
 
    if(Animated)
    {
        UpdateMu( &MuT, MuA, MuB );
        Interpolate( MuC, MuT, MuA, MuB );
    
        UpdateColor( &ColorT, ColorA, ColorB );
        Interpolate(ColorC, ColorT, ColorA, ColorB );
    }
    
    int err = Recompute();
    if (err != 0)
    {
        printf("Error %d from Recompute!\n", err);
        exit(1);
    }

    RenderTexture(HostImageBuffer);
    ReportInfo();
    
    glFinish(); // for timing
    
    uint64_t uiEndTime = GetCurrentTime();
    ReportStats(uiStartTime, uiEndTime);
    #ifndef __EMSCRIPTEN__
        DrawText(TextOffset[0], TextOffset[1], 1, (Animated == 0) ? "Press space to animate" : " ");
    #endif
    glutSwapBuffers();
}

static void 
Reshape (int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glClear(GL_COLOR_BUFFER_BIT);
    glutSwapBuffers();
        
    if(w > 2 * Width || h > 2 * Height)
    {
        Width = w;
        Height = h;
        Cleanup();
        if(Initialize(ComputeDeviceType == CL_DEVICE_TYPE_GPU) != GL_NO_ERROR)
            Shutdown();
    }
    
    Width = w;
    Height = h;    
}

void Keyboard( unsigned char key, int x, int y )
{
   const float fStepSize = 0.05f;

   switch( key )
   {
      case 27:
         exit(0);
         break;

      case ' ':
         Animated = !Animated;
#ifdef __EMSCRIPTEN__
         printf("Animated = %s\n", Animated ? "true" : "false");
#else
         sprintf(InfoString, "Animated = %s\n", Animated ? "true" : "false");
#endif
         ShowInfo = 1;
         break;

      case 'i':
         ShowInfo = ShowInfo > 0 ? 0 : 1;
         break;
      
      case 's':
         ShowStats = ShowStats > 0 ? 0 : 1;
         break;
         
      case '+':
      case '=':
         if(Epsilon >= 0.002f)
             Epsilon *= (1.0f / 1.05f);
#ifdef __EMSCRIPTEN__
         printf("Epsilon = %f\n", Epsilon);
#else
         sprintf(InfoString, "Epsilon = %f\n", Epsilon);
#endif
         ShowInfo = 1;
         break;

      case '-':
         if(Epsilon < 0.01f)
             Epsilon *= 1.05f;
#ifdef __EMSCRIPTEN__
         printf("Epsilon = %f\n", Epsilon);
#else
         sprintf(InfoString, "Epsilon = %f\n", Epsilon);
#endif
         ShowInfo = 1;
         break;

      case 'w':
         MuC[0] += fStepSize; 
         break;

      case 'x':
         MuC[0] -= fStepSize; 
         break;

      case 'q':
         MuC[1] += fStepSize; 
         break;

      case 'z':
         MuC[1] -= fStepSize; 
         break;

      case 'a':
         MuC[2] += fStepSize; 
         break;

      case 'd':
         MuC[2] -= fStepSize; 
         break;

      case 'e':
         MuC[3] += fStepSize; 
         break;

      case 'c':
         MuC[3] -= fStepSize; 
         break;

      case 'f':
         glutFullScreen(); 
         break;

    }
    Update = 1;
    glutPostRedisplay();
}

void Idle(void)
{
    glutPostRedisplay();
}

int main(int argc, char** argv)
{
    // Parse command line options
    //
    int i;
    int use_gpu = 1;
    for( i = 0; i < argc && argv; i++)
    {
        if(!argv[i])
            continue;
            
        if(strstr(argv[i], "cpu"))
            use_gpu = 0;        

        else if(strstr(argv[i], "gpu"))
            use_gpu = 1;
    }
    
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize (Width, Height);
    glutInitWindowPosition (100, 100);
    glutCreateWindow (argv[0]);
    if (Initialize (use_gpu) == GL_NO_ERROR)
    {
      
        glutDisplayFunc(Display);
        glutIdleFunc(Idle);
        glutReshapeFunc(Reshape);
        glutKeyboardFunc(Keyboard);

        atexit(Shutdown);
      	
        printf("Starting event loop...\n");

        glutMainLoop();     
    }

    return 0;
}

