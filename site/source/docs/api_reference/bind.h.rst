.. _bind-h:

===========================
bind.h (under-construction)
===========================

The C++ APIs in `bind.h <https://github.com/emscripten-core/emscripten/blob/main/system/include/emscripten/bind.h>`_ define

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

   This define is used to bind C++ classes, functions and other constructs to
   JavaScript. It is used differently depending on the construct being mapped
   — see the :ref:`embind guide <embind>` for examples.

   :param name: This is a label to mark a group of related bindings (for example ``EMSCRIPTEN_BINDINGS(physics)``, ``EMSCRIPTEN_BINDINGS(components)``, etc.)



.. cpp:type:: sharing_policy

   .. cpp:type:: sharing_policy::NONE

   .. cpp:type:: sharing_policy::INTRUSIVE

   .. cpp:type:: sharing_policy::BY_EMVAL


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


.. cpp:type:: ret_val

   .. cpp:member:: static int index

      .. code-block:: cpp

         // Prototype
         static constexpr int index


.. cpp:type:: allow_raw_pointers

   This policy is used to allow raw pointers.

   .. cpp:type:: Transform::type


.. cpp:type:: allow_raw_pointer

   .. code-block: cpp

      // Prototype
      template<typename Slot>
      struct allow_raw_pointer : public allow_raw_pointers


select_overload and select_const
======================================


.. cpp:function:: typename std::add_pointer<Signature>::type select_overload(typename std::add_pointer<Signature>::type fn)

   .. code-block:: cpp

      // Prototype
      template<typename Signature>
      typename std::add_pointer<Signature>::type select_overload(typename std::add_pointer<Signature>::type fn)

   :param typename std\:\:add_pointer<Signature>::type fn:

.. cpp:function:: typename internal::MemberFunctionType<ClassType, Signature>::type select_overload()

   .. code-block:: cpp

      // Prototype
      template<typename Signature, typename ClassType>
      typename internal::MemberFunctionType<ClassType, Signature>::type select_overload(Signature (ClassType::*fn))

   :param Signature (ClassType::\*fn):

.. cpp:function:: auto select_const()

   .. code-block:: cpp

      // Prototype
      template<typename ClassType, typename ReturnType, typename... Args>
      auto select_const(ReturnType (ClassType::*method)(Args...) const)

   :param ReturnType (ClassType::\*method)(Args...) const:


.. cpp:function:: typename internal::CalculateLambdaSignature<LambdaType>::type optional_override(const LambdaType& fp)

   .. code-block:: cpp

      // Prototype
      template<typename LambdaType>
      typename internal::CalculateLambdaSignature<LambdaType>::type optional_override(const LambdaType& fp)

   :param const LambdaType& fp:


Functions
=========

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



   :param const char* name: The name of the function to export (e.g. ``"lerp"``).
   :param ReturnType (\*fn)(Args...): Function pointer address for the exported function (e.g. ``&lerp``).
   :param Policies...: |policies-argument|



Value tuples
==============

.. cpp:class:: value_array : public internal::noncopyable

   .. cpp:type:: class_type

      A typedef of ``ClassType``, the typename of the templated type for the class.


   .. cpp:function:: value_array(const char* name)

      Constructor.

      :param const char* name:


   .. cpp:function:: ~value_array()

      Destructor.


   .. cpp:function:: value_array& element(ElementType InstanceType::*field)

      :param ElementType InstanceType::\*field: Note that ``ElementType`` and ``InstanceType`` are typenames (templated types).


   .. cpp:function:: value_array& element(Getter getter, Setter setter)

      :param Getter getter: Note that ``Getter`` is a typename (templated type).
      :param Setter setter: Note that ``Setter`` is a typename (templated type).


   .. cpp:function:: value_array& element(index<Index>)

      :param index<Index>: Note that ``Index`` is an integer template parameter.





Value structs
=============

