.. _embind:

======
Embind
======

*Embind* is used to bind C++ functions and classes to JavaScript, so
that the compiled code can be used in a natural way by "normal"
JavaScript. *Embind* also supports :ref:`calling JavaScript classes
from C++ <embind-val-guide>`.

Embind has support for binding most C++ constructs, including those
introduced in C++11 and C++14. Its only significant limitation is
that it does not currently support :ref:`raw pointers with complicated
lifetime semantics <embind-raw-pointers>`.

This article shows how to use :cpp:func:`EMSCRIPTEN_BINDINGS` blocks to
create bindings for functions, classes, value types, pointers (including
both raw and smart pointers), enums, and constants, and how to create
bindings for abstract classes that can be overridden in JavaScript. It
also briefly explains how to manage the memory of C++ object handles
passed to JavaScript.

.. tip:: In addition to the code in this article:

   - There are many other examples of how to use *Embind* in the `Test Suite`_.
   - `Connecting C++ and JavaScript on the Web with Embind`_ (slides from
     CppCon 2014) contains more examples and information about *Embind*'s
     design philosophy and implementation.

.. note:: *Embind* was inspired by `Boost.Python`_ and uses a very similar
   approach for defining bindings.


A quick example
===============

The following code uses an :cpp:func:`EMSCRIPTEN_BINDINGS` block to expose
the simple C++ ``lerp()`` :cpp:func:`function` to JavaScript.

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

To compile the above example using *embind*, we invoke *emcc* with the
:ref:`bind <emcc-bind>` option::

   emcc --bind -o quick_example.js quick_example.cpp

The resulting **quick_example.js** file can be loaded as a node module
or via a ``<script>`` tag:

.. code:: html

    <!doctype html>
    <html>
      <script>
        var Module = {
          onRuntimeInitialized: function() {
            console.log('lerp result: ' + Module.lerp(1, 2, 0.5));
          }
        };
      </script>
      <script src="quick_example.js"></script>
    </html>

.. note:: We use the ``onRuntimeInitialized`` callback to run code when the runtime is ready, which is an asynchronous operation (in order to compile WebAssembly).
.. note:: Open the developer tools console to see the output of ``console.log``.

The code in an :cpp:func:`EMSCRIPTEN_BINDINGS` block runs when the JavaScript
file is initially loaded (at the same time as the global constructors). The
function ``lerp()``'s parameter types and return type are automatically
inferred by *embind*.

All symbols exposed by *embind* are available on the Emscripten ``Module``
object.

.. important:: Always access objects through the :ref:`module` object, as
   shown above.

   While the objects are also available in the global namespace by default,
   there are cases where they will not be (for example, if you use the
   :term:`closure compiler` to minify code or wrap compiled code in a
   function to avoid polluting the global namespace). You can of course
   use whatever name you like for the module by assigning it to a new
   variable: ``var MyModuleName = Module;``.



Classes
=======

Exposing classes to JavaScript requires a more complicated binding statement.
For example:

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

   // Binding code
   EMSCRIPTEN_BINDINGS(my_class_example) {
     class_<MyClass>("MyClass")
       .constructor<int, std::string>()
       .function("incrementX", &MyClass::incrementX)
       .property("x", &MyClass::getX, &MyClass::setX)
       .class_function("getStringFromInstance", &MyClass::getStringFromInstance)
       ;
   }

The binding block defines a chain of member function calls on the temporary
:cpp:class:`class_` object (this same style is used in *Boost.Python*). The
functions register the class, its :cpp:func:`~class_::constructor`, member
:cpp:func:`~class_::function`, :cpp:func:`~class_::class_function` (static)
and :cpp:func:`~class_::property`.

.. note:: This binding block binds the class and all its methods. As a rule
   you should bind only those items that are actually needed, as each binding
   increases the code size. For example, it would be rare to bind private or
   internal methods.

An instance of ``MyClass`` can then be created and used in JavaScript as
shown below:

.. code:: javascript

   var instance = new Module.MyClass(10, "hello");
   instance.incrementX();
   instance.x; // 11
   instance.x = 20; // 20
   Module.MyClass.getStringFromInstance(instance); // "hello"
   instance.delete();


Memory management
=================

JavaScript, specifically ECMA-262 Edition 5.1, does not support `finalizers`_
or weak references with callbacks. Therefore there is no way for Emscripten
to automatically call the destructors on C++ objects.

