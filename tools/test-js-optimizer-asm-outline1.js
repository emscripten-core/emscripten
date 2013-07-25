function lin() {
  c(1);
  c(2);
  c(3);
  c(4);
  c(5);
  c(6);
  c(7);
  c(8);
  c(9);
  c(10);
  c(11);
  c(12);
  c(13);
  c(14);
  c(15);
  c(16);
  c(17);
  c(18);
  c(19);
  c(20);
}
function lin2() {
  while (1) {
    c(1);
    c(2);
    c(3);
    c(4);
    c(5);
    c(6);
    c(7);
    c(8);
    c(9);
    c(10);
    c(11);
    c(12);
    c(13);
    c(14);
    c(15);
    c(16);
    c(17);
    c(18);
    c(19);
    c(20);
  }
}
function lin3() {
  while (1) {
    c(1);
    c(2);
    c(3);
    c(4);
    c(5);
    c(6);
    c(7);
    c(8);
    c(9);
    c(10);
    c(11);
    c(12);
    c(13);
    c(14);
    c(15);
    c(16);
    c(17);
    c(18);
    c(19);
    c(20);
    return 10;
  }
  return 20;
}
function lin4() {
  while (1) {
    c(1);
    c(2);
    c(3);
    c(4);
    c(5);
    c(6);
    c(7);
    c(8);
    c(9);
    c(10);
    c(11);
    c(12);
    c(13);
    c(14);
    c(15);
    c(16);
    c(17);
    c(18);
    c(19);
    c(20);
    break;
  }
  return 20;
}
function lin5() {
  while (1) {
    c(1);
    c(2);
    c(3);
    c(4);
    c(5);
    c(6);
    c(7);
    c(8);
    c(9);
    c(10);
    c(11);
    c(12);
    c(13);
    c(14);
    c(15);
    c(16);
    c(17);
    c(18);
    c(19);
    c(20);
    continue;
  }
  return 20;
}
function mix() {
  main: while (1) {
    c(1);
    c(2);
    c(3);
    c(4);
    c(5);
    c(6);
    c(7);
    c(8);
    c(9);
    c(10);
    c(11);
    c(12);
    c(13);
    c(14);
    c(15);
    c(16);
    c(17);
    break main;
    c(18);
    break;
    while (1) {
      break; // no need to forward
    }
    inner: while (1) {
      break inner; // no need to forward
    }
    c(19);
    continue;
    c(20);
    continue main;
  }
  return 20;
}
function vars(x, y) {
  x = x | 0;
  y = +y;
  var a = 0, b = +0;
  a = x+y;
  b = y*x;
  c(1+a);
  c(2+b);
  c(3+a);
  c(4+b);
  c(5+a);
  c(6+b);
  c(7+a);
  c(8+b);
}
function vars2(x, y) {
  x = x | 0;
  y = +y;
  var a = 0, b = +0;
  a = x+y;
  b = y*x;
  a = c(1+a);
  b = c(2+b);
  a = c(3+a);
  b = c(4+b);
  a = c(5+a);
  b = c(6+b);
}
function vars3(x, y) {
  x = x | 0;
  y = +y;
  var a = 0, b = +0;
  a = x+y;
  b = y*x;
  a = c(1+a);
  a = c(2+b);
  a = c(3+a);
  a = c(4+b);
  a = c(5+a);
  a = c(6+b);
  a = c(7+a);
}
function vars4(x, y) {
  x = x | 0;
  y = +y;
  var a = 0, b = +0;
  a = x+y;
  b = y*x;
  a = c(1+a);
  a = c(2+a);
  a = c(3+a);
  a = c(4+a);
  a = c(5+a);
  a = c(6+a);
  b = c(7+a+x);
}
function vars_w_stack(x, y) {
  x = x | 0;
  y = +y;
  var a = 0, b = +0, sp = 0;
  sp = STACKTOP;
  STACKTOP = STACKTOP + 16 | 0;
  a = x+y;
  b = y*x;
  a = c(1+a);
  a = c(2+a);
  a = c(3+a);
  a = c(4+a);
  a = c(5+a);
  a = c(6+a);
  b = c(7+a);
  STACKTOP = sp;
}
function chain() {
  if (x == 1) {
    print(1);
  } else if (x == 2) {
    print(2);
  } else if (x == 3) {
    print(3);
  } else if (x == 4) {
    print(4);
  } else if (x == 5) {
    print(5);
  } else if (x == 6) {
    print(6);
  } else if (x == 7) {
    print(7);
  } else if (x == 8) {
    print(8);
  } else if (x == 9) {
    print(9);
  } else if (x == 10) {
    print(10);
  } else if (x == 11) {
    print(11);
  } else if (x == 12) {
    print(12);
  } else {
    print(99);
  }
}
// EMSCRIPTEN_GENERATED_FUNCTIONS
// EXTRA_INFO: { "sizeToOutline": 30, "allowCostlyOutlines": 1 }