.. cpp:class:: value_object : public internal::noncopyable

   .. cpp:type:: class_type

      A typedef of ``ClassType``, the typename of the templated type for the class.


   .. cpp:function:: value_object(const char* name)

      Constructor.

      :param const char* name:


   .. cpp:function:: ~value_object()

      Destructor.


   .. cpp:function:: value_object& field(const char* fieldName, FieldType InstanceType::*field)

      :param const char* fieldName:
      :param FieldType InstanceType\:\:\*field:



   .. cpp:function:: value_object& field(const char* fieldName, Getter getter, Setter setter)

      :param const char* fieldName:
      :param Getter getter: Note that ``Getter`` is a typename (templated type).
      :param Setter setter: Note that ``Setter`` is a typename (templated type).


   .. cpp:function:: value_object& field(const char* fieldName, index<Index>)

      :param const char* fieldName:
      :param index<Index>: Note that ``Index`` is an integer template parameter.


Smart pointers
==============


.. cpp:type:: default_smart_ptr_trait

   .. code-block:: cpp

      //prototype
      template<typename PointerType>
      struct default_smart_ptr_trait

   .. cpp:function:: static sharing_policy get_sharing_policy()


   .. cpp:function:: static void* share(void* v)

      :param void* v:


   .. cpp:function:: static PointerType* construct_null()

      :returns: Note that the ``PointerType`` returned is a typename (templated type).



.. cpp:type:: smart_ptr_trait

   .. code-block:: cpp

      //prototype
      template<typename PointerType>
      struct smart_ptr_trait : public default_smart_ptr_trait<PointerType>

   .. cpp:type:: PointerType::element_type element_type

      .. code-block:: cpp

         //prototype
         typedef typename PointerType::element_type element_type;


      A typedef for the PointerType::element_type, where ``PointerType`` is a typename (templated type).


   .. cpp:function:: static element_type* get(const PointerType& ptr)

      :param const PointerType& ptr: Note that ``PointerType`` is a typename (templated type)



.. cpp:type:: template<typename PointeeType> smart_ptr_trait<std::shared_ptr<PointeeType>>

   .. code-block:: cpp

      //prototype
      template<typename PointeeType>
      struct smart_ptr_trait<std::shared_ptr<PointeeType>>

   .. cpp:type:: PointerType

      A typedef to std::shared_ptr<PointeeType>, where ``PointeeType`` is a typename (templated type).

   .. cpp:type:: element_type

      A typedef for the ``PointerType::element_type``.


   .. cpp:function:: static element_type* get(const PointerType& ptr)

      :param const PointerType& ptr:

   .. cpp:function:: static sharing_policy get_sharing_policy()


   .. cpp:function:: static std::shared_ptr<PointeeType>* share(PointeeType* p, EM_VAL v)

      :param PointeeType* p: Note that ``PointeeType`` is a typename (templated type).
      :param EM_VAL v:


   .. cpp:function:: static PointerType* construct_null()



Classes
=======


.. cpp:class:: wrapper : public T, public internal::WrapperBase

   .. code-block:: cpp

      //prototype
      template<typename T>
      class wrapper : public T, public internal::WrapperBase

   .. cpp:type:: class_type

      A typedef of ``T``, the typename of the templated type for the class.


   .. cpp:function:: wrapper(val&& wrapped, Args&&... args)

      .. code-block:: cpp

         //prototype
         template<typename... Args>
         explicit wrapper(val&& wrapped, Args&&... args)
           : T(std::forward<Args>(args)...)
           , wrapped(std::forward<val>(wrapped))

      Constructor.

      :param val&& wrapped:
      :param Args&&... args: Note that ``Args`` is a typename (templated type).


   .. cpp:function:: ~wrapper()

      Destructor.


   .. cpp:function:: ReturnType call(const char* name, Args&&... args) const

      Constructor.

      :param const char* name:
      :param Args&&... args: Note that ``Args`` is a typename (templated type).
      :returns: Note that ``ReturnType`` is a typename (templated type).