.. warning:: JavaScript code must explicitly delete any C++ object handles
   it has received, or the Emscripten heap will grow indefinitely.

.. code:: javascript

    var x = new Module.MyClass;
    x.method();
    x.delete();

    var y = Module.myFunctionThatReturnsClassInstance();
    y.method();
    y.delete();

Value types
===========

Manual memory management for basic types is onerous, so *embind* provides
support for value types. :cpp:class:`Value arrays <value_array>` are
converted to and from JavaScript Arrays and :cpp:class:`value objects
<value_object>` are converted to and from JavaScript Objects.

Consider the example below:

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

The JavaScript code does not need to worry about lifetime management.

.. code:: javascript

    var person = Module.findPersonAtLocation([10.2, 156.5]);
    console.log('Found someone! Their name is ' + person.name + ' and they are ' + person.age + ' years old');


Advanced class concepts
=======================

.. _embind-raw-pointers:

Raw pointers
------------

Because raw pointers have unclear lifetime semantics, *embind* requires
their use to be marked with :cpp:type:`allow_raw_pointers`.

For example:

.. code:: cpp

    class C {};
    C* passThrough(C* ptr) { return ptr; }
    EMSCRIPTEN_BINDINGS(raw_pointers) {
        class_<C>("C");
        function("passThrough", &passThrough, allow_raw_pointers());
    }

.. note::

   Currently the markup serves only to whitelist raw pointer use, and
   show that you've thought about the use of the raw pointers. Eventually
   we hope to implement `Boost.Python-like raw pointer policies`_ for
   managing object ownership.

.. _embind-external-constructors:

External constructors
---------------------

There are two ways to specify constructors for a class.

The :ref:`zero-argument template form <embind-class-zero-argument-constructor>`
invokes the natural constructor with the arguments specified in the template.
For example:

.. code:: cpp

   class MyClass {
   public:
     MyClass(int, float);
     void someFunction();
   };

   EMSCRIPTEN_BINDINGS(external_constructors) {
     class_<MyClass>("MyClass")
       .constructor<int, float>()
       .function("someFunction", &MyClass::someFunction)
       ;
   }


The :ref:`second form of the constructor <embind-class-function-pointer-constructor>`
takes a function pointer argument, and is used for classes that construct
themselves using a factory function. For example:

.. code:: cpp

   class MyClass {
     virtual void someFunction() = 0;
   };
   MyClass* makeMyClass(int, float); //Factory function.

   EMSCRIPTEN_BINDINGS(external_constructors) {
     class_<MyClass>("MyClass")
       .constructor(&makeMyClass, allow_raw_pointers())
       .function("someFunction", &MyClass::someFunction)
       ;
   }

The two constructors present *exactly the same interface* for constructing
the object in JavaScript. Continuing the example above:

.. code-block:: cpp

   var instance = new MyClass(10, 15.5);
   // instance is backed by a raw pointer to a MyClass in the Emscripten heap


Smart pointers
--------------

To manage object lifetime with smart pointers, *embind* must be told about
the smart pointer type.

For example, consider managing a class ``C``'s lifetime with
``std::shared_ptr<C>``. The best way to do this is to use
:cpp:func:`~class_::smart_ptr_constructor` to register the
smart pointer type:

.. code:: cpp

    EMSCRIPTEN_BINDINGS(better_smart_pointers) {
        class_<C>("C")
            .smart_ptr_constructor("C", &std::make_shared<C>)
            ;
    }

When an object of this type is constructed (e.g. using ``new Module.C()``)
it returns a ``std::shared_ptr<C>``.

An alternative is to use :cpp:func:`~class_::smart_ptr` in the
:cpp:func:`EMSCRIPTEN_BINDINGS` block:

.. code:: cpp

    EMSCRIPTEN_BINDINGS(smart_pointers) {
        class_<C>("C")
            .constructor<>()
            .smart_ptr<std::shared_ptr<C>>("C")
            ;
    }

Using this definition, functions can return ``std::shared_ptr<C>`` or take
``std::shared_ptr<C>`` as arguments, but ``new Module.C()`` would still
return a raw pointer.


unique_ptr
++++++++++

*embind* has built-in support for return values of type ``std::unique_ptr``.

Custom smart pointers
+++++++++++++++++++++

To teach *embind* about custom smart pointer templates, you must specialize
the :cpp:type:`smart_ptr_trait` template.



