struct S {
  S() : a(0) {}
  virtual ~S() {}
  int a;
};

struct T : S {
  int b;
};

struct R : S {
  int c;
};

int main() {
  T t;
  S &s = t;
  R &r = static_cast<R&>(s);
}
