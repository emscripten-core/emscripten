.. _embind:

====================
embind (wiki-import)
====================
.. note:: This article was migrated from the wiki (Fri, 25 Jul 2014 04:21) and is now the "master copy" (the version in the wiki will be deleted). It may not be a perfect rendering of the original but we hope to fix that soon!

What is embind?
===============

*embind* allows binding C++ functions and classes to JavaScript so that
they can be used in a natural way. *embind* was inspired by
Boost.Python.

A Quick Example
===============

Imagine we want to expose a simple C++ function to JavaScript.

.. code:: cpp

    // quick_example.cpp
    #include <emscripten/bind.h>

    using namespace emscripten;

    float lerp(float a, float b, float t) {
        return (1 - t) * a + t * b;
    }

    EMSCRIPTEN_BINDINGS(my_module) {
        function("lerp", &lerp);
    }

To compile the above example, run
``emcc --bind -o quick_example.js quick_example.cpp``. The resulting
**quick\_example.js** file can be loaded as a script tag or a node
module.

.. code:: html

    <!doctype html>
    <html>
      <script src="quick_example.js"></script>
      <script>
        document.write('lerp result: ' + Module.lerp(1, 2, 0.5));
      </script>
    </html>

The code in the ``EMSCRIPTEN_BINDINGS`` block runs when the JavaScript
file is initially loaded. Notice that lerp's parameter types and return
type are automatically inferred by *embind*. All symbols exposed by
*embind* are available on the Emscripten ``Module`` object.

Classes
=======

Exposing classes to JavaScript requires a few more steps. An example:

.. code:: cpp


    class MyClass {
    public:
        MyClass(int x, std::string y)
            : x(x)
            , y(y)
        {}

        void incrementX() {
            ++x;
        }

        int getX() const { return x; }
        void setX(int x_) { x = x_; }

        static std::string getStringFromInstance(const MyClass& instance) {
            return instance.y;
        }

    private:
        int x;
        std::string y;
    };

    EMSCRIPTEN_BINDINGS(my_class_example) {
        class_<MyClass>("MyClass")
            .constructor<int, std::string>()
            .function("incrementX", &MyClass::incrementX)
            .property("x", &MyClass::getX, &MyClass::setX)
            .class_function("getStringFromInstance", &MyClass::getStringFromInstance)
            ;
    }

.. code:: javascript

    var instance = new Module.MyClass(10, "hello");
    instance.incrementX();
    instance.x; // 12
    instance.x = 20; // 20
    Module.MyClass.getStringFromInstance(instance); // "hello"
    instance.delete();

Memory Management
=================

JavaScript, specifically ECMA-262 Edition 5.1, does not support
finalizers or weak references with callbacks. Thus, JavaScript code must
explicitly delete any C++ object handles it has received. Otherwise the
Emscripten heap will grow indefinitely.

.. code:: javascript

    var x = new Module.MyClass;
    x.method();
    x.delete();

    var y = Module.myFunctionThatReturnsClassInstance();
    y.method();
    y.delete();

Value Types
===========

Imagine a common, small data type, like ``Point2f``. Because manual
memory management for basic types is onerous, *embind* provides support
for value types. Value arrays are converted to and from JavaScript
Arrays and value objects are converted to and from JavaScript Objects.

.. code:: cpp

    struct Point2f {
        float x;
        float y;
    };

    struct PersonRecord {
        std::string name;
        int age;
    };

    PersonRecord findPersonAtLocation(Point2f);

    EMSCRIPTEN_BINDINGS(my_value_example) {
        value_array<Point2f>("Point2f")
            .element(&Point2f::x)
            .element(&Point2f::y)
            ;

        value_object<PersonRecord>("PersonRecord")
            .field("name", &PersonRecord::name)
            .field("age", &PersonRecord::age)
            ;

        function("findPersonAtLocation", &findPersonAtLocation);
    }

The JavaScript code needs not worry about lifetime management.

.. code:: javascript

    var person = Module.findPersonAtLocation([10.2, 156.5]);
    console.log('Found someone! Their name is ' + person.name + ' and they are ' + person.age + ' years old');

Advanced Class Concepts
=======================

