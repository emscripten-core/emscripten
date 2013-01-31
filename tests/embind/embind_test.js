//=== testing glue

function module(ignore, func) {
  func({ Emscripten: Module });
}

function fixture(name, info) {
  Module.print('fixture: ' + name);
  for (var test in info) {
    var f = info[test];
    if (typeof f != 'function') continue;
    Module.print('--test: ' + test);
    // TODO: Base fixture!
    f();
  }
}

assert.true = assert;

assert.equal = function(x, y) {
  assert(x == y);
}

assert.notEqual = function(x, y) {
  assert(x != y);
}

assert.throws = function(exc, func) {
  var ret;
  try {
    func();
  } catch(e) {
    ret = e;
  }
  assert(ret); // TODO: check exc vs e
  return ret;
}

assert.instanceof = function(inst, clazz) {
  assert(inst instanceof clazz);
}

assert.deepEqual = function(x, y) {
  assert(JSON.stringify(x) == JSON.stringify(y));
}

//===

module({
    Emscripten: '../build/Emscripten.js'
}, function(imports) {
    var cm = imports.Emscripten;

    var checkForLeaks = {
        setUp: function() {
            this.originalBlockCount = cm.mallinfo().uordblks;
        },
        tearDown: function() {
            assert.equal(this.originalBlockCount, cm.mallinfo().uordblks);
        },
    };

    fixture("embind", {
        baseFixture: checkForLeaks,

        "test value creation": function() {
            assert.equal(15, cm.emval_test_new_integer());
            assert.equal("Hello everyone", cm.emval_test_new_string());

            var object = cm.emval_test_new_object();
            assert.equal('bar', object.foo);
            assert.equal(1, object.baz);
        },

        "test passthrough": function() {
            var a = {foo: 'bar'};
            var b = cm.emval_test_passthrough(a);
            a.bar = 'baz';
            assert.equal('baz', b.bar);
            
            assert.equal(0, cm.count_emval_handles());
        },

        "test void return converts to undefined": function() {
            assert.equal(undefined, cm.emval_test_return_void());
        },

        "test booleans can be marshalled": function() {
            assert.equal(false, cm.emval_test_not(true));
            assert.equal(true, cm.emval_test_not(false));
        },

        "test convert double to unsigned": function() {
            var rv = cm.emval_test_as_unsigned(1.5);
            assert.equal('number', typeof rv);
            assert.equal(1, rv);
            assert.equal(0, cm.count_emval_handles());
        },

        "test get length of array": function() {
            assert.equal(10, cm.emval_test_get_length([0, 1, 2, 3, 4, 5, 'a', 'b', 'c', 'd']));
            assert.equal(0, cm.count_emval_handles());
        },

        "test add a bunch of things": function() {
            assert.equal(66.0, cm.emval_test_add(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11));
            assert.equal(0, cm.count_emval_handles());
        },

        "test sum array": function() {
            assert.equal(66, cm.emval_test_sum([1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11]));
            assert.equal(0, cm.count_emval_handles());
        },

        "test strings": function() {
            assert.equal("foobar", "foo" + "bar");
            assert.equal("foobar", cm.emval_test_take_and_return_std_string("foobar"));

            assert.equal("foobar", cm.emval_test_take_and_return_std_string_const_ref("foobar"));    
        },

        "test no memory leak when passing strings in by const reference": function() {
            var original = cm.mallinfo().uordblks;
            cm.emval_test_take_and_return_std_string_const_ref("foobar");
            assert.equal(original, cm.mallinfo().uordblks);
        },
    });

    fixture("classes", {
        baseFixture: checkForLeaks,

        "test class instance": function() {
            var a = {foo: 'bar'};
            var c = new cm.ValHolder(a);
            assert.equal('bar', c.getVal().foo);
            
            c.setVal('1234');
            assert.equal('1234', c.getVal());

            assert.equal(1239, c.returnIntPlusFive(1234));

            c.delete();
            assert.equal(0, cm.count_emval_handles());
        },

        "test class methods": function() {
            assert.equal(10, cm.ValHolder.some_class_method(10));
        },

        "test can't call methods on deleted class instances": function() {
            var c = new cm.ValHolder(undefined);
            c.delete();
            assert.throws(cm.BindingError, function() {
                c.getVal();
            });
            assert.throws(cm.BindingError, function() {
                c.delete();
            });
        },

        "test isinstance": function() {
            var c = new cm.ValHolder(undefined);
            assert.instanceof(c, cm.ValHolder);
            c.delete();
        },

        "test can return class instances by value": function() {
            var c = cm.emval_test_return_ValHolder();
            assert.deepEqual({}, c.getVal());
            c.delete();
        },

        "test can pass class instances to functions by reference": function() {
            var a = {a:1};
            var c = new cm.ValHolder(a);
            cm.emval_test_set_ValHolder_to_empty_object(c);
            assert.deepEqual({}, c.getVal());
            c.delete();
        },

        "test can access struct fields": function() {
            var c = new cm.CustomStruct();
            assert.equal(10, c.field);
            c.delete();
        },

        "test can set struct fields": function() {
            var c = new cm.CustomStruct();
            c.field = 15;
            assert.equal(15, c.field);
            c.delete();
        },

        "test assignment returns value": function() {
            var c = new cm.CustomStruct();
            assert.equal(15, c.field = 15);
            c.delete();
        },

        "test assigning string to integer raises TypeError": function() {
            var c = new cm.CustomStruct();

            var e = assert.throws(TypeError, function() {
                c.field = "hi";
            });
            assert.equal('Cannot convert "hi" to int', e.message);

            var e = assert.throws(TypeError, function() {
                c.field = {foo:'bar'};
            });
            assert.equal('Cannot convert "[object Object]" to int', e.message);

            c.delete();
        },

        "test can return tuples by value": function() {
            var c = cm.emval_test_return_TupleVector();
            assert.deepEqual([1, 2, 3], c);
        },

        "test tuples can contain tuples": function() {
            var c = cm.emval_test_return_TupleVectorTuple();
            assert.deepEqual([[1, 2, 3]], c);
        },

        "test can pass tuples by value": function() {
            var c = cm.emval_test_take_and_return_TupleVector([4, 5, 6]);
            assert.deepEqual([4, 5, 6], c);
        },

        "test can return structs by value": function() {
            var c = cm.emval_test_return_StructVector();
            assert.deepEqual({x: 1, y: 2, z: 3}, c);
        },

        "test can pass structs by value": function() {
            var c = cm.emval_test_take_and_return_StructVector({x: 4, y: 5, z: 6});
            assert.deepEqual({x: 4, y: 5, z: 6}, c);
        },

        "test can pass and return tuples in structs": function() {
            var d = cm.emval_test_take_and_return_TupleInStruct({field: [1, 2, 3]});
            assert.deepEqual({field: [1, 2, 3]}, d);
        },

        "test can clone handles": function() {
            assert.equal(0, cm.count_emval_handles());

            var a = new cm.ValHolder({});
            var b = a.clone();
            a.delete();

            assert.equal(1, cm.count_emval_handles());

            assert.throws(cm.BindingError, function() {
                a.delete();
            });
            b.delete();

            assert.equal(0, cm.count_emval_handles());
        },

        "test can't clone if already deleted": function() {
            var a = new cm.ValHolder({});
            a.delete();
            assert.throws(cm.BindingError, function() {
                a.clone();
            });
        },

        "test moving handles is a clone+delete": function() {
            var a = new cm.ValHolder({});
            var b = a.move();
            assert.throws(cm.BindingError, function() {
                a.delete();
            });
            assert.equal(1, cm.count_emval_handles());
            b.delete();
            assert.equal(0, cm.count_emval_handles());
        },

        "test StringHolder": function() {
            var a = new cm.StringHolder("foobar");
            assert.equal("foobar", a.get());

            a.set("barfoo");
            assert.equal("barfoo", a.get());
            a.delete();
        },
    });

    fixture("embind enumerations", {
        baseFixture: checkForLeaks,

        "test can compare enumeration values": function() {
            assert.equal(cm.Enum.ONE, cm.Enum.ONE);
            assert.notEqual(cm.Enum.ONE, cm.Enum.TWO);
        },

        "test repr includes enum value": function() {
            return; // XXX IMVU?
            assert.equal('<#Enum_ONE {}>', IMVU.repr(cm.Enum.ONE));
            assert.equal('<#Enum_TWO {}>', IMVU.repr(cm.Enum.TWO));
        },

        "test instanceof": function() {
            assert.instanceof(cm.Enum.ONE, cm.Enum);
        },

        "test can pass and return enumeration values to functions": function() {
            assert.equal(cm.Enum.TWO, cm.emval_test_take_and_return_Enum(cm.Enum.TWO));
        },
    });

    fixture("C++11 enum class", {
        baseFixture: checkForLeaks,

        "test can compare enumeration values": function() {
            assert.equal(cm.EnumClass.ONE, cm.EnumClass.ONE);
            assert.notEqual(cm.EnumClass.ONE, cm.EnumClass.TWO);
        },

        "test repr includes enum value": function() {
            return; // XXX IMVU?
            assert.equal('<#EnumClass_ONE {}>', IMVU.repr(cm.EnumClass.ONE));
            assert.equal('<#EnumClass_TWO {}>', IMVU.repr(cm.EnumClass.TWO));
        },

        "test instanceof": function() {
            assert.instanceof(cm.EnumClass.ONE, cm.EnumClass);
        },

        "test can pass and return enumeration values to functions": function() {
            assert.equal(cm.EnumClass.TWO, cm.emval_test_take_and_return_EnumClass(cm.EnumClass.TWO));
        },
    });

    fixture("emval call tests", {
        "test can call functions from C++": function() {
            var called = false;
            cm.emval_test_call_function(function(i, f, tv, sv) {
                called = true;
                assert.equal(10, i);
                assert.equal(1.5, f);
                assert.deepEqual([1.25, 2.5, 3.75], tv);
                assert.deepEqual({x: 1.25, y: 2.5, z: 3.75}, sv);
            }, 10, 1.5, [1.25, 2.5, 3.75], {x: 1.25, y: 2.5, z: 3.75});
            assert.true(called);
        },
    });

    fixture("interfaces", {
        baseFixture: checkForLeaks,

        "test can wrap JS object in native interface": function() {
            var foo = {
                calls: [],
                method: function() {
                    this.calls.push('called');
                    return 10;
                }
            };

            assert.equal(10, cm.emval_test_call_method(foo));
            assert.deepEqual(['called'], foo.calls);
        },

        "test can pass arguments and return complicated values": function() {
            var calls = [];
            var foo = {
                method2: function(arg1, arg2) {
                    calls.push([arg1, arg2]);
                    return arg1;
                }
            };

            var result = cm.emval_test_call_method2(foo, {field: [1, 2, 3]}, 7);
            assert.deepEqual({field: [1, 2, 3]}, result);
            assert.deepEqual([[{field: [1, 2, 3]}, 7]], calls);
        },

        "test can call interface methods that return nothing": function() {
            var calls = [];
            var foo = {
                method3: function() {
                    calls.push('called');
                }
            };
            cm.emval_test_call_method3(foo);
            assert.deepEqual(['called'], calls);
        },
    });

    test('emscripten::internal::optional', function () {
        assert.true(cm.optional_test_copy());
        assert.false(cm.optional_test_copy2());
    });
});
