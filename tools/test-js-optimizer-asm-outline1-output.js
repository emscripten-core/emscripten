function lin() {
 var sp = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 72 | 0;
 c(1);
 c(2);
 c(3);
 c(4);
 HEAP32[sp + 16 >> 2] = 0;
 HEAP32[sp + 20 >> 2] = 0;
 sp = lin$1(sp) | 0;
 HEAP32[sp + 8 >> 2] = 0;
 HEAP32[sp + 12 >> 2] = 0;
 sp = lin$0(sp) | 0;
 STACKTOP = sp;
}
function lin2() {
 var sp = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 72 | 0;
 while (1) {
  c(1);
  c(2);
  c(3);
  c(4);
  HEAP32[sp + 16 >> 2] = 0;
  HEAP32[sp + 20 >> 2] = 0;
  sp = lin2$1(sp) | 0;
  HEAP32[sp + 8 >> 2] = 0;
  HEAP32[sp + 12 >> 2] = 0;
  sp = lin2$0(sp) | 0;
 }
 STACKTOP = sp;
}
function lin3() {
 var sp = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 72 | 0;
 while (1) {
  c(1);
  c(2);
  c(3);
  c(4);
  c(5);
  HEAP32[sp + 16 >> 2] = 0;
  HEAP32[sp + 20 >> 2] = 0;
  sp = lin3$1(sp) | 0;
  HEAP32[sp + 8 >> 2] = 0;
  HEAP32[sp + 12 >> 2] = 0;
  sp = lin3$0(sp) | 0;
  tempValue = HEAP32[sp + 8 >> 2] | 0;
  tempInt = HEAP32[sp + 12 >> 2] | 0;
  tempDouble = +HEAPF32[sp + 12 >> 2];
  HEAP32[sp + 8 >> 2] = 0;
  HEAP32[sp + 12 >> 2] = 0;
  if ((tempValue | 0) == 6) {
   STACKTOP = sp;
   return tempInt | 0;
  }
 }
 STACKTOP = sp;
 return 20;
}
function lin4() {
 var sp = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 72 | 0;
 while (1) {
  c(1);
  c(2);
  c(3);
  c(4);
  HEAP32[sp + 16 >> 2] = 0;
  HEAP32[sp + 20 >> 2] = 0;
  sp = lin4$1(sp) | 0;
  HEAP32[sp + 8 >> 2] = 0;
  HEAP32[sp + 12 >> 2] = 0;
  sp = lin4$0(sp) | 0;
  tempValue = HEAP32[sp + 8 >> 2] | 0;
  tempInt = HEAP32[sp + 12 >> 2] | 0;
  tempDouble = +HEAPF32[sp + 12 >> 2];
  HEAP32[sp + 8 >> 2] = 0;
  HEAP32[sp + 12 >> 2] = 0;
  if ((tempValue | 0) == 1) {
   break;
  }
 }
 STACKTOP = sp;
 return 20;
}
function lin5() {
 var sp = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 72 | 0;
 while (1) {
  c(1);
  c(2);
  c(3);
  c(4);
  HEAP32[sp + 16 >> 2] = 0;
  HEAP32[sp + 20 >> 2] = 0;
  sp = lin5$1(sp) | 0;
  HEAP32[sp + 8 >> 2] = 0;
  HEAP32[sp + 12 >> 2] = 0;
  sp = lin5$0(sp) | 0;
  tempValue = HEAP32[sp + 8 >> 2] | 0;
  tempInt = HEAP32[sp + 12 >> 2] | 0;
  tempDouble = +HEAPF32[sp + 12 >> 2];
  HEAP32[sp + 8 >> 2] = 0;
  HEAP32[sp + 12 >> 2] = 0;
  if ((tempValue | 0) == 3) {
   continue;
  }
 }
 STACKTOP = sp;
 return 20;
}
function mix() {
 var sp = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 88 | 0;
 main : while (1) {
  c(1);
  c(2);
  c(3);
  c(4);
  c(5);
  c(6);
  c(7);
  HEAP32[sp + 16 >> 2] = 0;
  HEAP32[sp + 20 >> 2] = 0;
  sp = mix$1(sp) | 0;
  HEAP32[sp + 8 >> 2] = 0;
  HEAP32[sp + 12 >> 2] = 0;
  sp = mix$0(sp) | 0;
  tempValue = HEAP32[sp + 8 >> 2] | 0;
  tempInt = HEAP32[sp + 12 >> 2] | 0;
  tempDouble = +HEAPF32[sp + 12 >> 2];
  HEAP32[sp + 8 >> 2] = 0;
  HEAP32[sp + 12 >> 2] = 0;
  if ((tempValue | 0) == 1) {
   break;
  }
  if ((tempValue | 0) == 2) {
   switch (tempInt | 0) {
   case 2:
    {
     break main;
    }
   }
  }
  if ((tempValue | 0) == 3) {
   continue;
  }
  if ((tempValue | 0) == 4) {
   switch (tempInt | 0) {
   case 3:
    {
     continue main;
    }
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
 STACKTOP = STACKTOP + 88 | 0;
 HEAP32[sp + 32 >> 2] = 0;
 HEAP32[sp + 36 >> 2] = 0;
 HEAP32[sp + 8 >> 2] = x;
 HEAPF32[sp + 16 >> 2] = y;
 sp = vars$1(sp) | 0;
 HEAP32[sp + 24 >> 2] = 0;
 HEAP32[sp + 28 >> 2] = 0;
 HEAP32[sp + 8 >> 2] = x;
 HEAPF32[sp + 16 >> 2] = y;
 sp = vars$0(sp) | 0;
 STACKTOP = sp;
}
function vars2(x, y) {
 x = x | 0;
 y = +y;
 var a = 0, b = +0, sp = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 88 | 0;
 a = x + y;
 b = y * x;
 a = c(1 + a);
 b = c(2 + b);
 HEAP32[sp + 40 >> 2] = 0;
 HEAP32[sp + 44 >> 2] = 0;
 HEAP32[sp + 24 >> 2] = a;
 HEAPF32[sp + 32 >> 2] = b;
 sp = vars2$0(sp) | 0;
 a = HEAP32[sp + 24 >> 2] | 0;
 b = +HEAPF32[sp + 32 >> 2];
 STACKTOP = sp;
}
function vars3(x, y) {
 x = x | 0;
 y = +y;
 var a = 0, sp = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 96 | 0;
 HEAP32[sp + 40 >> 2] = 0;
 HEAP32[sp + 44 >> 2] = 0;
 HEAP32[sp + 24 >> 2] = a;
 HEAP32[sp + 8 >> 2] = x;
 HEAPF32[sp + 16 >> 2] = y;
 sp = vars3$1(sp) | 0;
 a = HEAP32[sp + 24 >> 2] | 0;
 HEAP32[sp + 32 >> 2] = 0;
 HEAP32[sp + 36 >> 2] = 0;
 HEAP32[sp + 24 >> 2] = a;
 HEAPF32[sp + 16 >> 2] = y;
 HEAP32[sp + 8 >> 2] = x;
 sp = vars3$0(sp) | 0;
 a = HEAP32[sp + 24 >> 2] | 0;
 STACKTOP = sp;
}
function vars4(x, y) {
 x = x | 0;
 y = +y;
 var a = 0, b = +0, sp = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 104 | 0;
 a = x + y;
 HEAP32[sp + 48 >> 2] = 0;
 HEAP32[sp + 52 >> 2] = 0;
 HEAPF32[sp + 16 >> 2] = y;
 HEAP32[sp + 8 >> 2] = x;
 HEAP32[sp + 24 >> 2] = a;
 HEAPF32[sp + 32 >> 2] = b;
 sp = vars4$1(sp) | 0;
 b = +HEAPF32[sp + 32 >> 2];
 a = HEAP32[sp + 24 >> 2] | 0;
 HEAP32[sp + 40 >> 2] = 0;
 HEAP32[sp + 44 >> 2] = 0;
 HEAP32[sp + 24 >> 2] = a;
 HEAP32[sp + 8 >> 2] = x;
 HEAPF32[sp + 32 >> 2] = b;
 sp = vars4$0(sp) | 0;
 a = HEAP32[sp + 24 >> 2] | 0;
 b = +HEAPF32[sp + 32 >> 2];
 STACKTOP = sp;
}
function vars_w_stack(x, y) {
 x = x | 0;
 y = +y;
 var a = 0, b = +0, sp = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 144 | 0;
 a = x + y;
 HEAP32[sp + 72 >> 2] = 0;
 HEAP32[sp + 76 >> 2] = 0;
 HEAPF32[sp + 32 >> 2] = y;
 HEAP32[sp + 24 >> 2] = x;
 HEAP32[sp + 40 >> 2] = a;
 HEAPF32[sp + 48 >> 2] = b;
 sp = vars_w_stack$1(sp) | 0;
 b = +HEAPF32[sp + 48 >> 2];
 a = HEAP32[sp + 40 >> 2] | 0;
 HEAP32[sp + 64 >> 2] = 0;
 HEAP32[sp + 68 >> 2] = 0;
 HEAP32[sp + 40 >> 2] = a;
 HEAPF32[sp + 48 >> 2] = b;
 sp = vars_w_stack$0(sp) | 0;
 a = HEAP32[sp + 40 >> 2] | 0;
 b = +HEAPF32[sp + 48 >> 2];
}
function chain() {
 var helper$0 = 0, sp = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 176 | 0;
 helper$0 = 1;
 HEAP32[sp + 56 >> 2] = 0;
 HEAP32[sp + 60 >> 2] = 0;
 HEAP32[sp + 8 >> 2] = helper$0;
 sp = chain$5(sp) | 0;
 helper$0 = HEAP32[sp + 8 >> 2] | 0;
 HEAP32[sp + 48 >> 2] = 0;
 HEAP32[sp + 52 >> 2] = 0;
 HEAP32[sp + 8 >> 2] = helper$0;
 sp = chain$4(sp) | 0;
 helper$0 = HEAP32[sp + 8 >> 2] | 0;
 HEAP32[sp + 40 >> 2] = 0;
 HEAP32[sp + 44 >> 2] = 0;
 HEAP32[sp + 8 >> 2] = helper$0;
 sp = chain$3(sp) | 0;
 helper$0 = HEAP32[sp + 8 >> 2] | 0;
 HEAP32[sp + 32 >> 2] = 0;
 HEAP32[sp + 36 >> 2] = 0;
 HEAP32[sp + 8 >> 2] = helper$0;
 sp = chain$2(sp) | 0;
 helper$0 = HEAP32[sp + 8 >> 2] | 0;
 HEAP32[sp + 24 >> 2] = 0;
 HEAP32[sp + 28 >> 2] = 0;
 HEAP32[sp + 8 >> 2] = helper$0;
 sp = chain$1(sp) | 0;
 helper$0 = HEAP32[sp + 8 >> 2] | 0;
 HEAP32[sp + 16 >> 2] = 0;
 HEAP32[sp + 20 >> 2] = 0;
 HEAP32[sp + 8 >> 2] = helper$0;
 sp = chain$0(sp) | 0;
 helper$0 = HEAP32[sp + 8 >> 2] | 0;
 STACKTOP = sp;
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
 return sp | 0;
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
 return sp | 0;
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
 return sp | 0;
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
 return sp | 0;
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
  HEAP32[sp + 8 >> 2] = 6;
  HEAP32[sp + 12 >> 2] = 10;
  break OL;
 } while (0);
 return sp | 0;
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
 return sp | 0;
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
  HEAP32[sp + 8 >> 2] = 1;
  break OL;
 } while (0);
 return sp | 0;
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
 return sp | 0;
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
  HEAP32[sp + 8 >> 2] = 3;
  break OL;
 } while (0);
 return sp | 0;
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
 return sp | 0;
}
function mix$0(sp) {
 sp = sp | 0;
 OL : do {
  c(16);
  c(17);
  HEAP32[sp + 8 >> 2] = 2;
  HEAP32[sp + 12 >> 2] = 2;
  break OL;
  c(18);
  HEAP32[sp + 8 >> 2] = 1;
  break OL;
  while (1) {
   break;
  }
  inner : while (1) {
   break inner;
  }
  c(19);
  HEAP32[sp + 8 >> 2] = 3;
  break OL;
  c(20);
  HEAP32[sp + 8 >> 2] = 4;
  HEAP32[sp + 12 >> 2] = 3;
  break OL;
 } while (0);
 return sp | 0;
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
 return sp | 0;
}
function vars$0(sp) {
 sp = sp | 0;
 var x = 0, y = +0;
 y = +HEAPF32[sp + 16 >> 2];
 x = HEAP32[sp + 8 >> 2] | 0;
 c(5 + (x + y));
 c(6 + y * x);
 c(7 + (x + y));
 c(8 + y * x);
 return sp | 0;
}
function vars$1(sp) {
 sp = sp | 0;
 var x = 0, y = +0;
 y = +HEAPF32[sp + 16 >> 2];
 x = HEAP32[sp + 8 >> 2] | 0;
 c(1 + (x + y));
 c(2 + y * x);
 c(3 + (x + y));
 c(4 + y * x);
 return sp | 0;
}
function vars2$0(sp) {
 sp = sp | 0;
 var a = 0, b = +0;
 b = +HEAPF32[sp + 32 >> 2];
 a = HEAP32[sp + 24 >> 2] | 0;
 a = c(3 + a);
 b = c(4 + b);
 a = c(5 + a);
 b = c(6 + b);
 HEAP32[sp + 24 >> 2] = a;
 HEAPF32[sp + 32 >> 2] = b;
 return sp | 0;
}
function vars3$0(sp) {
 sp = sp | 0;
 var a = 0, y = +0, x = 0;
 x = HEAP32[sp + 8 >> 2] | 0;
 y = +HEAPF32[sp + 16 >> 2];
 a = HEAP32[sp + 24 >> 2] | 0;
 a = c(4 + y * x);
 a = c(5 + a);
 a = c(6 + y * x);
 a = c(7 + a);
 HEAP32[sp + 24 >> 2] = a;
 return sp | 0;
}
function vars3$1(sp) {
 sp = sp | 0;
 var a = 0, x = 0, y = +0;
 y = +HEAPF32[sp + 16 >> 2];
 x = HEAP32[sp + 8 >> 2] | 0;
 a = HEAP32[sp + 24 >> 2] | 0;
 a = x + y;
 a = c(1 + a);
 a = c(2 + y * x);
 a = c(3 + a);
 HEAP32[sp + 24 >> 2] = a;
 return sp | 0;
}
function vars4$0(sp) {
 sp = sp | 0;
 var a = 0, x = 0, b = +0;
 b = +HEAPF32[sp + 32 >> 2];
 x = HEAP32[sp + 8 >> 2] | 0;
 a = HEAP32[sp + 24 >> 2] | 0;
 a = c(4 + a);
 a = c(5 + a);
 a = c(6 + a);
 b = c(7 + a + x);
 HEAP32[sp + 24 >> 2] = a;
 HEAPF32[sp + 32 >> 2] = b;
 return sp | 0;
}
function vars4$1(sp) {
 sp = sp | 0;
 var y = +0, x = 0, a = 0, b = +0;
 b = +HEAPF32[sp + 32 >> 2];
 a = HEAP32[sp + 24 >> 2] | 0;
 x = HEAP32[sp + 8 >> 2] | 0;
 y = +HEAPF32[sp + 16 >> 2];
 b = y * x;
 a = c(1 + a);
 a = c(2 + a);
 a = c(3 + a);
 HEAPF32[sp + 32 >> 2] = b;
 HEAP32[sp + 24 >> 2] = a;
 return sp | 0;
}
function vars_w_stack$0(sp) {
 sp = sp | 0;
 var a = 0, b = +0;
 b = +HEAPF32[sp + 48 >> 2];
 a = HEAP32[sp + 40 >> 2] | 0;
 a = c(4 + a);
 a = c(5 + a);
 a = c(6 + a);
 b = c(7 + a);
 STACKTOP = sp;
 HEAP32[sp + 40 >> 2] = a;
 HEAPF32[sp + 48 >> 2] = b;
 return sp | 0;
}
function vars_w_stack$1(sp) {
 sp = sp | 0;
 var y = +0, x = 0, a = 0, b = +0;
 b = +HEAPF32[sp + 48 >> 2];
 a = HEAP32[sp + 40 >> 2] | 0;
 x = HEAP32[sp + 24 >> 2] | 0;
 y = +HEAPF32[sp + 32 >> 2];
 b = y * x;
 a = c(1 + a);
 a = c(2 + a);
 a = c(3 + a);
 HEAPF32[sp + 48 >> 2] = b;
 HEAP32[sp + 40 >> 2] = a;
 return sp | 0;
}
function chain$0(sp) {
 sp = sp | 0;
 var helper$0 = 0;
 helper$0 = HEAP32[sp + 8 >> 2] | 0;
 if (helper$0 ? x == 11 : 0) {
  helper$0 = 0;
  print(11);
 }
 if (helper$0 ? x == 12 : 0) {
  helper$0 = 0;
  print(12);
 }
 if (helper$0) {
  helper$0 = 0;
  print(99);
 }
 HEAP32[sp + 8 >> 2] = helper$0;
 return sp | 0;
}
function chain$1(sp) {
 sp = sp | 0;
 var helper$0 = 0;
 helper$0 = HEAP32[sp + 8 >> 2] | 0;
 if (helper$0 ? x == 9 : 0) {
  helper$0 = 0;
  print(9);
 }
 if (helper$0 ? x == 10 : 0) {
  helper$0 = 0;
  print(10);
 }
 HEAP32[sp + 8 >> 2] = helper$0;
 return sp | 0;
}
function chain$2(sp) {
 sp = sp | 0;
 var helper$0 = 0;
 helper$0 = HEAP32[sp + 8 >> 2] | 0;
 if (helper$0 ? x == 7 : 0) {
  helper$0 = 0;
  print(7);
 }
 if (helper$0 ? x == 8 : 0) {
  helper$0 = 0;
  print(8);
 }
 HEAP32[sp + 8 >> 2] = helper$0;
 return sp | 0;
}
function chain$3(sp) {
 sp = sp | 0;
 var helper$0 = 0;
 helper$0 = HEAP32[sp + 8 >> 2] | 0;
 if (helper$0 ? x == 5 : 0) {
  helper$0 = 0;
  print(5);
 }
 if (helper$0 ? x == 6 : 0) {
  helper$0 = 0;
  print(6);
 }
 HEAP32[sp + 8 >> 2] = helper$0;
 return sp | 0;
}
function chain$4(sp) {
 sp = sp | 0;
 var helper$0 = 0;
 helper$0 = HEAP32[sp + 8 >> 2] | 0;
 if (helper$0 ? x == 3 : 0) {
  helper$0 = 0;
  print(3);
 }
 if (helper$0 ? x == 4 : 0) {
  helper$0 = 0;
  print(4);
 }
 HEAP32[sp + 8 >> 2] = helper$0;
 return sp | 0;
}
function chain$5(sp) {
 sp = sp | 0;
 var helper$0 = 0;
 helper$0 = HEAP32[sp + 8 >> 2] | 0;
 if (helper$0 ? x == 1 : 0) {
  helper$0 = 0;
  print(1);
 }
 if (helper$0 ? x == 2 : 0) {
  helper$0 = 0;
  print(2);
 }
 HEAP32[sp + 8 >> 2] = helper$0;
 return sp | 0;
}

