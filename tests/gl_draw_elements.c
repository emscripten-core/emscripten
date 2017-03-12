#include <GL/gl.h>
#include <GL/glut.h>

typedef struct Vertex {
    short           x, y, z;
    unsigned char   c_r, c_g, c_b, c_a;
} Vertex;

Vertex triangle[3] = {
    0, 0, 0,   0, 255, 255, 255,
    1, 1, 0, 255,   0, 255, 255,
    1, 0, 0, 255, 255,   0, 255
};
unsigned short indices[3] = { 0, 1, 2};

void draw() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glClearColor(1, 1, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    int stride = sizeof(Vertex);

    glVertexPointer(3, GL_SHORT, stride, &triangle[0].x);
    glColorPointer(4, GL_UNSIGNED_BYTE, stride, &triangle[0].c_r);

    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, indices);

    glFlush();
    glutSwapBuffers();
}

int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(640, 480);
    glutCreateWindow("glDrawElements");

    glutDisplayFunc(draw);
    glutMainLoop();

    return 0;
}
