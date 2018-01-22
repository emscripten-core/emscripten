int test = 0;

void l() { test++; }

void k() { test++; }

void j() { test++; }

void i() { test++; }

void h() { test++; }

void g() { l(); test++; }

void f() { k(); test++; }

void e() { i(); j(); test++; }

void d() { test++; }

void c() { g(); h(); test++; }

void b() { f(); g(); test++; }

void a() { d(); e(); f(); test++; }

int main() { a(); b(); b(); }