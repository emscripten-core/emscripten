.. _bind-h:

================================
bind.h (under-construction)
================================

The C++ APIs in `bind.h <https://github.com/kripken/emscripten/blob/master/system/include/emscripten/bind.h>`_ define (**HamishW**-Replace with description.)

Guide documentation for this API can be found in :ref:`Embind`.

.. contents:: Table of Contents
    :local:
    :depth: 1

How to use this API
===================


Defines
-------


.. cpp:namespace: emscripten

.. cpp:function:: EMSCRIPTEN_BINDINGS(name)

   **HamishW** Confirm this is correct.

   This define is used to bind C++ classes, functions and other constructs to
   JavaScript. It is used differently depending on the construct being mapped
   — see the :ref:`embind guide <embind>` for examples.

   :param name: This is a label to mark a group of related bindings (for example ``EMSCRIPTEN_BINDINGS(physics)``, ``EMSCRIPTEN_BINDINGS(components)``, etc.)



.. cpp:type:: sharing_policy

   **HamishW**-Replace with description. Note this is a strongly typed enum.
   I can't see better way in Sphinx to represent it.

   .. cpp:type:: sharing_policy::NONE

      **HamishW**-Replace with description.

   .. cpp:type:: sharing_policy::INTRUSIVE

      **HamishW**-Replace with description.

   .. cpp:type:: sharing_policy::BY_EMVAL

      **HamishW**-Replace with description.


.. _bind-h-policies:

Policies
========

Currently only :cpp:type:`allow_raw_pointers` policy is supported.
Eventually we hope to implement `Boost.Python-like raw pointer policies
<https://wiki.python.org/moin/boost.python/CallPolicy>`_ for managing
object ownership.

.. cpp:type:: arg

   .. cpp:member:: static int index

      .. code-block:: cpp

         // Prototype
         static constexpr int index

      **HamishW** Add description.


.. cpp:type:: ret_val

   .. cpp:member:: static int index

      .. code-block:: cpp

         // Prototype
         static constexpr int index

      **HamishW** Add description.



.. cpp:type:: allow_raw_pointers

   This policy is used to whitelist raw pointers.

   .. cpp:type:: Transform::type

      **HamishW** Add description.


.. cpp:type:: allow_raw_pointer

   .. code-block: cpp

      // Prototype
      template<typename Slot>
      struct allow_raw_pointer : public allow_raw_pointers

   **HamishW** Add description. Note from source: "This type is temporary,
   it will be changed when arg policies are reworked"


select_overload and select_const
======================================

**HamishW** Nicer title ?


.. cpp:function:: typename std::add_pointer<Signature>::type select_overload(typename std::add_pointer<Signature>::type fn)

   .. code-block:: cpp

      // Prototype
      template<typename Signature>
      typename std::add_pointer<Signature>::type select_overload(typename std::add_pointer<Signature>::type fn)

   **HamishW** Add description.

   :param typename std\:\:add_pointer<Signature>::type fn: **HamishW** Add description.

   :returns: **HamishW** Add description.


.. cpp:function:: typename internal::MemberFunctionType<ClassType, Signature>::type select_overload()

   .. code-block:: cpp

      // Prototype
      template<typename Signature, typename ClassType>
      typename internal::MemberFunctionType<ClassType, Signature>::type select_overload(Signature (ClassType::*fn))

   **HamishW** Add description.

   :param Signature (ClassType::*fn): **HamishW** Add description.

   :returns: **HamishW** Add description.


.. cpp:function:: auto select_const()

   .. code-block:: cpp

      // Prototype
      template<typename ClassType, typename ReturnType, typename... Args>
      auto select_const(ReturnType (ClassType::*method)(Args...) const)

   **HamishW** Add description.

   :param ReturnType (ClassType::*method)(Args...) const: **HamishW** Add description.

   :returns: **HamishW** Add description.


.. cpp:function:: typename internal::CalculateLambdaSignature<LambdaType>::type optional_override(const LambdaType& fp)

   .. code-block:: cpp

      // Prototype
      template<typename LambdaType>
      typename internal::CalculateLambdaSignature<LambdaType>::type optional_override(const LambdaType& fp)

   **HamishW** Add description.

   :param const LambdaType& fp: **HamishW** Add description.

   :returns: **HamishW** Add description.



