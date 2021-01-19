#include <emscripten.h>
#include <stdint.h>

void v() {}
int iffddjj(float f, float g, double d, double e, int64_t i, int64_t j) { return 1;}

typedef void (*vptr)();
typedef int (*iffddjjptr)(float f, float g, double d, double e, int64_t i, int64_t j);

EM_JS(double, bench_static_direct_call_v, (vptr func), {
  testStarted('bench_static_direct_call_v');
  var numRuns = 11;
  var results = [];
  var run = setInterval(function() {
    var t0 = tick();
    for(var i = 0; i < 100000; ++i) {
      dynCall_v(func);
      dynCall_v(func);
      dynCall_v(func);
      dynCall_v(func);
      dynCall_v(func);
      dynCall_v(func);
      dynCall_v(func);
      dynCall_v(func);
    }
    var t1 = tick();
    if (numRuns-- >= 0) {
      results.push(t1 - t0);
    } else {
      clearInterval(run);
      testFinished('bench_static_direct_call_v', results);
    }
  }, 1);
});

EM_JS(double, bench_static_bound_call_v, (vptr func), {
  testStarted('bench_static_bound_call_v');
  var numRuns = 11;
  var results = [];
  var boundFunc = bindDynCall('v', func);
  var run = setInterval(function() {
    var t0 = tick();
    for(var i = 0; i < 100000; ++i) {
      boundFunc(func);
      boundFunc(func);
      boundFunc(func);
      boundFunc(func);
      boundFunc(func);
      boundFunc(func);
      boundFunc(func);
      boundFunc(func);
    }
    var t1 = tick();
    if (numRuns-- >= 0) {
      results.push(t1 - t0);
    } else {
      clearInterval(run);
      testFinished('bench_static_bound_call_v', results);
    }
  }, 1);
});

EM_JS(double, bench_dynamic_direct_call_v, (vptr func), {
  testStarted('bench_dynamic_direct_call_v');
  var numRuns = 11;
  var results = [];
  var run = setInterval(function() {
    var t0 = tick();
    for(var i = 0; i < 100000; ++i) {
      dynCall('v', func);
      dynCall('v', func);
      dynCall('v', func);
      dynCall('v', func);
      dynCall('v', func);
      dynCall('v', func);
      dynCall('v', func);
      dynCall('v', func);
    }
    var t1 = tick();
    if (numRuns-- >= 0) {
      results.push(t1 - t0);
    } else {
      clearInterval(run);
      testFinished('bench_dynamic_direct_call_v', results);
    }
  }, 1);
});

EM_JS(double, bench_dynamic_bound_call_v, (vptr func), {
  testStarted('bench_dynamic_bound_call_v');
  var numRuns = 11;
  var results = [];
  var boundFunc = bindDynCallArray('v', func);
  var run = setInterval(function() {
    var t0 = tick();
    for(var i = 0; i < 100000; ++i) {
      boundFunc(func);
      boundFunc(func);
      boundFunc(func);
      boundFunc(func);
      boundFunc(func);
      boundFunc(func);
      boundFunc(func);
      boundFunc(func);
    }
    var t1 = tick();
    if (numRuns-- >= 0) {
      results.push(t1 - t0);
    } else {
      clearInterval(run);
      testFinished('bench_dynamic_bound_call_v', results);
    }
  }, 1);
});

