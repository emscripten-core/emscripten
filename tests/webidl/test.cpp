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
  EM_ASM({
    // simple test that everything is functional
    var sme = new Module.Parent(42);
    sme.mulVal(2);
    var got = sme.getVal();
    assert(got === 84, "got: " + got);
  });
  REPORT_RESULT(1);
}
#endif

