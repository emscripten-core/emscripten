.. _WebIDL-Binder:

===================================
WebIDL Binder (under-construction)
===================================

The *WebIDL Binder* provides a simple and lightweight approach to binding C++ so that it can be called from JavaScript (when compiled) as if it were a normal JavaScript library. 

The *WebIDL Binder* uses `WebIDL <http://heycam.github.io/webidl/>`_ to define the bindings, an interface language which was *specifically designed* for gluing together C++ and JavaScript. Not only is this a natural choice for the bindings, but because it is low-level, it is relatively easy to optimize.

This topic shows how to define an IDL file for a C++ class (and other C++ items), generate the C++ and JavaScript wrapper/glue code, and then compile this as part of your Emscripten project. 

For a complete working example, see **test_webidl** in the `test suite <https://github.com/kripken/emscripten/blob/master/tests/test_core.py>`_ (sources `here <https://github.com/kripken/emscripten/tree/master/tests/webidl>`_). The test suite code is guaranteed to work and covers more material than this article. Another good example is `ammo.js <https://github.com/kripken/ammo.js/tree/master>`_, which uses the *WebIDL Binder* to port the Bullet Physics engine to the web.

.. note:: An alternative to the *WebIDL Binder* is to use :ref:`Embind`. This tool supports mapping sophisticated C++11 constructs between C++ and JavaScript in both directions.

	Both *Embind* and the *WebIDL Binder* require explicit declarations of what to bind together. However the *WebIDL Binder* is (arguably) easier to use, and does so at a lower-level which is simpler to optimize.
	
	The *WebIDL Binder* does not currently support binding in the other direction (calling Web APIs from C++).
	

Wrapping a C++ class
====================

The first step is to create a WebIDL file that describes the C++ class you are going to wrap. This file duplicates some of the information in the C++ header file, in a format that is explicitly designed both for easy parsing, and to represent code items in a way that is convenient for connecting to JavaScript.

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

The definitions include a constructor that allows you to construct them from JavaScript — this is just a function returning void that has the same name as the interface. Otherwise the definitions are fairly straightforward.

.. note:: The type names in WebIDL are not identical to those in C++. For example IDL ``long`` is a 32-bit integer, ``short`` is a 16-bit integer, etc. There are also types like ``DOMString`` which represent a JavaScript string.


Generating the bindings glue code
=================================

To generate bindings code, run

::

    python tools/webidl_binder.py my_classes.idl glue

where ``my_classes.idl`` is the name of that IDL file, and glue is the name of the output file you want. The bindings generator will emit two files: ``glue.cpp`` and ``glue.js``.

To use those files, you should

1. Add ``--post-js glue.js`` in your final emcc command, so that it is
   included (at the end of your normal output).
2. Add a cpp file to that emcc command, which contains something like

::

    #include<..all the stuff the glue code needs, basically headers for the classes you are binding>

    #include<glue.cpp>

That way your emcc command will include both the C++ glue and the JavaScript glue, which are built to work together. The output should contain everything you need, with the classes now usable through JavaScript.

Using C++ classes in JavaScript
=======================================

Continuing the above example, you can write things like

::

    var f = new Module.Foo();
    f.setVal(200);
    alert(f.getVal());

    var b = new Module.Bar(123);
    b.doSomething();

and so forth.

Extended properties
==========================

By default normal-looking IDL can bind a lot of regular C++ to JavaScript. However, you may need to use IDL extended properties to handle various things.

Ref and Value
-------------

For example, the IDL binder assumes when it sees

::

      MyClass process(MyClass input);

in an interface, then both input and output values are pointers, ``MyClass*`` in C++. If, instead, they are references ``MyClass&`` then you need to write

::

      [Ref] MyClass process([Ref] MyClass input);

If you don't do that, the generated glue C++ will not compile due to an error on failure to convert a pointer to an object.

If the C++ returns a new object and not a reference ``MyClass process(MyClass* input)``, then you should do

::

      [Value] MyClass process([Ref] MyClass input);

This will allocate a static instance of that class and return it. Note that that means there is a single such object, you should use it and immediately forget about it.

NoDelete
-------------

If a class cannot be deleted (because the destructor is private), you can do

::

    [NoDelete]
    interface Foo {
      ..
    };

Const
-------------

You may need ``const`` to appear in the glue C++ code, so that it matches your main C++ code. You can do that by adding ``[Const]`` to an attribute or argument.

