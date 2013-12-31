#include <stdio.h>
#include <string.h>

#define CONSTRLEN 32

char *(*func)(char *, const char *) = NULL;

void conoutfv(const char *fmt) {
  static char buf[CONSTRLEN];
  func(buf, fmt);  // call by function pointer to make sure we test strcpy here
  puts(buf);
}

struct XYZ {
  float x, y, z;
  XYZ(float a, float b, float c) : x(a), y(b), z(c) {}
  static const XYZ &getIdentity() {
    static XYZ iT(1, 2, 3);
    return iT;
  }
};
struct S {
  static const XYZ &getIdentity() {
    static const XYZ iT(XYZ::getIdentity());
    return iT;
  }
};

int main() {
  func = &strcpy;
  conoutfv("*staticccz*");
  printf("*%.2f,%.2f,%.2f*\n", S::getIdentity().x, S::getIdentity().y,
         S::getIdentity().z);
  return 0;
}
