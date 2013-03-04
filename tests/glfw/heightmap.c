//========================================================================
// Heightmap example program using OpenGL 3 core profile
// Copyright (c) 2010 Olivier Delannoy
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would
//    be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source
//    distribution.
//
//========================================================================

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <stddef.h>
#include "getopt.h"


#define GLFW_NO_GLU 1 
#include <GL/glfw.h>

/* OpenGL 3.3 support  
 * Functions are effectively mapped in init_opengl() */
#ifndef GL_VERSION_3_0
/* no defines */
#endif

#ifndef GL_VERSION_2_0

typedef char GLchar;

#define GL_BLEND_EQUATION_RGB             0x8009
#define GL_VERTEX_ATTRIB_ARRAY_ENABLED    0x8622
#define GL_VERTEX_ATTRIB_ARRAY_SIZE       0x8623
#define GL_VERTEX_ATTRIB_ARRAY_STRIDE     0x8624
#define GL_VERTEX_ATTRIB_ARRAY_TYPE       0x8625
#define GL_CURRENT_VERTEX_ATTRIB          0x8626
#define GL_VERTEX_PROGRAM_POINT_SIZE      0x8642
#define GL_VERTEX_ATTRIB_ARRAY_POINTER    0x8645
#define GL_STENCIL_BACK_FUNC              0x8800
#define GL_STENCIL_BACK_FAIL              0x8801
#define GL_STENCIL_BACK_PASS_DEPTH_FAIL   0x8802
#define GL_STENCIL_BACK_PASS_DEPTH_PASS   0x8803
#define GL_MAX_DRAW_BUFFERS               0x8824
#define GL_DRAW_BUFFER0                   0x8825
#define GL_DRAW_BUFFER1                   0x8826
#define GL_DRAW_BUFFER2                   0x8827
#define GL_DRAW_BUFFER3                   0x8828
#define GL_DRAW_BUFFER4                   0x8829
#define GL_DRAW_BUFFER5                   0x882A
#define GL_DRAW_BUFFER6                   0x882B
#define GL_DRAW_BUFFER7                   0x882C
#define GL_DRAW_BUFFER8                   0x882D
#define GL_DRAW_BUFFER9                   0x882E
#define GL_DRAW_BUFFER10                  0x882F
#define GL_DRAW_BUFFER11                  0x8830
#define GL_DRAW_BUFFER12                  0x8831
#define GL_DRAW_BUFFER13                  0x8832
#define GL_DRAW_BUFFER14                  0x8833
#define GL_DRAW_BUFFER15                  0x8834
#define GL_BLEND_EQUATION_ALPHA           0x883D
#define GL_MAX_VERTEX_ATTRIBS             0x8869
#define GL_VERTEX_ATTRIB_ARRAY_NORMALIZED 0x886A
#define GL_MAX_TEXTURE_IMAGE_UNITS        0x8872
#define GL_FRAGMENT_SHADER                0x8B30
#define GL_VERTEX_SHADER                  0x8B31
#define GL_MAX_FRAGMENT_UNIFORM_COMPONENTS 0x8B49
#define GL_MAX_VERTEX_UNIFORM_COMPONENTS  0x8B4A
#define GL_MAX_VARYING_FLOATS             0x8B4B
#define GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS 0x8B4C
#define GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS 0x8B4D
#define GL_SHADER_TYPE                    0x8B4F
#define GL_FLOAT_VEC2                     0x8B50
#define GL_FLOAT_VEC3                     0x8B51
#define GL_FLOAT_VEC4                     0x8B52
#define GL_INT_VEC2                       0x8B53
#define GL_INT_VEC3                       0x8B54
#define GL_INT_VEC4                       0x8B55
#define GL_BOOL                           0x8B56
#define GL_BOOL_VEC2                      0x8B57
#define GL_BOOL_VEC3                      0x8B58
#define GL_BOOL_VEC4                      0x8B59
#define GL_FLOAT_MAT2                     0x8B5A
#define GL_FLOAT_MAT3                     0x8B5B
#define GL_FLOAT_MAT4                     0x8B5C
#define GL_SAMPLER_1D                     0x8B5D
#define GL_SAMPLER_2D                     0x8B5E
#define GL_SAMPLER_3D                     0x8B5F
#define GL_SAMPLER_CUBE                   0x8B60
#define GL_SAMPLER_1D_SHADOW              0x8B61
#define GL_SAMPLER_2D_SHADOW              0x8B62
#define GL_DELETE_STATUS                  0x8B80
#define GL_COMPILE_STATUS                 0x8B81
#define GL_LINK_STATUS                    0x8B82
#define GL_VALIDATE_STATUS                0x8B83
#define GL_INFO_LOG_LENGTH                0x8B84
#define GL_ATTACHED_SHADERS               0x8B85
#define GL_ACTIVE_UNIFORMS                0x8B86
#define GL_ACTIVE_UNIFORM_MAX_LENGTH      0x8B87
#define GL_SHADER_SOURCE_LENGTH           0x8B88
#define GL_ACTIVE_ATTRIBUTES              0x8B89
#define GL_ACTIVE_ATTRIBUTE_MAX_LENGTH    0x8B8A
#define GL_FRAGMENT_SHADER_DERIVATIVE_HINT 0x8B8B
#define GL_SHADING_LANGUAGE_VERSION       0x8B8C
#define GL_CURRENT_PROGRAM                0x8B8D
#define GL_POINT_SPRITE_COORD_ORIGIN      0x8CA0
#define GL_LOWER_LEFT                     0x8CA1
#define GL_UPPER_LEFT                     0x8CA2
#define GL_STENCIL_BACK_REF               0x8CA3
#define GL_STENCIL_BACK_VALUE_MASK        0x8CA4
#define GL_STENCIL_BACK_WRITEMASK         0x8CA5
#endif 