Functions
=============

.. cpp:function:: void* __getDynamicPointerType(void* p)

   **HamishW** Add description.

   :param void* p: **HamishW** Add description.
   :returns: **HamishW** Add description.


.. cpp:function:: void* __getDynamicPointerType(void* p)

   **HamishW** Add description.

   :param void* p: **HamishW** Add description.
   :returns: **HamishW** Add description.


.. cpp:function:: void function()

   .. code-block:: cpp

      //prototype
      template<typename ReturnType, typename... Args, typename... Policies>
      void function(const char* name, ReturnType (*fn)(Args...), Policies...)

   Registers a function to export to JavaScript. This is called from within
   an :cpp:func:`EMSCRIPTEN_BINDINGS` block.

   For example to export the function ``lerp()``

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



   **HamishW** Check description. Note that Sphinx could not cope with the
   prototype, so have moved it into the body above.

   :param const char* name: The name of the function to export (e.g. ``"lerp"``)  **HamishW** Check description.
   :param ReturnType (\*fn)(Args...): Function pointer address for the exported function (e.g. ``&lerp``).
   :param Policies...: |policies-argument|



Value tuples
==============

.. cpp:class:: value_array : public internal::noncopyable

   **HamishW** Add description.

   .. cpp:type:: class_type

      A typedef of ``ClassType``, the typename of the templated type for the class.


   .. cpp:function:: value_array(const char* name)

      Constructor. **HamishW** Add description.

      :param const char* name: **HamishW** Add description.


   .. cpp:function:: ~value_array()

      Destructor. **HamishW** Add description.


   .. cpp:function:: value_array& element(ElementType InstanceType::*field)

      **HamishW** Add description.

      :param ElementType InstanceType::*field: **HamishW** Add description. Note that ``ElementType`` and ``InstanceType`` are typenames (templated types).
      :returns: **HamishW** Add description.


   .. cpp:function:: value_array& element(Getter getter, Setter setter)

      **HamishW** Add description.

      :param Getter getter: **HamishW** Add description. Note that ``Getter`` is a typename (templated type).
      :param Setter setter: **HamishW** Add description. Note that ``Setter`` is a typename (templated type).
      :returns: **HamishW** Add description.


   .. cpp:function:: value_array& element(index<Index>)

      **HamishW** Add description.

      :param index<Index>: **HamishW** Add description. Note that ``Index`` is an integer template parameter.
      :returns: **HamishW** Add description.





Value structs
======================================

.. cpp:class:: value_object : public internal::noncopyable

   **HamishW** Add description.

   .. cpp:type:: class_type

      A typedef of ``ClassType``, the typename of the templated type for the class.


   .. cpp:function:: value_object(const char* name)

      Constructor. **HamishW** Add description.

      :param const char* name: **HamishW** Add description.


   .. cpp:function:: ~value_object()

      Destructor. **HamishW** Add description.


   .. cpp:function:: value_object& field(const char* fieldName, FieldType InstanceType::*field)

      **HamishW** Add description.

      :param const char* fieldName: **HamishW** Add description.
      :param FieldType InstanceType::*field: **HamishW** Add description.

      :returns: **HamishW** Add description.



   .. cpp:function:: value_object& field(const char* fieldName, Getter getter, Setter setter)

      **HamishW** Add description.

      :param const char* fieldName: **HamishW** Add description.
      :param Getter getter: **HamishW** Add description. Note that ``Getter`` is a typename (templated type).
      :param Setter setter: **HamishW** Add description. Note that ``Setter`` is a typename (templated type).
      :returns: **HamishW** Add description.


   .. cpp:function:: value_object& field(const char* fieldName, index<Index>)

      **HamishW** Add description.

      :param const char* fieldName: **HamishW** Add description.
      :param index<Index>: **HamishW** Add description. Note that ``Index`` is an integer template parameter.
      :returns: **HamishW** Add description.





Smart pointers
======================================