Non-member-functions on the JavaScript prototype
------------------------------------------------

Methods on the JavaScript class prototype can be non-member functions, as
long as the instance handle can be converted to the first argument of the
non-member function. The classic example is when the function exposed to
JavaScript does not exactly match the behavior of a C++ method.

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

If JavaScript calls ``Array10.prototype.get`` with an invalid index, it will
return ``undefined``.

Deriving from C++ classes in JavaScript
---------------------------------------

If C++ classes have virtual or abstract member functions, it's possible to
override them in JavaScript. Because JavaScript has no knowledge of the C++
vtable, *embind* needs a bit of glue code to convert C++ virtual function
calls into JavaScript calls.

Abstract methods
++++++++++++++++

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
            .allow_subclass<InterfaceWrapper>("InterfaceWrapper")
            ;
    }

:cpp:func:`~class_::allow_subclass` adds two special methods to the
Interface binding: ``extend`` and ``implement``. ``extend`` allows
JavaScript to subclass in the style exemplified by `Backbone.js`_.
``implement`` is used when you have a JavaScript object, perhaps
provided by the browser or some other library, and you want to
use it to implement a C++ interface.

.. note:: The :cpp:type:`pure_virtual` annotation on the function binding
   allows JavaScript to throw a helpful error if the JavaScript class
   does not override ``invoke()``. Otherwise, you may run into confusing
   errors.


``extend`` example
+++++++++++++++++++

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

    var instance = new DerivedClass;

``implement`` example
+++++++++++++++++++++

.. code:: javascript

    var x = {
        invoke: function(str) {
            console.log('invoking with: ' + str);
        }
    };
    var interfaceObject = Module.Interface.implement(x);

Now ``interfaceObject`` can be passed to any function that takes an
``Interface`` pointer or reference.

Non-abstract virtual methods
++++++++++++++++++++++++++++

If a C++ class has a non-pure virtual function, it can be overridden — but
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
            .allow_subclass<BaseWrapper>("BaseWrapper")
            .function("invoke", optional_override([](Base& self, const std::string& str) {
                return self.Base::invoke(str);
            }))
            ;
    }

When implementing ``Base`` with a JavaScript object, overriding ``invoke`` is
optional. The special lambda binding for invoke is necessary to avoid infinite
mutual recursion between the wrapper and JavaScript.

Base classes
------------

Base class bindings are defined as shown:

.. code:: cpp

    EMSCRIPTEN_BINDINGS(base_example) {
        class_<BaseClass>("BaseClass");
        class_<DerivedClass, base<BaseClass>>("DerivedClass");
    }

Any member functions defined on ``BaseClass`` are then accessible to
instances of ``DerivedClass``. In addition, any function that accepts
an instance of ``BaseClass`` can be given an instance of ``DerivedClass``.

Automatic downcasting
+++++++++++++++++++++

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

Calling ``Module.getDerivedInstance`` from JavaScript will return a
``Derived`` instance handle from which all of ``Derived``'s methods
are available.

.. note:: *Embind* must understand the fully-derived type for automatic
   downcasting to work.


Overloaded functions
====================

Constructors and functions can be overloaded on the number of arguments,
but *embind* does not support overloading based on type. When specifying
an overload, use the :cpp:func:`select_overload` helper function to select
the appropriate signature.

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

.. _embind-enums:

Enums
=====

*Embind*'s :cpp:class:`enumeration support <enum_>` works with both C++98
enums and C++11 "enum classes".

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

In both cases, JavaScript accesses enumeration values as properties of the
type.

.. code:: javascript

    Module.OldStyle.ONE;
    Module.NewStyle.TWO;

.. _embind-constants:

Constants
=========

To expose a C++ :cpp:func:`constant` to JavaScript, simply write:

.. code:: cpp

    EMSCRIPTEN_BINDINGS(my_constant_example) {
        constant("SOME_CONSTANT", SOME_CONSTANT);
    }

``SOME_CONSTANT`` can have any type known to *embind*.


.. _embind-memory-view:

Memory views
============

In some cases it is valuable to expose raw binary data directly to
JavaScript code as a typed array, allowing it to be used without copying.
This is useful for instance for uploading large WebGL textures directly
from the heap.

Memory views should be treated like raw pointers; lifetime and validity
are not managed by the runtime and it's easy to corrupt data if the
underlying object is modified or deallocated.