Raw Pointers
------------

Because raw pointers have unclear lifetime semantics, *embind* requires
their use to be marked with ``allow_raw_pointers()``.

.. code:: cpp

    class C {};
    C* passThrough(C* ptr) { return ptr; }
    EMSCRIPTEN_BINDINGS(raw_pointers) {
        class_<C>("C");
        function("passThrough", &passThrough, allow_raw_pointers());
    }

External Constructors
---------------------

There are two ways to specify constructors on a class. The zero-argument
template form invokes the natural constructor with the arguments
specified in the template. However, if you pass a function pointer as
the constructor, then invoking ``new`` from JavaScript calls said
function and returns its result.

.. code:: cpp

    class C {}; // probably want to override operator delete
    C* getInstanceFromPool() {
        return pool.get();
    }
    EMSCRIPTEN_BINDINGS(external_constructors) {
        class_<C>("C")
            .constructor(&getInstanceFromPool)
            ;
    }

Smart Pointers
--------------

To manage object lifetime with smart pointers, *embind* must be told
about the smart pointer type. For example, imagine managing a class C's
lifetime with ``std::shared_ptr<C>``.

.. code:: cpp

    EMSCRIPTEN_BINDINGS(smart_pointers) {
        class_<C>("C")
            .constructor<>()
            .smart_ptr<std::shared_ptr<C>>()
            ;
    }

At this point, functions can return ``std::shared_ptr<C>`` or take
``std::shared_ptr<C>`` as arguments. However, ``new Module.C()`` would
still return a raw pointer.

To return a ``shared_ptr<C>`` from the constructor, write the following
instead:

.. code:: cpp

    EMSCRIPTEN_BINDINGS(better_smart_pointers) {
        class_<C>("C")
            .smart_ptr_constructor(&std::make_shared<C>)
            ;
    }

smart\_ptr\_constructor automatically registers the smart pointer type.

unique\_ptr
~~~~~~~~~~~

*embind* has built-in support for return values of type
``std::unique_ptr``.

Custom Smart Pointers
~~~~~~~~~~~~~~~~~~~~~

To teach *embind* about custom smart pointer templates, specialize the
``smart_ptr_trait`` template. See **bind.h** for details and an example.

Non-member-functions on the JavaScript prototype
------------------------------------------------

Methods on the JavaScript class prototype can be non-member functions,
as long as the instance handle can be converted to the first argument of
the non-member function. The classic example is when the function
exposed to JavaScript does not exactly match the behavior of a C++
method.

.. code:: cpp

    struct Array10 {
        int& get(size_t index) {
            return data[index];
        }
        int data[10];
    };

    val Array10_get(Array10& arr, size_t index) {
        if (index < 10) {
            return val(arr.get(index));
        } else {
            return val::undefined();
        }
    }

    EMSCRIPTEN_BINDINGS(non_member_functions) {
        class_<Array10>("Array10")
            .function("get", &Array10_get)
            ;
    }

If JavaScript calls ``Array10.prototype.get`` with an invalid index, it
will return ``undefined``.

Deriving From C++ Classes in JavaScript
---------------------------------------

If C++ classes have virtual or abstract member functions, it's possible
to override them in JavaScript. Because JavaScript has no knowledge of
the C++ vtable, *embind* needs a bit of glue code to convert C++ virtual
function calls into JavaScript calls.

Abstract Methods
~~~~~~~~~~~~~~~~

Let's begin with a simple case: pure virtual functions that must be
implemented in JavaScript.

.. code:: cpp

    struct Interface {
        virtual void invoke(const std::string& str) = 0;
    };

    struct InterfaceWrapper : public wrapper<Interface> {
        EMSCRIPTEN_WRAPPER(InterfaceWrapper);
        void invoke(const std::string& str) {
            return call<void>("invoke", str);
        }
    };

    EMSCRIPTEN_BINDINGS(interface) {
        class_<Interface>("Interface")
            .function("invoke", &Interface::invoke, pure_virtual())
            .allow_subclass<InterfaceWrapper>()
            ;
    }