.. cpp:type:: default_smart_ptr_trait

   .. code-block:: cpp

      //prototype
      template<typename PointerType>
      struct default_smart_ptr_trait

   **HamishW** Add description.

   .. cpp:function:: static sharing_policy get_sharing_policy()

      **HamishW** Add description.

      :returns: **HamishW** Add description.


   .. cpp:function:: static void* share(void* v)

      **HamishW** Add description.

      :param void* v: **HamishW** Add description.
      :returns: **HamishW** Add description.


   .. cpp:function:: static PointerType* construct_null()

      **HamishW** Add description.

      :returns: **HamishW** Add description. Note that the ``PointerType`` returned is a typename (templated type).



.. cpp:type:: smart_ptr_trait

   .. code-block:: cpp

      //prototype
      template<typename PointerType>
      struct smart_ptr_trait : public default_smart_ptr_trait<PointerType>

   **HamishW** Add description. Note from source is: // specialize if you have a different pointer type

   .. cpp:type:: PointerType::element_type element_type

      .. code-block:: cpp

         //prototype
         typedef typename PointerType::element_type element_type;


      **HamishW** Add description. A typedef for the PointerType::element_type, where ``PointerType`` is a typename (templated type).


   .. cpp:function:: static element_type* get(const PointerType& ptr)

      **HamishW** Add description.

      :param const PointerType& ptr: **HamishW** Add description. Note that ``PointerType`` is a typename (templated type)
      :returns: **HamishW** Add description.



.. cpp:type:: smart_ptr_trait<std::shared_ptr<PointeeType>>

   .. code-block:: cpp

      //prototype
      template<typename PointeeType>
      struct smart_ptr_trait<std::shared_ptr<PointeeType>>

   **HamishW** Add description.

   .. cpp:type:: PointerType

      **HamishW** Add description. A typedef to std::shared_ptr<PointeeType>, where ``PointeeType`` is a typename (templated type).

   .. cpp:type:: element_type

      **HamishW** Add description. A typedef for the ``PointerType::element_type``.


   .. cpp:function:: static element_type* get(const PointerType& ptr)

      **HamishW** Add description.

      :param const PointerType& ptr: **HamishW** Add description.
      :returns: **HamishW** Add description.

   .. cpp:function:: static sharing_policy get_sharing_policy()

      **HamishW** Add description.

      :returns: **HamishW** Add description.


   .. cpp:function:: static std::shared_ptr<PointeeType>* share(PointeeType* p, internal::EM_VAL v)

      **HamishW** Add description.

      :param PointeeType* p: **HamishW** Add description. Note that ``PointeeType`` is a typename (templated type).
      :param internal::EM_VAL v: **HamishW** Add description.
      :returns: **HamishW** Add description.

   .. cpp:function:: static PointerType* construct_null()

      **HamishW** Add description.

      :returns: **HamishW** Add description.


**HamishW** Note, did not include private class val_deleter. I am assuming all private classes are internal. Delete this Chad when read!


Classes
=======

**HamishW** Add description if needed. Note from source "// abstract classes"


.. cpp:class:: class wrapper : public T, public internal::WrapperBase

   .. code-block:: cpp

      //prototype
      template<typename T>
      class wrapper : public T, public internal::WrapperBase

   **HamishW** Add description.

   .. cpp:type:: class_type

      **HamishW** Add description. A typedef of ``T``, the typename of the templated type for the class.


   .. cpp:function:: wrapper(val&& wrapped, Args&&... args)

      .. code-block:: cpp

         //prototype
         template<typename... Args>
         explicit wrapper(val&& wrapped, Args&&... args)
           : T(std::forward<Args>(args)...)
           , wrapped(std::forward<val>(wrapped))

      Constructor. **HamishW** Add description.

      :param val&& wrapped: **HamishW** Add description.
      :param Args&&... args: **HamishW** Add description. Note that ``Args`` is a typename (templated type).
      :returns: **HamishW** Add description.


   .. cpp:function:: ~wrapper()

      Destructor. **HamishW** Add description.


   .. cpp:function:: ReturnType call(const char* name, Args&&... args) const

      Constructor. **HamishW** Add description.

      :param const char* name: **HamishW** Add description.
      :param Args&&... args: **HamishW** Add description. Note that ``Args`` is a typename (templated type).
      :returns: **HamishW** Add description. Note that ``ReturnType`` is a typename (templated type).


