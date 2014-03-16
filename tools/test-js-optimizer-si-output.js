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
}

