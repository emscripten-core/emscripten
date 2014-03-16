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
}

