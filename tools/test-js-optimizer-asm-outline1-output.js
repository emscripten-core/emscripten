function lin() {
 c(1);
 c(2);
 c(3);
 c(4);
 lin$1(sp);
 lin$0(sp);
}
function lin2() {
 while (1) {
  c(1);
  c(2);
  c(3);
  c(4);
  lin2$1(sp);
  lin2$0(sp);
 }
}
function lin3() {
 while (1) {
  c(1);
  c(2);
  c(3);
  c(4);
  c(5);
  lin3$1(sp);
  lin3$0(sp);
  if (HEAP32[sp + 0 >> 2] == 5) {
   return;
  }
  if (HEAP32[sp + 0 >> 2] == 6) {
   return HEAP32[sp + 8 >> 2];
  }
  if (HEAP32[sp + 0 >> 2] == 7) {
   return HEAPF32[sp + 8 >> 2];
  }
 }
 return 20;
}
function lin4() {
 while (1) {
  c(1);
  c(2);
  c(3);
  c(4);
  lin4$1(sp);
  lin4$0(sp);
  if (HEAP32[sp + 0 >> 2] == 1) {
   break;
  }
 }
 return 20;
}
function lin5() {
 while (1) {
  c(1);
  c(2);
  c(3);
  c(4);
  lin5$1(sp);
  lin5$0(sp);
  if (HEAP32[sp + 0 >> 2] == 3) {
   continue;
  }
 }
 return 20;
}
function mix() {
 main : while (1) {
  c(1);
  c(2);
  c(3);
  c(4);
  c(5);
  c(6);
  c(7);
  mix$1(sp);
  mix$0(sp);
  if (HEAP32[sp + 0 >> 2] == 1) {
   break;
  }
  if (HEAP32[sp + 0 >> 2] == 2) {
   switch (HEAP32[sp + 8 >> 2]) {
   case 2:
    break main;
   }
  }
  if (HEAP32[sp + 0 >> 2] == 3) {
   continue;
  }
  if (HEAP32[sp + 0 >> 2] == 4) {
   switch (HEAP32[sp + 8 >> 2]) {
   case 3:
    continue main;
   }
  }
 }
 return 20;
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
  HEAP32[sp + 8 >> 2] = 10, HEAP32[sp + 0 >> 2] = 6, break OL;
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
  HEAP32[sp + 0 >> 2] = 1, break OL;
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
  HEAP32[sp + 0 >> 2] = 3, break OL;
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
  HEAP32[sp + 8 >> 2] = 2, HEAP32[sp + 0 >> 2] = 2, break OL;
  c(18);
  HEAP32[sp + 0 >> 2] = 1, break OL;
  while (1) {
   break;
  }
  inner : while (1) {
   break inner;
  }
  c(19);
  HEAP32[sp + 0 >> 2] = 3, break OL;
  c(20);
  HEAP32[sp + 8 >> 2] = 3, HEAP32[sp + 0 >> 2] = 4, break OL;
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

