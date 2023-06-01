.. _WebIDL-Binder:

=============
WebIDL Binder
=============

The *WebIDL Binder* provides a simple and lightweight approach to binding C++, so that compiled code can be called from JavaScript as if it were a normal JavaScript library.

The *WebIDL Binder* uses `WebIDL <http://www.w3.org/TR/WebIDL/>`_ to define the bindings, an interface language that was *specifically designed* for gluing together C++ and JavaScript. Not only is this a natural choice for the bindings, but because it is low-level it is relatively easy to optimize.

The binder supports the subset of C++ types that can be expressed in WebIDL. This subset is more than sufficient for most use cases — examples of projects that have been ported using the binder include the `Box2D <https://github.com/kripken/box2d.js/#box2djs>`_ and `Bullet <https://github.com/kripken/ammo.js/#ammojs>`_ physics engines.

This topic shows how bind and use C++ classes, functions and other types using IDL.

.. note:: An alternative to the *WebIDL Binder* is to use :ref:`Embind`. For more information see :ref:`interacting-with-code-binding-cpp`.


A quick example
===============

Binding using the *WebIDL Binder* is a three-stage process:

- Create a WebIDL file that describes the C++ interface.
- Use the binder to generate C++ and JavaScript "glue" code.
- Compile this glue code with the Emscripten project.


Defining the WebIDL file
------------------------

The first step is to create a *WebIDL file* that describes the C++ types you are going to bind. This file will duplicate some of the information in the C++ header file, in a format that is explicitly designed both for easy parsing, and for representing code items.

For example, consider the following C++ classes:

.. code-block:: cpp

  class Foo {
  public:
    int getVal();
    void setVal(int v);
  };

  class Bar {
  public:
    Bar(long val);
    void doSomething();
  };

The following IDL file can be used to describe them:

.. code-block:: idl

  interface Foo {
    void Foo();
    long getVal();
    void setVal(long v);
  };

  interface Bar {
    void Bar(long val);
    void doSomething();
  };

The mapping between the IDL definition and the C++ is fairly obvious. The main things to notice are:

  - The IDL class definitions include a function returning ``void`` that has the same name as the interface. This constructor allows you to create the object from JavaScript, and must be defined in IDL even if the C++ uses the default constructor (see ``Foo`` above).
  - The type names in WebIDL are not identical to those in C++ (for example, ``int`` maps to ``long`` above). For more information about the mappings see :ref:`webidl-binder-type-name`.

.. note:: ``structs`` are defined in the same way as the classes above — using the ``interface`` keyword.

Generating the bindings glue code
---------------------------------

The *bindings generator* (`tools/webidl_binder.py <https://github.com/emscripten-core/emscripten/blob/main/tools/webidl_binder.py>`_) takes a Web IDL file name and an output file name as inputs, and creates C++ and JavaScript glue code files.

For example, to create the glue code files **glue.cpp** and **glue.js** for the IDL file **my_classes.idl**, you would use the following command:

.. code-block:: bash

    tools/webidl_binder my_classes.idl glue



Compiling the project (using the bindings glue code)
----------------------------------------------------

To use the glue code files (``glue.cpp`` and ``glue.js``) in a project:

#. Add ``--post-js glue.js`` in your final *emcc* command. The :ref:`post-js <emcc-post-js>` option adds the glue code at the end of the compiled output.
#. Create a file called something like **my_glue_wrapper.cpp** to ``#include`` the headers of the classes you are binding and *glue.cpp*. This might have the following content:

  .. code-block:: cpp

    #include <...> // Where "..." represents the headers for the classes we are binding.
    #include <glue.cpp>

  .. note:: The C++ glue code emitted by the *bindings generator* does not include the headers for the classes it binds because they are not present in the Web IDL file. The step above makes these available to the glue code. Another alternative would be to include the headers at the top of **glue.cpp**, but then they would be overwritten every time the IDL file is recompiled.

#. Add **my_glue_wrapper.cpp** to the final *emcc* command.


The final *emcc* command includes both the C++ and JavaScript glue code, which are built to work together:

.. code-block:: bash

  emcc my_classes.cpp my_glue_wrapper.cpp --post-js glue.js -o output.js

