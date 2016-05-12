#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <emscripten.h>

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

static const int WINDOWS_SIZE = 500;

static GLfloat vertices[] = { 0.0f,  250.f, 0.0f, 
                             -250.f, -250.f, 0.0f,
                              250.f, -250.f, 0.0f };

static GLfloat vertices2[] = { 0.0f,  250.f, -1.0f, 
                              -250.f, -250.f, -1.0f,
                               250.f, -250.f, -1.0f };

static GLuint shaderProgram = 0;		       
static GLuint verticesVBO = 0;
static GLuint verticesVBO2 = 0;

static unsigned char backgroundColor[4] = {255, 255, 255, 255};
static unsigned char triangleColor[4] = {255, 0, 0, 255};
static unsigned char triangleColor2[4] = {0, 255, 0, 255};

static char vertexShaderSrc[] =
    "precision highp float;"
    "precision highp int;"

    "uniform mat4 u_mvpMatrix;"
    "uniform vec4 u_color;"
    
    "attribute vec3 a_position;"
    
    "varying vec4 v_color;"

    "void main() {"
    "   gl_Position = u_mvpMatrix * vec4(a_position, 1.0);"
    "   v_color = u_color;"
    "}"
    ;

static char fragmentShaderSrc[] =
    "precision highp float;"
    "precision highp int;"

    "varying vec4 v_color;"

    "void main() {"
    "  gl_FragColor = v_color;"
    "}"
    ;

static GLuint createShader(const char *source, int type) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, (const GLchar**)(&source), NULL);
    glCompileShader(shader);
    return shader;
}

static GLuint createShaderProgram(const char *vertexShaderSrc, const char *fragmentShaderSrc) {
    GLuint program = glCreateProgram();
    glAttachShader(program, createShader(vertexShaderSrc, GL_VERTEX_SHADER));
    glAttachShader(program, createShader(fragmentShaderSrc, GL_FRAGMENT_SHADER));
    glLinkProgram(program);
    return program;
}

void ortho(float  left,  float  right,  float  bottom,  float  top,  float  nearVal,  float  farVal, GLfloat *projMatrix) {
    float tx = -(right+left)/(right-left);
    float ty = -(top+bottom)/(top-bottom);
    float tz = -(farVal+nearVal)/(farVal-nearVal);
    memset(projMatrix, 0, 16 * sizeof(GLfloat));
    projMatrix[0] = 2.0f / (right-left);
    projMatrix[3] = tx;
    projMatrix[1*4+1] = 2.0f / (top-bottom);
    projMatrix[1*4+3] = ty;
    projMatrix[2*4+2] = -2.0f / (farVal-nearVal);
    projMatrix[2*4+3] = tz;
    projMatrix[3*4+3] = 1.0f;
}

static void initGlObjects() {
    glGenBuffers(1, &verticesVBO);
    glBindBuffer(GL_ARRAY_BUFFER, verticesVBO);
    glBufferData(GL_ARRAY_BUFFER, 9*sizeof(float), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    glGenBuffers(1, &verticesVBO2);
    glBindBuffer(GL_ARRAY_BUFFER, verticesVBO2);
    glBufferData(GL_ARRAY_BUFFER, 9*sizeof(float), vertices2, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    shaderProgram = createShaderProgram(vertexShaderSrc, fragmentShaderSrc);
}

static void drawTriangle(GLuint verticesVBO, unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
    glUseProgram(shaderProgram);
    GLuint posLoc = glGetAttribLocation(shaderProgram, "a_position");
    GLuint mvpLoc = glGetUniformLocation(shaderProgram, "u_mvpMatrix");
    GLuint colorLoc = glGetUniformLocation(shaderProgram, "u_color");
    
    GLfloat mvpMat[16];
    ortho(-WINDOWS_SIZE/2, WINDOWS_SIZE/2, -WINDOWS_SIZE/2, WINDOWS_SIZE/2, -100, 100, mvpMat);
    
    glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, mvpMat);
    glUniform4f(colorLoc, r/255.f, g/255.f, b/255.f, a/255.f);
    
    glBindBuffer(GL_ARRAY_BUFFER, verticesVBO);  
    glEnableVertexAttribArray(posLoc);
    glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), BUFFER_OFFSET(0));
    
    glDrawArrays(GL_TRIANGLES, 0, 3);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glUseProgram(0);
}