#ifndef GL_VERSION_1_5

typedef ptrdiff_t GLintptr;
typedef ptrdiff_t GLsizeiptr;

#define GL_BUFFER_SIZE                    0x8764
#define GL_BUFFER_USAGE                   0x8765
#define GL_QUERY_COUNTER_BITS             0x8864
#define GL_CURRENT_QUERY                  0x8865
#define GL_QUERY_RESULT                   0x8866
#define GL_QUERY_RESULT_AVAILABLE         0x8867
#define GL_ARRAY_BUFFER                   0x8892
#define GL_ELEMENT_ARRAY_BUFFER           0x8893
#define GL_ARRAY_BUFFER_BINDING           0x8894
#define GL_ELEMENT_ARRAY_BUFFER_BINDING   0x8895
#define GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING 0x889F
#define GL_READ_ONLY                      0x88B8
#define GL_WRITE_ONLY                     0x88B9
#define GL_READ_WRITE                     0x88BA
#define GL_BUFFER_ACCESS                  0x88BB
#define GL_BUFFER_MAPPED                  0x88BC
#define GL_BUFFER_MAP_POINTER             0x88BD
#define GL_STREAM_DRAW                    0x88E0
#define GL_STREAM_READ                    0x88E1
#define GL_STREAM_COPY                    0x88E2
#define GL_STATIC_DRAW                    0x88E4
#define GL_STATIC_READ                    0x88E5
#define GL_STATIC_COPY                    0x88E6
#define GL_DYNAMIC_DRAW                   0x88E8
#define GL_DYNAMIC_READ                   0x88E9
#define GL_DYNAMIC_COPY                   0x88EA
#define GL_SAMPLES_PASSED                 0x8914
#endif 




