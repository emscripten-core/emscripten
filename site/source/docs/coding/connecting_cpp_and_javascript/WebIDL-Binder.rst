.. _WebIDL-Binder:

===========================
WebIDL Binder (wiki-import)
===========================
.. note:: This article was migrated from the wiki (Fri, 25 Jul 2014 04:21) and is now the "master copy" (the version in the wiki will be deleted). It may not be a perfect rendering of the original but we hope to fix that soon!

WebIDL Binder
=============

The WebIDL binder is the third tool providing bindings glue between C++
and JS in Emscripten. The first was the "bindings generator", which was
a hackish experiment (despite its hackishness, it managed to be good
enough for ammo.js and box2d.js). The second was Embind, which is fairly
high-level and supports mapping sophisticated C++11 constructs between
C++ and JS.

The goals of the WebIDL binder is to be low-level, efficient, and
simpler than Embind, while robust and more maintainable than the
bindings generator. Like Embind, it requires explicit declarations of
what to bind together, but like the bindings generator, it does so at a
low-level which is simpler to optimize.

The level at which it works is
`WebIDL <http://heycam.github.io/webidl/>`__, a language used to
describe the interfaces between JS and the browser's native code in C++.
WebIDL was designed for the particular purpose of gluing together C++
and JS, so it is a natural choice here; also, there are lots of
already-written WebIDL files for browser APIs, which could eventually be
reused.

The WebIDL binder is currently focused on wrapping C++ code so it is
usable from JS (write a C++ class, use it as a library in JS as if it
were a normal JS library), which is what the bindings generator does.
Embind also supports wrapping in the other direction, which the WebIDL
binder might do some day. In particular, it should be easy to let C++
call web APIs in a natural way by using WebIDL files describing those
APIs.

For a complete working example, see ``test_webidl`` in the test suite.
As always, the test suite code is guaranteed to work, so it's a good
place to learn from. Another good example is
`ammo.js <https://github.com/kripken/ammo.js/blob/webidl/ammo.idl>`__
which is the primary consumer of this tool.

Wrapping a C++ class
--------------------

The first stage is to create a WebIDL file. That describes the C++ class
you are going to wrap. This basically duplicates a little info from your
C++ header file, in a format that is explicitly designed for easy
parsing and to be able to represent things in a way that is convenient
for connecting to JS.

For example, let's say you have these C++ classes:

::

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

You could write this IDL file to describe them:

::

    interface Foo {
      void Foo();
      long getVal();
      void setVal(long v);
    };

    interface Bar {
      void Bar(long val);
      void doSomething();
    };

Note how both have a constructor (a function returning void that has the
same name as the interface), which allows you to construct them from
JS). Otherwise the definitions are fairly straightforward.

Note that types in WebIDL are not identically named to C++, for example
IDL ``long`` is a 32-bit integer, ``short`` is a 16-bit integer, etc.
There are also types like DOMString which represent a JS string.

To generate bindings code, run

::

    python tools/webidl_binder.py my_classes.idl glue

where ``my_classes.idl`` is the name of that IDL file, and glue is the
name of the output file you want. The bindings generator will emit two
files: ``glue.cpp`` and ``glue.js``.

To use those files, you should

1. Add ``--post-js glue.js`` in your final emcc command, so that it is
   included (at the end of your normal output).
2. Add a cpp file to that emcc command, which contains something like

::

    #include<..all the stuff the glue code needs, basically headers for the classes you are binding>

    #include<glue.cpp>

That way your emcc command will include both the C++ glue and the JS
glue, which are built to work together. The output should contain
everything you need, with the classes now usable through JS.

Using C++ classes in JS
-----------------------

Continuing the above example, you can write things like

::

    var f = new Module.Foo();
    f.setVal(200);
    alert(f.getVal());

    var b = new Module.Bar(123);
    b.doSomething();

and so forth.

Extended properties
-------------------

By default normal-looking IDL can bind a lot of regular C++ to JS.
However, you may need to use IDL extended properties to handle various
things.

Ref and Value
~~~~~~~~~~~~~

For example, the IDL binder assumes when it sees

::

      MyClass process(MyClass input);

in an interface, then both input and output values are pointers,
``MyClass*`` in C++. If, instead, they are references ``MyClass&`` then
you need to write

::

      [Ref] MyClass process([Ref] MyClass input);

If you don't do that, the generated glue C++ will not compile due to an
error on failure to convert a pointer to an object.

If the C++ returns a new object and not a reference
``MyClass process(MyClass* input)``, then you should do

::

      [Value] MyClass process([Ref] MyClass input);

This will allocate a static instance of that class and return it. Note
that that means there is a single such object, you should use it and
immediately forget about it.

NoDelete
~~~~~~~~