Prefix
-------------

If you have a C++ class that is inside a namespace or another class, you can still wrap it, by giving a prefix,

::

    [Prefix="Space::"]
    interface Inner {
      ..
    };

Everywhere that ``Inner`` appears in the C++ glue code, it will show up as ``Space::Inner``, and compilation can succeed.

Operators
-------------

You can bind operators using

::

      [Operator="+="] TYPE1 add(TYPE2 x);

You can call it anything you want (``add`` is just an example). Note that support is limited to ``+=,*=`` etc. for now (with ``=`` in them).

JSImplementation: Subclasses in JavaScript
====================================================

Imagine you have a class that has a virtual method called from C++, and you want to subclass it and implement it in JavaScript. To do so, you can use the JSImplementation option, for example in this IDL:

::

    [JSImplementation="Base"]
    interface ImplJS {
      void ImplJS();
      void virtualFunc();
      void virtualFunc2();
    };

``Base`` is the C++ class, and ImplJS does not exist in your C++ code. ``JSImplementation="Base"`` means "this class will be a JavaScript implementation of Base". After running the bindings generator and compiling, you can do this:

::

    var c = new ImplJS();
    c.virtualFunc = function() { .. };

When C++ code has a pointer to a ``Base`` instance and calls ``virtualFunc``, that call will reach the JavaScript code written here.

Note that you *must* implement all the methods you mentioned in the IDL of the JSImplementation class (ImplJS). If not, then an error will be shown (the technical reason is that C++ implements the virtual method, in a way that calls into JavaScript. If there is nothing in JavaScript to be called, it goes up through the prototype chain and calls that same C++ function once more).

Other Issues
=============

Object cleanup
--------------------------

If you create a JavaScript object wrapping a C++ object, we call the C++ constructor for you transparently. However, if nothing else holds on to the JavaScript object, it will be GC'd, but we have no way to know that (due to how JavaScript works) - so we can't call the destructor for you.

Therefore, if a C++ object needs to be cleaned up, you should call ``Module.destroy(obj)`` on it. That calls the destructor directly. You should then drop all references to the JavaScript object as well.

Namespace
-------------

You should normally access objects on the Module object, ``Module.MyClass`` etc. While they are also in the global namespace, if you use closure compiler or wrap the project in a function (to avoid polluting the global namespace), then they won't be. You can of course do ``var MyModuleName = Module;`` etc. to get whatever name you want (ammo uses ``Ammo.`` for example).

Pointers and comparisons
--------------------------

All the bindings functions expect to receive wrapper objects, that contain the raw pointer inside them, and not a raw pointer (which is just a memory address - an integer). You should normally not need to deal with raw pointers, but if you do, the following functions can help:

-  ``wrapPointer(ptr, Class)`` - Given a raw pointer (an integer), returns a wrapped object. Note that if you do not pass Class, it will be assumed to be the root class - this is likely not what you want!
-  ``getPointer(object)`` - Returns a raw pointer
-  ``castObject(object, Class)`` - Returns a wrapping of the same pointer but to another class ``compare(object1, object2)`` - Compares two objects' pointers

Note that there is always a *single* wrapped object for a certain pointer for a certain class. This allows you to add data on that object and use it elsewhere, by using normal JavaScript syntax (``object.attribute = someData`` etc.). Note that this almost means that ``compare()`` is not needed - since you can compare two objects of the same class, and if they have the same pointer they must be the same object - but not quite: The tricky case is where one is a subclass of the other, in which case the wrapped objects are different while the pointer is the same. So, the correct way to compare two objects is to call ``compare()``.

NULL
-------------

All the bindings functions that return pointers/references/objects will return wrapped pointers. The only potentially confusing case is when they are returning a null pointer. In that case, you will get NULL (a global singleton with a wrapped pointer of 0) instead of null (the JavaScript builtin object) or 0. The reason is that by always returning a wrapper, you can always take the output and pass it back to another binding function, without that function needing to check the type of the argument.

void*
-------------

The ``void*`` type is supported through a ``VoidPtr`` type which you can use in IDL files. You can also use the ``any`` type. The difference between them is that ``VoidPtr`` behaves like a pointer type in that you get a wrapper object, while ``any`` behaves like a 32-bit integer (which is what raw pointers are, in Emscripten-compiled code).

