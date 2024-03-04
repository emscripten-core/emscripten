// Copyright 2012 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <math.h>
#include <stdio.h>
#include <string.h>

#define GL_GLEXT_PROTOTYPES
#define EGL_EGLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glut.h>

static const char vertex_shader[] =
        "#ifdef GL_ES\n"
        "precision lowp float;\n"
        "#endif\n"
        "attribute float indices;\n"
        "uniform sampler2D nodeInfo;\n"
        "varying vec4 color;"
        "\n"
        "void main(void)\n"
        "{\n"
        "    float s = (indices + 0.5) / 512.; \n"
        "    vec4  v = texture2D(nodeInfo, vec2( s, 0.5));\n"
        "    gl_Position = vec4(v.x, v.y, 0.5, 1.);\n"
        "    gl_PointSize = v.z;\n"
        "    color = vec4(0.5 + v.w/2., 0.5 + 0.5 * v.w/2., 0.5, 1);\n"
        "}\n";

static const char fragment_shader[] =
        "#ifdef GL_ES\n"
        "precision lowp float;\n"
        "#endif\n"
        "\n"
        "varying vec4 color;\n"
        "void main(void)\n"
        "{\n"
        "  float dst = distance(vec2(0.5, 0.5), gl_PointCoord); \n"
        "  gl_FragColor = color;\n"
        "  if ( dst > 0.3) {"
        "    gl_FragColor = vec4(0., 0., 0.5, 0.2);\n"
        "}\n"
        "if ( dst > 0.5) discard;\n"
        "}";

typedef struct NodeInfo { //structure that we want to transmit to our shaders
    float x;
    float y;
    float s;
    float c;
} NodeInfo;

GLuint nodeTexture; //texture id used to bind
GLuint nodeSamplerLocation; //shader sampler address
GLuint indicesAttributeLocation; //shader attribute address
GLuint indicesVBO; //Vertex Buffer Object Id;
#define NUM_NODES 512
NodeInfo data[NUM_NODES]; //our data that will be transmitted using float texture.
double alpha = 0; //use to make a simple funny effect;

static void updateFloatTexture() {
    int count = 0;
    for (float x=0; x < NUM_NODES; ++x ) {
        data[count].x = 0.2*pow(cos(alpha), 3) + (sin(alpha)*3. + 3.5) * x/NUM_NODES * cos(alpha + x/NUM_NODES * 16. * M_PI);
        data[count].y = 0.2*pow(sin(alpha), 3) + (sin(alpha)*3. + 3.5) * x/NUM_NODES * sin(alpha + x/NUM_NODES * 16. * M_PI);
        data[count].s = (16. + 16. * cos(alpha + x/NUM_NODES * 32. * M_PI)) + 8.;// * fmod(x/NUM_NODES + alpha, 1.) + 5.;
        data[count].c = 0.5 + 0.5 * sin(alpha + x/NUM_NODES * 32. * M_PI);
        ++count;
    }
    glBindTexture(GL_TEXTURE_2D, nodeTexture);
#ifdef __EMSCRIPTEN__ // In GLES2 and WebGL1, we must use unsized texture internal formats.
    const GLenum internalFormat = GL_RGBA;
#else
    // In desktop GL, we can also use sized internal formats.
    const GLenum internalFormat = GL_RGBA32F;
#endif
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, NUM_NODES, 1, 0, GL_RGBA, GL_FLOAT, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
    alpha -= 0.001;
}

static void glut_draw_callback(void) {
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(1., 1., 1., 0.);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glActiveTexture(GL_TEXTURE0);
    updateFloatTexture(); //we change the texture each time to create the effect (it is just for the test)
    glBindTexture(GL_TEXTURE_2D, nodeTexture);
    glUniform1i(nodeSamplerLocation, 0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, indicesVBO);
    glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, 0, NULL);
    glDrawArrays(GL_POINTS, 0, NUM_NODES);
    glutSwapBuffers();
}

GLuint createShader(const char* source, int type) {
    char msg[512];
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    glGetShaderInfoLog(shader, sizeof msg, NULL, msg);
    printf("Shader info: %s\n", msg);
    return shader;
}

static void gl_init(void) {
    GLuint program = glCreateProgram();
    glAttachShader(program, createShader(vertex_shader  , GL_VERTEX_SHADER));
    glAttachShader(program, createShader(fragment_shader, GL_FRAGMENT_SHADER));
    glLinkProgram(program);
    char msg[512];
    glGetProgramInfoLog(program, sizeof msg, NULL, msg);
    printf("info: %s\n", msg);
    glUseProgram(program);
    float elements[NUM_NODES];
    int count = 0;
    for (float x=0; x < NUM_NODES; ++x ) {
        elements[count] = count;
        ++count;
    }
    /*Create one texture to store all the needed information */
    glGenTextures(1, &nodeTexture);
    /* Store the vertices in a vertex buffer object (VBO) */
    glGenBuffers(1, &indicesVBO);
    glBindBuffer(GL_ARRAY_BUFFER, indicesVBO);
    float zeroes[NUM_NODES];
    memset(zeroes, 0, sizeof(zeroes));
    glBufferData(GL_ARRAY_BUFFER, NUM_NODES * sizeof(float), zeroes, GL_STATIC_DRAW);
    for (int x = 0; x < NUM_NODES; x++) {
      glBufferSubData(GL_ARRAY_BUFFER, x * sizeof(float), sizeof(float), &elements[x]);
    }
    /* Get the locations of the uniforms so we can access them */
    nodeSamplerLocation = glGetUniformLocation(program, "nodeInfo");
    glBindAttribLocation(program, 0, "indices");
#ifndef __EMSCRIPTEN__ // GLES2 & WebGL do not have these, only pre 3.0 desktop GL and compatibility mode GL3.0+ GL do.
    //Enable glPoint size in shader, always enable in Open Gl ES 2.
    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
    glEnable(GL_POINT_SPRITE);
#endif
}

int main(int argc, char *argv[]) {
    glutInit(&argc, argv);
    glutInitWindowSize(640, 480);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    int w = glutCreateWindow("Simple FLOAT Texture Test");
    /* Set up glut callback functions */
    glutDisplayFunc(glut_draw_callback);
    gl_init();
    glutMainLoop();
    return 0;
}