``allow_subclass`` adds two special methods to the Interface binding:
``extend`` and ``implement``. ``extend`` allows JavaScript to subclass
in the style exemplified by **Backbone.js**. ``implement`` is used when
you have a JavaScript object, perhaps provided by the browser or some
other library, and you want to use it to implement a C++ interface.

By the way, note the ``pure_virtual()`` annotation on the function
binding. Specifying ``pure_virtual()`` allows JavaScript to throw a
helpful error if the JavaScript class does not override invoke().
Otherwise, you may run into confusing errors.

``extend`` Example
~~~~~~~~~~~~~~~~~~

.. code:: javascript

    var DerivedClass = Module.Interface.extend("Interface", {
        // __construct and __destruct are optional.  They are included
        // in this example for illustration purposes.
        // If you override __construct or __destruct, don't forget to
        // call the parent implementation!
        __construct: function() {
            this.__parent.__construct.call(this);
        },
        __destruct: function() {
            this.__parent.__destruct.call(this);
        },
        invoke: function() {
            // your code goes here
        },
    });

    var instance = new Derived;

``implement`` Example
~~~~~~~~~~~~~~~~~~~~~

.. code:: javascript

    var x = {
        invoke: function(str) {
            console.log('invoking with: ' + str);
        }
    };
    var interfaceObject = Module.Interface.implement(x);

Now ``interfaceObject`` can be passed to any function that takes an
``Interface`` pointer or reference.

Non-Abstract Virtual Methods
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

If a C++ class has a non-pure virtual function, it can be overridden but
does not have to be. This requires a slightly different wrapper
implementation:

.. code:: cpp

    struct Base {
        virtual void invoke(const std::string& str) {
            // default implementation
        }
    };

    struct BaseWrapper : public wrapper<Base> {
        EMSCRIPTEN_WRAPPER(BaseWrapper);
        void invoke(const std::string& str) {
            return call<void>("invoke", str);
        }
    };

    EMSCRIPTEN_BINDINGS(interface) {
        class_<Base>("Base")
            .allow_subclass<BaseWrapper>()
            .function("invoke", optional_override([](Base& self, const std::string& str) {
                return self.Base::invoke(str);
            }))
            ;
    }

When implementing Base with a JavaScript object, overriding ``invoke``
is optional. The special lambda binding for invoke is necessary to avoid
infinite mutual recursion between the wrapper and JavaScript.

Base Classes
------------

.. code:: cpp

    EMSCRIPTEN_BINDINGS(base_example) {
        class_<BaseClass>("BaseClass");
        class_<DerivedClass, base<BaseClass>>("DerivedClass");
    }

Any member functions defined on ``BaseClass`` are then accessible to
instances of ``DerivedClass``. In addition, any function that accepts an
instance of ``BaseClass`` can be given an instance of ``DerivedClass``.

Automatic Downcasting
~~~~~~~~~~~~~~~~~~~~~

If a C++ class is polymorphic (that is, it has a virtual method), then
*embind* supports automatic downcasting of function return values.

.. code:: cpp

    class Base { virtual ~Base() {} }; // the virtual makes Base and Derived polymorphic
    class Derived : public Base {};
    Base* getDerivedInstance() {
        return new Derived;
    }
    EMSCRIPTEN_BINDINGS(automatic_downcasting) {
        class_<Base>("Base");
        class_<Derived, base<Base>>("Derived");
        function("getDerivedInstance", &getDerivedInstance, allow_raw_pointers());
    }

Calling Module.getDerivedInstance from JavaScript will return a Derived
instance handle from which all of Derived's methods are available.

Note that the *embind* must understand the fully-derived type for
automatic downcasting to work.

Overloaded Functions
====================

Constructors and functions can be overloaded on the number of arguments.
*embind* does not support overloading based on type. When specifying an
overload, use the ``select_overload`` helper function to select the
appropriate signature.

.. code:: cpp

    struct HasOverloadedMethods {
        void foo();
        void foo(int i);
        void foo(float f) const;
    };

    EMSCRIPTEN_BINDING(overloads) {
        class_<HasOverloadedMethods>("HasOverloadedMethods")
            .function("foo", select_overload<void()>(&HasOverloadedMethods::foo))
            .function("foo_int", select_overload<void(int)>(&HasOverloadedMethods::foo))
            .function("foo_float", select_overload<void(float)const>(&HasOverloadedMethods::foo))
            ;
    }