// Draw a red triangle on a white background. If antialiasing is disabled, resulting pixels
// will only have white and red colors. If antialiasing is enabled, there will be pixels
// whose color is different from red and white.
static int testAntiAliasing(bool activated) {
    glViewport(0, 0, WINDOWS_SIZE, WINDOWS_SIZE);
    glClearColor(backgroundColor[0]/255.f, backgroundColor[1]/255.f, backgroundColor[2]/255.f, backgroundColor[3]/255.f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    drawTriangle(verticesVBO, triangleColor[0], triangleColor[1], triangleColor[2], triangleColor[3]);
    
    bool antialiased = false;
    
    unsigned char buffer[(WINDOWS_SIZE*WINDOWS_SIZE)*4];
    glReadPixels(0, 0, WINDOWS_SIZE, WINDOWS_SIZE, GL_RGBA, GL_UNSIGNED_BYTE, &buffer[0]);
    glFinish();
    for (unsigned int i = 0 ; i < WINDOWS_SIZE ; ++i) {
      for (unsigned int j = 0 ; j < WINDOWS_SIZE ; ++j) {
        unsigned char r = buffer[4*(i*WINDOWS_SIZE+j)];
        unsigned char g = buffer[4*(i*WINDOWS_SIZE+j)+1];
        unsigned char b = buffer[4*(i*WINDOWS_SIZE+j)+2];
        unsigned char a = buffer[4*(i*WINDOWS_SIZE+j)+3];
        if ((r == backgroundColor[0] && g == backgroundColor[1] && b == backgroundColor[2] && a == backgroundColor[3]) || 
            (r == triangleColor[0] && g == triangleColor[1] && b == triangleColor[2] && a == triangleColor[3])) {
          continue;
        } else {
          antialiased = true;
          break;
        }
      }
    }
    
    return (activated && antialiased) || (!activated && !antialiased);
}

// Draw a red triangle with depth equals to 0 then a green triangle whose depth equals -1.
// If there is an attached depth buffer, the resulting image will be a red triangle. If not,
// the resulting image will be a green triangle.
static int testDepth(bool activated) {
    glViewport(0, 0, WINDOWS_SIZE, WINDOWS_SIZE);
    glClearColor(backgroundColor[0]/255.f, backgroundColor[1]/255.f, backgroundColor[2]/255.f, backgroundColor[3]/255.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    
    drawTriangle(verticesVBO, triangleColor[0], triangleColor[1], triangleColor[2], triangleColor[3]);
    drawTriangle(verticesVBO2, triangleColor2[0], triangleColor2[1], triangleColor2[2], triangleColor2[3]);
    
    glDisable(GL_DEPTH_TEST);
    
    // read the pixel at the center of the resulting image.
    unsigned char buffer[4];
    glReadPixels(WINDOWS_SIZE/2, WINDOWS_SIZE/2, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &buffer[0]);
    
    bool frontTriangleColor = (buffer[0] == triangleColor[0] && buffer[1] == triangleColor[1] &&
                               buffer[2] == triangleColor[2] && buffer[3] == triangleColor[3]);
    
    bool backTriangleColor = (buffer[0] == triangleColor2[0] && buffer[1] == triangleColor2[1] &&
                              buffer[2] == triangleColor2[2] && buffer[3] == triangleColor2[3]);
    
    return (activated && frontTriangleColor) || (!activated && backTriangleColor);
}

// The stencil function is set to GL_LEQUAL so fragments will be written to the 
// back buffer only if the ref value is less or equal than the one in the stencil buffer.
// The content of the stencil buffer is initialized to 0xFF.
// First draw a red triangle whose stencil ref value is 0x1.
// Then draw a green triangle whose stencil ref value is 0xFF.
// If there is an attached stencil buffer, the resulting image will be a red triangle. If not,
// the resulting image will be a green triangle.
static int testStencil(bool activated) {
    glViewport(0, 0, WINDOWS_SIZE, WINDOWS_SIZE);
    glClearColor(backgroundColor[0]/255.f, backgroundColor[1]/255.f, backgroundColor[2]/255.f, backgroundColor[3]/255.f);
    glClearStencil(0xFF);
    glStencilOp(GL_KEEP,GL_KEEP,GL_REPLACE);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    
    glEnable(GL_STENCIL_TEST);
    
    glStencilFunc(GL_LEQUAL, 0x1, 0xFF);
    drawTriangle(verticesVBO, triangleColor[0], triangleColor[1], triangleColor[2], triangleColor[3]);
    
    glStencilFunc(GL_LEQUAL, 0xFF, 0xFF);
    drawTriangle(verticesVBO, triangleColor2[0], triangleColor2[1], triangleColor2[2], triangleColor2[3]);
    
    glDisable(GL_STENCIL_TEST);
    
    unsigned char buffer[4];
    glReadPixels(WINDOWS_SIZE/2, WINDOWS_SIZE/2, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &buffer[0]);
    
    bool firstTriangleColor = (buffer[0] == triangleColor[0] && buffer[1] == triangleColor[1] &&
                               buffer[2] == triangleColor[2] && buffer[3] == triangleColor[3]);
    
    bool secondTriangleColor = (buffer[0] == triangleColor2[0] && buffer[1] == triangleColor2[1] &&
                                buffer[2] == triangleColor2[2] && buffer[3] == triangleColor2[3]);
    
    return (activated && firstTriangleColor) || (!activated && secondTriangleColor);
}

// Clear to a color with alpha = 0. If alpha is enabled then all pixels will have alpha = 0.
// If alpha is disabled then pixels will have alpha of 255
static int testAlpha(bool activated) {
    glViewport(0, 0, WINDOWS_SIZE, WINDOWS_SIZE);
    glClearColor(backgroundColor[0]/255.f, backgroundColor[1]/255.f, backgroundColor[2]/255.f, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);
    
    bool hasAlpha = true;
    
    unsigned char buffer[(WINDOWS_SIZE*WINDOWS_SIZE)*4];
    glReadPixels(0, 0, WINDOWS_SIZE, WINDOWS_SIZE, GL_RGBA, GL_UNSIGNED_BYTE, &buffer[0]);
    glFinish();
    for (unsigned int i = 0 ; i < WINDOWS_SIZE ; ++i) {
      for (unsigned int j = 0 ; j < WINDOWS_SIZE ; ++j) {
        unsigned char r = buffer[4*(i*WINDOWS_SIZE+j)];
        unsigned char g = buffer[4*(i*WINDOWS_SIZE+j)+1];
        unsigned char b = buffer[4*(i*WINDOWS_SIZE+j)+2];
        unsigned char a = buffer[4*(i*WINDOWS_SIZE+j)+3];
        if (r == backgroundColor[0] && g == backgroundColor[1] && b == backgroundColor[2] && a == 0) {
          continue;
        } else {
          hasAlpha = false;
          break;
        }
      }
    }
    
    return (activated && hasAlpha) || (!activated && !hasAlpha);
}

static bool antiAliasingActivated = false;
static bool depthActivated = false;
static bool stencilActivated = false;
static bool alphaActivated = false;

static int result = 0;
static int resultAA = 0;
static int resultDepth = 0;
static int resultStencil = 0;
static int resultAlpha = 0;

static void draw() {
  
  if (!resultAA) resultAA = testAntiAliasing(antiAliasingActivated);
   
  if (!resultDepth) resultDepth = testDepth(depthActivated);
  
  if (!resultStencil) resultStencil = testStencil(stencilActivated);
  
  if (!resultAlpha) resultAlpha = testAlpha(alphaActivated);
  
  result = resultAA && resultDepth && resultStencil && resultAlpha;
 
}

extern int webglAntialiasSupported(void);
extern int webglDepthSupported(void);
extern int webglStencilSupported(void);
extern int webglAlphaSupported(void);

// Check attributes support in the WebGL implementation (see test_webgl_context_attributes function in test_browser.py)
// Tests will succeed if they are not.
static void checkContextAttributesSupport() {
  if (!webglAntialiasSupported()) {
    resultAA = 1;
    EM_ASM(alert('warning: no antialiasing\n'));
  }
  if (!webglDepthSupported()) {
    resultDepth = 1;
    EM_ASM(alert('warning: no depth\n'));
  }
  if (!webglStencilSupported()) {
    resultStencil = 1;
    EM_ASM(alert('warning: no stencil\n'));
  }
  if (!webglAlphaSupported()) {
    resultAlpha = 1;
    EM_ASM(alert('warning: no alpha\n'));
  }
}