.. cpp:function:: EMSCRIPTEN_WRAPPER(T)

   **HamishW** Add description. Note that this is actually a define, but I've implemented it as a function, because that is how it behaves, and it allows me to have the T as shown, which isn't possible on Sphinx type declaration.

   :param T: **HamishW** Add description.


.. cpp:type:: base

   **HamishW** Add description.

   .. cpp:type:: class_type

      **HamishW** Add description. A typedef of ``BaseClass``, the typename of the templated type for the class.


   .. cpp:function:: static void verify()

      **HamishW** Add description. Note, is templated function which takes typename ``ClassType``.


   .. cpp:function:: static internal::TYPEID get()

      **HamishW** Add description.

      :returns: **HamishW** Add description.



   .. cpp:function:: HAMISHW_ HELP_Needed

      **HamishW** I don't understand this C++, so not sure how to document. Putting code here for Chad to advise on how to document

      .. code-block:: cpp

         template<typename ClassType>
         using Upcaster = BaseClass* (*)(ClassType*);

         template<typename ClassType>
         using Downcaster = ClassType* (*)(BaseClass*);


   .. cpp:function:: static Upcaster<ClassType> getUpcaster()

      .. code-block:: cpp

         //prototype
         template<typename ClassType>
         static Upcaster<ClassType> getUpcaster()

      **HamishW** Add description.

      :returns: **HamishW** Add description.


   .. cpp:function:: static Downcaster<ClassType> getDowncaster()

      .. code-block:: cpp

         //prototype
         template<typename ClassType>
         static Downcaster<ClassType> getDowncaster()

      **HamishW** Add description.

      :returns: **HamishW** Add description.


   .. cpp:function:: static To* convertPointer(From* ptr)

      .. code-block:: cpp

         //prototype
         template<typename From, typename To>
         static To* convertPointer(From* ptr)

      **HamishW** Add description.

      :param From* ptr: **HamishW** Add description.
      :returns: **HamishW** Add description.



.. cpp:type:: pure_virtual

   **HamishW** Add description.

   .. cpp:type:: Transform

      **HamishW** Add description. Note that this is a templated struct taking typename parameter ``InputType`` and integer ``Index``.

      .. cpp:type:: type

         **HamishW** Add description. This is a typdef to the parent struct typename parameter ``InputType``.


.. cpp:type:: constructor

   **HamishW** Add description. Note that this is a template struct taking typename ``... ConstructorArgs``.