The output now contains everything needed to use the C++ classes through JavaScript.


Modular output
==============

When using the WebIDL binder, often what you are doing is creating a library. In that
case, the `MODULARIZE` option makes sense to use. It wraps the entire JavaScript output
in a function, and returns a Promise which resolves to the initialized Module instance.

.. code-block:: javascript

  var instance;
  Module().then(module => {
    instance = module;
  });

The promise is resolved when it is safe to run compiled code, i.e. after it
has been has been downloaded and instantiated. The promise is resolved at the
same time the `onRuntimeInitialized` callback is invoked, so there's no need to
use `onRuntimeInitialized` when using `MODULARIZE`.

You can use the `EXPORT_NAME` option to change `Module` to something else. This is
good practice for libraries, as then they don't include unnecessary things in the
global scope, and in some cases you want to create more than one.


Using C++ classes in JavaScript
================================

Once binding is complete, C++ objects can be created and used in JavaScript as though they were normal JavaScript objects. For example, continuing the above example, you can create the ``Foo`` and ``Bar`` objects and call methods on them.

.. code-block:: javascript

  var f = new Module.Foo();
  f.setVal(200);
  alert(f.getVal());

  var b = new Module.Bar(123);
  b.doSomething();

.. important:: Always access objects through the :ref:`module` object, as shown above.

  While the objects are also available in the global namespace by default, there are cases where they will not be (for example, if you use the :term:`closure compiler` to minify code or wrap compiled code in a function to avoid polluting the global namespace). You can of course use whatever name you like for the module by assigning it to a new variable: ``var MyModuleName = Module;``.

.. important:: You can only use this code when it is :ref:`safe to call compiled code <faq-when-safe-to-call-compiled-functions>`, see more details in that FAQ entry.

JavaScript will automatically garbage collect any of the wrapped C++ objects when there are no more references. If the C++ object doesn't require specific clean up (i.e. it doesn't have a destructor) then no other action needs to be taken.

If a C++ object does need to be cleaned up, you must explicitly call :js:func:`Module.destroy(obj) <Module.destroy>` to invoke its destructor — then drop all references to the object so that it can be garbage collected. For example, if ``Bar`` were to allocate memory that requires cleanup:

.. code-block:: javascript

  var b = new Module.Bar(123);
  b.doSomething();
  Module.destroy(b); // If the C++ object requires clean up

.. note:: The C++ constructor is called transparently when a C++ object is created in JavaScript. There is no way, however, to tell if a JavaScript object is about to be garbage collected, so the binder glue code can't automatically call the destructor.

  You will usually need to destroy the objects which you create, but this depends on the library being ported.

Attributes
==========

Object attributes are defined in IDL using the ``attribute`` keyword. These can then be accessed in JavaScript using either ``get_foo()``/``set_foo()`` accessor methods, or directly as a property of the object.

.. code-block:: cpp
	
	// C++
	int attr;

.. code-block:: idl

	// WebIDL
	attribute long attr;

.. code-block:: javascript

	// JavaScript
	var f = new Module.Foo();
	f.attr = 7;
	// Equivalent to:
	f.set_attr(7);

	console.log(f.attr);
	console.log(f.get_attr());

For read-only attributes, see :ref:`webidl-binder-const`.

Pointers, References, Value types (Ref and Value)
====================================================

C++ arguments and return types can be pointers, references, or value types (allocated on the stack). The IDL file uses different decoration to represent each of these cases.

Undecorated argument and return values of a custom type in the IDL are assumed to be *pointers* in the C++:

.. code-block:: cpp

  // C++
  MyClass* process(MyClass* input);

.. code-block:: idl

  // WebIDL
  MyClass process(MyClass input);
  
This assumption isn't true for base types like void,int,bool,DOMString,etc.

References should be decorated using ``[Ref]``:

.. code-block:: cpp

  // C++
  MyClass& process(MyClass& input);

.. code-block:: idl

  // WebIDL
  [Ref] MyClass process([Ref] MyClass input);


