module({
    Emscripten: '../../../../build/embind_test.js',
}, function(imports) {
    var cm = imports.Emscripten;

    var CheckForLeaks = fixture("check for leaks", function() {
        this.setUp(function() {
            cm.setDelayFunction(undefined);

            if (typeof INVOKED_FROM_EMSCRIPTEN_TEST_RUNNER === "undefined") { // TODO: Enable this to work in Emscripten runner as well!
                cm._mallocDebug(2);
                assert.equal(0, cm.count_emval_handles());
                cm._mallocAssertAllMemoryFree();
            }
        });
        this.tearDown(function() {
            cm.flushPendingDeletes();
            if (typeof INVOKED_FROM_EMSCRIPTEN_TEST_RUNNER === "undefined") { // TODO: Enable this to work in Emscripten runner as well!
                cm._mallocAssertAllMemoryFree();
                assert.equal(0, cm.count_emval_handles());
            }
        });
    });

    var BaseFixture = CheckForLeaks;

    BaseFixture.extend("temp jig", function() {
        test("temp test", function() {
        });
    });

    if (typeof INVOKED_FROM_EMSCRIPTEN_TEST_RUNNER === "undefined") { // TODO: Enable this to work in Emscripten runner as well!

    BaseFixture.extend("leak testing", function() {
        test("no memory allocated at start of test", function() {
            cm._mallocAssertAllMemoryFree();
        });
        test("assert when memory is allocated", function() {
            var ptr = cm._malloc(42);
            assert.throws(cm._MemoryAllocationError, function() {
                cm._mallocAssertAllMemoryFree();
            });
            cm._free(ptr);
        });
        test("allocated memory counts down again for free", function() {
            var ptr = cm._malloc(42);
            cm._free(ptr);
            cm._mallocAssertAllMemoryFree();
        });
        test("free without malloc throws MemoryAllocationError", function() {
            var ptr = cm._malloc(42);
            cm._free(ptr);
            assert.throws(cm._MemoryAllocationError, function() {
                cm._free(ptr);
            });
        });
    });
    
    }

    BaseFixture.extend("access to base class members", function() {
        test("method name in derived class silently overrides inherited name", function() {
            var derived = new cm.Derived();
            assert.equal("Derived", derived.getClassName());
            derived.delete();
        });
        test("can reference base method from derived class", function(){
            var derived = new cm.Derived();
            assert.equal("Base", derived.getClassNameFromBase());
            derived.delete();
        });
        test("can reference base method from doubly derived class", function() {
            var derivedTwice = new cm.DerivedTwice();
            assert.equal("Base", derivedTwice.getClassNameFromBase());
            derivedTwice.delete();
        });
        test("can reference base method through unbound classes", function() {
            var derivedThrice = new cm.DerivedThrice();
            assert.equal("Base", derivedThrice.getClassNameFromBase());
            derivedThrice.delete();
        });
        test("property name in derived class hides identically named property in base class for set", function() {
            var derived = new cm.Derived();
            derived.setMember(7);

            derived.member = 17;

            assert.equal(17, derived.getMember());
            derived.delete();
        });
        test("can reference base property from derived class for get", function(){
            var derived = new cm.Derived();
            derived.setBaseMember(5);

            assert.equal(5, derived.baseMember);

            derived.delete();
        });
        test("can reference property of any base class for get when multiply derived", function(){
            var derived = new cm.MultiplyDerived();
            derived.setBaseMember(11);

            assert.equal(11, derived.baseMember);

            derived.delete();
        });
        test("can reference base property from derived class for set", function(){
            var derived = new cm.Derived();

            derived.baseMember = 32;

            assert.equal(32, derived.getBaseMember());
            derived.delete();
        });
        test("can reference property of any base for set when multiply derived", function(){
            var derived = new cm.MultiplyDerived();
            derived.setBaseMember(97);

            derived.baseMember = 32;

            assert.equal(32, derived.getBaseMember());
            derived.delete();
        });
        test("can reach around derived property to access base property with same name for get", function() {
            var derived = new cm.Derived();
            derived.setMember(12);
            derived.delete();
        });

        test("if deriving from second base adjusts pointer", function() {
            var derived = new cm.HasTwoBases;
            assert.equal("Base2", derived.getField());
            derived.delete();
        });

        test("properties adjust pointer", function() {
            var derived = new cm.HasTwoBases;
            derived.field = "Foo";
            assert.equal("Foo", derived.getField());
            assert.equal("Foo", derived.field);
            derived.delete();
        });

        test("calling method on unrelated class throws error", function() {
            var a = new cm.HasTwoBases;
            var e = assert.throws(cm.BindingError, function() {
                cm.Derived.prototype.setMember.call(a, "foo");
            });
            assert.equal('Expected null or instance of Derived, got an instance of Base2', e.message);
            a.delete();

            // Base1 and Base2 both have the method 'getField()' exposed - make sure
            // that calling the Base2 function with a 'this' instance of Base1 doesn't accidentally work!
            var b = new cm.Base1;
            var e = assert.throws(cm.BindingError, function() {
                cm.Base2.prototype.getField.call(b);
            });
            assert.equal('Expected null or instance of Base2, got an instance of Base1', e.message);
            b.delete();
        });

        test("calling method with invalid this throws error", function() {
            var e = assert.throws(cm.BindingError, function() {
                cm.Derived.prototype.setMember.call(undefined, "foo");
            });
            assert.equal('Cannot pass "[object global]" as a Derived*', e.message);

            var e = assert.throws(cm.BindingError, function() {
                cm.Derived.prototype.setMember.call(true, "foo");
            });
            assert.equal('Cannot pass "true" as a Derived*', e.message);

            var e = assert.throws(cm.BindingError, function() {
                cm.Derived.prototype.setMember.call(null, "foo");
            });
            assert.equal('Cannot pass "[object global]" as a Derived*', e.message);

            var e = assert.throws(cm.BindingError, function() {
                cm.Derived.prototype.setMember.call(42, "foo");
            });
            assert.equal('Cannot pass "42" as a Derived*', e.message);

            var e = assert.throws(cm.BindingError, function() {
                cm.Derived.prototype.setMember.call("this", "foo");
            });
            assert.equal('Cannot pass "this" as a Derived*', e.message);

            var e = assert.throws(cm.BindingError, function() {
                cm.Derived.prototype.setMember.call({}, "foo");
            });
            assert.equal('Cannot pass "[object Object]" as a Derived*', e.message);
        });

        test("setting and getting property on unrelated class throws error", function() {
            var a = new cm.HasTwoBases;
            var e = assert.throws(cm.BindingError, function() {
                Object.getOwnPropertyDescriptor(cm.HeldBySmartPtr.prototype, 'i').set.call(a, 10);
            });
            assert.equal('HeldBySmartPtr.i setter incompatible with "this" of type HasTwoBases', e.message);

            var e = assert.throws(cm.BindingError, function() {
                Object.getOwnPropertyDescriptor(cm.HeldBySmartPtr.prototype, 'i').get.call(a);
            });
            assert.equal('HeldBySmartPtr.i getter incompatible with "this" of type HasTwoBases', e.message);

            a.delete();
        });
    });

    BaseFixture.extend("automatic upcasting of parameters passed to C++", function() {
        // raw
        test("raw pointer argument is upcast to parameter type", function() {
            var derived = new cm.Derived();
            var name = cm.embind_test_get_class_name_via_base_ptr(derived);
            assert.equal("Base", name);
            derived.delete();
        });

        test("automatic raw pointer upcasting works with multiple inheritance", function() {
            var derived = new cm.MultiplyDerived();
            var name = cm.embind_test_get_class_name_via_base_ptr(derived);
            assert.equal("Base", name);
            derived.delete();
        });

        test("automatic raw pointer upcasting does not change local pointer", function() {
            var derived = new cm.MultiplyDerived();
            cm.embind_test_get_class_name_via_base_ptr(derived);
            assert.equal("MultiplyDerived", derived.getClassName());
            derived.delete();
        });

        test("passing incompatible raw pointer to method throws exception", function() {
            var base = new cm.Base();
            assert.throws(cm.BindingError, function() {
                cm.embind_test_get_class_name_via_second_base_ptr(base);
            });
            base.delete();
        });

        // raw polymorphic
        test("polymorphic raw pointer argument is upcast to parameter type", function() {
            var derived = new cm.PolyDerived();
            var name = cm.embind_test_get_class_name_via_polymorphic_base_ptr(derived);
            assert.equal("PolyBase", name);
            derived.delete();
        });

        test("automatic polymorphic raw pointer upcasting works with multiple inheritance", function() {
            var derived = new cm.PolyMultiplyDerived();
            var name = cm.embind_test_get_class_name_via_polymorphic_base_ptr(derived);
            assert.equal("PolyBase", name);
            derived.delete();
        });

        test("passing incompatible raw polymorphic pointer to method throws exception", function() {
            var base = new cm.PolyBase();
            assert.throws(cm.BindingError, function() {
                cm.embind_test_get_class_name_via_polymorphic_second_base_ptr(base);
            });
            base.delete();

        });

        // smart
        test("can pass smart pointer to raw pointer parameter", function() {
            var smartBase = cm.embind_test_return_smart_base_ptr();
            assert.equal("Base", cm.embind_test_get_class_name_via_base_ptr(smartBase));
            smartBase.delete();
        });

        test("can pass and upcast smart pointer to raw pointer parameter", function() {
            var smartDerived = cm.embind_test_return_smart_derived_ptr();
            assert.equal("Base", cm.embind_test_get_class_name_via_base_ptr(smartDerived));
            smartDerived.delete();
        });

        test("smart pointer argument is upcast to parameter type", function() {
            var derived = cm.embind_test_return_smart_derived_ptr();
            assert.instanceof(derived, cm.Derived);
            assert.instanceof(derived, cm.Base);
            var name = cm.embind_test_get_class_name_via_smart_base_ptr(derived);
            assert.equal("Base", name);
            derived.delete();
        });

        test("return smart derived ptr as base", function() {
            var derived = cm.embind_test_return_smart_derived_ptr_as_base();
            assert.equal("PolyDerived", cm.embind_test_get_virtual_class_name_via_smart_polymorphic_base_ptr(derived));
            assert.equal("PolyDerived", derived.getClassName());
            derived.delete();
        });

        test("return smart derived ptr as val", function() {
            var derived = cm.embind_test_return_smart_derived_ptr_as_val();
            assert.equal("PolyDerived", cm.embind_test_get_virtual_class_name_via_smart_polymorphic_base_ptr(derived));
            derived.delete();
        });

        test("automatic smart pointer upcasting works with multiple inheritance", function() {
            var derived = cm.embind_test_return_smart_multiply_derived_ptr();
            var name = cm.embind_test_get_class_name_via_smart_base_ptr(derived);
            assert.equal("Base", name);
            derived.delete();
        });

        test("automatically upcasted smart pointer parameter shares ownership with original argument", function() {
            var derived = cm.embind_test_return_smart_multiply_derived_ptr();
            assert.equal(1, cm.MultiplyDerived.getInstanceCount());
            cm.embind_save_smart_base_pointer(derived);
            assert.equal(1, cm.MultiplyDerived.getInstanceCount());
            derived.delete();
            assert.equal(1, cm.MultiplyDerived.getInstanceCount());
            cm.embind_save_smart_base_pointer(null);
            assert.equal(0, cm.MultiplyDerived.getInstanceCount());
        });

        // smart polymorphic
        test("smart polymorphic pointer argument is upcast to parameter type", function() {
            var derived = cm.embind_test_return_smart_polymorphic_derived_ptr();
            var name = cm.embind_test_get_class_name_via_smart_polymorphic_base_ptr(derived);
            assert.equal("PolyBase", name);
            derived.delete();
        });

        test("automatic smart polymorphic pointer upcasting works with multiple inheritance", function() {
            var derived = cm.embind_test_return_smart_polymorphic_multiply_derived_ptr();
            var name = cm.embind_test_get_class_name_via_smart_polymorphic_base_ptr(derived);
            assert.equal("PolyBase", name);
            derived.delete();
        });
    });

    BaseFixture.extend("automatic downcasting of return values received from C++", function() {
        // raw
        test("non-polymorphic raw pointers are not downcast and do not break automatic casting mechanism", function() {
            var base = cm.embind_test_return_raw_derived_ptr_as_base();
            assert.equal("Base", base.getClassName());
            assert.instanceof(base, cm.Base);
            base.delete();
        });

        // raw polymorphic
        test("polymorphic raw pointer return value is downcast to allocated type (if that is bound)", function() {
            var derived = cm.embind_test_return_raw_polymorphic_derived_ptr_as_base();
            assert.instanceof(derived, cm.PolyBase);
            assert.instanceof(derived, cm.PolyDerived);
            assert.equal("PolyDerived", derived.getClassName());
            var siblingDerived = cm.embind_test_return_raw_polymorphic_sibling_derived_ptr_as_base();
            assert.equal("PolySiblingDerived", siblingDerived.getClassName());
            siblingDerived.delete();
            derived.delete();
        });

        test("polymorphic raw pointer return value is downcast to the most derived bound type", function() {
            var derivedThrice = cm.embind_test_return_raw_polymorphic_derived_four_times_not_bound_as_base();
            // if the actual returned type is not bound, then don't assume anything
            assert.equal("PolyBase", derivedThrice.getClassName());
            // if we ever fix this, then reverse the assertion
            //assert.equal("PolyDerivedThrice", derivedThrice.getClassName());
            derivedThrice.delete();
        });

        test("polymorphic smart pointer return value is downcast to the most derived type which has an associated smart pointer", function() {
            var derived = cm.embind_test_return_poly_derived_twice_without_smart_pointer_as_poly_base();
            // if the actual returned type is not bound, then don't assume anything
            assert.equal("PolyBase", derived.getClassName());
            // if we ever fix this, then remove the assertion
            //assert.equal("PolyDerived", derived.getClassName());
            derived.delete();
        });

        test("automatic downcasting works with multiple inheritance", function() {
            var base = cm.embind_test_return_raw_polymorphic_multiply_derived_ptr_as_base();
            var secondBase = cm.embind_test_return_raw_polymorphic_multiply_derived_ptr_as_second_base();
            assert.equal("PolyMultiplyDerived", base.getClassName());
            // embind does not support multiple inheritance
            //assert.equal("PolyMultiplyDerived", secondBase.getClassName());
            secondBase.delete();
            base.delete();
        });

        // smart
        test("non-polymorphic smart pointers do not break automatic casting mechanism", function() {
        });

        // smart polymorphic
        test("automatically downcasting a smart pointer does not change the underlying pointer", function() {
            cm.PolyDerived.setPtrDerived();
            assert.equal("PolyBase", cm.PolyDerived.getPtrClassName());
            var derived = cm.PolyDerived.getPtr();
            assert.equal("PolyDerived", derived.getClassName());
            assert.equal("PolyBase", cm.PolyDerived.getPtrClassName());
            derived.delete();
            cm.PolyDerived.releasePtr();
        });

        test("polymorphic smart pointer return value is actual allocated type (when bound)", function() {
            var derived = cm.embind_test_return_smart_polymorphic_derived_ptr_as_base();
            assert.equal("PolyDerived", derived.getClassName());

            var siblingDerived = cm.embind_test_return_smart_polymorphic_sibling_derived_ptr_as_base();
            assert.equal("PolySiblingDerived", siblingDerived.getClassName());

            siblingDerived.delete();
            derived.delete();
        });
    });

    BaseFixture.extend("string", function() {
        test("non-ascii strings", function() {
            var expected = '';
            for (var i = 0; i < 128; ++i) {
                expected += String.fromCharCode(128 + i);
            }
            assert.equal(expected, cm.get_non_ascii_string());
        });

        test("passing non-8-bit strings from JS to std::string throws", function() {
            assert.throws(cm.BindingError, function() {
                cm.emval_test_take_and_return_std_string("\u1234");
            });
        });

        test("can't pass integers as strings", function() {
            var e = assert.throws(cm.BindingError, function() {
                cm.emval_test_take_and_return_std_string(10);
            });
        });

        test("can pass Uint8Array to std::string", function() {
            var e = cm.emval_test_take_and_return_std_string(new Uint8Array([65, 66, 67, 68]));
            assert.equal('ABCD', e);
        });

        test("can pass Int8Array to std::string", function() {
            var e = cm.emval_test_take_and_return_std_string(new Int8Array([65, 66, 67, 68]));
            assert.equal('ABCD', e);
        });

        test("can pass ArrayBuffer to std::string", function() {
            var e = cm.emval_test_take_and_return_std_string((new Int8Array([65, 66, 67, 68])).buffer);
            assert.equal('ABCD', e);
        });

        test("non-ascii wstrings", function() {
            var expected = String.fromCharCode(10) +
                String.fromCharCode(1234) +
                String.fromCharCode(2345) +
                String.fromCharCode(65535);
            assert.equal(expected, cm.get_non_ascii_wstring());
        });

        test("passing unicode string into C++", function() {
            var expected = String.fromCharCode(10) +
                String.fromCharCode(1234) +
                String.fromCharCode(2345) +
                String.fromCharCode(65535);
            assert.equal(expected, cm.take_and_return_std_wstring(expected));
        });
    });

    BaseFixture.extend("embind", function() {
        test("value creation", function() {
            assert.equal(15, cm.emval_test_new_integer());
            assert.equal("Hello everyone", cm.emval_test_new_string());
            assert.equal("Hello everyone", cm.emval_test_get_string_from_val({key: "Hello everyone"}));

            var object = cm.emval_test_new_object();
            assert.equal('bar', object.foo);
            assert.equal(1, object.baz);
        });

        test("pass const reference to primitive", function() {
            assert.equal(3, cm.const_ref_adder(1, 2));
        });

        test("passthrough", function() {
            var a = {foo: 'bar'};
            var b = cm.emval_test_passthrough(a);
            a.bar = 'baz';
            assert.equal('baz', b.bar);

            assert.equal(0, cm.count_emval_handles());
        });

        test("void return converts to undefined", function() {
            assert.equal(undefined, cm.emval_test_return_void());
        });

        test("booleans can be marshalled", function() {
            assert.equal(false, cm.emval_test_not(true));
            assert.equal(true, cm.emval_test_not(false));
        });

        test("can pass booleans as integers", function() {
            assert.equal(1, cm.emval_test_as_unsigned(true));
            assert.equal(0, cm.emval_test_as_unsigned(false));
        });

        test("can pass booleans as floats", function() {
            assert.equal(2, cm.const_ref_adder(true, true));
        });

        test("convert double to unsigned", function() {
            var rv = cm.emval_test_as_unsigned(1.5);
            assert.equal('number', typeof rv);
            assert.equal(1, rv);
            assert.equal(0, cm.count_emval_handles());
        });

        test("get length of array", function() {
            assert.equal(10, cm.emval_test_get_length([0, 1, 2, 3, 4, 5, 'a', 'b', 'c', 'd']));
            assert.equal(0, cm.count_emval_handles());
        });

        test("add a bunch of things", function() {
            assert.equal(66.0, cm.emval_test_add(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11));
            assert.equal(0, cm.count_emval_handles());
        });

        test("sum array", function() {
            assert.equal(66, cm.emval_test_sum([1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11]));
            assert.equal(0, cm.count_emval_handles());
        });

        test("strings", function() {
            assert.equal("foobar", "foo" + "bar");
            assert.equal("foobar", cm.emval_test_take_and_return_std_string("foobar"));

            assert.equal("foobar", cm.emval_test_take_and_return_std_string_const_ref("foobar"));
        });

        test("nuls pass through strings", function() {
            assert.equal("foo\0bar", cm.emval_test_take_and_return_std_string("foo\0bar"));
        });

        test("no memory leak when passing strings in by const reference", function() {
            cm.emval_test_take_and_return_std_string_const_ref("foobar");
        });

        test("can create new object", function() {
            assert.deepEqual({}, cm.embind_test_new_Object());
        });

        test("can invoke constructors with arguments", function() {
            function constructor(i, s, argument) {
                this.i = i;
                this.s = s;
                this.argument = argument;
            }
            constructor.prototype.method = function() {
                return this.argument;
            };
            var x = {};
            var instance = cm.embind_test_new_factory(constructor, x);
            assert.equal(10, instance.i);
            assert.equal("hello", instance.s);
            assert.equal(x, instance.argument);
        });

        test("can return module property objects", function() {
            assert.equal(cm.HEAP8, cm.get_module_property("HEAP8"));
        });

        test("can return big class instances", function() {
            var c = cm.embind_test_return_big_class_instance();
            assert.equal(11, c.member);
            c.delete();
        });

        test("can return small class instances", function() {
            var c = cm.embind_test_return_small_class_instance();
            assert.equal(7, c.member);
            c.delete();
        });

        test("can pass small class instances", function() {
            var c = new cm.SmallClass();
            var m = cm.embind_test_accept_small_class_instance(c);
            assert.equal(7, m);
            c.delete();
        });

        test("can pass big class instances", function() {
            var c = new cm.BigClass();
            var m = cm.embind_test_accept_big_class_instance(c);
            assert.equal(11, m);
            c.delete();
        });

        test("can get member classes then call its member functions", function() {
            var p = new cm.ParentClass();
            var c = p.getBigClass();
            var m = c.getMember();
            assert.equal(11, m);
            c.delete();
            p.delete();
        });

        test('C++ -> JS primitive type range checks', function() {
            // all types should have zero.
            assert.equal("0", cm.char_to_string(0));
            assert.equal("0", cm.signed_char_to_string(0));
            assert.equal("0", cm.unsigned_char_to_string(0));
            assert.equal("0", cm.short_to_string(0));
            assert.equal("0", cm.unsigned_short_to_string(0));
            assert.equal("0", cm.int_to_string(0));
            assert.equal("0", cm.unsigned_int_to_string(0));
            assert.equal("0", cm.long_to_string(0));
            assert.equal("0", cm.unsigned_long_to_string(0));
            
            // all types should have positive values.
            assert.equal("5", cm.char_to_string(5));
            assert.equal("5", cm.signed_char_to_string(5));
            assert.equal("5", cm.unsigned_char_to_string(5));
            assert.equal("5", cm.short_to_string(5));
            assert.equal("5", cm.unsigned_short_to_string(5));
            assert.equal("5", cm.int_to_string(5));
            assert.equal("5", cm.unsigned_int_to_string(5));
            assert.equal("5", cm.long_to_string(5));
            assert.equal("5", cm.unsigned_long_to_string(5));

            // signed types should have negative values.
            assert.equal("-5", cm.char_to_string(-5)); // Assuming char as signed.
            assert.equal("-5", cm.signed_char_to_string(-5));
            assert.equal("-5", cm.short_to_string(-5));
            assert.equal("-5", cm.int_to_string(-5));
            assert.equal("-5", cm.long_to_string(-5));

            // assumptions: char == signed char == 8 bits
            //              unsigned char == 8 bits
            //              short == 16 bits
            //              int == long == 32 bits

            // all types should have their max positive values.
            assert.equal("127", cm.char_to_string(127));
            assert.equal("127", cm.signed_char_to_string(127));
            assert.equal("255", cm.unsigned_char_to_string(255));
            assert.equal("32767", cm.short_to_string(32767));
            assert.equal("65535", cm.unsigned_short_to_string(65535));
            assert.equal("2147483647", cm.int_to_string(2147483647));
            assert.equal("4294967295", cm.unsigned_int_to_string(4294967295));
            assert.equal("2147483647", cm.long_to_string(2147483647));
            assert.equal("4294967295", cm.unsigned_long_to_string(4294967295));

            // signed types should have their min negative values.
            assert.equal("-128", cm.char_to_string(-128));
            assert.equal("-128", cm.signed_char_to_string(-128));
            assert.equal("-32768", cm.short_to_string(-32768));
            assert.equal("-2147483648", cm.int_to_string(-2147483648));
            assert.equal("-2147483648", cm.long_to_string(-2147483648));
            
            // passing out of range values should fail.
            assert.throws(TypeError, function() { cm.char_to_string(-129); });
            assert.throws(TypeError, function() { cm.char_to_string(128); });
            assert.throws(TypeError, function() { cm.signed_char_to_string(-129); });
            assert.throws(TypeError, function() { cm.signed_char_to_string(128); });
            assert.throws(TypeError, function() { cm.unsigned_char_to_string(-1); });
            assert.throws(TypeError, function() { cm.unsigned_char_to_string(256); });
            assert.throws(TypeError, function() { cm.short_to_string(-32769); });
            assert.throws(TypeError, function() { cm.short_to_string(32768); });
            assert.throws(TypeError, function() { cm.unsigned_short_to_string(-1); });
            assert.throws(TypeError, function() { cm.unsigned_short_to_string(65536); });
            assert.throws(TypeError, function() { cm.int_to_string(-2147483649); });
            assert.throws(TypeError, function() { cm.int_to_string(2147483648); });
            assert.throws(TypeError, function() { cm.unsigned_int_to_string(-1); });
            assert.throws(TypeError, function() { cm.unsigned_int_to_string(4294967296); });
            assert.throws(TypeError, function() { cm.long_to_string(-2147483649); });
            assert.throws(TypeError, function() { cm.long_to_string(2147483648); });
            assert.throws(TypeError, function() { cm.unsigned_long_to_string(-1); });
            assert.throws(TypeError, function() { cm.unsigned_long_to_string(4294967296); });
        });

        test("access multiple class ctors", function() {
            var a = new cm.MultipleCtors(10);
            assert.equal(a.WhichCtorCalled(), 1);
            var b = new cm.MultipleCtors(20, 20);
            assert.equal(b.WhichCtorCalled(), 2);
            var c = new cm.MultipleCtors(30, 30, 30);
            assert.equal(c.WhichCtorCalled(), 3);
            a.delete();
            b.delete();
            c.delete();
        });

        test("access multiple smart ptr ctors", function() {
            var a = new cm.MultipleSmartCtors(10);
            assert.equal(a.WhichCtorCalled(), 1);
            var b = new cm.MultipleCtors(20, 20);
            assert.equal(b.WhichCtorCalled(), 2);
            a.delete();
            b.delete();
        });

        test("wrong number of constructor arguments throws", function() {
            assert.throws(cm.BindingError, function() { new cm.MultipleCtors(); });
            assert.throws(cm.BindingError, function() { new cm.MultipleCtors(1,2,3,4); });
        });

        test("overloading of free functions", function() {
            var a = cm.overloaded_function(10);
            assert.equal(a, 1);
            var b = cm.overloaded_function(20, 20);
            assert.equal(b, 2);
        });

        test("wrong number of arguments to an overloaded free function", function() {
            assert.throws(cm.BindingError, function() { cm.overloaded_function(); });
            assert.throws(cm.BindingError, function() { cm.overloaded_function(30, 30, 30); });
        });

        test("overloading of class member functions", function() {
            var foo = new cm.MultipleOverloads();
            assert.equal(foo.Func(10), 1);
            assert.equal(foo.WhichFuncCalled(), 1);
            assert.equal(foo.Func(20, 20), 2);
            assert.equal(foo.WhichFuncCalled(), 2);
            foo.delete();
        });

        test("wrong number of arguments to an overloaded class member function", function() {
            var foo = new cm.MultipleOverloads();
            assert.throws(cm.BindingError, function() { foo.Func(); });
            assert.throws(cm.BindingError, function() { foo.Func(30, 30, 30); });
            foo.delete();
        });

        test("wrong number of arguments to an overloaded class static function", function() {
            assert.throws(cm.BindingError, function() { cm.MultipleOverloads.StaticFunc(); });
            assert.throws(cm.BindingError, function() { cm.MultipleOverloads.StaticFunc(30, 30, 30); });
        });

        test("overloading of derived class member functions", function() {
            var foo = new cm.MultipleOverloadsDerived();
            
            // NOTE: In C++, default lookup rules will hide overloads from base class if derived class creates them.
            // In JS, we make the base class overloads implicitly available. In C++, they would need to be explicitly
            // invoked, like foo.MultipleOverloads::Func(10);
            assert.equal(foo.Func(10), 1);
            assert.equal(foo.WhichFuncCalled(), 1);
            assert.equal(foo.Func(20, 20), 2);
            assert.equal(foo.WhichFuncCalled(), 2);

            assert.equal(foo.Func(30, 30, 30), 3);
            assert.equal(foo.WhichFuncCalled(), 3);
            assert.equal(foo.Func(40, 40, 40, 40), 4);
            assert.equal(foo.WhichFuncCalled(), 4);
            foo.delete();
        });
        
        test("overloading of class static functions", function() {
            assert.equal(cm.MultipleOverloads.StaticFunc(10), 1);
            assert.equal(cm.MultipleOverloads.WhichStaticFuncCalled(), 1);
            assert.equal(cm.MultipleOverloads.StaticFunc(20, 20), 2);
            assert.equal(cm.MultipleOverloads.WhichStaticFuncCalled(), 2);
        });

        test("overloading of derived class static functions", function() {
            assert.equal(cm.MultipleOverloadsDerived.StaticFunc(30, 30, 30), 3);
            // TODO: Cannot access static member functions of a Base class via Derived.
//            assert.equal(cm.MultipleOverloadsDerived.WhichStaticFuncCalled(), 3);
            assert.equal(cm.MultipleOverloads.WhichStaticFuncCalled(), 3);
            assert.equal(cm.MultipleOverloadsDerived.StaticFunc(40, 40, 40, 40), 4);
            // TODO: Cannot access static member functions of a Base class via Derived.
//            assert.equal(cm.MultipleOverloadsDerived.WhichStaticFuncCalled(), 4);
            assert.equal(cm.MultipleOverloads.WhichStaticFuncCalled(), 4);
        });
/*
        test("can get templated member classes then call its member functions", function() {
            var p = new cm.ContainsTemplatedMemberClass();
            var c = p.getTestTemplate();
            var m = c.getMember(1);
            assert.equal(87, m);
            c.delete();
            p.delete();
        });
*/
    });

    BaseFixture.extend("vector", function() {
        test("std::vector returns as an native object", function() {
            var vec = cm.emval_test_return_vector();

            assert.equal(3, vec.size());
            assert.equal(10, vec.get(0));
            assert.equal(20, vec.get(1));
            assert.equal(30, vec.get(2));
            vec.delete();
        });

        test("out of bounds std::vector access returns undefined", function() {
            var vec = cm.emval_test_return_vector();

            assert.throws(TypeError, function() { vec.get(-1); });
            assert.equal(undefined, vec.get(4));

            vec.delete();
        });

        test("std::vector<std::shared_ptr<>> can be passed back", function() {
            var vec = cm.emval_test_return_shared_ptr_vector();

            assert.equal(2, vec.size());
            var str0 = vec.get(0);
            var str1 = vec.get(1);

            assert.equal('string #1', str0.get());
            assert.equal('string #2', str1.get());
            str0.delete();
            str1.delete();

            vec.delete();
        });

        test("objects can be pushed back", function() {
            var vectorHolder = new cm.VectorHolder();
            var vec = vectorHolder.get();
            assert.equal(2, vec.size());

            var str = new cm.StringHolder('abc');
            vec.push_back(str);
            str.delete();
            assert.equal(3, vec.size());
            var str = vec.get(2);
            assert.equal('abc', str.get());

            str.delete();
            vec.delete();
            vectorHolder.delete();
        });

        test("can get elements with array operator", function(){
            var vec = cm.emval_test_return_vector();
            assert.equal(10, vec.get(0));
            vec.delete();
        });

        test("can set elements with array operator", function() {
            var vec = cm.emval_test_return_vector();
            assert.equal(10, vec.get(0));
            vec.set(2, 60);
            assert.equal(60, vec.get(2));
            vec.delete();
        });

        test("can set and get objects", function() {
            var vec = cm.emval_test_return_shared_ptr_vector();
            var str = vec.get(0);
            assert.equal('string #1', str.get());
            str.delete();
            vec.delete();
        });
    });

    BaseFixture.extend("map", function() {
       test("std::map returns as native object", function() {
           var map = cm.embind_test_get_string_int_map();

           assert.equal(2, map.size());
           assert.equal(1, map.get("one"));
           assert.equal(2, map.get("two"));

           map.delete();
       });

       test("std::map can set keys and values", function() {
           var map = cm.embind_test_get_string_int_map();

           assert.equal(2, map.size());

           map.set("three", 3);

           assert.equal(3, map.size());
           assert.equal(3, map.get("three"));

           map.set("three", 4);

           assert.equal(3, map.size());
           assert.equal(4, map.get("three"));

           map.delete();
       });
    });

    BaseFixture.extend("functors", function() {
        test("can get and call function ptrs", function() {
            var ptr = cm.emval_test_get_function_ptr();
            assert.equal("foobar", ptr.opcall("foobar"));
            ptr.delete();
        });

        test("can pass functor to C++", function() {
            var ptr = cm.emval_test_get_function_ptr();
            assert.equal("asdf", cm.emval_test_take_and_call_functor(ptr));
            ptr.delete();
        });

        test("can clone handles", function() {
            var a = cm.emval_test_get_function_ptr();
            assert.equal(1, a.$$.count.value);
            var b = a.clone();
            assert.equal(2, a.$$.count.value);
            assert.equal(2, b.$$.count.value);
            a.delete();

            assert.throws(cm.BindingError, function() {
                a.delete();
            });
            b.delete();
        });
    });

    BaseFixture.extend("classes", function() {
        test("class instance", function() {
            var a = {foo: 'bar'};
            assert.equal(0, cm.count_emval_handles());
            var c = new cm.ValHolder(a);
            assert.equal(1, cm.count_emval_handles());
            assert.equal('bar', c.getVal().foo);
            assert.equal(1, cm.count_emval_handles());

            c.setVal('1234');
            assert.equal('1234', c.getVal());

            c.delete();
            assert.equal(0, cm.count_emval_handles());
        });

        test("class properties can be methods", function() {
            var a = {};
            var b = {foo: 'foo'};
            var c = new cm.ValHolder(a);
            assert.equal(a, c.val);
            c.val = b;
            assert.equal(b, c.val);
            c.delete();
        });

        test("class properties can be read-only", function() {
            var a = {};
            var h = new cm.ValHolder(a);
            assert.equal(a, h.val_readonly);
            var e = assert.throws(cm.BindingError, function() {
                h.val_readonly = 10;
            });
            assert.equal('ValHolder.val_readonly is a read-only property', e.message);
            h.delete();
        });

        test("read-only member field", function() {
            var a = new cm.HasReadOnlyProperty(10);
            assert.equal(10, a.i);
            var e = assert.throws(cm.BindingError, function() {
                a.i = 20;
            });
            assert.equal('HasReadOnlyProperty.i is a read-only property', e.message);
            a.delete();
        });

        test("class instance $$ property is non-enumerable", function() {
            var c = new cm.ValHolder(undefined);
            assert.deepEqual([], Object.keys(c));
            var d = c.clone();
            c.delete();

            assert.deepEqual([], Object.keys(d));
            d.delete();
        });

        test("class methods", function() {
            assert.equal(10, cm.ValHolder.some_class_method(10));

            var b = cm.ValHolder.makeValHolder("foo");
            assert.equal("foo", b.getVal());
            b.delete();
        });

        test("can't call methods on deleted class instances", function() {
            var c = new cm.ValHolder(undefined);
            c.delete();
            assert.throws(cm.BindingError, function() {
                c.getVal();
            });
            assert.throws(cm.BindingError, function() {
                c.delete();
            });
        });

        test("calling constructor without new raises BindingError", function() {
            var e = assert.throws(cm.BindingError, function() {
                cm.ValHolder(undefined);
            });
            assert.equal("Use 'new' to construct ValHolder", e.message);
        });

        test("can return class instances by value", function() {
            var c = cm.emval_test_return_ValHolder();
            assert.deepEqual({}, c.getVal());
            c.delete();
        });

        test("can pass class instances to functions by reference", function() {
            var a = {a:1};
            var c = new cm.ValHolder(a);
            cm.emval_test_set_ValHolder_to_empty_object(c);
            assert.deepEqual({}, c.getVal());
            c.delete();
        });

        test("can pass smart pointer by reference", function() {
            var base = cm.embind_test_return_smart_base_ptr();
            var name = cm.embind_test_get_class_name_via_reference_to_smart_base_ptr(base);
            assert.equal("Base", name);
            base.delete();
        });

        test("can pass smart pointer by value", function() {
            var base = cm.embind_test_return_smart_base_ptr();
            var name = cm.embind_test_get_class_name_via_smart_base_ptr(base);
            assert.equal("Base", name);
            base.delete();
        });

        // todo: fix this
        // This test does not work because we make no provision for argument values
        // having been changed after returning from a C++ routine invocation. In
        // this specific case, the original pointee of the smart pointer was
        // freed and replaced by a new one, but the ptr in our local handle
        // was never updated after returning from the call.
        test("can modify smart pointers passed by reference", function() {
//            var base = cm.embind_test_return_smart_base_ptr();
//            cm.embind_modify_smart_pointer_passed_by_reference(base);
//            assert.equal("Changed", base.getClassName());
//            base.delete();
        });

        test("can not modify smart pointers passed by value", function() {
            var base = cm.embind_test_return_smart_base_ptr();
            cm.embind_attempt_to_modify_smart_pointer_when_passed_by_value(base);
            assert.equal("Base", base.getClassName());
            base.delete();
        });

        test("const return value", function() {
            var c = new cm.ValHolder("foo");
            assert.equal("foo", c.getConstVal());
            c.delete();
        });

        test("return object by const ref", function() {
            var c = new cm.ValHolder("foo");
            assert.equal("foo", c.getValConstRef());
            c.delete();
        });

        test("instanceof", function() {
            var c = new cm.ValHolder("foo");
            assert.instanceof(c, cm.ValHolder);
            c.delete();
        });

        test("can access struct fields", function() {
            var c = new cm.CustomStruct();
            assert.equal(10, c.field);
            assert.equal(10, c.getField());
            c.delete();
        });

        test("can set struct fields", function() {
            var c = new cm.CustomStruct();
            c.field = 15;
            assert.equal(15, c.field);
            c.delete();
        });

        test("assignment returns value", function() {
            var c = new cm.CustomStruct();
            assert.equal(15, c.field = 15);
            c.delete();
        });

        test("assigning string to integer raises TypeError", function() {
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
        });

        test("can return tuples by value", function() {
            var c = cm.emval_test_return_TupleVector();
            assert.deepEqual([1, 2, 3, 4], c);
        });

        test("tuples can contain tuples", function() {
            var c = cm.emval_test_return_TupleVectorTuple();
            assert.deepEqual([[1, 2, 3, 4]], c);
        });

        test("can pass tuples by value", function() {
            var c = cm.emval_test_take_and_return_TupleVector([4, 5, 6, 7]);
            assert.deepEqual([4, 5, 6, 7], c);
        });

        test("can return structs by value", function() {
            var c = cm.emval_test_return_StructVector();
            assert.deepEqual({x: 1, y: 2, z: 3, w: 4}, c);
        });

        test("can pass structs by value", function() {
            var c = cm.emval_test_take_and_return_StructVector({x: 4, y: 5, z: 6, w: 7});
            assert.deepEqual({x: 4, y: 5, z: 6, w: 7}, c);
        });

        test("can pass and return tuples in structs", function() {
            var d = cm.emval_test_take_and_return_TupleInStruct({field: [1, 2, 3, 4]});
            assert.deepEqual({field: [1, 2, 3, 4]}, d);
        });

        test("can clone handles", function() {
            var a = new cm.ValHolder({});
            assert.equal(1, cm.count_emval_handles());
            var b = a.clone();
            a.delete();

            assert.equal(1, cm.count_emval_handles());

            assert.throws(cm.BindingError, function() {
                a.delete();
            });
            b.delete();

            assert.equal(0, cm.count_emval_handles());
        });

        test("A shared pointer set/get point to the same underlying pointer", function() {
            var a = new cm.SharedPtrHolder();
            var b = a.get();

            a.set(b);
            var c = a.get();

            assert.equal(b.$$.ptr, c.$$.ptr);
            b.delete();
            c.delete();
            a.delete();
        });

        test("can return shared ptrs from instance methods", function() {
            var a = new cm.SharedPtrHolder();

            // returns the shared_ptr.
            var b = a.get();

            assert.equal("a string", b.get());
            b.delete();
            a.delete();
        });

        test("smart ptrs clone correctly", function() {
            assert.equal(0, cm.count_emval_handles());

            var a = cm.emval_test_return_shared_ptr();

            var b = a.clone();
            a.delete();

            assert.equal(1, cm.count_emval_handles());

            assert.throws(cm.BindingError, function() {
                a.delete();
            });
            b.delete();

            assert.equal(0, cm.count_emval_handles());
        });

        test("can't clone if already deleted", function() {
            var a = new cm.ValHolder({});
            a.delete();
            assert.throws(cm.BindingError, function() {
                a.clone();
            });
        });

        test("virtual calls work correctly", function() {
            var derived = cm.embind_test_return_raw_polymorphic_derived_ptr_as_base();
            assert.equal("PolyDerived", derived.virtualGetClassName());
            derived.delete();
        });

        test("virtual calls work correctly on smart ptrs", function() {
            var derived = cm.embind_test_return_smart_polymorphic_derived_ptr_as_base();
            assert.equal("PolyDerived", derived.virtualGetClassName());
            derived.delete();
        });

        test("Empty smart ptr is null", function() {
            var a = cm.emval_test_return_empty_shared_ptr();
            assert.equal(null, a);
        });

        test("string cannot be given as smart pointer argument", function() {
            assert.throws(cm.BindingError, function() {
                cm.emval_test_is_shared_ptr_null("hello world");
            });
        });

        test("number cannot be given as smart pointer argument", function() {
            assert.throws(cm.BindingError, function() {
                cm.emval_test_is_shared_ptr_null(105);
            });
        });

        test("raw pointer cannot be given as smart pointer argument", function() {
            var p = new cm.ValHolder({});
            assert.throws(cm.BindingError, function() { cm.emval_test_is_shared_ptr_null(p); });
            p.delete();
        });

        test("null is passed as empty smart pointer", function() {
            assert.true(cm.emval_test_is_shared_ptr_null(null));
        });

        test("Deleting already deleted smart ptrs fails", function() {
            var a = cm.emval_test_return_shared_ptr();
            a.delete();
            assert.throws(cm.BindingError, function() {
                a.delete();
            });
        });

        test("StringHolder", function() {
            var a = new cm.StringHolder("foobar");
            assert.equal("foobar", a.get());

            a.set("barfoo");
            assert.equal("barfoo", a.get());

            assert.equal("barfoo", a.get_const_ref());

            a.delete();
        });

        test("can call methods on unique ptr", function() {
            var result = cm.emval_test_return_unique_ptr();

            result.setVal('1234');
            assert.equal('1234', result.getVal());
            result.delete();
        });

        test("can call methods on shared ptr", function(){
            var result = cm.emval_test_return_shared_ptr();
            result.setVal('1234');

            assert.equal('1234', result.getVal());
            result.delete();
        });

        test("Non functors throw exception", function() {
            var a = {foo: 'bar'};
            var c = new cm.ValHolder(a);
            assert.throws(TypeError, function() {
                c();
            });
            c.delete();
        });

        test("non-member methods", function() {
            var a = {foo: 'bar'};
            var c = new cm.ValHolder(a);
            c.setEmpty(); // non-member method
            assert.deepEqual({}, c.getValNonMember());
            c.delete();
        });

        test("instantiating class without constructor gives error", function() {
            var e = assert.throws(cm.BindingError, function() {
                cm.AbstractClass();
            });
            assert.equal("Use 'new' to construct AbstractClass", e.message);

            var e = assert.throws(cm.BindingError, function() {
                new cm.AbstractClass();
            });
            assert.equal("AbstractClass has no accessible constructor", e.message);
        });

        test("can construct class with external constructor", function() {
            var e = new cm.HasExternalConstructor("foo");
            assert.instanceof(e, cm.HasExternalConstructor);
            assert.equal("foo", e.getString());
            e.delete();
        });
    });

    BaseFixture.extend("const", function() {
        test("calling non-const method with const handle is error", function() {
            var vh = cm.ValHolder.makeConst({});
            var e = assert.throws(cm.BindingError, function() {
                vh.setVal({});
            });
            assert.equal('Cannot convert argument of type ValHolder const* to parameter type ValHolder*', e.message);
            vh.delete();
        });

        test("passing const pointer to non-const pointer is error", function() {
            var vh = new cm.ValHolder.makeConst({});
            var e = assert.throws(cm.BindingError, function() {
                cm.ValHolder.set_via_raw_pointer(vh, {});
            });
            assert.equal('Cannot convert argument of type ValHolder const* to parameter type ValHolder*', e.message);
            vh.delete();
        });
    });

    BaseFixture.extend("smart pointers", function() {
        test("constructor can return smart pointer", function() {
            var e = new cm.HeldBySmartPtr(10, "foo");
            assert.instanceof(e, cm.HeldBySmartPtr);
            assert.equal(10, e.i);
            assert.equal("foo", e.s);
            var f = cm.takesHeldBySmartPtr(e);
            f.delete();
            e.delete();
        });

        test("cannot pass incorrect smart pointer type", function() {
            var e = cm.emval_test_return_shared_ptr();
            assert.throws(cm.BindingError, function() {
                cm.takesHeldBySmartPtr(e);
            });
            e.delete();
        });

        test("smart pointer object has no object keys", function() {
            var e = new cm.HeldBySmartPtr(10, "foo");
            assert.deepEqual([], Object.keys(e));

            var f = e.clone();
            e.delete();

            assert.deepEqual([], Object.keys(f));
            f.delete();
        });

        test("smart pointer object has correct constructor name", function() {
            var e = new cm.HeldBySmartPtr(10, "foo");
            assert.equal('HeldBySmartPtr', e.constructor.name);
            e.delete();
        });

        test("constructor can return smart pointer", function() {
            var e = new cm.HeldBySmartPtr(10, "foo");
            assert.instanceof(e, cm.HeldBySmartPtr);
            assert.equal(10, e.i);
            assert.equal("foo", e.s);
            var f = cm.takesHeldBySmartPtr(e);
            assert.instanceof(f, cm.HeldBySmartPtr);
            f.delete();
            e.delete();
        });

        test("custom smart pointer", function() {
            var e = new cm.HeldByCustomSmartPtr(20, "bar");
            assert.instanceof(e, cm.HeldByCustomSmartPtr);
            assert.equal(20, e.i);
            assert.equal("bar", e.s);
            e.delete();
        });

        test("custom smart pointer passed through wiretype", function() {
            var e = new cm.HeldByCustomSmartPtr(20, "bar");
            var f = cm.passThroughCustomSmartPtr(e);
            e.delete();

            assert.instanceof(f, cm.HeldByCustomSmartPtr);
            assert.equal(20, f.i);
            assert.equal("bar", f.s);

            f.delete();
        });

        test("cannot give null to by-value argument", function() {
            var e = assert.throws(cm.BindingError, function() {
                cm.takesHeldBySmartPtr(null);
            });
            assert.equal('null is not a valid HeldBySmartPtr', e.message);
        });

        test("raw pointer can take and give null", function() {
            assert.equal(null, cm.passThroughRawPtr(null));
        });

        test("custom smart pointer can take and give null", function() {
            assert.equal(null, cm.passThroughCustomSmartPtr(null));
        });

        test("cannot pass shared_ptr to CustomSmartPtr", function() {
            var o = cm.HeldByCustomSmartPtr.createSharedPtr(10, "foo");
            var e = assert.throws(cm.BindingError, function() {
                cm.passThroughCustomSmartPtr(o);
            });
            assert.equal('Cannot convert argument of type NSt3__110shared_ptrI20HeldByCustomSmartPtrEE to parameter type 14CustomSmartPtrI20HeldByCustomSmartPtrE', e.message);
            o.delete();
        });

        test("custom smart pointers can be passed to shared_ptr parameter", function() {
            var e = cm.HeldBySmartPtr.newCustomPtr(10, "abc");
            assert.equal(10, e.i);
            assert.equal("abc", e.s);

            cm.takesHeldBySmartPtrSharedPtr(e).delete();
            e.delete();
        });

        test("can call non-member functions as methods", function() {
            var e = new cm.HeldBySmartPtr(20, "bar");
            var f = e.returnThis();
            e.delete();
            assert.equal(20, f.i);
            assert.equal("bar", f.s);
            f.delete();
        });
    });

    BaseFixture.extend("enumerations", function() {
        test("can compare enumeration values", function() {
            assert.equal(cm.Enum.ONE, cm.Enum.ONE);
            assert.notEqual(cm.Enum.ONE, cm.Enum.TWO);
        });

        if (typeof INVOKED_FROM_EMSCRIPTEN_TEST_RUNNER === "undefined") { // TODO: Enable this to work in Emscripten runner as well!
            test("repr includes enum value", function() {
                assert.equal('<#Enum_ONE {}>', IMVU.repr(cm.Enum.ONE));
                assert.equal('<#Enum_TWO {}>', IMVU.repr(cm.Enum.TWO));
            });        
        }

        test("instanceof", function() {
            assert.instanceof(cm.Enum.ONE, cm.Enum);
        });

        test("can pass and return enumeration values to functions", function() {
            assert.equal(cm.Enum.TWO, cm.emval_test_take_and_return_Enum(cm.Enum.TWO));
        });
    });

    BaseFixture.extend("C++11 enum class", function() {
        test("can compare enumeration values", function() {
            assert.equal(cm.EnumClass.ONE, cm.EnumClass.ONE);
            assert.notEqual(cm.EnumClass.ONE, cm.EnumClass.TWO);
        });

        if (typeof INVOKED_FROM_EMSCRIPTEN_TEST_RUNNER === "undefined") { // TODO: Enable this to work in Emscripten runner as well!
            test("repr includes enum value", function() {
                assert.equal('<#EnumClass_ONE {}>', IMVU.repr(cm.EnumClass.ONE));
                assert.equal('<#EnumClass_TWO {}>', IMVU.repr(cm.EnumClass.TWO));
            });
        }

        test("instanceof", function() {
            assert.instanceof(cm.EnumClass.ONE, cm.EnumClass);
        });

        test("can pass and return enumeration values to functions", function() {
            assert.equal(cm.EnumClass.TWO, cm.emval_test_take_and_return_EnumClass(cm.EnumClass.TWO));
        });
    });

    BaseFixture.extend("emval call tests", function() {
        test("can call functions from C++", function() {
            var called = false;
            cm.emval_test_call_function(function(i, f, tv, sv) {
                called = true;
                assert.equal(10, i);
                assert.equal(1.5, f);
                assert.deepEqual([1.25, 2.5, 3.75, 4], tv);
                assert.deepEqual({x: 1.25, y: 2.5, z: 3.75, w:4}, sv);
            }, 10, 1.5, [1.25, 2.5, 3.75, 4], {x: 1.25, y: 2.5, z: 3.75, w:4});
            assert.true(called);
        });
    });

    BaseFixture.extend("extending built-in classes", function() {
        // cm.ValHolder.prototype.patched = 10; // this sets instanceCounts.patched inside of Deletable module !?!

        test("can access patched value on new instances", function() {
            cm.ValHolder.prototype.patched = 10;
            var c = new cm.ValHolder(undefined);
            assert.equal(10, c.patched);
            c.delete();
            cm.ValHolder.prototype.patched = undefined;
        });

        test("can access patched value on returned instances", function() {
            cm.ValHolder.prototype.patched = 10;
            var c = cm.emval_test_return_ValHolder();
            assert.equal(10, c.patched);
            c.delete();
            cm.ValHolder.prototype.patched = undefined;
        });
    });

    BaseFixture.extend("raw pointers", function() {
        test("can pass raw pointers into functions if explicitly allowed", function() {
            var vh = new cm.ValHolder({});
            cm.ValHolder.set_via_raw_pointer(vh, 10);
            assert.equal(10, cm.ValHolder.get_via_raw_pointer(vh));
            vh.delete();
        });

        test("can return raw pointers from functions if explicitly allowed", function() {
            var p = cm.embind_test_return_raw_base_ptr();
            assert.equal("Base", p.getClassName());
            p.delete();
        });

        test("can pass multiple raw pointers to functions", function() {
            var target = new cm.ValHolder(undefined);
            var source = new cm.ValHolder("hi");
            cm.ValHolder.transfer_via_raw_pointer(target, source);
            assert.equal("hi", target.getVal());
            target.delete();
            source.delete();
        });
    });

    BaseFixture.extend("abstract methods", function() {
        test("can call abstract methods", function() {
            var obj = cm.getAbstractClass();
            assert.equal("from concrete", obj.abstractMethod());
            obj.delete();
        });

        test("can implement abstract methods in JavaScript", function() {
            var expected = "my JS string";
            function MyImplementation() {
                this.rv = expected;
            }
            MyImplementation.prototype.abstractMethod = function() {
                return this.rv;
            };

            var impl = cm.AbstractClass.implement(new MyImplementation);
            assert.equal(expected, impl.abstractMethod());
            assert.equal(expected, cm.callAbstractMethod(impl));
            impl.delete();
        });

        test("can implement optional methods in JavaScript", function() {
            var expected = "my JS string";
            function MyImplementation() {
                this.rv = expected;
            }
            MyImplementation.prototype.optionalMethod = function() {
                return this.rv;
            };

            var impl = cm.AbstractClass.implement(new MyImplementation);
            assert.equal(expected, impl.optionalMethod(expected));
            assert.equal(expected, cm.callOptionalMethod(impl, expected));
            impl.delete();
        });

        test("if not implemented then optional method runs default", function() {
            var impl = cm.AbstractClass.implement({});
            assert.equal("optionalfoo", impl.optionalMethod("foo"));
            assert.equal("optionalfoo", cm.callOptionalMethod(impl, "foo"));
            impl.delete();
        });

        test("returning null shared pointer from interfaces implemented in JS code does not leak", function() {
            var impl = cm.AbstractClass.implement({
                returnsSharedPtr: function() {
                    return null;
                }
            });
            cm.callReturnsSharedPtrMethod(impl);
            impl.delete();
            // Let the memory leak test superfixture check that no leaks occurred.
        });

        test("returning a new shared pointer from interfaces implemented in JS code does not leak", function() {
            var impl = cm.AbstractClass.implement({
                returnsSharedPtr: function() {
                    return cm.embind_test_return_smart_derived_ptr();
                }
            });
            cm.callReturnsSharedPtrMethod(impl);
            impl.delete();
            // Let the memory leak test superfixture check that no leaks occurred.
        });

        test("void methods work", function() {
            var saved = {};
            var impl = cm.AbstractClass.implement({
                differentArguments: function(i, d, f, q, s) {
                    saved.i = i;
                    saved.d = d;
                    saved.f = f;
                    saved.q = q;
                    saved.s = s;
                }
            });

            cm.callDifferentArguments(impl, 1, 2, 3, 4, "foo");

            assert.deepEqual(saved, {
                i: 1,
                d: 2,
                f: 3,
                q: 4,
                s: "foo",
            });

            impl.delete();
        });
    });

    BaseFixture.extend("registration order", function() {
        test("registration of tuple elements out of order leaves them in order", function() {
            var ot = cm.getOrderedTuple();
            assert.instanceof(ot[0], cm.FirstElement);
            assert.instanceof(ot[1], cm.SecondElement);
            ot[0].delete();
            ot[1].delete();
        });

        test("registration of struct elements out of order", function() {
            var os = cm.getOrderedStruct();
            assert.instanceof(os.first, cm.FirstElement);
            assert.instanceof(os.second, cm.SecondElement);
            os.first.delete();
            os.second.delete();
        });
    });

    if (typeof INVOKED_FROM_EMSCRIPTEN_TEST_RUNNER === "undefined") { // TODO: Enable this to work in Emscripten runner as well!
    
    BaseFixture.extend("unbound types", function() {
        function assertMessage(fn, message) {
            var e = assert.throws(cm.UnboundTypeError, fn);
            assert.equal(message, e.message);
        }

        test("calling function with unbound types produces error", function() {
            assertMessage(
                function() {
                    cm.getUnboundClass();
                },
                'Cannot call getUnboundClass due to unbound types: 12UnboundClass');
        });

        test("unbound base class produces error", function() {
            assertMessage(
                function() {
                    cm.getHasUnboundBase();
                },
                'Cannot call getHasUnboundBase due to unbound types: 12UnboundClass');
        });

        test("construct of class with unbound base", function() {
            assertMessage(
                function() {
                    new cm.HasUnboundBase;
                }, 'Cannot construct HasUnboundBase due to unbound types: 12UnboundClass');
        });

        test("unbound constructor argument", function() {
            assertMessage(
                function() {
                    new cm.HasConstructorUsingUnboundArgument(1);
                },
                'Cannot construct HasConstructorUsingUnboundArgument due to unbound types: 12UnboundClass');
        });

        test("unbound constructor argument of class with unbound base", function() {
            assertMessage(
                function() {
                    new cm.HasConstructorUsingUnboundArgumentAndUnboundBase;
                },
                'Cannot construct HasConstructorUsingUnboundArgumentAndUnboundBase due to unbound types: 18SecondUnboundClass');
        });
        
        test('class function with unbound argument', function() {
            var x = new cm.BoundClass;
            assertMessage(
                function() {
                    x.method();
                }, 'Cannot call BoundClass.method due to unbound types: 12UnboundClass');
            x.delete();
        });

        test('class class function with unbound argument', function() {
            assertMessage(
                function() {
                    cm.BoundClass.classfunction();
                }, 'Cannot call BoundClass.classfunction due to unbound types: 12UnboundClass');
        });

        test('class property of unbound type', function() {
            var x = new cm.BoundClass;
            var y;
            assertMessage(
                function() {
                    y = x.property;
                }, 'Cannot access BoundClass.property due to unbound types: 12UnboundClass');
            assertMessage(
                function() {
                    x.property = 10;
                }, 'Cannot access BoundClass.property due to unbound types: 12UnboundClass');
            x.delete();
        });
        
        // todo: tuple elements
        // todo: tuple element accessors
        // todo: struct fields
    });
    
    }

    BaseFixture.extend("noncopyable", function() {
        test('can call method on noncopyable object', function() {
            var x = new cm.Noncopyable;
            assert.equal('foo', x.method());
            x.delete();
        });
    });

    BaseFixture.extend("function names", function() {
        assert.equal('ValHolder', cm.ValHolder.name);
        assert.equal('ValHolder$setVal', cm.ValHolder.prototype.setVal.name);
        assert.equal('ValHolder$makeConst', cm.ValHolder.makeConst.name);
    });

    BaseFixture.extend("constants", function() {
        assert.equal(10, cm.INT_CONSTANT);
        assert.equal("some string", cm.STRING_CONSTANT);
        assert.deepEqual([1, 2, 3, 4], cm.VALUE_TUPLE_CONSTANT);
        assert.deepEqual({x:1,y:2,z:3,w:4}, cm.VALUE_STRUCT_CONSTANT);
    });

    BaseFixture.extend("object handle comparison", function() {
        var e = new cm.ValHolder("foo");
        var f = new cm.ValHolder("foo");
        assert.false(e.isAliasOf(undefined));
        assert.false(e.isAliasOf(10));
        assert.true(e.isAliasOf(e));
        assert.false(e.isAliasOf(f));
        assert.false(f.isAliasOf(e));
        e.delete();
        f.delete();
    });

    BaseFixture.extend("derived-with-offset types compare with base", function() {
        var e = new cm.DerivedWithOffset;
        var f = cm.return_Base_from_DerivedWithOffset(e);
        assert.true(e.isAliasOf(f));
        assert.true(f.isAliasOf(e));
        e.delete();
        f.delete();
    });

    BaseFixture.extend("memory view", function() {
        test("can pass memory view from C++ to JS", function() {
            var views = [];
            cm.callWithMemoryView(function(view) {
                views.push(view);
            });
            assert.equal(3, views.length);

            assert.instanceof(views[0], Uint8Array);
            assert.equal(8, views[0].length);
            assert.deepEqual([0, 1, 2, 3, 4, 5, 6, 7], [].slice.call(new Uint8Array(views[0])));

            assert.instanceof(views[1], Float32Array);
            assert.equal(4, views[1].length);
            assert.deepEqual([1.5, 2.5, 3.5, 4.5], [].slice.call(views[1]));

            assert.instanceof(views[2], Int16Array);
            assert.equal(4, views[2].length);
            assert.deepEqual([1000, 100, 10, 1], [].slice.call(views[2]));
        });
    });

    BaseFixture.extend("delete pool", function() {
        test("can delete objects later", function() {
            var v = new cm.ValHolder({});
            v.deleteLater();
            assert.deepEqual({}, v.getVal());
            cm.flushPendingDeletes();
            assert.throws(cm.BindingError, function() {
                v.getVal();
            });
        });

        test("calling deleteLater twice is an error", function() {
            var v = new cm.ValHolder({});
            v.deleteLater();
            assert.throws(cm.BindingError, function() {
                v.deleteLater();
            });
        });

        test("deleteLater returns the object", function() {
            var v = (new cm.ValHolder({})).deleteLater();
            assert.deepEqual({}, v.getVal());
        });

        test("deleteLater throws if object is already deleted", function() {
            var v = new cm.ValHolder({});
            v.delete();
            assert.throws(cm.BindingError, function() {
                v.deleteLater();
            });
        });

        test("delete throws if object is already scheduled for deletion", function() {
            var v = new cm.ValHolder({});
            v.deleteLater();
            assert.throws(cm.BindingError, function() {
                v.delete();
            });
        });

        test("deleteLater invokes delay function", function() {
            var runLater;
            cm.setDelayFunction(function(fn) {
                runLater = fn;
            });

            var v = new cm.ValHolder({});
            assert.false(runLater);
            v.deleteLater();
            assert.true(runLater);
            assert.false(v.isDeleted());
            runLater();
            assert.true(v.isDeleted());
        });

        test("deleteLater twice invokes delay function once", function() {
            var count = 0;
            var runLater;
            cm.setDelayFunction(function(fn) {
                ++count;
                runLater = fn;
            });

            (new cm.ValHolder({})).deleteLater();
            (new cm.ValHolder({})).deleteLater();
            assert.equal(1, count);
            runLater();
            (new cm.ValHolder({})).deleteLater();
            assert.equal(2, count);
        });

        test('The delay function is immediately invoked if the deletion queue is not empty', function() {
            (new cm.ValHolder({})).deleteLater();
            var count = 0;
            cm.setDelayFunction(function(fn) {
                ++count;
            });
            assert.equal(1, count);
        });

        // The idea is that an interactive application would
        // periodically flush the deleteLater queue by calling
        //
        // setDelayFunction(function(fn) {
        //     setTimeout(fn, 0);
        // });
    });
});

/* global run_all_tests */
// If running as part of the emscripten test runner suite, and not as part of the IMVU suite,
// we launch the test execution from here. IMVU suite uses its own dedicated mechanism instead of this.
if (typeof run_all_tests !== "undefined")
    run_all_tests();
