function lin() {
 var sp = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 16 | 0;
 c(1);
 c(2);
 c(3);
 c(4);
 HEAP32[sp + 0 >> 2] = 0;
 HEAP32[sp + 8 >> 2] = 0;
 lin$1(sp);
 HEAP32[sp + 0 >> 2] = 0;
 HEAP32[sp + 8 >> 2] = 0;
 lin$0(sp);
 STACKTOP = sp;
}
function lin2() {
 var sp = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 16 | 0;
 while (1) {
  c(1);
  c(2);
  c(3);
  c(4);
  HEAP32[sp + 0 >> 2] = 0;
  HEAP32[sp + 8 >> 2] = 0;
  lin2$1(sp);
  HEAP32[sp + 0 >> 2] = 0;
  HEAP32[sp + 8 >> 2] = 0;
  lin2$0(sp);
 }
 STACKTOP = sp;
}
function lin3() {
 var sp = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 16 | 0;
 while (1) {
  c(1);
  c(2);
  c(3);
  c(4);
  c(5);
  HEAP32[sp + 0 >> 2] = 0;
  HEAP32[sp + 8 >> 2] = 0;
  lin3$1(sp);
  HEAP32[sp + 0 >> 2] = 0;
  HEAP32[sp + 8 >> 2] = 0;
  lin3$0(sp);
  if ((HEAP32[sp + 0 >> 2] | 0) == 6) {
   STACKTOP = sp;
   return HEAP32[sp + 8 >> 2] | 0;
  }
 }
 STACKTOP = sp;
 return 20;
}
function lin4() {
 var sp = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 16 | 0;
 while (1) {
  c(1);
  c(2);
  c(3);
  c(4);
  HEAP32[sp + 0 >> 2] = 0;
  HEAP32[sp + 8 >> 2] = 0;
  lin4$1(sp);
  HEAP32[sp + 0 >> 2] = 0;
  HEAP32[sp + 8 >> 2] = 0;
  lin4$0(sp);
  if ((HEAP32[sp + 0 >> 2] | 0) == 1) {
   break;
  }
 }
 STACKTOP = sp;
 return 20;
}
function lin5() {
 var sp = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 16 | 0;
 while (1) {
  c(1);
  c(2);
  c(3);
  c(4);
  HEAP32[sp + 0 >> 2] = 0;
  HEAP32[sp + 8 >> 2] = 0;
  lin5$1(sp);
  HEAP32[sp + 0 >> 2] = 0;
  HEAP32[sp + 8 >> 2] = 0;
  lin5$0(sp);
  if ((HEAP32[sp + 0 >> 2] | 0) == 3) {
   continue;
  }
 }
 STACKTOP = sp;
 return 20;
}
function mix() {
 var sp = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 16 | 0;
 main : while (1) {
  c(1);
  c(2);
  c(3);
  c(4);
  c(5);
  c(6);
  c(7);
  HEAP32[sp + 0 >> 2] = 0;
  HEAP32[sp + 8 >> 2] = 0;
  mix$1(sp);
  HEAP32[sp + 0 >> 2] = 0;
  HEAP32[sp + 8 >> 2] = 0;
  mix$0(sp);
  if ((HEAP32[sp + 0 >> 2] | 0) == 1) {
   break;
  }
  if ((HEAP32[sp + 0 >> 2] | 0) == 2) {
   switch (HEAP32[sp + 8 >> 2] | 0) {
   case 2:
    break main;
   }
  }
  if ((HEAP32[sp + 0 >> 2] | 0) == 3) {
   continue;
  }
  if ((HEAP32[sp + 0 >> 2] | 0) == 4) {
   switch (HEAP32[sp + 8 >> 2] | 0) {
   case 3:
    continue main;
   }
  }
 }
 STACKTOP = sp;
 return 20;
}
function vars(x, y) {
 x = x | 0;
 y = +y;
 var sp = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 32 | 0;
 HEAP32[sp + 16 >> 2] = 0;
 HEAP32[sp + 24 >> 2] = 0;
 HEAP32[sp + 0 >> 2] = x;
 HEAPF32[sp + 8 >> 2] = y;
 vars$1(sp);
 HEAP32[sp + 16 >> 2] = 0;
 HEAP32[sp + 24 >> 2] = 0;
 HEAP32[sp + 0 >> 2] = x;
 HEAPF32[sp + 8 >> 2] = y;
 vars$0(sp);
 STACKTOP = sp;
}
function vars2(x, y) {
 x = x | 0;
 y = +y;
 var a = 0, b = +0, sp = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 48 | 0;
 a = x + y;
 b = y * x;
 a = c(1 + a);
 b = c(2 + b);
 HEAP32[sp + 32 >> 2] = 0;
 HEAP32[sp + 40 >> 2] = 0;
 HEAP32[sp + 16 >> 2] = a;
 HEAPF32[sp + 24 >> 2] = b;
 vars2$0(sp);
 a = HEAP32[sp + 16 >> 2] | 0;
 b = +HEAPF32[sp + 24 >> 2];
 STACKTOP = sp;
}
function vars3(x, y) {
 x = x | 0;
 y = +y;
 var a = 0, sp = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 40 | 0;
 HEAP32[sp + 24 >> 2] = 0;
 HEAP32[sp + 32 >> 2] = 0;
 HEAP32[sp + 16 >> 2] = a;
 HEAP32[sp + 0 >> 2] = x;
 HEAPF32[sp + 8 >> 2] = y;
 vars3$1(sp);
 a = HEAP32[sp + 16 >> 2] | 0;
 HEAP32[sp + 24 >> 2] = 0;
 HEAP32[sp + 32 >> 2] = 0;
 HEAP32[sp + 16 >> 2] = a;
 HEAPF32[sp + 8 >> 2] = y;
 HEAP32[sp + 0 >> 2] = x;
 vars3$0(sp);
 a = HEAP32[sp + 16 >> 2] | 0;
 STACKTOP = sp;
}
function vars4(x, y) {
 x = x | 0;
 y = +y;
 var a = 0, b = +0, sp = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 48 | 0;
 a = x + y;
 HEAP32[sp + 32 >> 2] = 0;
 HEAP32[sp + 40 >> 2] = 0;
 HEAPF32[sp + 8 >> 2] = y;
 HEAP32[sp + 0 >> 2] = x;
 HEAP32[sp + 16 >> 2] = a;
 HEAPF32[sp + 24 >> 2] = b;
 vars4$1(sp);
 b = +HEAPF32[sp + 24 >> 2];
 a = HEAP32[sp + 16 >> 2] | 0;
 HEAP32[sp + 32 >> 2] = 0;
 HEAP32[sp + 40 >> 2] = 0;
 HEAP32[sp + 16 >> 2] = a;
 HEAP32[sp + 0 >> 2] = x;
 HEAPF32[sp + 24 >> 2] = b;
 vars4$0(sp);
 a = HEAP32[sp + 16 >> 2] | 0;
 b = +HEAPF32[sp + 24 >> 2];
 STACKTOP = sp;
}
function vars_w_stack(x, y) {
 x = x | 0;
 y = +y;
 var a = 0, b = +0, sp = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 72 | 0;
 a = x + y;
 HEAP32[sp + 56 >> 2] = 0;
 HEAP32[sp + 64 >> 2] = 0;
 HEAPF32[sp + 24 >> 2] = y;
 HEAP32[sp + 16 >> 2] = x;
 HEAP32[sp + 32 >> 2] = a;
 HEAPF32[sp + 40 >> 2] = b;
 vars_w_stack$1(sp);
 b = +HEAPF32[sp + 40 >> 2];
 a = HEAP32[sp + 32 >> 2] | 0;
 HEAP32[sp + 56 >> 2] = 0;
 HEAP32[sp + 64 >> 2] = 0;
 HEAP32[sp + 32 >> 2] = a;
 HEAPF32[sp + 40 >> 2] = b;
 vars_w_stack$0(sp);
 a = HEAP32[sp + 32 >> 2] | 0;
 b = +HEAPF32[sp + 40 >> 2];
}
function lin$0(sp) {
 sp = sp | 0;
 c(13);
 c(14);
 c(15);
 c(16);
 c(17);
 c(18);
 c(19);
 c(20);
}
function lin$1(sp) {
 sp = sp | 0;
 c(5);
 c(6);
 c(7);
 c(8);
 c(9);
 c(10);
 c(11);
 c(12);
}
function lin2$0(sp) {
 sp = sp | 0;
 c(13);
 c(14);
 c(15);
 c(16);
 c(17);
 c(18);
 c(19);
 c(20);
}
function lin2$1(sp) {
 sp = sp | 0;
 c(5);
 c(6);
 c(7);
 c(8);
 c(9);
 c(10);
 c(11);
 c(12);
}
function lin3$0(sp) {
 sp = sp | 0;
 OL : do {
  c(14);
  c(15);
  c(16);
  c(17);
  c(18);
  c(19);
  c(20);
  HEAP32[sp + 0 >> 2] = 6;
  HEAP32[sp + 8 >> 2] = 10;
  break OL;
 } while (0);
}
function lin3$1(sp) {
 sp = sp | 0;
 c(6);
 c(7);
 c(8);
 c(9);
 c(10);
 c(11);
 c(12);
 c(13);
}
function lin4$0(sp) {
 sp = sp | 0;
 OL : do {
  c(13);
  c(14);
  c(15);
  c(16);
  c(17);
  c(18);
  c(19);
  c(20);
  HEAP32[sp + 0 >> 2] = 1;
  break OL;
 } while (0);
}
function lin4$1(sp) {
 sp = sp | 0;
 c(5);
 c(6);
 c(7);
 c(8);
 c(9);
 c(10);
 c(11);
 c(12);
}
function lin5$0(sp) {
 sp = sp | 0;
 OL : do {
  c(13);
  c(14);
  c(15);
  c(16);
  c(17);
  c(18);
  c(19);
  c(20);
  HEAP32[sp + 0 >> 2] = 3;
  break OL;
 } while (0);
}
function lin5$1(sp) {
 sp = sp | 0;
 c(5);
 c(6);
 c(7);
 c(8);
 c(9);
 c(10);
 c(11);
 c(12);
}
function mix$0(sp) {
 sp = sp | 0;
 OL : do {
  c(16);
  c(17);
  HEAP32[sp + 0 >> 2] = 2;
  HEAP32[sp + 8 >> 2] = 2;
  break OL;
  c(18);
  HEAP32[sp + 0 >> 2] = 1;
  break OL;
  while (1) {
   break;
  }
  inner : while (1) {
   break inner;
  }
  c(19);
  HEAP32[sp + 0 >> 2] = 3;
  break OL;
  c(20);
  HEAP32[sp + 0 >> 2] = 4;
  HEAP32[sp + 8 >> 2] = 3;
  break OL;
 } while (0);
}
function mix$1(sp) {
 sp = sp | 0;
 c(8);
 c(9);
 c(10);
 c(11);
 c(12);
 c(13);
 c(14);
 c(15);
}
function vars$0(sp) {
 sp = sp | 0;
 var x = 0, y = +0;
 y = +HEAPF32[sp + 8 >> 2];
 x = HEAP32[sp + 0 >> 2] | 0;
 c(5 + (x + y));
 c(6 + y * x);
 c(7 + (x + y));
 c(8 + y * x);
}
function vars$1(sp) {
 sp = sp | 0;
 var x = 0, y = +0;
 y = +HEAPF32[sp + 8 >> 2];
 x = HEAP32[sp + 0 >> 2] | 0;
 c(1 + (x + y));
 c(2 + y * x);
 c(3 + (x + y));
 c(4 + y * x);
}
function vars2$0(sp) {
 sp = sp | 0;
 var a = 0, b = +0;
 b = +HEAPF32[sp + 24 >> 2];
 a = HEAP32[sp + 16 >> 2] | 0;
 a = c(3 + a);
 b = c(4 + b);
 a = c(5 + a);
 b = c(6 + b);
 HEAP32[sp + 16 >> 2] = a;
 HEAPF32[sp + 24 >> 2] = b;
}
function vars3$0(sp) {
 sp = sp | 0;
 var a = 0, y = +0, x = 0;
 x = HEAP32[sp + 0 >> 2] | 0;
 y = +HEAPF32[sp + 8 >> 2];
 a = HEAP32[sp + 16 >> 2] | 0;
 a = c(4 + y * x);
 a = c(5 + a);
 a = c(6 + y * x);
 a = c(7 + a);
 HEAP32[sp + 16 >> 2] = a;
}
function vars3$1(sp) {
 sp = sp | 0;
 var a = 0, x = 0, y = +0;
 y = +HEAPF32[sp + 8 >> 2];
 x = HEAP32[sp + 0 >> 2] | 0;
 a = HEAP32[sp + 16 >> 2] | 0;
 a = x + y;
 a = c(1 + a);
 a = c(2 + y * x);
 a = c(3 + a);
 HEAP32[sp + 16 >> 2] = a;
}
function vars4$0(sp) {
 sp = sp | 0;
 var a = 0, x = 0, b = +0;
 b = +HEAPF32[sp + 24 >> 2];
 x = HEAP32[sp + 0 >> 2] | 0;
 a = HEAP32[sp + 16 >> 2] | 0;
 a = c(4 + a);
 a = c(5 + a);
 a = c(6 + a);
 b = c(7 + a + x);
 HEAP32[sp + 16 >> 2] = a;
 HEAPF32[sp + 24 >> 2] = b;
}
function vars4$1(sp) {
 sp = sp | 0;
 var y = +0, x = 0, a = 0, b = +0;
 b = +HEAPF32[sp + 24 >> 2];
 a = HEAP32[sp + 16 >> 2] | 0;
 x = HEAP32[sp + 0 >> 2] | 0;
 y = +HEAPF32[sp + 8 >> 2];
 b = y * x;
 a = c(1 + a);
 a = c(2 + a);
 a = c(3 + a);
 HEAPF32[sp + 24 >> 2] = b;
 HEAP32[sp + 16 >> 2] = a;
}
function vars_w_stack$0(sp) {
 sp = sp | 0;
 var a = 0, b = +0;
 b = +HEAPF32[sp + 40 >> 2];
 a = HEAP32[sp + 32 >> 2] | 0;
 a = c(4 + a);
 a = c(5 + a);
 a = c(6 + a);
 b = c(7 + a);
 STACKTOP = sp;
 HEAP32[sp + 32 >> 2] = a;
 HEAPF32[sp + 40 >> 2] = b;
}
function vars_w_stack$1(sp) {
 sp = sp | 0;
 var y = +0, x = 0, a = 0, b = +0;
 b = +HEAPF32[sp + 40 >> 2];
 a = HEAP32[sp + 32 >> 2] | 0;
 x = HEAP32[sp + 16 >> 2] | 0;
 y = +HEAPF32[sp + 24 >> 2];
 b = y * x;
 a = c(1 + a);
 a = c(2 + a);
 a = c(3 + a);
 HEAPF32[sp + 40 >> 2] = b;
 HEAP32[sp + 32 >> 2] = a;
}

