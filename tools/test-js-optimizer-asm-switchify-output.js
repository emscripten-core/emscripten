function asmModule(stdlib) {
 "use asm";
 function _f(a, b) {
  a = a | 0;
  b = b | 0;
  if ((a | 0) == 1) {
   return 1;
  }
  L0 : for (;;) {
   switch (a | 0) {
   case 1:
    return 1;
   case 2:
    return 2;
   case 3:
    break L0;
   case 4:
    S0 : while (1) {
     switch (b | 0) {
     case 1:
      return 1;
     case 2:
      return 2;
     case 3:
      return 3;
     case 4:
      _g();
      break;
     case 5:
      break L0;
     default:
      break S0;
     }
    }
    break;
   case 5:
    switch (b | 0) {
    case 1:
     return 1;
    case 2:
     return 2;
    case 3:
     return 3;
    case 4:
     _g();
     break;
    case 5:
     break L0;
    default:
     break L0;
    }
    break;
   default:
    break L0;
   }
   _g();
  }
  if ((a | 0) == 1) {
   return 1;
  } else if ((b | 0) == 2) {
   return 2;
  } else if ((a | 0) == 3) {
   return 3;
  } else if ((b | 0) == 4) {
   return 4;
  } else if ((a | 0) == 5) {
   return 5;
  }
  return 0;
 }
 function _g() {}
 return {};
}