.. cpp:function:: EMSCRIPTEN_WRAPPER(T)

   :param T:


.. cpp:type:: base

   .. cpp:type:: class_type

      A typedef of ``BaseClass``, the typename of the templated type for the class.


   .. cpp:function:: static void verify()

      Note, is templated function which takes typename ``ClassType``.


   .. cpp:function:: static internal::TYPEID get()


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


   .. cpp:function:: static Downcaster<ClassType> getDowncaster()

      .. code-block:: cpp

         //prototype
         template<typename ClassType>
         static Downcaster<ClassType> getDowncaster()


   .. cpp:function:: static To* convertPointer(From* ptr)

      .. code-block:: cpp

         //prototype
         template<typename From, typename To>
         static To* convertPointer(From* ptr)

      :param From* ptr:



.. cpp:type:: pure_virtual

   .. cpp:type:: Transform

      Note that this is a templated struct taking typename parameter ``InputType`` and integer ``Index``.

      .. cpp:type:: type

         This is a typedef to the parent struct typename parameter ``InputType``.


.. cpp:type:: constructor

   Note that this is a template struct taking typename ``... ConstructorArgs``.



.. cpp:class:: class_

   Note that this is a templated class with typename parameters ``ClassType`` and ``BaseSpecifier``.

   .. cpp:type:: class_type

      A typedef of ``ClassType`` (a typename for the class).


   .. cpp:type:: base_specifier

      A typedef of ``BaseSpecifier`` (a typename for the class).


   .. cpp:type:: HELPNEEDEDHERE

      ::

         class_() = delete;


   .. cpp:function:: explicit class_(const char* name)

      .. code-block:: cpp

         //prototype
         EMSCRIPTEN_ALWAYS_INLINE explicit class_(const char* name)


      Constructor.

      :param const char* name:


   .. cpp:function:: const class_& smart_ptr(const char* name) const

      .. code-block:: cpp

         //prototype
         template<typename PointerType>
         EMSCRIPTEN_ALWAYS_INLINE const class_& smart_ptr(const char* name) const

      :param const char* name:
      :returns: |class_-function-returns|


      .. _embind-class-zero-argument-constructor:

   .. cpp:function:: const class_& constructor() const

      .. code-block:: cpp

         //prototype
         template<typename... ConstructorArgs, typename... Policies>
         EMSCRIPTEN_ALWAYS_INLINE const class_& constructor(Policies... policies) const

      Zero-argument form of the class constructor. This invokes the natural constructor with the arguments specified in the template. See :ref:`embind-external-constructors` for more information.

      :param Policies... policies: |policies-argument|
      :returns: |class_-function-returns|

      .. _embind-class-function-pointer-constructor:


   .. cpp:function:: const class_& constructor(Callable callable, Policies...) const

      .. code-block:: cpp

         //prototype
         template<typename Signature = internal::DeduceArgumentsTag, typename Callable, typename... Policies>
         EMSCRIPTEN_ALWAYS_INLINE const class_& constructor(Callable callable, Policies...) const

      Class constructor for objects that use a factory function to create the object.  This method will accept either a function pointer, ``std::function``
      object or function object which will return a newly constructed object.  When the ``Callable`` is a function object the function signature must be
      explicitly specified in the ``Signature`` template parameter in the format ``ReturnType (Args...)``.  For ``Callable`` types other than function objects
      the method signature will be deduced.

      The following are all valid calls to ``constructor``:

      .. code-block:: cpp

         using namespace std::placeholders;
         myClass1.constructor(&my_factory);
         myClass2.constructor(std::function<ClassType2(float, float)>(&class2_factory));
         myClass3.constructor<ClassType3(const val&)>(std::bind(Class3Functor(), _1));

      See :ref:`embind-external-constructors` for more information.


      :param Callable callable: Note that ``Callable`` may be either a member function pointer, function pointer, ``std::function`` or function object.
      :param Policies... policies: |policies-argument|
      :returns: |class_-function-returns|


   .. cpp:function:: const class_& smart_ptr_constructor() const

      .. code-block:: cpp

         //prototype
         template<typename SmartPtr, typename... Args, typename... Policies>
         EMSCRIPTEN_ALWAYS_INLINE const class_& smart_ptr_constructor(const char* smartPtrName, SmartPtr (*factory)(Args...), Policies...) const

      :param const char* smartPtrName:
      :param SmartPtr (\*factory)(Args...):
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

      :param const char* wrapperClassName:
      :param const char* pointerName:
      :param emscripten\:\:constructor<ConstructorArgs...> constructor):
      :returns: |class_-function-returns|


   .. cpp:function:: const class_& allow_subclass(const char* wrapperClassName, ::emscripten::constructor<ConstructorArgs...> constructor) const

      .. code-block:: cpp

         //prototype
         template<typename WrapperType, typename... ConstructorArgs>
         EMSCRIPTEN_ALWAYS_INLINE const class_& allow_subclass(
           const char* wrapperClassName,
           ::emscripten::constructor<ConstructorArgs...> constructor = ::emscripten::constructor<>()
         ) const

      :param const char* wrapperClassName:
      :param \:\:emscripten\:\:constructor<ConstructorArgs...> constructor):

      :returns: |class_-function-returns|


   .. cpp:function:: const class_& function() const

      .. code-block:: cpp

         //prototype
         template<typename Signature = internal::DeduceArgumentsTag, typename Callable, typename... Policies>
         EMSCRIPTEN_ALWAYS_INLINE const class_& function(const char* methodName, Callable callable, Policies...) const

      This method is for declaring a method belonging to a class.

      On the JavaScript side this is a function that gets bound as a property of the prototype. For example ``.function("myClassMember", &MyClass::myClassMember)``
      would bind ``myClassMember`` to ``MyClass.prototype.myClassMember`` in the JavaScript.  This method will accept either a pointer-to-member-function, function
      pointer, ``std::function`` object or function object.  When the ``Callable`` is not a pointer-to-member-function it must accept the ``ClassType`` as the first
      (``this``) parameter.  When the ``Callable`` is a function object the function signature must be explicitly specified in the ``Signature`` template parameter
      in the format ``ReturnType (Args...)``.  For ``Callable`` types other than function objects the method signature will be deduced.

      A method name specified in the human-readable well-known symbol format (e.g., ``@@iterator``)
      is bound using the named ``Symbol`` for JavaScript (e.g., ``Symbol.iterator``).

      The following are all valid calls to ``function``:

      .. code-block:: cpp

         using namespace std::placeholders;
         myClass.function("myClassMember", &MyClass::myClassMember)
             .function("myFreeFunction", &my_free_function)
             .function("myStdFunction", std::function<float(ClassType&, float, float)>(&my_function))
             .function<val(const MyClass&)>("myFunctor", std::bind(&my_functor_taking_this, _1));


      :param const char* methodName:
      :param Callable callable: Note that ``Callable`` may be either a member function pointer, function pointer, ``std::function`` or function object.
      :param typename... Policies: |policies-argument|
      :returns: |class_-function-returns|


   .. cpp:function:: const class_& property() const

      .. code-block:: cpp

         //prototype
         template<typename FieldType, typename = typename std::enable_if<!std::is_function<FieldType>::value>::type>
         EMSCRIPTEN_ALWAYS_INLINE const class_& property(const char* fieldName, const FieldType ClassType::*field) const

      :param const char* fieldName:
      :param const FieldType ClassType\:\:\*field:

      :returns: |class_-function-returns|


   .. cpp:function:: const class_& property(const char* fieldName, FieldType ClassType::*field) const

      .. code-block:: cpp

         //prototype
         template<typename FieldType, typename = typename std::enable_if<!std::is_function<FieldType>::value>::type>
         EMSCRIPTEN_ALWAYS_INLINE const class_& property(const char* fieldName, FieldType ClassType::*field) const

      :param const char* fieldName:
      :param FieldType ClassType\:\:\*field:

      :returns: |class_-function-returns|


   .. cpp:function:: const class_& property(const char* fieldName, Getter getter) const

      .. code-block:: cpp

         //prototype
         template<typename PropertyType = internal::DeduceArgumentsTag, typename Getter>
         EMSCRIPTEN_ALWAYS_INLINE const class_& property(const char* fieldName, Getter getter) const;

      Declare a read-only property with the specified ``fieldName`` on the class using the specified ``getter`` to retrieve the property
      value.  ``Getter`` may be either a class method, a function, a ``std::function`` or a function object.  When ``Getter``
      is not pointer-to-member-function, it must accept an instance of the ``ClassType`` as the ``this`` argument.  When
      ``Getter`` is a function object, the property type must be specified as a template parameter as it cannot be deduced,
      e.g.: ``myClass.property<int>("myIntProperty", MyIntGetterFunctor());``

      :param const char* fieldName:
      :param Getter getter: Note that ``Getter`` is a function template typename.
      :returns: |class_-function-returns|


   .. cpp:function:: const class_& property(const char* fieldName, Getter getter, Setter setter) const

      .. code-block:: cpp

         //prototype
         template<typename PropertyType = internal::DeduceArgumentsTag, typename Getter, typename Setter>
         EMSCRIPTEN_ALWAYS_INLINE const class_& property(const char* fieldName, Getter getter, Setter setter) const

      This is a function template taking typenames ``Setter`` and ``Getter``: ``template<typename Getter, typename Setter>``
      which declares a read-write property with the specified ``fieldName`` on the class. ``Getter`` and ``Setter`` may be either a
      class method, a function, a ``std::function`` or a function object.  When ``Getter`` or ``Setter`` is not pointer-to-member-function,
      it must accept an instance of the ``ClassType`` as the ``this`` argument. When ``Getter`` or ``Setter`` is a function object, the
      property type must be specified as a template parameter as it cannot be deduced, e.g.:
      ``myClass.property<int>("myIntProperty", MyIntGetterFunctor(), MyIntSetterFunctor());``

      :param const char* fieldName:
      :param Getter getter: Note that ``Getter`` is a function template typename.
      :param Setter setter: Note that ``Setter`` is a function template typename.
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

      A method name specified in the human-readable well-known symbol format (e.g., ``@@species``)
      is bound using the named ``Symbol`` for JavaScript (e.g., ``Symbol.species``).

      :param const char* methodName:
      :param ReturnType (\*classMethod)(Args...):
      :param Policies...: |policies-argument|
      :returns: |class_-function-returns|

   .. cpp:function:: const class_& class_property(const char* fieldName, FieldType *field) const

      .. code-block:: cpp

         //prototype
         template<typename FieldType>
         EMSCRIPTEN_ALWAYS_INLINE const class_& property(const char* fieldName, FieldType *field) const

      :param const char* fieldName:
      :param FieldType ClassType\:\:\*field:

      :returns: |class_-function-returns|





Vectors
=======

.. cpp:function:: class_<std::vector<T>> register_vector(const char* name)

   .. code-block:: cpp

      //prototype
      template<typename T>
      class_<std::vector<T>> register_vector(const char* name)

   A function to register a ``std::vector<T>``.

   :param const char* name:


Maps
====

.. cpp:function::  class_<std::map<K, V>> register_map(const char* name)

   .. code-block:: cpp

      //prototype
      template<typename K, typename V>
      class_<std::map<K, V>> register_map(const char* name)

   A function to register a ``std::map<K, V>``.

   :param const char* name:



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

      A typedef of ``EnumType`` (a typename for the class).


   .. cpp:function::  enum_(const char* name)

      Constructor.

      :param const char* name:


   .. cpp:function::  enum_& value(const char* name, EnumType value)

      Registers an enum value.

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