.. code:: cpp

    #include <emscripten/bind.h>
    #include <emscripten/val.h>

    using namespace emscripten;

    unsigned char *byteBuffer = /* ... */;
    size_t bufferLength = /* ... */;

    val getBytes() {
        return val(typed_memory_view(bufferLength, byteBuffer));
    }

    EMSCRIPTEN_BINDINGS(memory_view_example) {
        function("getBytes", &getBytes);
    }

The calling JavaScript code will receive a typed array view into the emscripten heap:

.. code:: js

   var myUint8Array = Module.getBytes()
   var xhr = new XMLHttpRequest();
   xhr.open('POST', /* ... */);
   xhr.send(myUint8Array);

The typed array view will be of the appropriate matching type, such as Uint8Array
for an ``unsigned char`` array or pointer.


.. _embind-val-guide:

Using ``val`` to transliterate JavaScript to C++
================================================

*Embind* provides a C++ class, :cpp:class:`emscripten::val`, which you can
use to transliterate JavaScript code to C++. Using ``val`` you can call
JavaScript objects from your C++, read and write their properties, or
coerce them to C++ values like a ``bool``, ``int``, or ``std::string``.

.. _Using-Web-Audio-API-from-Cpp-with-the-Embind-val-class:

The example below shows how you can use ``val`` to call the JavaScript
`Web Audio API`_ from C++:

.. note:: This example is based on the excellent Web Audio tutorial:
   `Making sine, square, sawtooth and triangle waves`_ (stuartmemo.com).
   There is an even simpler example in the :cpp:class:`emscripten::val`
   documentation.

First consider the JavaScript below, which shows how to use the API:


.. code-block:: javascript

   // Get web audio api context
   var AudioContext = window.AudioContext || window.webkitAudioContext;

   // Got an AudioContext: Create context and OscillatorNode
   var context = new AudioContext();
   var oscillator = context.createOscillator();

   // Configuring oscillator: set OscillatorNode type and frequency
   oscillator.type = 'triangle';
   oscillator.frequency.value = 261.63; // value in hertz - middle C

   // Playing
   oscillator.connect(context.destination);
   oscillator.start();

   // All done!

The code can be transliterated to C++ using ``val``, as shown below:

.. code-block:: cpp

   #include <emscripten/val.h>
   #include <stdio.h>
   #include <math.h>

   using namespace emscripten;

   int main() {
     val AudioContext = val::global("AudioContext");
     if (!AudioContext.as<bool>()) {
       printf("No global AudioContext, trying webkitAudioContext\n");
       AudioContext = val::global("webkitAudioContext");
     }

     printf("Got an AudioContext\n");
     val context = AudioContext.new_();
     val oscillator = context.call<val>("createOscillator");

     printf("Configuring oscillator\n");
     oscillator.set("type", val("triangle"));
     oscillator["frequency"].set("value", val(261.63)); // Middle C

     printf("Playing\n");
     oscillator.call<void>("connect", context["destination"]);
     oscillator.call<void>("start", 0);

     printf("All done!\n");
   }

First we use :cpp:func:`~emscripten::val::global` to get the symbol for
the global ``AudioContext`` object (or ``webkitAudioContext`` if that
does not exist). We then use :cpp:func:`~emscripten::val::new_` to create
the context, and from this context we can create an ``oscillator``,
:cpp:func:`~emscripten::val::set` its properties (again using ``val``)
and then play the tone.

The example can be compiled on the Linux/macOS terminal with::

   emcc -O2 -Wall -Werror --bind -o oscillator.html oscillator.cpp


Built-in type conversions
=========================

Out of the box, *embind* provides converters for many standard C++ types:

