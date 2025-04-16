/*
 * Copyright 2013 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <GL/glew.h>
#include <GL/glut.h>
#include <emscripten.h>

#include "test_webgl_context_attributes_common.c"

int main(int argc, char *argv[]) {
    
    checkContextAttributesSupport(); 
    
    unsigned int glutDisplayMode = GLUT_RGBA | GLUT_DOUBLE;
        
#ifdef AA_ACTIVATED
    antiAliasingActivated = true;
    glutDisplayMode |= GLUT_MULTISAMPLE;
#endif
    
#ifdef DEPTH_ACTIVATED
    depthActivated = true;
    glutDisplayMode |= GLUT_DEPTH;
#endif
    
#ifdef STENCIL_ACTIVATED
    stencilActivated = true;
    glutDisplayMode |= GLUT_STENCIL;
#endif
    
#ifdef ALPHA_ACTIVATED
    alphaActivated = true;
    glutDisplayMode |= GLUT_ALPHA;
#endif
    
    glutInit(&argc, argv);
    glutInitWindowSize(WINDOWS_SIZE, WINDOWS_SIZE);
    glutInitDisplayMode(glutDisplayMode);
    glutCreateWindow("WebGL");
    glutDisplayFunc(draw);  
     
    glewInit();
    initGlObjects();
    
    draw();
    
    return 0;
}
