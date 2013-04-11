function _increment_counter_benchmark_js(N) {
    var ctr = _get_counter();
    var a = _emscripten_get_now();
    for(i = 0; i < N; ++i) {
        _increment_counter();
        _increment_counter();
        _increment_counter();
        _increment_counter();
        _increment_counter();
        _increment_counter();
        _increment_counter();
        _increment_counter();
        _increment_counter();
        _increment_counter();
    }
    var b = _emscripten_get_now();
    var ctr2 = _get_counter();
    Module.print("JS increment_counter " + N + " iters: " + (b-a)*1000 + " msecs. result: " + (ctr2-ctr));
}

function _increment_class_counter_benchmark_embind_js(N) {
    var foo = new Module.Foo();
    var a = _emscripten_get_now();
    for(i = 0; i < N; ++i) {
        foo.incr_class_counter();
        foo.incr_class_counter();
        foo.incr_class_counter();
        foo.incr_class_counter();
        foo.incr_class_counter();
        foo.incr_class_counter();
        foo.incr_class_counter();
        foo.incr_class_counter();
        foo.incr_class_counter();
        foo.incr_class_counter();
    }
    var b = _emscripten_get_now();
    Module.print("JS increment_class_counter_embind " + N + " iters: " + (b-a)*1000 + " msecs. result: " + foo.class_counter_val());
    foo.delete();
}

function _returns_input_benchmark_js() {
    var a = _emscripten_get_now();
    var t = 0;
    for(i = 0; i < 100000; ++i) {
        t += _returns_input(i);
        t += _returns_input(i);
        t += _returns_input(i);
        t += _returns_input(i);
        t += _returns_input(i);
        t += _returns_input(i);
        t += _returns_input(i);
        t += _returns_input(i);
        t += _returns_input(i);
        t += _returns_input(i);
    }
    var b = _emscripten_get_now();
    Module.print("JS returns_input 100000 iters: " + (b-a)*1000 + " msecs. result: " + t);
}

function _sum_int_benchmark_js() {
    var a = _emscripten_get_now();
    var r = 0;
    for(i = 0; i < 100000; ++i) {
        r += _sum_int(i, 2, 3, 4, 5, 6, 7, 8, 9);
        r += _sum_int(i, 2, 3, 4, 5, 6, 7, 8, 9);
        r += _sum_int(i, 2, 3, 4, 5, 6, 7, 8, 9);
        r += _sum_int(i, 2, 3, 4, 5, 6, 7, 8, 9);
        r += _sum_int(i, 2, 3, 4, 5, 6, 7, 8, 9);
        r += _sum_int(i, 2, 3, 4, 5, 6, 7, 8, 9);
        r += _sum_int(i, 2, 3, 4, 5, 6, 7, 8, 9);
        r += _sum_int(i, 2, 3, 4, 5, 6, 7, 8, 9);
        r += _sum_int(i, 2, 3, 4, 5, 6, 7, 8, 9);
        r += _sum_int(i, 2, 3, 4, 5, 6, 7, 8, 9);
    }
    var b = _emscripten_get_now();
    Module.print("JS sum_int 100000 iters: " + (b-a)*1000 + " msecs. result: " + r);
}

function _sum_float_benchmark_js() {
    var a = _emscripten_get_now();
    var r = 0;
    for(i = 0; i < 100000; ++i) {
        r += _sum_float(i, 2, 3, 4, 5, 6, 7, 8, 9);
        r += _sum_float(i, 2, 3, 4, 5, 6, 7, 8, 9);
        r += _sum_float(i, 2, 3, 4, 5, 6, 7, 8, 9);
        r += _sum_float(i, 2, 3, 4, 5, 6, 7, 8, 9);
        r += _sum_float(i, 2, 3, 4, 5, 6, 7, 8, 9);
        r += _sum_float(i, 2, 3, 4, 5, 6, 7, 8, 9);
        r += _sum_float(i, 2, 3, 4, 5, 6, 7, 8, 9);
        r += _sum_float(i, 2, 3, 4, 5, 6, 7, 8, 9);
        r += _sum_float(i, 2, 3, 4, 5, 6, 7, 8, 9);
        r += _sum_float(i, 2, 3, 4, 5, 6, 7, 8, 9);
    }
    var b = _emscripten_get_now();
    Module.print("JS sum_float 100000 iters: " + (b-a)*1000 + " msecs. result: " + r);
}