EM_JS(double, bench_static_direct_call_iffddjj, (iffddjjptr func), {
  testStarted('bench_static_direct_call_iffddjj');
  var numRuns = 11;
  var results = [];
  var run = setInterval(function() {
    var t0 = tick();
    for(var i = 0; i < 100000; ++i) {
      dynCall_iffddjj(func, 42.5, 16.5, 13.3, -12.1, 2, 2, 3, 3);
      dynCall_iffddjj(func, 42.5, 16.5, 13.3, -12.1, 2, 2, 3, 3);
      dynCall_iffddjj(func, 42.5, 16.5, 13.3, -12.1, 2, 2, 3, 3);
      dynCall_iffddjj(func, 42.5, 16.5, 13.3, -12.1, 2, 2, 3, 3);
      dynCall_iffddjj(func, 42.5, 16.5, 13.3, -12.1, 2, 2, 3, 3);
      dynCall_iffddjj(func, 42.5, 16.5, 13.3, -12.1, 2, 2, 3, 3);
      dynCall_iffddjj(func, 42.5, 16.5, 13.3, -12.1, 2, 2, 3, 3);
      dynCall_iffddjj(func, 42.5, 16.5, 13.3, -12.1, 2, 2, 3, 3);
    }
    var t1 = tick();
    if (numRuns-- >= 0) {
      results.push(t1 - t0);
    } else {
      clearInterval(run);
      testFinished('bench_static_direct_call_iffddjj', results);
    }
  }, 1);
});

EM_JS(double, bench_static_bound_call_iffddjj, (iffddjjptr func), {
  testStarted('bench_static_bound_call_iffddjj');
  var numRuns = 11;
  var results = [];
  var boundFunc = bindDynCall('iffddjj', func);
  var run = setInterval(function() {
    var t0 = tick();
    for(var i = 0; i < 100000; ++i) {
      boundFunc(42.5, 16.5, 13.3, -12.1, 2, 2, 3, 3);
      boundFunc(42.5, 16.5, 13.3, -12.1, 2, 2, 3, 3);
      boundFunc(42.5, 16.5, 13.3, -12.1, 2, 2, 3, 3);
      boundFunc(42.5, 16.5, 13.3, -12.1, 2, 2, 3, 3);
      boundFunc(42.5, 16.5, 13.3, -12.1, 2, 2, 3, 3);
      boundFunc(42.5, 16.5, 13.3, -12.1, 2, 2, 3, 3);
      boundFunc(42.5, 16.5, 13.3, -12.1, 2, 2, 3, 3);
      boundFunc(42.5, 16.5, 13.3, -12.1, 2, 2, 3, 3);
    }
    var t1 = tick();
    if (numRuns-- >= 0) {
      results.push(t1 - t0);
    } else {
      clearInterval(run);
      testFinished('bench_static_bound_call_iffddjj', results);
    }
  }, 1);
});

EM_JS(double, bench_dynamic_direct_call_iffddjj, (iffddjjptr func), {
  testStarted('bench_dynamic_direct_call_iffddjj');
  var numRuns = 11;
  var results = [];
  var run = setInterval(function() {
    var t0 = tick();
    for(var i = 0; i < 100000; ++i) {
      dynCall('iffddjj', func, [42.5, 16.5, 13.3, -12.1, 2, 2, 3, 3]);
      dynCall('iffddjj', func, [42.5, 16.5, 13.3, -12.1, 2, 2, 3, 3]);
      dynCall('iffddjj', func, [42.5, 16.5, 13.3, -12.1, 2, 2, 3, 3]);
      dynCall('iffddjj', func, [42.5, 16.5, 13.3, -12.1, 2, 2, 3, 3]);
      dynCall('iffddjj', func, [42.5, 16.5, 13.3, -12.1, 2, 2, 3, 3]);
      dynCall('iffddjj', func, [42.5, 16.5, 13.3, -12.1, 2, 2, 3, 3]);
      dynCall('iffddjj', func, [42.5, 16.5, 13.3, -12.1, 2, 2, 3, 3]);
      dynCall('iffddjj', func, [42.5, 16.5, 13.3, -12.1, 2, 2, 3, 3]);
    }
    var t1 = tick();
    if (numRuns-- >= 0) {
      results.push(t1 - t0);
    } else {
      clearInterval(run);
      testFinished('bench_dynamic_direct_call_iffddjj', results);
    }
  }, 1);
});

