#include <stdio.h>
#include <stdlib.h>

// Part 1

class Parent {
protected:
  int value;
public:
  Parent(int val);
  Parent(Parent *p, Parent *q); // overload constructor
  int getVal() { return value; }; // inline should work just fine here, unlike Way 1 before
  void mulVal(int mul);
  void parentFunc() {}
  const Parent *getAsConst() { return NULL; }
  void *voidStar(void *something) { return something; }
  bool getBoolean() { return true; }

  const int immutableAttr;
};

class Child1 : public Parent {
public:
  Child1() : Parent(7) { printf("Child1:%d\n", value); };
  Child1(int val) : Parent(val*2) { value -= 1; printf("Child1:%d\n", value); };
  int getValSqr() { return value*value; }
  int getValSqr(int more) { return value*value*more; }
  int getValTimes(int times=1) { return value*times; }
  void parentFunc(int x) { printf("Child1::parentFunc(%d)\n", x); }
};

// Child2 has vtable, parent does not. Checks we cast child->parent properly - (Parent*)child is not a no-op, must offset
class Child2 : public Parent {
public:
  Child2() : Parent(9) { printf("Child2:%d\n", value); };
  int getValCube() { return value*value*value; }
  static void printStatic() { printf("*static*\n"); }

  virtual void virtualFunc() { printf("*virtualf*\n"); }
  virtual void virtualFunc2() { printf("*virtualf2*\n"); }
  static void runVirtualFunc(Child2 *self) { self->virtualFunc(); };
  virtual void virtualFunc3(int x) { printf("*virtualf3: %d*\n", x); }
  virtual void virtualFunc4(int x) { printf("*virtualf4: %d*\n", x); }
  static void runVirtualFunc3(Child2 *self, int x) { self->virtualFunc3(x); };

private:
  void doSomethingSecret() { printf("security breached!\n"); }; // we should not be able to do this
};

// Part 2

#include <string.h>

class StringUser {
  char *s;
  int i;
public:
  StringUser(char *string="NO", int integer=99) : s(strdup(string)), i(integer) {}
  ~StringUser() { free(s); }
  void Print(int anotherInteger, char *anotherString) {
    printf("|%s|%d|%s|%d|\n", s, i, anotherString, anotherInteger);
  }
  void PrintFloat(float f) { printf("%.2f\n", f); }
  const char* returnAString() { return "a returned string"; }
};

struct RefUser {
  int value;
  RefUser(int x = 77) : value(x) {}
  int getValue(RefUser b) { return b.value; }
  RefUser &getMe() { return *this; }
  RefUser getCopy() { return RefUser(value*2); }
  StringUser getAnother() { return StringUser("another", 5); }
};

struct VoidPointerUser {
  void *ptr;

  void *GetVoidPointer() { return ptr; }
  void SetVoidPointer(void *p) { ptr = p; }
};

namespace Space {
  struct Inner {
    Inner() {}
    int get() { return 198; }
    Inner& operator*=(float x) { return *this; }
    int operator[](int x) { return x*2; }
  };
}

enum AnEnum {
  enum_value1,
  enum_value2
};

namespace EnumNamespace {
  enum EnumInNamespace {
    e_namespace_val = 78
  };
};

class EnumClass {
 public:
  enum EnumWithinClass {
    e_val = 34
  };
  EnumWithinClass GetEnum() { return e_val; }

  EnumNamespace::EnumInNamespace GetEnumFromNameSpace() { return EnumNamespace::e_namespace_val; }
};

class TypeTestClass {
 public:
  char ReturnCharMethod() { return (2<<6)-1; }
  void AcceptCharMethod(char x) { printf("char: %d\n", x); }

  unsigned char ReturnUnsignedCharMethod() { return (2<<7)-1; }
  void AcceptUnsignedCharMethod(unsigned char x) { printf("unsigned char: %u\n", x); }

  unsigned short int ReturnUnsignedShortMethod() { return (2<<15)-1; }
  void AcceptUnsignedShortMethod(unsigned short x) { printf("unsigned short int: %u\n", x); }

  unsigned long ReturnUnsignedLongMethod() { return 0xffffffff; }
  void AcceptUnsignedLongMethod(unsigned long x) { printf("unsigned long int: %u\n", x); }
};

struct StructInArray {
  StructInArray() : attr1(0), attr2(0) {}
  StructInArray(int _attr1, int _attr2) : attr1(_attr1), attr2(_attr2) {}
  int attr1;
  int attr2;
};

class ArrayClass {
 public:
  ArrayClass() {
    for (int i = 0; i < 8; i++) {
      int_array[i] = i;
      struct_array[i] = StructInArray(i, -i);
      struct_ptr_array[i] = NULL;
    }
  }
  int int_array[8];
  StructInArray struct_array[8];
  StructInArray* struct_ptr_array[8];
};

struct ReceiveArrays {
  void giveMeArrays(float* vertices, int* triangles, int num) {
    for (int i = 0; i < num; i++) {
      printf("%d : %.2f\n", triangles[i], vertices[i]);
    }
  }
};

