// Copyright 2026 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <cstdio>
#include <expected>
#include <string>
#include <emscripten.h>
#include <emscripten/bind.h>
#include <emscripten/val.h>

using namespace emscripten;

struct Widget {
  int id;
  explicit Widget(int id) : id(id) {}
  static std::expected<Widget*, val> create(bool ok, int id) {
    if (ok) {
      return new Widget(id);
    }
    return std::unexpected(val::global("Error").new_(std::string("bad widget")));
  }
  std::expected<int, val> compute(bool ok) const {
    if (ok) {
      return id * 10;
    }
    return std::unexpected(val::global("Error").new_(std::string("compute failed")));
  }

  std::expected<int, val> getProp() const {
    return id + 1;
  }
};

std::expected<int, val> divide(int a, int b) {
  if (b == 0) {
    return std::unexpected(val::global("Error").new_(std::string("division by zero")));
  }
  return a / b;
}

std::expected<void, val> checkPositive(int x) {
  if (x <= 0) {
    return std::unexpected(val::global("RangeError").new_(std::string("not positive")));
  }
  return {};
}

std::expected<int, std::string> parseNum(const std::string& s) {
  if (s == "42") {
    return 42;
  }
  return std::unexpected(std::string("invalid number: ") + s);
}

EMSCRIPTEN_BINDINGS(expected_module) {
  register_expected<int>();
  register_expected<void>();
  register_expected<int, std::string>();
  register_expected<Widget*>();
  function("divide", &divide);
  function("checkPositive", &checkPositive);
  function("parseNum", &parseNum);
  class_<Widget>("Widget")
    .constructor(&Widget::create)
    .function("compute", &Widget::compute)
    .property("id", &Widget::id)
    .property("prop", &Widget::getProp);
}

int main() {
  EM_ASM({
    assert(Module.divide(10, 2) === 5);
    let err = null;
    try {
      Module.divide(10, 0);
    } catch (e) {
      err = e;
    }
    assert(err instanceof Error && err.message === 'division by zero');

    assert(Module.checkPositive(1) === undefined);
    err = null;
    try {
      Module.checkPositive(0);
    } catch (e) {
      err = e;
    }
    assert(err instanceof RangeError && err.message === 'not positive');

    assert(Module.parseNum('42') === 42);
    err = null;
    try {
      Module.parseNum('abc');
    } catch (e) {
      err = e;
    }
    assert(err === 'invalid number: abc');

    err = null;
    try {
      new Module.Widget(false, 7);
    } catch (e) {
      err = e;
    }
    assert(err instanceof Error && err.message === 'bad widget');

    const w = new Module.Widget(true, 7);
    assert(w.id === 7);
    assert(w.prop === 8);
    assert(w.compute(true) === 70);
    err = null;
    try {
      w.compute(false);
    } catch (e) {
      err = e;
    }
    assert(err instanceof Error && err.message === 'compute failed');
    w.delete();
  });
  printf("done\n");
  return 0;
}
