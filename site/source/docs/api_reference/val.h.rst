.. _val-h:

================================
val.h (under-construction)
================================

.. COMMENT (Not rendered) : This created from val.h header file on 10 Aug 2014-03

The *Embind* C++ class :cpp:class:`emscripten::val` (defined in `val.h <https://github.com/emscripten-core/emscripten/blob/master/system/include/emscripten/val.h>`_) is used to *transliterate* JavaScript code to C++.

Guide material for this class can be found in :ref:`embind-val-guide`.


.. cpp:namespace:: emscripten

.. cpp:class:: emscripten::val

  This class is a C++ data type that can be used to represent (and provide convenient access to) any JavaScript object. You can use it to call a JavaScript object, read and write its properties, or coerce it to a C++ value like a ``bool``, ``int``, or ``std::string``.

  For example, the code below shows some simple JavaScript for making an XHR request on a URL:

  .. code:: javascript

    var xhr = new XMLHttpRequest;
    xhr.open("GET", "http://url");


  This same code can be written in C++, using :cpp:func:`~emscripten::val::global` to get the symbol for the global ``XMLHttpRequest`` object and then using it to open a URL.


  .. code:: cpp

    val xhr = val::global("XMLHttpRequest").new_();
    xhr.call("open", std::string("GET"), std::string("http://url"));


  See :ref:`embind-val-guide` for other examples.


  .. todo::

    **HamishW** Notes from source FYI: Can/should these be included? ::

      // missing operators:
      // * delete
      // * in
      // * instanceof
      // * ! ~ - + ++ --
      // * * / %
      // * + -
      // * << >> >>>
      // * < <= > >=
      // * == != === !==
      // * & ^ | && || ?:
      //
      // exposing void, comma, and conditional is unnecessary
      // same with: = += -= *= /= %= <<= >>= >>>= &= ^= |=


  .. cpp:function:: static val array()

    Creates and returns a new ``Array``.

    :returns: The new ``Array``.


  .. cpp:function:: static val object()

    Creates and returns a new ``Object``.

    :returns: The new ``Object``.


  .. cpp:function:: static val undefined()

    Creates a ``val`` that represents ``undefined``.

    :returns: The ``val`` that represents ``undefined``.


  .. cpp:function:: static val null()

    Creates a ``val`` that represents ``null``. ``val::undefined()`` is the same, but for undefined.

    :returns: A ``val`` that represents ``null``.


  .. cpp:function:: static val take_ownership(internal::EM_VAL e)

    **HamishW**-Replace with description.

    :returns: **HamishW**-Replace with description.


  .. cpp:function:: static val global(const char* name)

    Looks up a global symbol.

    :param const char* name: **HamishW**-Replace with description.
    :returns: **HamishW**-Replace with description.



  .. cpp:function:: static val module_property(const char* name)

    Looks up a symbol on the emscripten Module object.

    :param const char* name: **HamishW**-Replace with description.
    :returns: **HamishW**-Replace with description.


  .. cpp:function:: explicit val(T&& value)

    Constructor.

    A ``val`` can be constructed by explicit construction from any C++ type. For example, ``val(true)`` or ``val(std::string("foo"))``.

    :param T&& value: Any C++ type.


  **HamishW** Don't know how following "floating statement works". Leaving here for discussion
  ``val() = delete;``


  .. cpp:function:: explicit val(const char* v)

    **HamishW**-Replace with description.

    :param const char* v: **HamishW**-Replace with description.


  .. cpp:function:: val(val&& v)

    **HamishW**-Replace with description.

    :param val&& v: **HamishW**-Replace with description.


  .. cpp:function:: val(const val& v)

    **HamishW**-Replace with description.

    :param const val& v: **HamishW**-Replace with description.


  .. cpp:function:: ~val()

    Destructor. **HamishW**-Replace with further description or delete comment.


  .. cpp:function:: val(const val& v)

    **HamishW**-Replace with description.

    :param const val& v: **HamishW**-Replace with description.


  .. cpp:function:: val& operator=(val&& v)

    **HamishW**-Replace with description.

    :param val&& v: **HamishW**-Replace with description.
    :returns: **HamishW**-Replace with description.


  .. cpp:function:: val& operator=(const val& v)

    **HamishW**-Replace with description.

    :param val&& v: **HamishW**-Replace with description.
    :returns: **HamishW**-Replace with description.


  .. cpp:function:: bool hasOwnProperty(const char* key) const

    Test whether ... **HamishW**-Replace with description.

    :param const char* key: **HamishW**-Replace with description.
    :returns: **HamishW**-Replace with description.


  .. cpp:function:: val new_()

    prototype:

    ::

      template<typename... Args>
      val new_(Args&&... args) const

    **HamishW**-Replace with description.

    :param Args&&... args: **HamishW**-Replace with description. Note that this is a templated value.
    :returns: **HamishW**-Replace with description.



  .. cpp:function:: val operator[](const T& key) const

    **HamishW**-Replace with description.

    :param const T& key: **HamishW**-Replace with description. Note that this is a templated value.
    :returns: **HamishW**-Replace with description.


  .. cpp:function:: void set(const K& key, const val& v)

    Set the specified (``key``) property of a JavaScript object (accessed through a ``val``) with the value ``v``. **HamishW**-Replace with description.

    :param const K& key: **HamishW**-Replace with description. Note that this is a templated value.
    :param const val& v: **HamishW**-Replace with description.   Note that this is a templated value.


  .. cpp:function:: val operator()(Args&&... args)

    **HamishW**-Replace with description.

    :param Args&&... args: **HamishW**-Replace with description. Note that this is a templated value.


  .. cpp:function:: ReturnValue call(const char* name, Args&&... args) const

    **HamishW**-Replace with description.

    :param const char* name: **HamishW**-Replace with description.
    :param Args&&... args: **HamishW**-Replace with description. Note that this is a templated value.


  .. cpp:function:: T as() const

    **HamishW**-Replace with description.

    :returns: **HamishW**-Replace with description. Note that this is a templated value.


  .. cpp:function:: val typeof() const

    **HamishW**-Replace with description.

    :returns: **HamishW**-Replace with description.


  .. cpp:function:: std::vector<T> vecFromJSArray(val v)

    **HamishW**-Replace with description.

    **HamishW**. I believe NOT internal. Please confirm.

    :param val v: **HamishW**-Replace with description.
    :returns: **HamishW**-Replace with description.


.. cpp:type: EMSCRIPTEN_SYMBOL(name)

  **HamishW**-Replace with description.
