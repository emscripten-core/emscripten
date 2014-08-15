#include "test.h"

Parent::Parent(int val) : value(val) { printf("Parent:%d\n", val); }
Parent::Parent(Parent *p, Parent *q) : value(p->value + q->value) { printf("Parent:%d\n", value); }
void Parent::mulVal(int mul) { value *= mul; }

typedef EnumClass::EnumWithinClass EnumClass_EnumWithinClass;
typedef EnumNamespace::EnumInNamespace EnumNamespace_EnumInNamespace;

#include "glue.cpp"

