#include "test.h"

Parent::Parent(int val) : value(val), immutableAttr(8) { printf("Parent:%d\n", val); }
Parent::Parent(Parent *p, Parent *q) : value(p->value + q->value), immutableAttr(8) { printf("Parent:%d\n", value); }
void Parent::mulVal(int mul) { value *= mul; }

typedef EnumClass::EnumWithinClass EnumClass_EnumWithinClass;
typedef EnumNamespace::EnumInNamespace EnumNamespace_EnumInNamespace;

#include "glue.cpp"

#ifdef BROWSER
int main() {
  printf("main().\n");
  int result = 1;
  REPORT_RESULT();
}
#endif

