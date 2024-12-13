function foo() {}

function bar() {}

// caller() calls foo(). There is also another function called "caller", down
// below, which should not confuse us (if it does, nothing would refer to foo,
// and instead we'd think the toplevel caller calls bar).
function caller() {
  foo();
}

caller();

var object = {
  method() {
    function caller(data) {
      bar();
    }
  }
};

// Similar, with an arrow function. This should also not confuse us (it would
// make "caller" refer to "arrowed".

function arrowed() {}

var arrow = () => {
  function caller(data) {
    arrowed();
  }
}

wasmImports = {};
