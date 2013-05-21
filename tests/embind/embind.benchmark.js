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
    Module.print("JS increment_counter " + N + " iters: " + (b-a) + " msecs. result: " + (ctr2-ctr));
}

function _increment_class_counter_benchmark_embind_js(N) {
    var foo = new Module['Foo']();
    var a = _emscripten_get_now();
    for(i = 0; i < N; ++i) {
        foo['incr_class_counter']();
        foo['incr_class_counter']();
        foo['incr_class_counter']();
        foo['incr_class_counter']();
        foo['incr_class_counter']();
        foo['incr_class_counter']();
        foo['incr_class_counter']();
        foo['incr_class_counter']();
        foo['incr_class_counter']();
        foo['incr_class_counter']();
    }
    var b = _emscripten_get_now();
    Module.print("JS embind increment_class_counter " + N + " iters: " + (b-a) + " msecs. result: " + foo['class_counter_val']());
    foo['delete']();
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
    Module.print("JS returns_input 100000 iters: " + (b-a) + " msecs. result: " + t);
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
    Module.print("JS sum_int 100000 iters: " + (b-a) + " msecs. result: " + r);
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
    Module.print("JS sum_float 100000 iters: " + (b-a) + " msecs. result: " + r);
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
    Module.print("JS embind increment_counter " + N + " iters: " + (b-a) + " msecs. result: " + (ctr2-ctr));
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
    Module.print("JS embind returns_input 100000 iters: " + (b-a) + " msecs. result: " + t);
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
    Module.print("JS embind sum_int 100000 iters: " + (b-a) + " msecs. result: " + r);
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
    Module.print("JS embind sum_float 100000 iters: " + (b-a) + " msecs. result: " + r);
}

function _move_gameobjects_benchmark_embind_js() {
    var N = 10000;
    var objects = [];
    for(i = 0; i < N; ++i) {
        objects.push(Module['create_game_object']());
    }
    
    var a = _emscripten_get_now();
    for(i = 0; i < N; ++i) {
        var t = objects[i]['GetTransform']();
        var pos = Module['add'](t['GetPosition'](), [2, 0, 1]);
        var rot = Module['add'](t['GetRotation'](), [0.1, 0.2, 0.3]);
        t['SetPosition'](pos);
        t['SetRotation'](rot);
        t['delete']();
    }
    var b = _emscripten_get_now();
    
    var accum = [0,0,0];
    for(i = 0; i < N; ++i) {
        var t = objects[i]['GetTransform']();
        accum = Module['add'](Module['add'](accum, t['GetPosition']()), t['GetRotation']());
        t['delete']();
        objects[i]['delete']();
    }
    
    Module.print("JS embind move_gameobjects " + N + " iters: " + (b-a) + " msecs. Result: " + (accum[0] + accum[1] + accum[2]));
}

function _pass_gameobject_ptr_benchmark_embind_js() {
    var N = 100000;
    var objects = [];
    for(i = 0; i < N; ++i) {
        objects.push(Module['create_game_object']());
    }
    
    var a = _emscripten_get_now();
    for(i = 0; i < N; ++i) {
        var t = Module['pass_gameobject_ptr'](objects[i]);
        t['delete']();
    }
    var b = _emscripten_get_now();
    
    for(i = 0; i < N; ++i) {
        objects[i]['delete']();
    }
    
    Module.print("JS embind pass_gameobject_ptr " + N + " iters: " + (b-a) + " msecs.");
}

function _call_through_interface0() {
    var N = 1000000;
    var obj = Module['Interface'].implement({
        call0: function() {
        }
    });
    var start = _emscripten_get_now();
    Module['callInterface0'](N, obj);
    var elapsed = _emscripten_get_now() - start;
    Module.print("C++ -> JS void through interface " + N + " iters: " + elapsed + " msecs.");
    obj.delete();
}

function _call_through_interface1() {
    var N = 1000000;
    var obj = Module['Interface'].implement({
        call1: function(s1, s2) {
            return s1 + s2;
        }
    });
    var start = _emscripten_get_now();
    Module['callInterface1'](N, obj);
    var elapsed = _emscripten_get_now() - start;
    Module.print("C++ -> JS std::wstring through interface " + N + " iters: " + elapsed + " msecs.");
    obj.delete();
}

function _call_through_interface2() {
    var N = 1000000;
    var total = 0;
    var obj = Module['Interface'].implement({
        call_with_typed_array: function(ta) {
            total += ta.length;
        },
        call_with_memory_view: function(ta) {
            total += ta.length;
        },
    });

    var start = _emscripten_get_now();
    Module['callInterface2'](N, obj);
    var elapsed = _emscripten_get_now() - start;
    Module.print("C++ -> JS typed array instantiation " + N + " iters: " + elapsed + " msecs.");

    var start = _emscripten_get_now();
    Module['callInterface3'](N, obj);
    var elapsed = _emscripten_get_now() - start;
    Module.print("C++ -> JS memory_view instantiation" + N + " iters: " + elapsed + " msecs.");
    obj.delete();
}

function _returns_val_benchmark() {
    var N = 1000000;
    var v = 1;
    var start = _emscripten_get_now();
    for(var i = 0; i < N; ++i) {
        v = Module['returns_val'](v);
    }
    var elapsed = _emscripten_get_now() - start;
    Module.print("returns_val " + N + " iters: " + elapsed + " msecs");
}