Enums
=====

embind's enumeration support works with both C++98 enums and C++11 "enum
classes".

.. code:: cpp


    enum OldStyle {
        OLD_STYLE_ONE,
        OLD_STYLE_TWO
    };

    enum class NewStyle {
        ONE,
        TWO
    };

    EMSCRIPTEN_BINDINGS(my_enum_example) {
        enum_<OldStyle>("OldStyle")
            .value("ONE", OLD_STYLE_ONE)
            .value("TWO", OLD_STYLE_TWO)
            ;
        enum_<NewStyle>("NewStyle")
            .value("ONE", NewStyle::ONE)
            .value("TWO", NewStyle::TWO)
            ;
    }

In both cases, JavaScript accesses enumeration values as properties of
the type.

.. code:: javascript

    Module.OldStyle.ONE;
    Module.NewStyle.TWO;

Constants
=========

To expose a C++ constant to JavaScript, simply write:

.. code:: cpp

    EMSCRIPTEN_BINDINGS(my_constant_example) {
        constant("SOME_CONSTANT", SOME_CONSTANT);
    }

``SOME_CONSTANT`` can have any type known to *embind*.

val
===

``emscripten::val`` is a data type that represents any JavaScript
object.

``val::array()`` creates a new Array.

``val::object()`` creates a new Object.

``val::null()`` creates a val that represents null. ``val::undefined()``
is the same, but for undefined.

``val::global(const char* name)`` looks up a global symbol.

``val::module_property(const char* name)`` looks up a symbol on the
emscripten Module object.

Otherwise a val can be constructed by explicit construction from any C++
type. For example, ``val(true)`` or ``val(std::string("foo"))``.

We can use ``val`` to transliterate JavaScript code to C++. Here is the
JavaScript.

.. code:: javascript

    var xhr = new XMLHttpRequest;
    xhr.open("GET", "http://url");

.. code:: cpp

    val xhr = val::global("XMLHttpRequest").new_();
    xhr.call("open", std::string("GET"), std::string("http://url"));

Built-in Type Conversions
=========================

Out of the box, *embind* provides converters for many standard C++
types:

+-------------------+-------------------------------------------------+
| C++ type          | JavaScript type                                 |
+===================+=================================================+
| void              | undefined                                       |
+-------------------+-------------------------------------------------+
| bool              | true or false                                   |
+-------------------+-------------------------------------------------+
| char              | Number                                          |
+-------------------+-------------------------------------------------+
| signed char       | Number                                          |
+-------------------+-------------------------------------------------+
| unsigned char     | Number                                          |
+-------------------+-------------------------------------------------+
| short             | Number                                          |
+-------------------+-------------------------------------------------+
| unsigned short    | Number                                          |
+-------------------+-------------------------------------------------+
| int               | Number                                          |
+-------------------+-------------------------------------------------+
| unsigned int      | Number                                          |
+-------------------+-------------------------------------------------+
| long              | Number                                          |
+-------------------+-------------------------------------------------+
| unsigned long     | Number                                          |
+-------------------+-------------------------------------------------+
| float             | Number                                          |
+-------------------+-------------------------------------------------+
| double            | Number                                          |
+-------------------+-------------------------------------------------+
| std::string       | ArrayBuffer, Uint8Array, Int8Array, or String   |
+-------------------+-------------------------------------------------+
| std::wstring      | String (UTF-16 code units)                      |
+-------------------+-------------------------------------------------+
| emscripten::val   | anything                                        |
+-------------------+-------------------------------------------------+

For convenience, *embind* provides factory functions to register
``std::vector<T>`` and ``std::map<K, V>`` types:

.. code:: cpp

    EMSCRIPTEN_BINDINGS(stl_wrappers) {
        register_vector<int>("VectorInt");
        register_map<int,int>("MapIntInt");
    }

Performance
===========

[TODO: Jukka, want to flesh this out?]

Future Work
===========

-  global variables
-  class static variables

How does it work?
=================

[TODO]