If a class cannot be deleted (because the destructor is private), you
can do

::

    [NoDelete]
    interface Foo {
      ..
    };

Const
~~~~~

You may need ``const`` to appear in the glue C++ code, so that it
matches your main C++ code. You can do that by adding ``[Const]`` to an
attribute or argument.

Prefix
~~~~~~

If you have a C++ class that is inside a namespace or another class, you
can still wrap it, by giving a prefix,

::

    [Prefix="Space::"]
    interface Inner {
      ..
    };

Everywhere that ``Inner`` appears in the C++ glue code, it will show up
as ``Space::Inner``, and compilation can succeed.

Operators
~~~~~~~~~

You can bind operators using

::

      [Operator="+="] TYPE1 add(TYPE2 x);

You can call it anything you want (``add`` is just an example). Note
that support is limited to ``+=,*=`` etc. for now (with ``=`` in them).

JSImplementation: Subclasses in JS
----------------------------------

Imagine you have a class that has a virtual method called from C++, and
you want to subclass it and implement it in JS. To do so, you can use
the JSImplementation option, for example in this IDL:

::

    [JSImplementation="Base"]
    interface ImplJS {
      void ImplJS();
      void virtualFunc();
      void virtualFunc2();
    };

``Base`` is the C++ class, and ImplJS does not exist in your C++ code.
``JSImplementation="Base"`` means "this class will be a JS
implementation of Base". After running the bindings generator and
compiling, you can do this:

::

    var c = new ImplJS();
    c.virtualFunc = function() { .. };

When C++ code has a pointer to a ``Base`` instance and calls
``virtualFunc``, that call will reach the JS code written here.

Note that you *must* implement all the methods you mentioned in the IDL
of the JSImplementation class (ImplJS). If not, then an error will be
shown (the technical reason is that C++ implements the virtual method,
in a way that calls into JS. If there is nothing in JS to be called, it
goes up through the prototype chain and calls that same C++ function
once more).

Other Issues
------------

Object cleanup
~~~~~~~~~~~~~~

If you create a JS object wrapping a C++ object, we call the C++
constructor for you transparently. However, if nothing else holds on to
the JS object, it will be GC'd, but we have no way to know that (due to
how JS works) - so we can't call the destructor for you.

Therefore, if a C++ object needs to be cleaned up, you should call
``Module.destroy(obj)`` on it. That calls the destructor directly. You
should then drop all references to the JS object as well.

Namespace
~~~~~~~~~

You should normally access objects on the Module object,
``Module.MyClass`` etc. While they are also in the global namespace, if
you use closure compiler or wrap the project in a function (to avoid
polluting the global namespace), then they won't be. You can of course
do ``var MyModuleName = Module;`` etc. to get whatever name you want
(ammo uses ``Ammo.`` for example).

Pointers and Comparisons
~~~~~~~~~~~~~~~~~~~~~~~~

All the bindings functions expect to receive wrapper objects, that
contain the raw pointer inside them, and not a raw pointer (which is
just a memory address - an integer). You should normally not need to
deal with raw pointers, but if you do, the following functions can help:

-  ``wrapPointer(ptr, Class)`` - Given a raw pointer (an integer),
   returns a wrapped object. Note that if you do not pass Class, it will
   be assumed to be the root class - this is likely not what you want!
-  ``getPointer(object)`` - Returns a raw pointer
-  ``castObject(object, Class)`` - Returns a wrapping of the same
   pointer but to another class ``compare(object1, object2)`` - Compares
   two objects' pointers

Note that there is always a *single* wrapped object for a certain
pointer for a certain class. This allows you to add data on that object
and use it elsewhere, by using normal JavaScript syntax
(``object.attribute = someData`` etc.). Note that this almost means that
``compare()`` is not needed - since you can compare two objects of the
same class, and if they have the same pointer they must be the same
object - but not quite: The tricky case is where one is a subclass of
the other, in which case the wrapped objects are different while the
pointer is the same. So, the correct way to compare two objects is to
call ``compare()``.

NULL
~~~~

All the bindings functions that return pointers/references/objects will
return wrapped pointers. The only potentially confusing case is when
they are returning a null pointer. In that case, you will get NULL (a
global singleton with a wrapped pointer of 0) instead of null (the
JavaScript builtin object) or 0. The reason is that by always returning
a wrapper, you can always take the output and pass it back to another
binding function, without that function needing to check the type of the
argument.

void*
~~~~~

The ``void*`` type is supported through a ``VoidPtr`` type which you can use
in IDL files. You can also use the ``any`` type. The difference between
them is that ``VoidPtr`` behaves like a pointer type in that you get
a wrapper object, while ``any`` behaves like a 32-bit integer (which is
what raw pointers are, in Emscripten-compiled code).

