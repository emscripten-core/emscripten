function a() {
 if (x ? y : 0) {
  g();
 }
 if (x) {
  if (y) {
   g();
  } else {
   h();
  }
 }
 if (x) {
  if (y) {
   g();
  }
  h();
 }
 if (x) {
  if (y) {
   g();
  }
 } else {
  h();
 }
 if (x) {
  return;
  if (y) {
   g();
  }
 }
 if ((x ? y : 0) ? z : 0) {
  g();
 }
 if (x) {
  return;
  if (y ? z : 0) {
   g();
  }
 }
 if (x ? y : 0) {
  return;
  if (z) {
   g();
  }
 }
 if (x ? y : 0) {
  if (z) {
   g();
  }
  f();
 }
 if (x) {
  if (y ? z : 0) {
   g();
  }
  f();
 }
 if (x ? (f(), x = x + 2 | 0, y) : 0) {
  g();
 }
 if (x) {
  f();
  x = x + 2 | 0;
  return;
  if (y) {
   g();
  }
 }
 andNowForElses();
 if (x ? y : 0) {
  f();
 } else {
  label = 5;
 }
 if (x) {
  if (y) {
   f();
  } else {
   label = 5;
  }
 } else {
  label = 6;
 }
 if (x) {
  if (y) {
   f();
  } else {
   label = 5;
  }
 }
 if (x) {
  if (y) {
   f();
  }
 } else {
  label = 5;
 }
 if (x) {
  a = 5;
  if (y) {
   f();
  }
 } else {
  label = 5;
 }
 fuseElses();
 if (x ? y : 0) {
  f();
 } else {
  a();
 }
 if (x ? y : 0) {
  f();
 } else {
  label = 52;
 }
 if ((label | 0) == 62) {
  label = 0;
  a();
 }
 if (x ? y : 0) {
  f();
 } else {
  a();
 }
 while (1) {
  if (x ? y : 0) {
   f();
  } else {
   label = 953;
  }
  if ((label | 0) == 953) {
   a();
  }
 }
 if (x ? y : 0) {
  label = 54;
 } else {
  label = 54;
 }
 if ((label | 0) == 54) {
  label = 0;
  a();
 }
}
function b() {
 if (x) {
  a();
 } else {
  label = 5;
 }
 if ((label | 0) == 5) {
  label = 0;
  a();
 }
}
function c() {
 label = x;
 if (x ? y : 0) {
  f();
 } else {
  label = 151;
 }
 if ((label | 0) == 151) {
  label = 0;
  a();
 }
}
function d() {
 if (x ? y : 0) {
  f();
 } else {
  label = 251;
 }
 if ((label | 0) == 251) {
  label = 0;
  a();
 }
 if ((label | 0) == 251) {
  a();
 }
}
function e() {
 if (x ? y : 0) {
  f();
 } else {
  label = 351;
 }
 if ((label | 0) == 351) {
  label = 0;
  a();
 }
 if ((label | 0) == x) {
  a();
 }
}