.. note:: If ``[Ref]`` is omitted on a reference, the generated glue C++ will not compile (it fails when it tries to convert the reference — which it thinks is a pointer — to an object).

If the C++ returns an object (rather than a reference or a pointer) then the return type should be decorated using ``[Value]``. This will allocate a static (singleton) instance of that class and return it. You should use it immediately, and drop any references to it after use.

.. code-block:: cpp

  // C++
  MyClass process(MyClass& input);

.. code-block:: idl

  // WebIDL
  [Value] MyClass process([Ref] MyClass input);

.. _webidl-binder-const:

Const
=====

C++ arguments or return types that use ``const`` can be specified in IDL using ``[Const]``.

For example, the following code fragments show the C++ and IDL for a function that returns a constant pointer object.

.. code-block:: cpp

  //C++
  const myObject* getAsConst();

.. code-block:: idl

  // WebIDL
  [Const] myObject getAsConst();

Attributes that correspond to const data members must be specified with the ``readonly`` keyword, not with ``[Const]``. For example:

.. code-block:: cpp

  //C++
  const int numericalConstant;

.. code-block:: idl

  // WebIDL
  readonly attribute long numericalConstant;

This will generate a ``get_numericalConstant()`` method in the bindings, but not a corresponding setter. The attribute will also be defined as read-only in JavaScript, meaning that trying to set it will have no effect on the value, and will throw an error in strict mode.

.. tip:: It is possible for a return type to have multiple specifiers. For example, a method that returns a constant reference would be marked up in the IDL using ``[Ref, Const]``.


Un-deletable classes (NoDelete)
===============================

If a class cannot be deleted (because the destructor is private), specify ``[NoDelete]`` in the IDL file.

.. code-block:: idl

  [NoDelete]
  interface Foo {
  ...
  };



Defining inner classes and classes inside namespaces (Prefix)
=============================================================

C++ classes that are declared inside a namespace (or another class) must use the IDL file ``Prefix`` keyword to specify the scope. The prefix is then used whenever the class is referred to in C++ glue code.

For example, the following IDL definition ensures that ``Inner`` class is referred to as ``MyNameSpace::Inner``

.. code-block:: idl

  [Prefix="MyNameSpace::"]
  interface Inner {
  ..
  };


Operators
=========

You can bind to C++ operators using ``[Operator=]``:

.. code-block:: idl

  [Operator="+="] TYPE1 add(TYPE2 x);


.. note::

  - The operator name can be anything (``add`` is just an example).
  - Support is currently limited to operators that contain ``=``: ``+=``, ``*=``, ``-=`` etc., and to the array indexing operator ``[]``.


enums
=====

Enums are declared very similarly in C++ and IDL:

.. code-block:: cpp

  // C++
  enum AnEnum {
    enum_value1,
    enum_value2
  };

  // WebIDL
  enum AnEnum {
    "enum_value1",
    "enum_value2"
  };

The syntax is slightly more complicated for enums declared inside a namespace:

.. code-block:: cpp

  // C++
  namespace EnumNamespace {
    enum EnumInNamespace {
    e_namespace_val = 78
    };
  };

  // WebIDL
  enum EnumNamespace_EnumInNamespace {
    "EnumNamespace::e_namespace_val"
  };

When the enum is defined inside a class, the IDL definitions for the enum and class interface are separate:

.. code-block:: cpp

  // C++
  class EnumClass {
   public:
    enum EnumWithinClass {
    e_val = 34
    };
    EnumWithinClass GetEnum() { return e_val; }

    EnumNamespace::EnumInNamespace GetEnumFromNameSpace() { return EnumNamespace::e_namespace_val; }
  };



  // WebIDL
  enum EnumClass_EnumWithinClass {
    "EnumClass::e_val"
  };

  interface EnumClass {
    void EnumClass();

    EnumClass_EnumWithinClass GetEnum();

    EnumNamespace_EnumInNamespace GetEnumFromNameSpace();
  };



Sub-classing C++ base classes in JavaScript (JSImplementation)
===============================================================

The *WebIDL Binder* allows C++ base classes to be sub-classed in JavaScript. In the IDL fragment below, ``JSImplementation="Base"`` means that the associated interface (``ImplJS``) will be a JavaScript implementation of the C++ class ``Base``.