/* OpenGL 3.0 */
typedef void (APIENTRY * PFN_glGenVertexArrays)(GLsizei n, GLuint *arrays);
typedef void (APIENTRY * PFN_glBindVertexArray)(GLuint array);
typedef void (APIENTRY * PFN_glDeleteVertexArrays)(GLsizei n, GLuint *arrays);
/* OpenGL 2.0 */
typedef GLuint (APIENTRY * PFN_glCreateShader)(GLenum type);
typedef void (APIENTRY * PFN_glDeleteShader)(GLuint shader);
typedef void (APIENTRY * PFN_glCompileShader)(GLuint shader);
typedef void (APIENTRY * PFN_glShaderSource)(GLuint shader, GLsizei count, const GLchar* *string, const GLint *length);
typedef void (APIENTRY * PFN_glGetShaderiv)(GLuint shader, GLenum pname, GLint *params);
typedef void (APIENTRY * PFN_glGetShaderInfoLog)(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef GLuint (APIENTRY * PFN_glCreateProgram)(void);
typedef void (APIENTRY * PFN_glDeleteProgram)(GLuint program);
typedef void (APIENTRY * PFN_glAttachShader)(GLuint program, GLuint shader);
typedef void (APIENTRY * PFN_glLinkProgram)(GLuint program);
typedef void (APIENTRY * PFN_glGetProgramiv)(GLuint program, GLenum pname, GLint *params);
typedef void (APIENTRY * PFN_glGetProgramInfoLog)(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef void (APIENTRY * PFN_glValidateProgram)(GLuint program);
typedef void (APIENTRY * PFN_glUseProgram)(GLuint program);
typedef GLint (APIENTRY * PFN_glGetUniformLocation)(GLuint program, const GLchar *name);
typedef void (APIENTRY * PFN_glUniformMatrix4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef GLint (APIENTRY * PFN_glGetAttribLocation)(GLuint program, const GLchar *name);
typedef void (APIENTRY * PFN_glEnableVertexAttribArray)(GLuint index);
typedef void (APIENTRY * PFN_glVertexAttrib1f)(GLuint index, GLfloat x);
typedef void (APIENTRY * PFN_glVertexAttribPointer)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer);
/* OpenGL 1.5 */ 
typedef void (APIENTRY * PFN_glBindBuffer)(GLenum target, GLuint buffer);
typedef void (APIENTRY * PFN_glDeleteBuffers)(GLsizei n, const GLuint *buffers);
typedef void (APIENTRY * PFN_glGenBuffers)(GLsizei n, GLuint *buffers);
typedef void (APIENTRY * PFN_glBufferData)(GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage);
typedef void (APIENTRY * PFN_glBufferSubData)(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid *data);

/* OpenGL function pointers */ 
static PFN_glGenBuffers                 pglGenBuffers = NULL;
static PFN_glGenVertexArrays            pglGenVertexArrays = NULL;
static PFN_glDeleteVertexArrays         pglDeleteVertexArrays = NULL;
static PFN_glCreateShader               pglCreateShader = NULL;
static PFN_glShaderSource               pglShaderSource = NULL;
static PFN_glCompileShader              pglCompileShader = NULL;
static PFN_glGetShaderiv                pglGetShaderiv = NULL;
static PFN_glGetShaderInfoLog           pglGetShaderInfoLog = NULL;
static PFN_glDeleteShader               pglDeleteShader = NULL;
static PFN_glCreateProgram              pglCreateProgram = NULL;
static PFN_glAttachShader               pglAttachShader = NULL;
static PFN_glLinkProgram                pglLinkProgram = NULL;
static PFN_glUseProgram                 pglUseProgram = NULL;
static PFN_glGetProgramiv               pglGetProgramiv = NULL;
static PFN_glGetProgramInfoLog          pglGetProgramInfoLog = NULL;
static PFN_glDeleteProgram              pglDeleteProgram = NULL;
static PFN_glGetUniformLocation         pglGetUniformLocation = NULL;
static PFN_glUniformMatrix4fv           pglUniformMatrix4fv = NULL;
static PFN_glGetAttribLocation          pglGetAttribLocation = NULL;

/* Map height updates */
#define MAX_CIRCLE_SIZE (5.0f)
#define MAX_DISPLACEMENT (1.0f)
#define DISPLACEMENT_SIGN_LIMIT (0.3f)
#define MAX_ITER (200)
#define NUM_ITER_AT_A_TIME (1)

/* Map general information */
#define MAP_SIZE (10.0f)
#define MAP_NUM_VERTICES (80)
#define MAP_NUM_TOTAL_VERTICES (MAP_NUM_VERTICES*MAP_NUM_VERTICES)
#define MAP_NUM_LINES (3* (MAP_NUM_VERTICES - 1) * (MAP_NUM_VERTICES - 1) + \
               2 * (MAP_NUM_VERTICES - 1))


/* OpenGL function pointers */

static PFN_glBindVertexArray            pglBindVertexArray = NULL;
static PFN_glBufferData                 pglBufferData = NULL;
static PFN_glBindBuffer                 pglBindBuffer = NULL;
static PFN_glBufferSubData              pglBufferSubData = NULL;
static PFN_glEnableVertexAttribArray    pglEnableVertexAttribArray = NULL;
static PFN_glVertexAttribPointer        pglVertexAttribPointer = NULL;

#define RESOLVE_GL_FCN(type, var, name) \
    if (status == GL_TRUE) \
    {\
        var = glfwGetProcAddress((name));\
        if ((var) == NULL)\
        {\
            status = GL_FALSE;\
        }\
    }


static GLboolean init_opengl(void)
{
    GLboolean status = GL_TRUE;
    RESOLVE_GL_FCN(PFN_glCreateShader, pglCreateShader, "glCreateShader");
    RESOLVE_GL_FCN(PFN_glShaderSource, pglShaderSource, "glShaderSource");
    RESOLVE_GL_FCN(PFN_glCompileShader, pglCompileShader, "glCompileShader");
    RESOLVE_GL_FCN(PFN_glGetShaderiv, pglGetShaderiv, "glGetShaderiv");
    RESOLVE_GL_FCN(PFN_glGetShaderInfoLog, pglGetShaderInfoLog, "glGetShaderInfoLog");
    RESOLVE_GL_FCN(PFN_glDeleteShader, pglDeleteShader, "glDeleteShader");
    RESOLVE_GL_FCN(PFN_glCreateProgram, pglCreateProgram, "glCreateProgram");
    RESOLVE_GL_FCN(PFN_glAttachShader, pglAttachShader, "glAttachShader");
    RESOLVE_GL_FCN(PFN_glLinkProgram, pglLinkProgram, "glLinkProgram");
    RESOLVE_GL_FCN(PFN_glUseProgram, pglUseProgram, "glUseProgram");
    RESOLVE_GL_FCN(PFN_glGetProgramiv, pglGetProgramiv, "glGetProgramiv");
    RESOLVE_GL_FCN(PFN_glGetProgramInfoLog, pglGetProgramInfoLog, "glGetProgramInfoLog");
    RESOLVE_GL_FCN(PFN_glDeleteProgram, pglDeleteProgram, "glDeleteProgram");
    RESOLVE_GL_FCN(PFN_glGetUniformLocation, pglGetUniformLocation, "glGetUniformLocation");
    RESOLVE_GL_FCN(PFN_glUniformMatrix4fv, pglUniformMatrix4fv, "glUniformMatrix4fv");
    RESOLVE_GL_FCN(PFN_glGetAttribLocation, pglGetAttribLocation, "glGetAttribLocation");
    RESOLVE_GL_FCN(PFN_glGenVertexArrays, pglGenVertexArrays, "glGenVertexArrays");
    RESOLVE_GL_FCN(PFN_glDeleteVertexArrays, pglDeleteVertexArrays, "glDeleteVertexArrays");
    RESOLVE_GL_FCN(PFN_glBindVertexArray, pglBindVertexArray, "glBindVertexArray");
    RESOLVE_GL_FCN(PFN_glGenBuffers, pglGenBuffers, "glGenBuffers");
    RESOLVE_GL_FCN(PFN_glBindBuffer, pglBindBuffer, "glBindBuffer");
    RESOLVE_GL_FCN(PFN_glBufferData, pglBufferData, "glBufferData");
    RESOLVE_GL_FCN(PFN_glBufferSubData, pglBufferSubData, "glBufferSubData");
    RESOLVE_GL_FCN(PFN_glEnableVertexAttribArray, pglEnableVertexAttribArray, "glEnableVertexAttribArray");
    RESOLVE_GL_FCN(PFN_glVertexAttribPointer, pglVertexAttribPointer, "glVertexAttribPointer");
    return status;
}
/**********************************************************************
 * Default shader programs
 *********************************************************************/

static const char* default_vertex_shader =
"#version 150\n"
"uniform mat4 project;\n"
"uniform mat4 modelview;\n"
"in float x;\n"
"in float y;\n"
"in float z;\n"
"\n"
"void main()\n"
"{\n"
"   gl_Position = project * modelview * vec4(x, y, z, 1.0);\n"
"}\n";

static const char* default_fragment_shader =
"#version 150\n"
"out vec4 gl_FragColor;\n"
"void main()\n"
"{\n"
"    gl_FragColor = vec4(0.2, 1.0, 0.2, 1.0); \n"
"}\n";

/**********************************************************************
 * Values for shader uniforms
 *********************************************************************/

/* Frustum configuration */
static GLfloat view_angle = 45.0f;
static GLfloat aspect_ratio = 4.0f/3.0f;
static GLfloat z_near = 1.0f;
static GLfloat z_far = 100.f;

/* Projection matrix */
static GLfloat projection_matrix[16] = {
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
};

/* Model view matrix */
static GLfloat modelview_matrix[16] = {
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
};

/**********************************************************************
 * Heightmap vertex and index data
 *********************************************************************/

static GLfloat map_vertices[3][MAP_NUM_TOTAL_VERTICES];
static GLuint  map_line_indices[2*MAP_NUM_LINES];

/* Store uniform location for the shaders
 * Those values are setup as part of the process of creating
 * the shader program. They should not be used before creating
 * the program.
 */
static GLuint mesh;
static GLuint mesh_vbo[4];

/**********************************************************************
 * OpenGL helper functions
 *********************************************************************/

/* Load a (text) file into memory and return its contents
 */
static char* read_file_content(const char* filename)
{
    FILE* fd;
    size_t size = 0;
    char* result = NULL;

    fd = fopen(filename, "r");
    if (fd != NULL)
    {
        size = fseek(fd, 0, SEEK_END);
        (void) fseek(fd, 0, SEEK_SET);

        result = malloc(size + 1);
        result[size] = '\0';
        if (fread(result, size, 1, fd) != 1)
        {
            free(result);
            result = NULL;
        }
        (void) fclose(fd);
    }
    return result;
}

/* Creates a shader object of the specified type using the specified text
 */
static GLuint make_shader(GLenum type, const char* shader_src)
{
    GLuint shader;
    GLint shader_ok;
    GLsizei log_length;
    char info_log[8192];

    shader = pglCreateShader(type);
    if (shader != 0)
    {
        pglShaderSource(shader, 1, (const GLchar**)&shader_src, NULL);
        pglCompileShader(shader);
        pglGetShaderiv(shader, GL_COMPILE_STATUS, &shader_ok);
        if (shader_ok != GL_TRUE)
        {
            fprintf(stderr, "ERROR: Failed to compile %s shader\n", (type == GL_FRAGMENT_SHADER) ? "fragment" : "vertex" );
            pglGetShaderInfoLog(shader, 8192, &log_length,info_log);
            fprintf(stderr, "ERROR: \n%s\n\n", info_log);
            pglDeleteShader(shader);
            shader = 0;
        }
    }
    return shader;
}

/* Creates a program object using the specified vertex and fragment text
 */
static GLuint make_shader_program(const char* vertex_shader_src, const char* fragment_shader_src)
{
    GLuint program = 0u;
    GLint program_ok;
    GLuint vertex_shader = 0u;
    GLuint fragment_shader = 0u;
    GLsizei log_length;
    char info_log[8192];

    vertex_shader = make_shader(GL_VERTEX_SHADER, (vertex_shader_src == NULL) ? default_vertex_shader : vertex_shader_src);
    if (vertex_shader != 0u)
    {
        fragment_shader = make_shader(GL_FRAGMENT_SHADER, (fragment_shader_src == NULL) ? default_fragment_shader : fragment_shader_src);
        if (fragment_shader != 0u)
        {
            /* make the program that connect the two shader and link it */
            program = pglCreateProgram();
            if (program != 0u)
            {
                /* attach both shader and link */
                pglAttachShader(program, vertex_shader);
                pglAttachShader(program, fragment_shader);
                pglLinkProgram(program);
                pglGetProgramiv(program, GL_LINK_STATUS, &program_ok);

                if (program_ok != GL_TRUE)
                {
                    fprintf(stderr, "ERROR, failed to link shader program\n");
                    pglGetProgramInfoLog(program, 8192, &log_length, info_log);
                    fprintf(stderr, "ERROR: \n%s\n\n", info_log);
                    pglDeleteProgram(program);
                    pglDeleteShader(fragment_shader);
                    pglDeleteShader(vertex_shader);
                    program = 0u;
                }
            }
        }
        else
        {
            fprintf(stderr, "ERROR: Unable to load fragment shader\n");
            pglDeleteShader(vertex_shader);
        }
    }
    else
    {
        fprintf(stderr, "ERROR: Unable to load vertex shader\n");
    }
    return program;
}

/**********************************************************************
 * Geometry creation functions
 *********************************************************************/

/* Generate vertices and indices for the heightmap
 */
static void init_map(void)
{
    int i;
    int j;
    int k;
    GLfloat step = MAP_SIZE / (MAP_NUM_VERTICES - 1);
    GLfloat x = 0.0f;
    GLfloat z = 0.0f;
    /* Create a flat grid */
    k = 0;
    for (i = 0 ; i < MAP_NUM_VERTICES ; ++i)
    {
        for (j = 0 ; j < MAP_NUM_VERTICES ; ++j)
        {
            map_vertices[0][k] = x;
            map_vertices[1][k] = 0.0f;
            map_vertices[2][k] = z;
            z += step;
            ++k;
        }
        x += step;
        z = 0.0f;
    }
#if DEBUG_ENABLED
    for (i = 0 ; i < MAP_NUM_TOTAL_VERTICES ; ++i)
    {
        printf ("Vertice %d (%f, %f, %f)\n",
                i, map_vertices[0][i], map_vertices[1][i], map_vertices[2][i]);

    }
#endif
    /* create indices */
    /* line fan based on i
     * i+1
     * |  / i + n + 1
     * | /
     * |/
     * i --- i + n
     */

    /* close the top of the square */
    k = 0;
    for (i = 0 ; i < MAP_NUM_VERTICES  -1 ; ++i)
    {
        map_line_indices[k++] = (i + 1) * MAP_NUM_VERTICES -1;
        map_line_indices[k++] = (i + 2) * MAP_NUM_VERTICES -1;
    }
    /* close the right of the square */
    for (i = 0 ; i < MAP_NUM_VERTICES -1 ; ++i)
    {
        map_line_indices[k++] = (MAP_NUM_VERTICES - 1) * MAP_NUM_VERTICES + i;
        map_line_indices[k++] = (MAP_NUM_VERTICES - 1) * MAP_NUM_VERTICES + i + 1;
    }

    for (i = 0 ; i < (MAP_NUM_VERTICES - 1) ; ++i)
    {
        for (j = 0 ; j < (MAP_NUM_VERTICES - 1) ; ++j)
        {
            int ref = i * (MAP_NUM_VERTICES) + j;
            map_line_indices[k++] = ref;
            map_line_indices[k++] = ref + 1;

            map_line_indices[k++] = ref;
            map_line_indices[k++] = ref + MAP_NUM_VERTICES;

            map_line_indices[k++] = ref;
            map_line_indices[k++] = ref + MAP_NUM_VERTICES + 1;
        }
    }

#ifdef DEBUG_ENABLED
    for (k = 0 ; k < 2 * MAP_NUM_LINES ; k += 2)
    {
        int beg, end;
        beg = map_line_indices[k];
        end = map_line_indices[k+1];
        printf ("Line %d: %d -> %d (%f, %f, %f) -> (%f, %f, %f)\n",
                k / 2, beg, end,
                map_vertices[0][beg], map_vertices[1][beg], map_vertices[2][beg],
                map_vertices[0][end], map_vertices[1][end], map_vertices[2][end]);
    }
#endif
}

static void generate_heightmap__circle(float* center_x, float* center_y,
        float* size, float* displacement)
{
    float sign;
    /* random value for element in between [0-1.0] */
    *center_x = (MAP_SIZE * rand()) / (1.0f * RAND_MAX);
    *center_y = (MAP_SIZE * rand()) / (1.0f * RAND_MAX);
    *size = (MAX_CIRCLE_SIZE * rand()) / (1.0f * RAND_MAX);
    sign = (1.0f * rand()) / (1.0f * RAND_MAX);
    sign = (sign < DISPLACEMENT_SIGN_LIMIT) ? -1.0f : 1.0f;
    *displacement = (sign * (MAX_DISPLACEMENT * rand())) / (1.0f * RAND_MAX);
}

/* Run the specified number of iterations of the generation process for the
 * heightmap
 */
static void update_map(int num_iter)
{
    assert(num_iter > 0);
    while(num_iter)
    {
        /* center of the circle */
        float center_x;
        float center_z;
        float circle_size;
        float disp;
        size_t ii;
        generate_heightmap__circle(&center_x, &center_z, &circle_size, &disp);
        disp = disp / 2.0f;
        for (ii = 0u ; ii < MAP_NUM_TOTAL_VERTICES ; ++ii)
        {
            GLfloat dx = center_x - map_vertices[0][ii];
            GLfloat dz = center_z - map_vertices[2][ii];
            GLfloat pd = (2.0f * sqrtf((dx * dx) + (dz * dz))) / circle_size;
            if (fabs(pd) <= 1.0f)
            {
                /* tx,tz is within the circle */
                GLfloat new_height = disp + ((GLfloat) cos(pd*3.14f) * disp);
                map_vertices[1][ii] += new_height;
            }
        }
        --num_iter;
    }
}

/**********************************************************************
 * OpenGL helper functions
 *********************************************************************/

/* Create VBO, IBO and VAO objects for the heightmap geometry and bind them to
 * the specified program object
 */
static void make_mesh(GLuint program)
{
    GLuint attrloc;

    pglGenVertexArrays(1, &mesh);
    pglGenBuffers(4, mesh_vbo);
    pglBindVertexArray(mesh);
    /* Prepare the data for drawing through a buffer inidices */
    pglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh_vbo[3]);
    pglBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)* MAP_NUM_LINES * 2, map_line_indices, GL_STATIC_DRAW);

    /* Prepare the attributes for rendering */
    attrloc = pglGetAttribLocation(program, "x");
    pglBindBuffer(GL_ARRAY_BUFFER, mesh_vbo[0]);
    pglBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * MAP_NUM_TOTAL_VERTICES, &map_vertices[0][0], GL_STATIC_DRAW);
    pglEnableVertexAttribArray(attrloc);
    pglVertexAttribPointer(attrloc, 1, GL_FLOAT, GL_FALSE, 0, 0);

    attrloc = pglGetAttribLocation(program, "z");
    pglBindBuffer(GL_ARRAY_BUFFER, mesh_vbo[2]);
    pglBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * MAP_NUM_TOTAL_VERTICES, &map_vertices[2][0], GL_STATIC_DRAW);
    pglEnableVertexAttribArray(attrloc);
    pglVertexAttribPointer(attrloc, 1, GL_FLOAT, GL_FALSE, 0, 0);

    attrloc = pglGetAttribLocation(program, "y");
    pglBindBuffer(GL_ARRAY_BUFFER, mesh_vbo[1]);
    pglBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * MAP_NUM_TOTAL_VERTICES, &map_vertices[1][0], GL_DYNAMIC_DRAW);
    pglEnableVertexAttribArray(attrloc);
    pglVertexAttribPointer(attrloc, 1, GL_FLOAT, GL_FALSE, 0, 0);
}