function _increment_counter_benchmark_embind_js(N) {
    var ctr = _get_counter();
    var a = _emscripten_get_now();
    for(i = 0; i < N; ++i) {
        Module['increment_counter']();
        Module['increment_counter']();
        Module['increment_counter']();
        Module['increment_counter']();
        Module['increment_counter']();
        Module['increment_counter']();
        Module['increment_counter']();
        Module['increment_counter']();
        Module['increment_counter']();
        Module['increment_counter']();
    }
    var b = _emscripten_get_now();
    var ctr2 = _get_counter();
    Module.print("JS embind increment_counter " + N + " iters: " + (b-a)*1000 + " msecs. result: " + (ctr2-ctr));
}

function _returns_input_benchmark_embind_js() {
    var a = _emscripten_get_now();
    var t = 0;
    for(i = 0; i < 100000; ++i) {
        t += Module['returns_input'](i);
        t += Module['returns_input'](i);
        t += Module['returns_input'](i);
        t += Module['returns_input'](i);
        t += Module['returns_input'](i);
        t += Module['returns_input'](i);
        t += Module['returns_input'](i);
        t += Module['returns_input'](i);
        t += Module['returns_input'](i);
        t += Module['returns_input'](i);
    }
    var b = _emscripten_get_now();
    Module.print("JS embind returns_input 100000 iters: " + (b-a)*1000 + " msecs. result: " + t);
}

function _sum_int_benchmark_embind_js() {
    var a = _emscripten_get_now();
    var r = 0;
    for(i = 0; i < 100000; ++i) {
        r += Module['sum_int'](i, 2, 3, 4, 5, 6, 7, 8, 9);
        r += Module['sum_int'](i, 2, 3, 4, 5, 6, 7, 8, 9);
        r += Module['sum_int'](i, 2, 3, 4, 5, 6, 7, 8, 9);
        r += Module['sum_int'](i, 2, 3, 4, 5, 6, 7, 8, 9);
        r += Module['sum_int'](i, 2, 3, 4, 5, 6, 7, 8, 9);
        r += Module['sum_int'](i, 2, 3, 4, 5, 6, 7, 8, 9);
        r += Module['sum_int'](i, 2, 3, 4, 5, 6, 7, 8, 9);
        r += Module['sum_int'](i, 2, 3, 4, 5, 6, 7, 8, 9);
        r += Module['sum_int'](i, 2, 3, 4, 5, 6, 7, 8, 9);
        r += Module['sum_int'](i, 2, 3, 4, 5, 6, 7, 8, 9);
    }
    var b = _emscripten_get_now();
    Module.print("JS embind sum_int 100000 iters: " + (b-a)*1000 + " msecs. result: " + r);
}

function _sum_float_benchmark_embind_js() {
    var a = _emscripten_get_now();
    var r = 0;
    for(i = 0; i < 100000; ++i) {
        r += Module['sum_float'](i, 2, 3, 4, 5, 6, 7, 8, 9);
        r += Module['sum_float'](i, 2, 3, 4, 5, 6, 7, 8, 9);
        r += Module['sum_float'](i, 2, 3, 4, 5, 6, 7, 8, 9);
        r += Module['sum_float'](i, 2, 3, 4, 5, 6, 7, 8, 9);
        r += Module['sum_float'](i, 2, 3, 4, 5, 6, 7, 8, 9);
        r += Module['sum_float'](i, 2, 3, 4, 5, 6, 7, 8, 9);
        r += Module['sum_float'](i, 2, 3, 4, 5, 6, 7, 8, 9);
        r += Module['sum_float'](i, 2, 3, 4, 5, 6, 7, 8, 9);
        r += Module['sum_float'](i, 2, 3, 4, 5, 6, 7, 8, 9);
        r += Module['sum_float'](i, 2, 3, 4, 5, 6, 7, 8, 9);
    }
    var b = _emscripten_get_now();
    Module.print("JS embind sum_float 100000 iters: " + (b-a)*1000 + " msecs. result: " + r);
}
