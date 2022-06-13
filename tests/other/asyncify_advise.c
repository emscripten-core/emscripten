#include <emscripten.h>

int test = 0;

EM_JS(void, async_func, (), {
  Asyncify.handleSleep(wakeUp => {
    setTimeout(wakeUp, 10);
  });
});

void i() { async_func(); test++; }

void h() { test++; }

void g() { h(); i(); test++; }

void f() { test++; }

void e() { async_func(); test++; }

void d() { test++; }

void c() { d(); e(); test++; }

void b() { test++; }

void a() { b(); c(); test++; }

int main() { a(); f(); g(); }