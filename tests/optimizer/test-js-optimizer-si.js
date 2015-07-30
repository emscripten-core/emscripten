function a() {
  if (x) {
    if (y) {
      g();
    }
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
  if (x) {
    if (y) {
      if (z) {
        g();
      }
    }
  }
  if (x) {
    return;
    if (y) {
      if (z) {
        g();
      }
    }
  }
  if (x) {
    if (y) {
      return;
      if (z) {
        g();
      }
    }
  }
  if (x) {
    if (y) {
      if (z) {
        g();
      }
      f();
    }
  }
  if (x) {
    if (y) {
      if (z) {
        g();
      }
    }
    f();
  }
  if (x) {
    f();
    x = x + 2 | 0;
    if (y) {
      g();
    }
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
  if (x) {
    if (y) {
      f();
    } else {
      label = 5;
    }
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
    a = 5; // do not commify me
    if (y) {
      f();
    }
  } else {
    label = 5;
  }
  fuseElses();
  if (x) {
    if (y) {
      f();
    } else {
      label = 51;
    }
  } else {
    label = 51;
  }
  if ((label|0) == 51) {
    label = 0;
    a();
  }
  if (x) {
    if (y) {
      f();
    } else {
      label = 52;
    }
  } else {
    label = 52;
  }
  if ((label|0) == 62) {
    label = 0;
    a();
  }
  if (x) {
    if (y) {
      f();
    } else {
      label = 53;
    }
  } else {
    label = 53;
  }
  if ((label|0) == 53) {
    a();
  }
  while (1) {
    if (x) {
      if (y) {
        f();
      } else {
        label = 953;
      }
    } else {
      label = 953;
    }
    if ((label|0) == 953) {
      a();
    }
  }
  if (x) {
    if (y) {
      label = 54; // extra label setting, cannot fuse here
    } else {
      label = 54;
    }
  } else {
    label = 54;
  }
  if ((label|0) == 54) {
    label = 0;
    a();
  }
}
function b() {
  if (x) { // will not be fused, since we did not eliminate with elses
    a();
  } else {
    label = 5;
  }
  if ((label|0) == 5) {
    label = 0;
    a();
  }
}
function c() {
  label = x; // dynamic assign to label, suppresses label removal
  if (x) {
    if (y) {
      f();
    } else {
      label = 151;
    }
  } else {
    label = 151;
  }
  if ((label|0) == 151) {
    label = 0;
    a();
  }
}
function d() {
  if (x) {
    if (y) {
      f();
    } else {
      label = 251;
    }
  } else {
    label = 251;
  }
  if ((label|0) == 251) {
    label = 0;
    a();
  }
  if ((label|0) == 251) { // extra check of label, suppresses label removal
    a();
  }
}
function e() {
  if (x) {
    if (y) {
      f();
    } else {
      label = 351;
    }
  } else {
    label = 351;
  }
  if ((label|0) == 351) {
    label = 0;
    a();
  }
  if ((label|0) == x) { // dynamic check of label, suppresses label removal
    a();
  }
}
// EMSCRIPTEN_GENERATED_FUNCTIONS: ["a", "b", "c", "d"]
