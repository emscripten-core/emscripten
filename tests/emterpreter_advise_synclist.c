/*
 * Copyright 2018 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

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