.. code-block:: idl

  [JSImplementation="Base"]
  interface ImplJS {
    void ImplJS();
    void virtualFunc();
    void virtualFunc2();
  };

After running the bindings generator and compiling, you can implement the interface in JavaScript as shown:

.. code-block:: javascript

    var c = new ImplJS();
    c.virtualFunc = function() { .. };

When C++ code has a pointer to a ``Base`` instance and calls ``virtualFunc()``, that call will reach the JavaScript code defined above.

.. note::

  - You *must* implement all the methods you mentioned in the IDL of the ``JSImplementation`` class (``ImplJS``) or compilation will fail with an error.
  - You will also need to provide an interface definition for the ``Base`` class in the IDL file.

Pointers and comparisons
=========================

All the binding functions expect to receive wrapper objects (which contain a raw pointer) rather than a raw pointer. You shouldn't normally need to deal with raw pointers (these are simply memory addresses/integers). If you do, the following functions in the compiled code can be useful:

- ``wrapPointer(ptr, Class)`` — Given a raw pointer (an integer), returns a wrapped object.

  .. note:: If you do not pass the ``Class``, it will be assumed to be the root class — this probably isn't what you want!

- ``getPointer(object)`` — Returns a raw pointer.
- ``castObject(object, Class)`` — Returns a wrapping of the same pointer but to another class.
- ``compare(object1, object2)`` — Compares two objects' pointers.

.. note:: There is always a *single* wrapped object for a certain pointer to a certain class. This allows you to add data on that object and use it elsewhere using normal JavaScript syntax (``object.attribute = someData`` etc.)

  ``compare()`` should be used instead of direct pointer comparison because it is possible to have different wrapped objects with the same pointer if one class is a subclass of the other.



NULL
====

All the binding functions that return pointers, references, or objects will return wrapped pointers. The reason is that by always returning a wrapper, you can take the output and pass it to another binding function without that function needing to check the type of the argument.

One case where this can be confusing is when returning a ``NULL`` pointer. When using bindings, the returned pointer will be ``NULL`` (a global singleton with a wrapped pointer of 0) rather than ``null`` (the JavaScript built-in object) or 0.


.. _webidl-binder-voidstar:

void*
=====

The ``void*`` type is supported through a ``VoidPtr`` type that you can use in IDL files. You can also use the ``any`` type.

The difference between them is that ``VoidPtr`` behaves like a pointer type in that you get a wrapper object, while ``any`` behaves like a 32-bit integer (which is what raw pointers are in Emscripten-compiled code).


.. _webidl-binder-type-name:

WebIDL types
============

The type names in WebIDL are not identical to those in C++. This section shows the mapping for the more common types you'll encounter.

.. csv-table::
  :header: "C++", "IDL"
  :widths: 50, 50

  "``bool``", "``boolean``"
  "``float``", "``float``"
  "``double``", "``double``"
  "``char``", "``byte``"
  "``char*``", "``DOMString`` (represents a JavaScript string)"
  "``unsigned char``", "``octet``"
  "``int``", "``long``"
  "``long``", "``long``"
  "``unsigned short``", "``unsigned short``"
  "``unsigned long``", "``unsigned long``"
  "``long long``", "``long long``"
  "``void``", "``void``"
  "``void*``", "``any`` or ``VoidPtr`` (see :ref:`webidl-binder-voidstar`)"

.. note:: The WebIDL types are fully documented in `this W3C specification <http://www.w3.org/TR/WebIDL/>`_.


.. _webidl-binder-test-code:

Test and example code
=====================

For a complete working example, see `test_webidl <https://github.com/emscripten-core/emscripten/tree/main/test/webidl>`_ in the `test suite <https://github.com/emscripten-core/emscripten/blob/main/test/test_core.py>`_. The test suite code is guaranteed to work and covers more cases than this article alone.

Another good example is `ammo.js <https://github.com/kripken/ammo.js/tree/master>`_, which uses the *WebIDL Binder* to port the `Bullet Physics engine <http://bulletphysics.org/wordpress/>`_ to the Web.
