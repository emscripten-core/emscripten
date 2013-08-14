function asmModule(stdlib) {
 "use asm";
 function _f(a, b) {
  a = a | 0;
  b = b | 0;
  if ((a | 0) == 1) {
   return 1;
  }
  if ((a | 0) == 1) {
   return 1;
  } else if ((a | 0) != 2) {
   return 2;
  } else if ((a | 0) == 3) {
   return 3;
  } else if ((a | 0) == 4) {
   return 4;
  }
  L0 : for (;;) {
   if ((a | 0) == 1) {
    return 1;
   } else if ((a | 0) == 2) {
    return 2;
   } else if ((a | 0) == 3) {
    break;
   } else if ((a | 0) == 4) {
    while (1) {
     if ((b | 0) == 1) {
      return 1;
     } else if ((b | 0) == 2) {
      return 2;
     } else if ((b | 0) == 3) {
      return 3;
     } else if ((b | 0) == 4) {
      _g();
     } else if ((b | 0) == 5) {
      break L0;
     } else {
      break;
     }
    }
   } else if ((a | 0) == 5) {
    if ((b | 0) == 1) {
     return 1;
    } else if ((b | 0) == 2) {
     return 2;
    } else if ((b | 0) == 3) {
     return 3;
    } else if ((b | 0) == 4) {
     _g();
    } else if ((b | 0) == 5) {
     break L0;
    } else {
     break;
    }
   } else {
    break;
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
  if ((a | 0) == 10) {
   return 1;
  } else if ((b | 0) == 20) {
   return 2;
  } else if ((a | 0) == 30) {
   return 3;
  } else if ((b | 0) == 40) {
   return 4;
  }
  return 0;
 }
 function _g() {}
 return {};
}