+---------------------+--------------------------------------------------------------------+
| C++ type            | JavaScript type                                                    |
+=====================+====================================================================+
| ``void``            | undefined                                                          |
+---------------------+--------------------------------------------------------------------+
| ``bool``            | true or false                                                      |
+---------------------+--------------------------------------------------------------------+
| ``char``            | Number                                                             |
+---------------------+--------------------------------------------------------------------+
| ``signed char``     | Number                                                             |
+---------------------+--------------------------------------------------------------------+
| ``unsigned char``   | Number                                                             |
+---------------------+--------------------------------------------------------------------+
| ``short``           | Number                                                             |
+---------------------+--------------------------------------------------------------------+
| ``unsigned short``  | Number                                                             |
+---------------------+--------------------------------------------------------------------+
| ``int``             | Number                                                             |
+---------------------+--------------------------------------------------------------------+
| ``unsigned int``    | Number                                                             |
+---------------------+--------------------------------------------------------------------+
| ``long``            | Number                                                             |
+---------------------+--------------------------------------------------------------------+
| ``unsigned long``   | Number                                                             |
+---------------------+--------------------------------------------------------------------+
| ``float``           | Number                                                             |
+---------------------+--------------------------------------------------------------------+
| ``double``          | Number                                                             |
+---------------------+--------------------------------------------------------------------+
| ``std::string``     | ArrayBuffer, Uint8Array, Uint8ClampedArray, Int8Array, or String   |
+---------------------+--------------------------------------------------------------------+
| ``std::wstring``    | String (UTF-16 code units)                                         |
+---------------------+--------------------------------------------------------------------+
| ``emscripten::val`` | anything                                                           |
+---------------------+--------------------------------------------------------------------+

For convenience, *embind* provides factory functions to register
``std::vector<T>`` (:cpp:func:`register_vector`) and ``std::map<K, V>``
(:cpp:func:`register_map`) types:

.. code:: cpp

    EMSCRIPTEN_BINDINGS(stl_wrappers) {
        register_vector<int>("VectorInt");
        register_map<int,int>("MapIntInt");
    }

A full example is shown below:

.. code:: cpp

    #include <emscripten/bind.h>
    #include <string>
    #include <vector>

    using namespace emscripten;

    std::vector<int> returnVectorData () {
      std::vector<int> v(10, 1);
      return v;
    }

    std::map<int, std::string> returnMapData () {
      std::map<int, std::string> m;
      m.insert(std::pair<int, std::string>(10, "This is a string."));
      return m;
    }

    EMSCRIPTEN_BINDINGS(module) {
      function("returnVectorData", &returnVectorData);
      function("returnMapData", &returnMapData);

      // register bindings for std::vector<int> and std::map<int, std::string>.
      register_vector<int>("vector<int>");
      register_map<int, std::string>("map<int, string>");
    }


The following JavaScript can be used to interact with the above C++.

.. code:: js

    var retVector = Module['returnVectorData']();

    // vector size
    var vectorSize = retVector.size();

    // reset vector value
    retVector.set(vectorSize - 1, 11);

    // push value into vector
    retVector.push_back(12);

    // retrieve value from the vector
    for (var i = 0; i < retVector.size(); i++) {
        console.log("Vector Value: ", retVector.get(i));
    }

    // expand vector size
    retVector.resize(20, 1);

    var retMap = Module['returnMapData']();

    // map size
    var mapSize = retMap.size();

    // retrieve value from map
    console.log("Map Value: ", retMap.get(10));

    // figure out which map keys are available
    // NB! You must call `register_vector<key_type>`
    // to make vectors available
    var mapKeys = retMap.keys();
    for (var i = 0; i < mapKeys.size(); i++) {
        var key = mapKeys.get(i);
        console.log("Map key/value: ", key, retMap.get(key));
    }

    // reset the value at the given index position
    retMap.set(10, "OtherValue");


Performance
===========

At time of writing there has been no *comprehensive* *embind* performance
testing, either against standard benchmarks, or relative to
:ref:`WebIDL-Binder`.

The call overhead for simple functions has been measured at about 200 ns.
While there is room for further optimisation, so far its performance in
real-world applications has proved to be more than acceptable.

.. _Test Suite: https://github.com/emscripten-core/emscripten/tree/master/tests/embind
.. _Connecting C++ and JavaScript on the Web with Embind: http://chadaustin.me/2014/09/connecting-c-and-javascript-on-the-web-with-embind/
.. _Boost.Python: http://www.boost.org/doc/libs/1_56_0/libs/python/doc/
.. _finalizers: http://en.wikipedia.org/wiki/Finalizer
.. _Boost.Python-like raw pointer policies: https://wiki.python.org/moin/boost.python/CallPolicy
.. _Backbone.js: http://backbonejs.org/#Model-extend
.. _Web Audio API: https://developer.mozilla.org/en-US/docs/Web/API/Web_Audio_API
.. _Making sine, square, sawtooth and triangle waves: http://stuartmemo.com/making-sine-square-sawtooth-and-triangle-waves/