/* Update VBO vertices from source data
 */
static void update_mesh(void)
{
    pglBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * MAP_NUM_TOTAL_VERTICES, &map_vertices[1][0]);
}

/**********************************************************************
 * GLFW callback functions
 *********************************************************************/

/* The program runs as long as this is GL_TRUE
 */
static GLboolean running = GL_TRUE;

/* GLFW Window management functions */
static int GLFWCALL close_window_callback(void)
{
    running = GL_FALSE;

    /* Disallow window closing
     * The window will be closed when the main loop terminates */
    return GL_FALSE;
}

static void GLFWCALL key_callback(int key, int action)
{
    switch(key)
    {
        case GLFW_KEY_ESC:
            /* Exit program on Escape */
            running = GL_FALSE;
            break;
    }
}

/* Print usage information */
static void usage(void)
{
    printf("Usage: heightmap [-v <vertex_shader_path>] [-f <fragment_shader_path>]\n");
    printf("       heightmap [-h]\n");
}

int main(int argc, char** argv)
{
    int ch, iter;
    double dt;
    double last_update_time;
    int frame;
    float f;
    GLint uloc_modelview;
    GLint uloc_project;

    char* vertex_shader_path = NULL;
    char* fragment_shader_path = NULL;
    char* vertex_shader_src = NULL;
    char* fragment_shader_src = NULL;
    GLuint shader_program;

    while ((ch = getopt(argc, argv, "f:v:h")) != -1)
    {
        switch (ch)
        {
            case 'f':
                fragment_shader_path = optarg;
                break;
            case 'v':
                vertex_shader_path = optarg;
                break;
            case 'h':
                usage();
                exit(EXIT_SUCCESS);
            default:
                usage();
                exit(EXIT_FAILURE);
        }
    }

    if (fragment_shader_path)
    {
        vertex_shader_src = read_file_content(fragment_shader_path);
        if (!fragment_shader_src)
        {
            fprintf(stderr,
                    "ERROR: unable to load fragment shader from '%s'\n",
                    fragment_shader_path);
            exit(EXIT_FAILURE);
        }
    }

    if (vertex_shader_path)
    {
        vertex_shader_src = read_file_content(vertex_shader_path);
        if (!vertex_shader_src)
        {
            fprintf(stderr,
                    "ERROR: unable to load vertex shader from '%s'\n",
                    fragment_shader_path);
            exit(EXIT_FAILURE);
        }
    }

    if (GL_TRUE != glfwInit())
    {
        fprintf(stderr, "ERROR: Unable to initialize GLFW\n");
        usage();

        free(vertex_shader_src);
        free(fragment_shader_src);
        exit(EXIT_FAILURE);
    }

    glfwEnable(GLFW_AUTO_POLL_EVENTS); /* No explicit call to glfwPollEvents() */

    glfwOpenWindowHint(GLFW_WINDOW_NO_RESIZE, GL_TRUE);
    glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 3);
    glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 2);
    glfwOpenWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwOpenWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_FALSE);

    if (GL_TRUE != glfwOpenWindow(800, 600, 0, 0, 0, 0, 0, 0, GLFW_WINDOW))
    {
        fprintf(stderr, "ERROR: Unable to create the OpenGL context and associated window\n");
        usage();

        free(vertex_shader_src);
        free(fragment_shader_src);
        exit(EXIT_FAILURE);
    }

    glfwSetWindowTitle("GLFW OpenGL3 Heightmap demo");
    /* Register events callback */
    glfwSetWindowCloseCallback(close_window_callback);
    glfwSetKeyCallback(key_callback);

    if (GL_TRUE != init_opengl())
    {
        fprintf(stderr, "ERROR: unable to resolve OpenGL function pointers\n");
        free(vertex_shader_src);
        free(fragment_shader_src);
        exit(EXIT_FAILURE);
    }
    /* Prepare opengl resources for rendering */
    shader_program = make_shader_program(vertex_shader_src , fragment_shader_src);
    free(vertex_shader_src);
    free(fragment_shader_src);

    if (shader_program == 0u)
    {
        fprintf(stderr, "ERROR: during creation of the shader program\n");
        usage();
        exit(EXIT_FAILURE);
    }

    pglUseProgram(shader_program);
    uloc_project   = pglGetUniformLocation(shader_program, "project");
    uloc_modelview = pglGetUniformLocation(shader_program, "modelview");

    /* Compute the projection matrix */
    f = 1.0f / tanf(view_angle / 2.0f);
    projection_matrix[0]  = f / aspect_ratio;
    projection_matrix[5]  = f;
    projection_matrix[10] = (z_far + z_near)/ (z_near - z_far);
    projection_matrix[11] = -1.0f;
    projection_matrix[14] = 2.0f * (z_far * z_near) / (z_near - z_far);
    pglUniformMatrix4fv(uloc_project, 1, GL_FALSE, projection_matrix);

    /* Set the camera position */
    modelview_matrix[12]  = -5.0f;
    modelview_matrix[13]  = -5.0f;
    modelview_matrix[14]  = -20.0f;
    pglUniformMatrix4fv(uloc_modelview, 1, GL_FALSE, modelview_matrix);

    /* Create mesh data */
    init_map();
    make_mesh(shader_program);

    /* Create vao + vbo to store the mesh */
    /* Create the vbo to store all the information for the grid and the height */

    /* setup the scene ready for rendering */
    glViewport(0, 0, 800, 600);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    /* main loop */
    frame = 0;
    iter = 0;
    dt = last_update_time = glfwGetTime();

    while (running)
    {
        ++frame;
        /* render the next frame */
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawElements(GL_LINES, 2* MAP_NUM_LINES , GL_UNSIGNED_INT, 0);

        /* display and process events through callbacks */
        glfwSwapBuffers();
        /* Check the frame rate and update the heightmap if needed */
        dt = glfwGetTime();
        if ((dt - last_update_time) > 0.2)
        {
            /* generate the next iteration of the heightmap */
            if (iter < MAX_ITER)
            {
                update_map(NUM_ITER_AT_A_TIME);
                update_mesh();
                iter += NUM_ITER_AT_A_TIME;
            }
            last_update_time = dt;
            frame = 0;
        }
    }

    exit(EXIT_SUCCESS);
}

