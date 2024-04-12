.. _val-h:

=====
val.h
=====

The *Embind* C++ class :cpp:class:`emscripten::val` (defined in `val.h <https://github.com/emscripten-core/emscripten/blob/main/system/include/emscripten/val.h>`_) is used to *transliterate* JavaScript code to C++.

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
    xhr.call<void>("open", std::string("GET"), std::string("http://url"));

  You can test whether the ``open`` method call was successful using :cpp:func:`~emscripten::val::operator[]` to read an object property, then :cpp:func:`~emscripten::val::as` to coerce the type:

  .. code:: cpp

    const char* state;
    switch (xhr["readyState"].as<int>()) {
    case 0:
      state = "UNSENT"; break;
    case 1:
      state = "OPENED"; break;
    default:
      state = "etc";
    }

  See :ref:`embind-val-guide` for other examples.


  .. warning:: JavaScript values can't be shared across threads, so neither can ``val`` instances that bind them.

    For example, if you want to cache some JavaScript global as a ``val``, you need to retrieve and bind separate instances of that global by its name in each thread.
    The easiest way to do this is with a ``thread_local`` declaration:

    .. code:: cpp

      thread_local const val Uint8Array = val::global("Uint8Array");

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


  .. cpp:function:: static val object()

    Creates and returns a new ``Object``.


  .. cpp:function:: static val u8string(const char* s)

    Creates a ``val`` from a string literal in UTF-8 encoding.


  .. cpp:function:: static val u16string(const char16_t* s)

    Creates a ``val`` from a string literal in UTF-16 encoding.


  .. cpp:function:: static val undefined()

    Creates a ``val`` that represents ``undefined``.


  .. cpp:function:: static val null()

    Creates a ``val`` that represents ``null``.

  .. _val_as_handle:
  .. cpp:function:: EM_VAL as_handle() const

    Returns a raw handle representing this ``val``. This can be used for
    passing raw value handles to JavaScript and retrieving the values on the
    other side via ``Emval.toValue`` function. Example:

    .. code:: cpp

      EM_JS(void, log_value, (EM_VAL val_handle), {
        var value = Emval.toValue(val_handle);
        console.log(value); // 42
      });

      val foo(42);
      log_value(foo.as_handle());


  .. cpp:function:: static val take_ownership(EM_VAL e)

    Creates a ``val`` from a raw handle. This can be used for retrieving values
    from JavaScript, where the JavaScript side should wrap a value with
    ``Emval.toHandle``, pass it to C++, and then C++ can use ``take_ownership``
    to convert it to a ``val`` instance. Example:

    .. code:: cpp

      EM_ASYNC_JS(EM_VAL, fetch_json_from_url, (const char *url), {
        var url = UTF8ToString(url);
        var response = await fetch(url);
        var json = await response.json();
        return Emval.toHandle(json);
      });

      val obj = val::take_ownership(fetch_json_from_url("https://httpbin.org/json"));
      std::string author = obj["slideshow"]["author"].as<std::string>();


  .. cpp:function:: static val global(const char* name)

    Looks up a global value by the specified ``name``.



  .. cpp:function:: static val module_property(const char* name)

    Looks up a value by the provided ``name`` on the Emscripten Module object.


  .. cpp:function:: explicit val(T&& value)

    Constructor.

    Creates a ``val`` by conversion from any Embind-compatible C++ type.
    For example, ``val(true)`` or ``val(std::string("foo"))``.


  .. cpp:function:: explicit val(const char* v)

    Constructs a ``val`` instance from a string literal.


  .. cpp:function:: val(val&& v)

    Moves ownership of a value to a new ``val`` instance.


  .. cpp:function:: val(const val& v)

    Creates another reference to the same value behind the provided ``val`` instance.


  .. cpp:function:: ~val()

    Removes the currently bound value by decreasing its refcount.


  .. cpp:function:: val& operator=(val&& v)

    Removes a reference to the currently bound value and takes over the provided one.


  .. cpp:function:: val& operator=(const val& v)

    Removes a reference to the currently bound value and creates another reference to
    the value behind the provided ``val`` instance.


  .. cpp:function:: bool hasOwnProperty(const char* key) const

    Checks if the JavaScript object has own (non-inherited) property with the specified name.


  .. cpp:function:: val new_(Args&&... args) const

    Assumes that current value is a constructor, and creates an instance of it.
    Equivalent to a JavaScript expression `new currentValue(...)`.



  .. cpp:function:: val operator[](const T& key) const

    Get the specified (``key``) property of a JavaScript object.


  .. cpp:function:: void set(const K& key, const val& v)

    Set the specified (``key``) property of a JavaScript object (accessed through a ``val``) with the value ``v``.


  .. cpp:function:: val operator()(Args&&... args) const

    Assumes that current value is a function, and invokes it with provided arguments.


  .. cpp:function:: ReturnValue call(const char* name, Args&&... args) const

    Invokes the specified method (``name``) on the current object with provided arguments.


  .. cpp:function:: T as() const

    Converts current value to the specified C++ type.


  .. cpp:function:: val typeof() const

    Returns the result of a JavaScript ``typeof`` operator invoked on the current value.


  .. cpp:function:: std::vector<T> vecFromJSArray(const val& v)

    Copies a JavaScript array into a ``std::vector<T>``, converting each element via ``.as<T>()``.
    For a more efficient but unsafe version working with numbers, see ``convertJSArrayToNumberVector``.

    :param val v: The JavaScript array to be copied
    :returns: A ``std::vector<T>`` made from the javascript array

  .. cpp:function:: std::vector<T> convertJSArrayToNumberVector(const val& v)

    Converts a JavaScript array into a ``std::vector<T>`` efficiently, as if using the javascript `Number()` function on each element.
    This is way more efficient than ``vecFromJSArray`` on any array with more than 2 values, but is not suitable for arrays of non-numeric values.
    No type checking is done, so any invalid array entry will silently be replaced by a NaN value (or 0 for integer types).

    :param val v: The JavaScript (typed) array to be copied
    :returns: A std::vector<T> made from the javascript array


  .. cpp:function:: val await() const

    Pauses the C++ to ``await`` the ``Promise`` / thenable.

    :returns: The fulfilled value.

      .. note:: This method requires :ref:`ASYNCIFY` to be enabled.

  .. cpp:function:: val operator co_await() const

   The ``co_await`` operator allows awaiting JavaScript promises represented by ``val``.

   It's compatible with any C++20 coroutines, but should be normally used inside
   a ``val``-returning coroutine which will also become a ``Promise``.

   For example, it allows you to implement the equivalent of this JavaScript ``async``/``await`` function:

    .. code:: javascript

      async function foo() {
        const response = await fetch("http://url");
        const json = await response.json();
        return json;
      }

      export { foo };

    as a C++ coroutine:

    .. code:: cpp

      val foo() {
        val response = co_await val::global("fetch")(std::string("http://url"));
        val json = co_await response.call<val>("json");
        return json;
      }

      EMSCRIPTEN_BINDINGS(module) {
        function("foo", &foo);
      }

   Unlike the ``await()`` method, it doesn't need Asyncify as it uses native C++ coroutine transform.

   :returns: A ``val`` representing the fulfilled value of this promise.

.. cpp:type: EMSCRIPTEN_SYMBOL(name)

  **HamishW**-Replace with description.
