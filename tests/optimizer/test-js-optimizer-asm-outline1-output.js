function lin() {
 var sp = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 144 | 0;
 c(1);
 c(2);
 c(3);
 HEAP32[sp + 24 >> 2] = 0;
 HEAP32[sp + 28 >> 2] = 0;
 lin$1(sp);
 HEAP32[sp + 16 >> 2] = 0;
 HEAP32[sp + 20 >> 2] = 0;
 lin$0(sp);
 STACKTOP = sp;
}
function lin2() {
 var sp = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 144 | 0;
 while (1) {
  c(1);
  c(2);
  HEAP32[sp + 24 >> 2] = 0;
  HEAP32[sp + 28 >> 2] = 0;
  lin2$1(sp);
  HEAP32[sp + 16 >> 2] = 0;
  HEAP32[sp + 20 >> 2] = 0;
  lin2$0(sp);
 }
 STACKTOP = sp;
}
function lin3() {
 var sp = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 160 | 0;
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
  HEAP32[sp + 16 >> 2] = 0;
  HEAP32[sp + 20 >> 2] = 0;
  lin3$0(sp);
  tempValue = HEAP32[sp + 16 >> 2] | 0;
  tempInt = HEAP32[sp + 20 >> 2] | 0;
  tempDouble = +HEAPF32[sp + 20 >> 2];
  HEAP32[sp + 16 >> 2] = 0;
  HEAP32[sp + 20 >> 2] = 0;
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
 STACKTOP = STACKTOP + 160 | 0;
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
  HEAP32[sp + 16 >> 2] = 0;
  HEAP32[sp + 20 >> 2] = 0;
  lin4$0(sp);
  tempValue = HEAP32[sp + 16 >> 2] | 0;
  tempInt = HEAP32[sp + 20 >> 2] | 0;
  tempDouble = +HEAPF32[sp + 20 >> 2];
  HEAP32[sp + 16 >> 2] = 0;
  HEAP32[sp + 20 >> 2] = 0;
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
 STACKTOP = STACKTOP + 160 | 0;
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
  HEAP32[sp + 16 >> 2] = 0;
  HEAP32[sp + 20 >> 2] = 0;
  lin5$0(sp);
  tempValue = HEAP32[sp + 16 >> 2] | 0;
  tempInt = HEAP32[sp + 20 >> 2] | 0;
  tempDouble = +HEAPF32[sp + 20 >> 2];
  HEAP32[sp + 16 >> 2] = 0;
  HEAP32[sp + 20 >> 2] = 0;
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
 STACKTOP = STACKTOP + 176 | 0;
 main : while (1) {
  HEAP32[sp + 24 >> 2] = 0;
  HEAP32[sp + 28 >> 2] = 0;
  mix$1(sp);
  HEAP32[sp + 16 >> 2] = 0;
  HEAP32[sp + 20 >> 2] = 0;
  mix$0(sp);
  tempValue = HEAP32[sp + 16 >> 2] | 0;
  tempInt = HEAP32[sp + 20 >> 2] | 0;
  tempDouble = +HEAPF32[sp + 20 >> 2];
  HEAP32[sp + 16 >> 2] = 0;
  HEAP32[sp + 20 >> 2] = 0;
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
 STACKTOP = STACKTOP + 144 | 0;
 c(1 + (x + y));
 c(2 + y * x);
 c(3 + (x + y));
 c(4 + y * x);
 c(5 + (x + y));
 HEAP32[sp + 16 >> 2] = x;
 HEAPF32[sp + 24 >> 2] = y;
 HEAP32[sp + 32 >> 2] = 0;
 HEAP32[sp + 36 >> 2] = 0;
 vars$0(sp);
 STACKTOP = sp;
}
function vars2(x, y) {
 x = x | 0;
 y = +y;
 var a = 0, b = +0, sp = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 160 | 0;
 a = x + y;
 b = y * x;
 a = c(1 + a);
 b = c(2 + b);
 a = c(3 + a);
 HEAP32[sp + 32 >> 2] = a;
 HEAPF32[sp + 40 >> 2] = b;
 HEAP32[sp + 48 >> 2] = 0;
 HEAP32[sp + 52 >> 2] = 0;
 vars2$0(sp);
 a = HEAP32[sp + 32 >> 2] | 0;
 b = +HEAPF32[sp + 40 >> 2];
 STACKTOP = sp;
}
function vars3(x, y) {
 x = x | 0;
 y = +y;
 var a = 0, sp = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 160 | 0;
 a = x + y;
 a = c(1 + a);
 a = c(2 + y * x);
 a = c(3 + a);
 a = c(4 + y * x);
 HEAP32[sp + 16 >> 2] = x;
 HEAPF32[sp + 24 >> 2] = y;
 HEAP32[sp + 32 >> 2] = a;
 HEAP32[sp + 40 >> 2] = 0;
 HEAP32[sp + 44 >> 2] = 0;
 vars3$0(sp);
 a = HEAP32[sp + 32 >> 2] | 0;
 STACKTOP = sp;
}
function vars4(x, y) {
 x = x | 0;
 y = +y;
 var a = 0, b = +0, sp = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 176 | 0;
 a = x + y;
 b = y * x;
 a = c(1 + a);
 a = c(2 + a);
 a = c(3 + a);
 a = c(4 + a);
 HEAP32[sp + 16 >> 2] = x;
 HEAP32[sp + 32 >> 2] = a;
 HEAPF32[sp + 40 >> 2] = b;
 HEAP32[sp + 48 >> 2] = 0;
 HEAP32[sp + 52 >> 2] = 0;
 vars4$0(sp);
 a = HEAP32[sp + 32 >> 2] | 0;
 b = +HEAPF32[sp + 40 >> 2];
 STACKTOP = sp;
}
function vars_w_stack(x, y) {
 x = x | 0;
 y = +y;
 var a = 0, b = +0, sp = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 224 | 0;
 a = x + y;
 b = y * x;
 a = c(1 + a);
 a = c(2 + a);
 a = c(3 + a);
 HEAP32[sp + 48 >> 2] = a;
 HEAPF32[sp + 56 >> 2] = b;
 HEAP32[sp + 72 >> 2] = 0;
 HEAP32[sp + 76 >> 2] = 0;
 vars_w_stack$0(sp);
 a = HEAP32[sp + 48 >> 2] | 0;
 b = +HEAPF32[sp + 56 >> 2];
}
function chain() {
 var helper$0 = 0, sp = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 480 | 0;
 helper$0 = 1;
 if (helper$0) {
  helper$0 = 0;
  if (x == 1) {
   print(1);
  } else {
   helper$0 = 1;
  }
 }
 if (helper$0) {
  helper$0 = 0;
  if (x == 2) {
   print(2);
  } else {
   helper$0 = 1;
  }
 }
 HEAP32[sp + 16 >> 2] = helper$0;
 HEAP32[sp + 56 >> 2] = 0;
 HEAP32[sp + 60 >> 2] = 0;
 chain$4(sp);
 helper$0 = HEAP32[sp + 16 >> 2] | 0;
 HEAP32[sp + 16 >> 2] = helper$0;
 HEAP32[sp + 48 >> 2] = 0;
 HEAP32[sp + 52 >> 2] = 0;
 chain$3(sp);
 helper$0 = HEAP32[sp + 16 >> 2] | 0;
 HEAP32[sp + 16 >> 2] = helper$0;
 HEAP32[sp + 40 >> 2] = 0;
 HEAP32[sp + 44 >> 2] = 0;
 chain$2(sp);
 helper$0 = HEAP32[sp + 16 >> 2] | 0;
 HEAP32[sp + 16 >> 2] = helper$0;
 HEAP32[sp + 32 >> 2] = 0;
 HEAP32[sp + 36 >> 2] = 0;
 chain$1(sp);
 helper$0 = HEAP32[sp + 16 >> 2] | 0;
 HEAP32[sp + 16 >> 2] = helper$0;
 HEAP32[sp + 24 >> 2] = 0;
 HEAP32[sp + 28 >> 2] = 0;
 chain$0(sp);
 helper$0 = HEAP32[sp + 16 >> 2] | 0;
 STACKTOP = sp;
}
function switchh() {
 var helper$0 = 0, helper$1 = 0, sp = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 304 | 0;
 helper$0 = 1;
 helper$1 = x;
 if (helper$0) {
  helper$0 = 0;
  switch (helper$1 | 0) {
  case 0:
   {
    f(0);
    g();
    break;
   }
  default:
   {
    helper$0 = 1;
   }
  }
 }
 HEAP32[sp + 16 >> 2] = helper$0;
 HEAP32[sp + 24 >> 2] = helper$1;
 HEAP32[sp + 48 >> 2] = 0;
 HEAP32[sp + 52 >> 2] = 0;
 switchh$2(sp);
 helper$0 = HEAP32[sp + 16 >> 2] | 0;
 tempValue = HEAP32[sp + 48 >> 2] | 0;
 tempInt = HEAP32[sp + 52 >> 2] | 0;
 tempDouble = +HEAPF32[sp + 52 >> 2];
 HEAP32[sp + 48 >> 2] = 0;
 HEAP32[sp + 52 >> 2] = 0;
 if ((tempValue | 0) == 5) {
  STACKTOP = sp;
  return;
 }
 HEAP32[sp + 16 >> 2] = helper$0;
 HEAP32[sp + 24 >> 2] = helper$1;
 HEAP32[sp + 40 >> 2] = 0;
 HEAP32[sp + 44 >> 2] = 0;
 switchh$1(sp);
 helper$0 = HEAP32[sp + 16 >> 2] | 0;
 if (helper$0) {
  helper$0 = 0;
  HEAP32[sp + 24 >> 2] = helper$1;
  HEAP32[sp + 32 >> 2] = 0;
  HEAP32[sp + 36 >> 2] = 0;
  switchh$0(sp);
 }
 STACKTOP = sp;
}
function switchh2() {
 var helper$0 = 0, helper$1 = 0, sp = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 288 | 0;
 while (1) {
  helper$0 = 1;
  helper$1 = x;
  if (helper$0) {
   helper$0 = 0;
   switch (helper$1 | 0) {
   case 0:
    f(0);
    g();
    break;
   default:
    {
     helper$0 = 1;
    }
   }
  }
  HEAP32[sp + 16 >> 2] = helper$0;
  HEAP32[sp + 24 >> 2] = helper$1;
  HEAP32[sp + 48 >> 2] = 0;
  HEAP32[sp + 52 >> 2] = 0;
  switchh2$2(sp);
  helper$0 = HEAP32[sp + 16 >> 2] | 0;
  tempValue = HEAP32[sp + 48 >> 2] | 0;
  tempInt = HEAP32[sp + 52 >> 2] | 0;
  tempDouble = +HEAPF32[sp + 52 >> 2];
  HEAP32[sp + 48 >> 2] = 0;
  HEAP32[sp + 52 >> 2] = 0;
  if ((tempValue | 0) == 5) {
   STACKTOP = sp;
   return;
  }
  HEAP32[sp + 16 >> 2] = helper$0;
  HEAP32[sp + 24 >> 2] = helper$1;
  HEAP32[sp + 40 >> 2] = 0;
  HEAP32[sp + 44 >> 2] = 0;
  switchh2$1(sp);
  helper$0 = HEAP32[sp + 16 >> 2] | 0;
  if (helper$0) {
   helper$0 = 0;
   HEAP32[sp + 24 >> 2] = helper$1;
   HEAP32[sp + 32 >> 2] = 0;
   HEAP32[sp + 36 >> 2] = 0;
   switchh2$0(sp);
  }
 }
 STACKTOP = sp;
}
function stackSet(x1, x2, x3, x4, x5) {
 x1 = x1 | 0;
 x2 = x2 | 0;
 x3 = x3 | 0;
 x4 = x4 | 0;
 x5 = x5 | 0;
 var sp = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 304 | 0;
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
 HEAP32[sp + 88 >> 2] = 0;
 HEAP32[sp + 92 >> 2] = 0;
 stackSet$1(sp);
 HEAP32[sp + 80 >> 2] = 0;
 HEAP32[sp + 84 >> 2] = 0;
 stackSet$0(sp);
}
function linf(d) {
 d = +d;
 var sp = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 176 | 0;
 while (1) {
  HEAP32[sp + 32 >> 2] = 0;
  HEAP32[sp + 36 >> 2] = 0;
  linf$1(sp);
  HEAPF32[sp + 16 >> 2] = d;
  HEAP32[sp + 24 >> 2] = 0;
  HEAP32[sp + 28 >> 2] = 0;
  linf$0(sp);
  tempValue = HEAP32[sp + 24 >> 2] | 0;
  tempInt = HEAP32[sp + 28 >> 2] | 0;
  tempDouble = +HEAPF32[sp + 28 >> 2];
  HEAP32[sp + 24 >> 2] = 0;
  HEAP32[sp + 28 >> 2] = 0;
  if ((tempValue | 0) == 7) {
   STACKTOP = sp;
   return +tempDouble;
  }
 }
 STACKTOP = sp;
 return +d;
}
function leaveLabelsMagic() {
 var fakeLabel = 0, helper$0 = 0, sp = 0;
 sp = STACKTOP;
 STACKTOP = STACKTOP + 336 | 0;
 fakeLabel = 0;
 helper$0 = 1;
 if (helper$0) {
  helper$0 = 0;
  if (x) {
   c(1);
   c(2);
   c(3);
   c(4);
   c(5);
   c(6);
   HEAP32[sp + 16 >> 2] = fakeLabel;
   HEAP32[sp + 48 >> 2] = 0;
   HEAP32[sp + 52 >> 2] = 0;
   leaveLabelsMagic$2(sp);
   fakeLabel = HEAP32[sp + 16 >> 2] | 0;
  } else {
   helper$0 = 1;
  }
 }
 if (helper$0) {
  helper$0 = 0;
  if (!y) {} else {
   helper$0 = 1;
  }
 }
 if (helper$0) {
  helper$0 = 0;
  if (1) {
   HEAP32[sp + 40 >> 2] = 0;
   HEAP32[sp + 44 >> 2] = 0;
   leaveLabelsMagic$1(sp);
   HEAP32[sp + 32 >> 2] = 0;
   HEAP32[sp + 36 >> 2] = 0;
   leaveLabelsMagic$0(sp);
  } else {
   helper$0 = 1;
  }
 }
 if ((fakeLabel | 0) == 1) print(1);
 STACKTOP = sp;
}
function lin$0(sp) {
 sp = sp | 0;
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
}
function lin2$0(sp) {
 sp = sp | 0;
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
  HEAP32[sp + 16 >> 2] = 6;
  HEAP32[sp + 20 >> 2] = 10;
  break OL;
 } while (0);
}
function lin4$0(sp) {
 sp = sp | 0;
 OL : do {
  c(14);
  c(15);
  c(16);
  c(17);
  c(18);
  c(19);
  c(20);
  HEAP32[sp + 16 >> 2] = 1;
  break OL;
 } while (0);
}
function lin5$0(sp) {
 sp = sp | 0;
 OL : do {
  c(14);
  c(15);
  c(16);
  c(17);
  c(18);
  c(19);
  c(20);
  HEAP32[sp + 16 >> 2] = 3;
  break OL;
 } while (0);
}
function mix$0(sp) {
 sp = sp | 0;
 OL : do {
  HEAP32[sp + 16 >> 2] = 2;
  HEAP32[sp + 20 >> 2] = 2;
  break OL;
  c(18);
  HEAP32[sp + 16 >> 2] = 1;
  break OL;
  while (1) {
   break;
  }
  inner : while (1) {
   break inner;
  }
  c(19);
  HEAP32[sp + 16 >> 2] = 3;
  break OL;
  c(20);
  HEAP32[sp + 16 >> 2] = 4;
  HEAP32[sp + 20 >> 2] = 3;
  break OL;
 } while (0);
}
function mix$1(sp) {
 sp = sp | 0;
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
}
function vars$0(sp) {
 sp = sp | 0;
 var y = +0, x = 0;
 x = HEAP32[sp + 16 >> 2] | 0;
 y = +HEAPF32[sp + 24 >> 2];
 c(6 + y * x);
 c(7 + (x + y));
 c(8 + y * x);
}
function vars2$0(sp) {
 sp = sp | 0;
 var b = +0, a = 0;
 a = HEAP32[sp + 32 >> 2] | 0;
 b = +HEAPF32[sp + 40 >> 2];
 b = c(4 + b);
 a = c(5 + a);
 b = c(6 + b);
 HEAP32[sp + 32 >> 2] = a;
 HEAPF32[sp + 40 >> 2] = b;
}
function vars3$0(sp) {
 sp = sp | 0;
 var a = 0, y = +0, x = 0;
 x = HEAP32[sp + 16 >> 2] | 0;
 y = +HEAPF32[sp + 24 >> 2];
 a = HEAP32[sp + 32 >> 2] | 0;
 a = c(5 + a);
 a = c(6 + y * x);
 a = c(7 + a);
 HEAP32[sp + 32 >> 2] = a;
}
function vars4$0(sp) {
 sp = sp | 0;
 var a = 0, x = 0, b = +0;
 x = HEAP32[sp + 16 >> 2] | 0;
 a = HEAP32[sp + 32 >> 2] | 0;
 b = +HEAPF32[sp + 40 >> 2];
 a = c(5 + a);
 a = c(6 + a);
 b = c(7 + a + x);
 HEAP32[sp + 32 >> 2] = a;
 HEAPF32[sp + 40 >> 2] = b;
}
function vars_w_stack$0(sp) {
 sp = sp | 0;
 var a = 0, b = +0;
 a = HEAP32[sp + 48 >> 2] | 0;
 b = +HEAPF32[sp + 56 >> 2];
 a = c(4 + a);
 a = c(5 + a);
 a = c(6 + a);
 b = c(7 + a);
 STACKTOP = sp;
 HEAP32[sp + 48 >> 2] = a;
 HEAPF32[sp + 56 >> 2] = b;
}
function chain$0(sp) {
 sp = sp | 0;
 var helper$0 = 0;
 helper$0 = HEAP32[sp + 16 >> 2] | 0;
 if (helper$0) {
  helper$0 = 0;
  if (x == 12) {
   print(12);
  } else {
   helper$0 = 1;
  }
 }
 if (helper$0) {
  helper$0 = 0;
  if (1) {
   print(99);
  } else {
   helper$0 = 1;
  }
 }
 HEAP32[sp + 16 >> 2] = helper$0;
}
function chain$1(sp) {
 sp = sp | 0;
 var helper$0 = 0;
 helper$0 = HEAP32[sp + 16 >> 2] | 0;
 if (helper$0) {
  helper$0 = 0;
  if (x == 10) {
   print(10);
  } else {
   helper$0 = 1;
  }
 }
 if (helper$0) {
  helper$0 = 0;
  if (x == 11) {
   print(11);
  } else {
   helper$0 = 1;
  }
 }
 HEAP32[sp + 16 >> 2] = helper$0;
}
function chain$2(sp) {
 sp = sp | 0;
 var helper$0 = 0;
 helper$0 = HEAP32[sp + 16 >> 2] | 0;
 if (helper$0) {
  helper$0 = 0;
  if (x == 8) {
   print(8);
  } else {
   helper$0 = 1;
  }
 }
 if (helper$0) {
  helper$0 = 0;
  if (x == 9) {
   print(9);
  } else {
   helper$0 = 1;
  }
 }
 HEAP32[sp + 16 >> 2] = helper$0;
}
function chain$3(sp) {
 sp = sp | 0;
 var helper$0 = 0;
 helper$0 = HEAP32[sp + 16 >> 2] | 0;
 if (helper$0) {
  helper$0 = 0;
  if (x == 6) {
   print(6);
  } else {
   helper$0 = 1;
  }
 }
 if (helper$0) {
  helper$0 = 0;
  if (x == 7) {
   print(7);
  } else {
   helper$0 = 1;
  }
 }
 HEAP32[sp + 16 >> 2] = helper$0;
}
function chain$4(sp) {
 sp = sp | 0;
 var helper$0 = 0;
 helper$0 = HEAP32[sp + 16 >> 2] | 0;
 if (helper$0) {
  helper$0 = 0;
  if (x == 3) {
   print(3);
  } else {
   helper$0 = 1;
  }
 }
 if (helper$0) {
  helper$0 = 0;
  if (x == 4) {
   print(4);
  } else {
   helper$0 = 1;
  }
 }
 if (helper$0) {
  helper$0 = 0;
  if (x == 5) {
   print(5);
  } else {
   helper$0 = 1;
  }
 }
 HEAP32[sp + 16 >> 2] = helper$0;
}
function switchh$0(sp) {
 sp = sp | 0;
 var helper$1 = 0;
 helper$1 = HEAP32[sp + 24 >> 2] | 0;
 switch (helper$1 | 0) {
 case 4:
  {
   f(4);
   g();
  }
 case 5:
  {
   f(5);
   g();
  }
 case 6:
  {
   f(6);
   g();
  }
 default:
  {
   print(9);
  }
 }
}
function switchh$1(sp) {
 sp = sp | 0;
 var helper$0 = 0, helper$1 = 0;
 helper$0 = HEAP32[sp + 16 >> 2] | 0;
 helper$1 = HEAP32[sp + 24 >> 2] | 0;
 if (helper$0) {
  helper$0 = 0;
  switch (helper$1 | 0) {
  case 21:
  case 22:
  case 23:
  case 24:
  case 25:
  case 26:
  case 27:
  case 28:
  case 29:
  case 3:
   {
    f(3);
    g();
    break;
   }
  default:
   {
    helper$0 = 1;
   }
  }
 }
 HEAP32[sp + 16 >> 2] = helper$0;
}
function switchh$2(sp) {
 sp = sp | 0;
 var helper$0 = 0, helper$1 = 0;
 helper$0 = HEAP32[sp + 16 >> 2] | 0;
 helper$1 = HEAP32[sp + 24 >> 2] | 0;
 OL : do {
  if (helper$0) {
   helper$0 = 0;
   switch (helper$1 | 0) {
   case 1:
    {
     f(1);
     g();
     HEAP32[sp + 48 >> 2] = 5;
     break OL;
    }
   default:
    {
     helper$0 = 1;
    }
   }
  }
  if (helper$0) {
   helper$0 = 0;
   switch (helper$1 | 0) {
   case 2:
    {
     f(2);
     g();
     break;
    }
   default:
    {
     helper$0 = 1;
    }
   }
  }
 } while (0);
 HEAP32[sp + 16 >> 2] = helper$0;
}
function switchh2$0(sp) {
 sp = sp | 0;
 var helper$1 = 0;
 helper$1 = HEAP32[sp + 24 >> 2] | 0;
 switch (helper$1 | 0) {
 case 4:
  f(4);
  g();
 case 5:
  f(5);
  g();
 case 6:
  f(6);
  g();
 default:
  print(9);
 }
}
function switchh2$1(sp) {
 sp = sp | 0;
 var helper$0 = 0, helper$1 = 0;
 helper$0 = HEAP32[sp + 16 >> 2] | 0;
 helper$1 = HEAP32[sp + 24 >> 2] | 0;
 if (helper$0) {
  helper$0 = 0;
  switch (helper$1 | 0) {
  case 21:
  case 22:
  case 23:
  case 24:
  case 25:
  case 26:
  case 27:
  case 28:
  case 29:
  case 3:
   f(3);
   g();
   break;
  default:
   {
    helper$0 = 1;
   }
  }
 }
 HEAP32[sp + 16 >> 2] = helper$0;
}
function switchh2$2(sp) {
 sp = sp | 0;
 var helper$0 = 0, helper$1 = 0;
 helper$0 = HEAP32[sp + 16 >> 2] | 0;
 helper$1 = HEAP32[sp + 24 >> 2] | 0;
 OL : do {
  if (helper$0) {
   helper$0 = 0;
   switch (helper$1 | 0) {
   case 1:
    f(1);
    g();
    return;
   default:
    {
     helper$0 = 1;
    }
   }
  }
  if (helper$0) {
   helper$0 = 0;
   switch (helper$1 | 0) {
   case 2:
    f(2);
    g();
    break;
   default:
    {
     helper$0 = 1;
    }
   }
  }
 } while (0);
 HEAP32[sp + 16 >> 2] = helper$0;
}
function stackSet$0(sp) {
 sp = sp | 0;
 c(7);
 c(8);
 c(9);
 c(10);
 c(11);
 c(12);
 c(13);
}
function stackSet$1(sp) {
 sp = sp | 0;
 c(11);
 c(12);
 c(13);
 c(1);
 c(2);
 c(3);
 c(4);
 c(5);
 c(6);
}
function linf$0(sp) {
 sp = sp | 0;
 var d = +0;
 d = +HEAPF32[sp + 16 >> 2];
 OL : do {
  c(16);
  c(17);
  c(18);
  c(19);
  c(20);
  HEAP32[sp + 24 >> 2] = 7;
  HEAPF32[sp + 28 >> 2] = +d;
  break OL;
 } while (0);
}
function linf$1(sp) {
 sp = sp | 0;
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
}
function leaveLabelsMagic$0(sp) {
 sp = sp | 0;
 c(9);
 c(10);
 c(11);
 c(12);
 c(13);
 c(14);
 c(15);
}
function leaveLabelsMagic$1(sp) {
 sp = sp | 0;
 c(1);
 c(2);
 c(3);
 c(4);
 c(5);
 c(6);
 c(7);
 c(8);
}
function leaveLabelsMagic$2(sp) {
 sp = sp | 0;
 var fakeLabel = 0;
 fakeLabel = HEAP32[sp + 16 >> 2] | 0;
 c(7);
 c(8);
 c(9);
 c(10);
 c(11);
 c(12);
 c(13);
 c(14);
 c(15);
 fakeLabel = 1;
 HEAP32[sp + 16 >> 2] = fakeLabel;
}