.. cpp:class:: class_

   **HamishW** Add description. Note that this is a templated class with typename parameters ``ClassType`` and ``BaseSpecifier``.

   .. cpp:type:: class_type

      **HamishW** Add description. A typedef of ``ClassType`` (a typename for the class).


   .. cpp:type:: base_specifier

      **HamishW** Add description. A typedef of ``BaseSpecifier`` (a typename for the class).


   .. cpp:type:: HELPNEEDEDHERE

      **HamishW** Don't know what to do with this: ::

         class_() = delete;


   .. cpp:function:: explicit class_(const char* name)

      .. code-block:: cpp

         //prototype
         EMSCRIPTEN_ALWAYS_INLINE explicit class_(const char* name)


      Constructor. **HamishW** Add description.

      :param const char* name: **HamishW** Add description.
      :returns: **HamishW** Add description.


   .. cpp:function:: const class_& smart_ptr(const char* name) const

      .. code-block:: cpp

         //prototype
         template<typename PointerType>
         EMSCRIPTEN_ALWAYS_INLINE const class_& smart_ptr(const char* name) const

      **HamishW** Add description.

      :param const char* name: **HamishW** Add description.
      :returns: |class_-function-returns|


      .. _embind-class-zero-argument-constructor:

   .. cpp:function:: const class_& constructor() const

      .. code-block:: cpp

         //prototype
         template<typename... ConstructorArgs, typename... Policies>
         EMSCRIPTEN_ALWAYS_INLINE const class_& constructor(Policies... policies) const

      Zero-argument form of the class constructor. This invokes the natural constructor with the arguments specified in the template. See :ref:`embind-external-constructors` for more information.

      **HamishW** Check description. Note that prototype moved into block as was breaking Sphinx.

      :param Policies... policies: |policies-argument|
      :returns: |class_-function-returns|

      .. _embind-class-function-pointer-constructor:


   .. cpp:function:: const class_& constructor() const

      .. code-block:: cpp

         //prototype
         template<typename... Args, typename ReturnType, typename... Policies>
         EMSCRIPTEN_ALWAYS_INLINE const class_& constructor(ReturnType (*factory)(Args...), Policies...) const

      Class constructor for objects that use a factory function to create the object. See :ref:`embind-external-constructors` for more information.

      :param ReturnType (\*factory)(Args...): The address of the class factory function.
      :param Policies... policies: |policies-argument|
      :returns: |class_-function-returns|


   .. cpp:function:: const class_& smart_ptr_constructor() const

      .. code-block:: cpp

         //prototype
         template<typename SmartPtr, typename... Args, typename... Policies>
         EMSCRIPTEN_ALWAYS_INLINE const class_& smart_ptr_constructor(const char* smartPtrName, SmartPtr (*factory)(Args...), Policies...) const

      **HamishW** Add description. Note that Sphinx could NOT cope with the prototype, so have pulled it into the body of the text.

      :param const char* smartPtrName: **HamishW** Add description.
      :param SmartPtr (\*factory)(Args...): **HamishW** Add description.
      :param Policies... policies: |policies-argument|
      :returns: |class_-function-returns|


   .. cpp:function:: const class_& allow_subclass() const

      .. code-block:: cpp

         //prototype
          template<typename WrapperType, typename PointerType, typename... ConstructorArgs>
         EMSCRIPTEN_ALWAYS_INLINE const class_& allow_subclass(
           const char* wrapperClassName,
           const char* pointerName,
           ::emscripten::constructor<ConstructorArgs...> = ::emscripten::constructor<>()
         ) const

      **HamishW** Add description.

      :param const char* wrapperClassName: **HamishW** Add description.
      :param const char* pointerName: **HamishW** Add description.
      :param ::emscripten::constructor<ConstructorArgs...> constructor): **HamishW** Add description.
      :returns: |class_-function-returns|


   .. cpp:function:: const class_& allow_subclass() const

      .. code-block:: cpp

         //prototype
         template<typename WrapperType, typename... ConstructorArgs>
         EMSCRIPTEN_ALWAYS_INLINE const class_& allow_subclass(
           const char* wrapperClassName,
           ::emscripten::constructor<ConstructorArgs...> constructor = ::emscripten::constructor<>()
         ) const

      **HamishW** Add description. Explain how this constructor differs from other one.

      :param const char* wrapperClassName: **HamishW** Add description.
      :param ::emscripten::constructor<ConstructorArgs...> constructor): **HamishW** Add description.

      :returns: |class_-function-returns|


   .. cpp:function:: const class_& function() const

      .. code-block:: cpp

         //prototype
         template<typename ReturnType, typename... Args, typename... Policies>
         EMSCRIPTEN_ALWAYS_INLINE const class_& function(const char* methodName, ReturnType (ClassType::*memberFunction)(Args...), Policies...) const

      This method is for declaring a method belonging to a class.

      On the JavaScript side this is a function that gets bound as a property of the prototype. For example ``.function("myClassMember", &MyClass::myClassMember)`` would bind ``myClassMember`` to ``MyClass.prototype.myClassMember`` in the JavaScript.

      **HamishW** Check description. Note prototype moved to "prototype" block above because syntax broke Sphinx. Also explain how this method differs from the other overloads.

      :param const char* methodName: **HamishW** Add description.
      :param ReturnType (ClassType::*memberFunction)(Args...): **HamishW** Add description. Note that ``ReturnType`` is a template typename for this function and ``ClassType`` is a template typename for the class.
      :param typename... Policies: |policies-argument|
      :returns: |class_-function-returns|


   .. cpp:function:: const class_& function() const

      .. code-block:: cpp

         //prototype
         template<typename ReturnType, typename... Args, typename... Policies>
         EMSCRIPTEN_ALWAYS_INLINE const class_& function(const char* methodName, ReturnType (ClassType::*memberFunction)(Args...) const, Policies...) const

      **HamishW** Add description. Note, prototype moved into block above as it broke Sphinx. Also this only differs by a const on the ReturnType from the previous function

      :param const char* methodName: **HamishW** Add description.
      :param ReturnType (ClassType::*memberFunction)(Args...) const: **HamishW** Add description. Note that ``ReturnType`` is a template typename for this function and ``ClassType`` is a template typename for the class.
      :param typename... Policies: |policies-argument|
      :returns: |class_-function-returns|


   .. cpp:function:: const class_& function() const

      .. code-block:: cpp

         //prototype
         template<typename ReturnType, typename ThisType, typename... Args, typename... Policies>
         EMSCRIPTEN_ALWAYS_INLINE const class_& function(const char* methodName, ReturnType (*function)(ThisType, Args...), Policies...) const

      **HamishW** Add description. Note, prototype moved into block above as it broke Sphinx.

      :param const char* methodName: **HamishW** Add description.
      :param ReturnType (\*function)(ThisType, Args...): **HamishW** Add description.
      :param typename... Policies: |policies-argument|
      :returns: |class_-function-returns|


   .. cpp:function:: const class_& property() const

      .. code-block:: cpp

         //prototype
         template<typename FieldType, typename = typename std::enable_if<!std::is_function<FieldType>::value>::type>
         EMSCRIPTEN_ALWAYS_INLINE const class_& property(const char* fieldName, const FieldType ClassType::*field) const

      **HamishW** Add description. Note, signature copied to prototype block above because proper signature broke Sphinx. Also because it is useful to include the template information.

      :param const char* fieldName: **HamishW** Add description.
      :param const FieldType ClassType::*field: **HamishW** Add description.

      :returns: |class_-function-returns|


   .. cpp:function:: const class_& property(const char* fieldName, FieldType ClassType::*field) const

      .. code-block:: cpp

         //prototype
         template<typename FieldType, typename = typename std::enable_if<!std::is_function<FieldType>::value>::type>
         EMSCRIPTEN_ALWAYS_INLINE const class_& property(const char* fieldName, FieldType ClassType::*field) const

      **HamishW** Add description.

      :param const char* fieldName: **HamishW** Add description.
      :param FieldType ClassType::*field: **HamishW** Add description.

      :returns: |class_-function-returns|


   .. cpp:function:: const class_& property(const char* fieldName, Getter getter) const

      .. code-block:: cpp

         //prototype
         template<typename Getter>
         EMSCRIPTEN_ALWAYS_INLINE const class_& property(const char* fieldName, Getter getter) const

      **HamishW** Add description.

      :param const char* fieldName: **HamishW** Add description.
      :param Getter getter: **HamishW** Add description. Note that ``Getter`` is a function template typename.
      :returns: |class_-function-returns|


   .. cpp:function:: const class_& property(const char* fieldName, Getter getter, Setter setter) const

      .. code-block:: cpp

         //prototype
         template<typename Getter, typename Setter>
         EMSCRIPTEN_ALWAYS_INLINE const class_& property(const char* fieldName, Getter getter, Setter setter) const

      **HamishW** Add description. Note that this is a function template taking typenames ``Setter`` and ``Getter``: ``template<typename Getter, typename Setter>``

      :param const char* fieldName: **HamishW** Add description.
      :param Getter getter: **HamishW** Add description. Note that ``Getter`` is a function template typename.
      :param Setter setter: **HamishW** Add description. Note that ``Setter`` is a function template typename.
      :returns: |class_-function-returns|

   .. cpp:function:: const class_& class_function() const

      .. code-block:: cpp

         //prototype
         template<typename ReturnType, typename... Args, typename... Policies>
         EMSCRIPTEN_ALWAYS_INLINE const class_& class_function(const char* methodName, ReturnType (*classMethod)(Args...), Policies...) const

      This method is for declaring a static function belonging to a class.

      On the JavaScript side this is a function that gets bound as a property
      of the constructor. For example ``.class_function("myStaticFunction",
      &MyClass::myStaticFunction)`` binds ``myStaticFunction`` to
      ``MyClass.myStaticFunction``.

      **HamishW** Check description. Note prototype moved to "prototype" block above because syntax broke Sphinx.

      :param const char* methodName: **HamishW** Add description.
      :param ReturnType (\*classMethod)(Args...): **HamishW** Add description.
      :param Policies...: |policies-argument|
      :returns: |class_-function-returns|

   .. cpp:function:: const class_& class_property(const char* fieldName, FieldType *field) const

      .. code-block:: cpp

         //prototype
         template<typename FieldType>
         EMSCRIPTEN_ALWAYS_INLINE const class_& property(const char* fieldName, FieldType *field) const

      **HamishW** Add description.

      :param const char* fieldName: **HamishW** Add description.
      :param FieldType ClassType::*field: **HamishW** Add description.

      :returns: |class_-function-returns|





