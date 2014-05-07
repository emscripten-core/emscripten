#include "test.h"

Parent::Parent(int val) : value(val) { printf("Parent:%d\n", val); }
Parent::Parent(Parent *p, Parent *q) : value(p->value + q->value) { printf("Parent:%d\n", value); }
void Parent::mulVal(int mul) { value *= mul; }

#include "glue.cpp"

