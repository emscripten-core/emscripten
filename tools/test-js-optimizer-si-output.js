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
  f();
  if (y) {
   g();
  }
 }
 if ((x ? y : 0) ? z : 0) {
  g();
 }
 if (x) {
  f();
  if (y ? z : 0) {
   g();
  }
 }
 if (x ? y : 0) {
  f();
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
}