Vectors
=======

.. cpp:function:: class_<std::vector<T>> register_vector(const char* name)

   .. code-block:: cpp

      //prototype
      template<typename T>
      class_<std::vector<T>> register_vector(const char* name)

   **HamishW** Check description.

   A function to register a ``std::vector<T>``.

   :param const char* name: **HamishW** Add description.
   :returns: **HamishW** Add description.


Maps
====

.. cpp:function::  class_<std::map<K, V>> register_map(const char* name)

   .. code-block:: cpp

      //prototype
      template<typename K, typename V>
      class_<std::map<K, V>> register_map(const char* name)

   **HamishW** Check description.

   A function to register a ``std::map<K, V>``.

   :param const char* name: **HamishW** Add description.
   :returns: **HamishW** Add description.



Enums
=====


.. cpp:class:: enum_

   .. code-block:: cpp

      //prototype
      template<typename EnumType>
      class enum_

   Registers an enum to export to JavaScript. This is called from within an
   :cpp:func:`EMSCRIPTEN_BINDINGS` block and works with both C++98 enums
   and C++11 "enum classes". See :ref:`embind-enums` for more information.


   .. cpp:type:: enum_type

      **HamishW** Add description. A typedef of ``EnumType`` (a typename for the class).


   .. cpp:function::  enum_(const char* name)

      Constructor. **HamishW** Add description.

      :param const char* name: **HamishW** Add description.
      :returns: **HamishW** Add description.


   .. cpp:function::  enum_& value(const char* name, EnumType value)

      Registers an enum value. **HamishW** Check description.

      :param const char* name: The name of the enumerated value.
      :param EnumType value: The type of the enumerated value.
      :returns: A reference to the current object. This allows chaining of multiple enum values in the :cpp:func:`EMSCRIPTEN_BINDINGS` block.



Constants
=========

.. cpp:function:: void constant(const char* name, const ConstantType& v)

   .. code-block:: cpp

      //prototype
      template<typename ConstantType>
      void constant(const char* name, const ConstantType& v)

   **HamishW** Check description.

   Registers a constant to export to JavaScript. This is called from within
   an :cpp:func:`EMSCRIPTEN_BINDINGS` block.

   .. code:: cpp

      EMSCRIPTEN_BINDINGS(my_constant_example) {
        constant("SOME_CONSTANT", SOME_CONSTANT);
      }

   :param const char* name: The name of the constant.
   :param const ConstantType& v: The constant type. This can be any type known to *embind*.





.. COMMENT (not rendered): Following values are common to many functions, and currently only updated in one place (here).
.. COMMENT (not rendered): These can be properly replaced if required either wholesale or on an individual basis.

.. |policies-argument| replace:: :ref:`Policy <bind-h-policies>` for managing raw pointer object ownership. Currently must be :cpp:type:`allow_raw_pointers`.
.. |class_-function-returns| replace:: A ``const`` reference to the current object. This allows chaining of the :cpp:class:`class_` functions that define the binding in the :cpp:func:`EMSCRIPTEN_BINDINGS` block.
