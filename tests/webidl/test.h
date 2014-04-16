#include <stdio.h>

class Parent {
protected:
  int value;
public:
  Parent(int val);
  Parent(Parent *p, Parent *q); // overload constructor
  int getVal() { return value; }; // inline should work just fine here, unlike Way 1 before
  void mulVal(int mul);
};

class Child1 : public Parent {
public:
  Child1() : Parent(7) { printf("Child1:%d\n", value); };
  Child1(int val) : Parent(val*2) { value -= 1; printf("Child1:%d\n", value); };
  int getValSqr() { return value*value; }
  int getValSqr(int more) { return value*value*more; }
  int getValTimes(int times=1) { return value*times; }
};

class Child2 : public Parent {
public:
  Child2() : Parent(9) { printf("Child2:%d\n", value); };
  int getValCube() { return value*value*value; }
  static void printStatic() { printf("*static*\n"); }

  virtual void virtualFunc() { printf("*virtualf*\n"); }
  virtual void virtualFunc2() { printf("*virtualf2*\n"); }
  static void runVirtualFunc(Child2 *self) { self->virtualFunc(); };
private:
  void doSomethingSecret() { printf("security breached!\n"); }; // we should not be able to do this
};