EM_JS(double, bench_dynamic_bound_call_iffddjj, (iffddjjptr func), {
  testStarted('bench_dynamic_bound_call_iffddjj');
  var numRuns = 11;
  var results = [];
  var boundFunc = bindDynCallArray('iffddjj', func);
  var run = setInterval(function() {
    var t0 = tick();
    for(var i = 0; i < 100000; ++i) {
      boundFunc([42.5, 16.5, 13.3, -12.1, 2, 2, 3, 3]);
      boundFunc([42.5, 16.5, 13.3, -12.1, 2, 2, 3, 3]);
      boundFunc([42.5, 16.5, 13.3, -12.1, 2, 2, 3, 3]);
      boundFunc([42.5, 16.5, 13.3, -12.1, 2, 2, 3, 3]);
      boundFunc([42.5, 16.5, 13.3, -12.1, 2, 2, 3, 3]);
      boundFunc([42.5, 16.5, 13.3, -12.1, 2, 2, 3, 3]);
      boundFunc([42.5, 16.5, 13.3, -12.1, 2, 2, 3, 3]);
      boundFunc([42.5, 16.5, 13.3, -12.1, 2, 2, 3, 3]);
    }
    var t1 = tick();
    if (numRuns-- >= 0) {
      results.push(t1 - t0);
    } else {
      clearInterval(run);
      testFinished('bench_dynamic_bound_call_iffddjj', results);
    }
  }, 1);
});
























EM_JS(double, bench_wbind_static_direct_call_iffddjj, (iffddjjptr func), {
  testStarted('bench_wbind_static_direct_call_iffddjj');
  var numRuns = 11;
  var results = [];
  var run = setInterval(function() {
    var t0 = tick();
    for(var i = 0; i < 100000; ++i) {
      wbind(func)(42.5, 16.5, 13.3, -12.1, 2, 2, 3, 3);
      wbind(func)(42.5, 16.5, 13.3, -12.1, 2, 2, 3, 3);
      wbind(func)(42.5, 16.5, 13.3, -12.1, 2, 2, 3, 3);
      wbind(func)(42.5, 16.5, 13.3, -12.1, 2, 2, 3, 3);
      wbind(func)(42.5, 16.5, 13.3, -12.1, 2, 2, 3, 3);
      wbind(func)(42.5, 16.5, 13.3, -12.1, 2, 2, 3, 3);
      wbind(func)(42.5, 16.5, 13.3, -12.1, 2, 2, 3, 3);
      wbind(func)(42.5, 16.5, 13.3, -12.1, 2, 2, 3, 3);
    }
    var t1 = tick();
    if (numRuns-- >= 0) {
      results.push(t1 - t0);
    } else {
      clearInterval(run);
      testFinished('bench_wbind_static_direct_call_iffddjj', results);
    }
  }, 1);
});

EM_JS(double, bench_wbind_static_bound_call_iffddjj, (iffddjjptr func), {
  testStarted('bench_wbind_static_bound_call_iffddjj');
  var numRuns = 11;
  var results = [];
  var boundFunc = wbind(func);
  var run = setInterval(function() {
    var t0 = tick();
    for(var i = 0; i < 100000; ++i) {
      boundFunc(42.5, 16.5, 13.3, -12.1, 2, 2, 3, 3);
      boundFunc(42.5, 16.5, 13.3, -12.1, 2, 2, 3, 3);
      boundFunc(42.5, 16.5, 13.3, -12.1, 2, 2, 3, 3);
      boundFunc(42.5, 16.5, 13.3, -12.1, 2, 2, 3, 3);
      boundFunc(42.5, 16.5, 13.3, -12.1, 2, 2, 3, 3);
      boundFunc(42.5, 16.5, 13.3, -12.1, 2, 2, 3, 3);
      boundFunc(42.5, 16.5, 13.3, -12.1, 2, 2, 3, 3);
      boundFunc(42.5, 16.5, 13.3, -12.1, 2, 2, 3, 3);
    }
    var t1 = tick();
    if (numRuns-- >= 0) {
      results.push(t1 - t0);
    } else {
      clearInterval(run);
      testFinished('bench_wbind_static_bound_call_iffddjj', results);
    }
  }, 1);
});

