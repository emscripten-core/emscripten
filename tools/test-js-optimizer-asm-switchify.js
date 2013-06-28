function asmModule(stdlib) {
  "use asm";

  // handcrafted cases to make sure edge cases are exercised
  function _f(a, b) {
    a = a | 0;
    b = b | 0;
    // no point making a tiny if-else statement into a switch clause;
    // this should be preserved as-is
    if ((a | 0) == 1) {
      return 1;
    }

    // this should get switchified. Worth testing as a separate case to make
    // sure we correctly handle switch statements that aren't nested in loops.
    if ((a | 0) == 1) {
      return 1;
    }
    else if ((a | 0) == 2) {
      return 2;
    }
    else {
      return 3;
    }

    // this for loop is already labeled; don't generate another label for it
    L0:
    for (;;) {
      if ((a | 0) == 1) {
        return 1;
      }
      else if ((a | 0) == 2) {
        return 2;
      }
      else if ((a | 0) == 3) {
        break;
      }
      else if ((a | 0) == 4) {
        while (1) {
          if ((b | 0) == 1) {
            return 1;
          }
          else if ((b | 0) == 2) {
            return 2;
          }
          else if ((b | 0) == 3) {
            return 3;
          }
          else if ((b | 0) == 4) {
            _g();
          }
          else if ((b | 0) == 5) {
            break L0;
          }
          else {
            break;
          }
        }
      }
      else if ((a | 0) == 5) {
        // ensure nested switches get compiled correctly
        if ((b | 0) == 1) {
          return 1;
        }
        else if ((b | 0) == 2) {
          return 2;
        }
        else if ((b | 0) == 3) {
          return 3;
        }
        else if ((b | 0) == 4) {
          _g();
        }
        else if ((b | 0) == 5) {
          break L0;
        }
        else {
          break;
        }
      }
      else {
        break;
      }
      _g();
    }

    // we can only convert to switch-case if all the conditions are dispatching
    // on the same variable. so this should be left intact
    if ((a | 0) == 1) {
      return 1;
    }
    else if ((b | 0) == 2) {
      return 2;
    }
    else if ((a | 0) == 3) {
      return 3;
    }
    else if ((b | 0) == 4) {
      return 4;
    }
    else if ((a | 0) == 5) {
      return 5;
    }

    return 0;
  }

  function _g() {}

  return {};
}


// EMSCRIPTEN_GENERATED_FUNCTIONS: ["_f", "_g"]