EM_JS(double, bench_wbind_dynamic_direct_call_iffddjj, (iffddjjptr func), {
  testStarted('bench_wbind_dynamic_direct_call_iffddjj');
  var numRuns = 11;
  var results = [];
  var run = setInterval(function() {
    var t0 = tick();
    for(var i = 0; i < 100000; ++i) {
      wbindArray(func, [42.5, 16.5, 13.3, -12.1, 2, 2, 3, 3]);
      wbindArray(func, [42.5, 16.5, 13.3, -12.1, 2, 2, 3, 3]);
      wbindArray(func, [42.5, 16.5, 13.3, -12.1, 2, 2, 3, 3]);
      wbindArray(func, [42.5, 16.5, 13.3, -12.1, 2, 2, 3, 3]);
      wbindArray(func, [42.5, 16.5, 13.3, -12.1, 2, 2, 3, 3]);
      wbindArray(func, [42.5, 16.5, 13.3, -12.1, 2, 2, 3, 3]);
      wbindArray(func, [42.5, 16.5, 13.3, -12.1, 2, 2, 3, 3]);
      wbindArray(func, [42.5, 16.5, 13.3, -12.1, 2, 2, 3, 3]);
    }
    var t1 = tick();
    if (numRuns-- >= 0) {
      results.push(t1 - t0);
    } else {
      clearInterval(run);
      testFinished('bench_wbind_dynamic_direct_call_iffddjj', results);
    }
  }, 1);
});

EM_JS(double, bench_wbind_dynamic_bound_call_iffddjj, (iffddjjptr func), {
  testStarted('bench_wbind_dynamic_bound_call_iffddjj');
  var numRuns = 11;
  var results = [];
  var boundFunc = wbindArray(func);
  var run = setInterval(function() {
    var t0 = tick();
    for(var i = 0; i < 100000; ++i) {
      boundFunc([42.5, 16.5, 13.3, -12.1, 2, 2, 3, 3]);
      boundFunc([42.5, 16.5, 13.3, -12.1, 2, 2, 3, 3]);
      boundFunc([42.5, 16.5, 13.3, -12.1, 2, 2, 3, 3]);
      boundFunc([42.5, 16.5, 13.3, -12.1, 2, 2, 3, 3]);
      boundFunc([42.5, 16.5, 13.3, -12.1, 2, 2, 3, 3]);
      boundFunc([42.5, 16.5, 13.3, -12.1, 2, 2, 3, 3]);
      boundFunc([42.5, 16.5, 13.3, -12.1, 2, 2, 3, 3]);
      boundFunc([42.5, 16.5, 13.3, -12.1, 2, 2, 3, 3]);
    }
    var t1 = tick();
    if (numRuns-- >= 0) {
      results.push(t1 - t0);
    } else {
      clearInterval(run);
      testFinished('bench_wbind_dynamic_bound_call_iffddjj', results);
    }
  }, 1);
});





int main() {
  EM_ASM({
    var g = ENVIRONMENT_IS_NODE ? global : window;
    g.node_tick = function() { var t = process.hrtime(); return t[0]*1e3 + t[1]/1e6; };
    g.tick = function() { return ENVIRONMENT_IS_NODE ? node_tick() : performance.now(); };
    g.avg = function(arr) { var sum = 0; arr.forEach((val) => { sum += val; }); return sum / arr.length; };
    g.testsPending = {};
    g.testStarted = function(name) { g.testsPending[name] = true; };
    g.testFinished = function(name, results) {
      results.splice(0, 2); // Remove first two runs to remove any results from JIT warmup
      var numRuns = results.length;
      var result = avg(results);
      console.log(name + ': ' + result + ' msecs (averaged over ' + numRuns + ' runs)');
      delete g.testsPending[name];
      if (Object.keys(testsPending).length == 0) exit(0);
    }
  });
  bench_dynamic_direct_call_v(v);
  bench_dynamic_bound_call_v(v);
  bench_static_direct_call_v(v);
  bench_static_bound_call_v(v);

  bench_dynamic_direct_call_iffddjj(iffddjj);
  bench_dynamic_bound_call_iffddjj(iffddjj);
  bench_static_direct_call_iffddjj(iffddjj);
  bench_static_bound_call_iffddjj(iffddjj);
}
