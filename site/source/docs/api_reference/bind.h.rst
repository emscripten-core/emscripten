.. _bind-h:

================================
bind.h (under-construction)
================================

The C++ APIs in `bind.h <https://github.com/kripken/emscripten/blob/master/system/include/emscripten/bind.h>`_ define (**HamishW**-Replace with description.)

.. contents:: Table of Contents
    :local:
    :depth: 1
	
How to use this API
===================
	

Defines
------- 



.. cpp:namespace: emscripten


.. cpp:type:: sharing_policy 

	**HamishW**-Replace with description. Note this is a strongly typed enum. I can't see better way in Sphinx to represent it.

	.. cpp:type:: sharing_policy::NONE
	
		**HamishW**-Replace with description.
	
	.. cpp:type:: sharing_policy::INTRUSIVE
	
		**HamishW**-Replace with description.
		
	.. cpp:type:: sharing_policy::BY_EMVAL
	
		**HamishW**-Replace with description.


Policies
===================

	
.. cpp:type:: arg

	.. cpp:member:: static constexpr int index
	
		**HamishW** Add description. Not sure if this best way to define a templated object which takes an argument like this. Kept declaration below for discussion: ::
	
			template<int Index>
			struct arg {
				static constexpr int index = Index + 1;
			};

.. cpp:type:: ret_val

	.. cpp:member:: static constexpr int index
	
		**HamishW** Add description. Not sure if this best way to define member in struct like this. Perhaps an example of usage, or even define as ``static constexpr int arg::index``. Kept declaration below for discussion: ::
	
			struct ret_val {
				static constexpr int index = 0;
			};


.. cpp:type:: allow_raw_pointers::Transform::type

	**HamishW** Add description. Note from source is: // whitelist all raw pointers
	
	**HamishW** Not sure if this best way to define this data structure in sphinx, but is less cumbersome than having separate nested type declarations, particularly where I doubt they need individual descriptions. This works in sphinx because all the nested objects are "types"
	
	
.. cpp:type:: allow_raw_pointer

	**HamishW** Add description.
	
	.. note:: This type is temporary, it will be changed when arg policies are reworked
	
	**HamishW** Not sure if this best way to define this data structure. The templated parameter SLOT doesn't appear to be used, so no easy place to explain what is going on here. Declaration kept below for discussion: ::

		template<typename Slot>
		struct allow_raw_pointer : public allow_raw_pointers {


select_overload and select_const
======================================

**HamishW** Nicer title please. What are these actually "for" a good start.


.. cpp:function:: typename std::add_pointer<Signature>::type select_overload(typename std::add_pointer<Signature>::type fn)

	**HamishW** Add description.

	:param typename std::add_pointer<Signature>::type fn: **HamishW** Add description. Note that ``Signature`` is a typename (templated type).
	:returns: **HamishW** Add description.


.. cpp:function:: typename internal::MemberFunctionType<ClassType, Signature>::type select_overload( Signature (ClassType::*fn) )

	**HamishW** Add description.

	:param Signature (ClassType::*fn): **HamishW** Add description. Note that ``Signature`` and ``ClassType`` are typenames (templated types).
	:returns: **HamishW** Add description.

	
.. cpp:function:: auto select_const(ReturnType (ClassType::*method)(Args...) const)

	**HamishW** Add description.

	:param ReturnType (ClassType::*method)(Args...) const: **HamishW** Add description. Note that ``ClassType``, ``ReturnType``, and ``Args`` are typenames (templated types).
	:returns: **HamishW** Add description.


.. cpp:function:: typename internal::CalculateLambdaSignature<LambdaType>::type optional_override(const LambdaType& fp)

	**HamishW** Add description.

	:param const LambdaType& fp: **HamishW** Add description. Note that ``LambdaType``is a typename (templated type).
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

	
.. cpp:function:: void function(const char* name, ReturnType (*fn)(Args...), Policies...)

	**HamishW** Add description.

	:param const char* name: **HamishW** Add description. 
	:param ReturnType (*fn)(Args...): **HamishW** Add description. Note that ``ReturnType`` and ``Args`` are typenames (templated types).	
	:param Policies...: **HamishW** Add description. Note that ``Policies`` is a typename (templated type).



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
		:param FieldType InstanceType::*field: **HamishW** Add description. Note that ``InstanceType`` and ``FieldType`` are typenames (templated types).
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

		

.. cpp:type:: default_smart_ptr_trait

	**HamishW** Add description. Note from source is: // specialize if you have a different pointer type
	
	.. cpp:type:: element_type;
	
		**HamishW** Add description. A typedef for the PointerType::element_type, where ``PointerType`` is a typename (templated type).
		

		
	.. cpp:function:: static element_type* get(const PointerType& ptr) 
	
		**HamishW** Add description.
		
		:param const PointerType& ptr: **HamishW** Add description. Note that ``PointerType`` is a typename (templated type)
		:returns: **HamishW** Add description.  

		

.. cpp:type:: smart_ptr_trait

	**HamishW** Add description.

	.. cpp:type:: PointerType;
	
		**HamishW** Add description. A typedef to std::shared_ptr<PointeeType>, where ``PointeeType`` is a typename (templated type).		
	
	.. cpp:type:: element_type;
	
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
======================================

**HamishW** Add description if needed. Note from source "// abstract classes"


.. cpp:class:: class wrapper : public T, public internal::WrapperBase

	**HamishW** Add description. 
	
	This is a templated class: ``template<typename T>``. 

	.. cpp:type:: class_type
	
		**HamishW** Add description. A typedef of ``T``, the typename of the templated type for the class.
    

	.. cpp:function:: explicit wrapper(val&& wrapped, Args&&... args)
	
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
	
		**HamishW** I don't understand this C++, so not sure how to document. Putting code here for Chad to advise on how to document :: 
		
		template<typename ClassType>
        using Upcaster = BaseClass* (*)(ClassType*);
		
		template<typename ClassType>
        using Downcaster = ClassType* (*)(BaseClass*);

		
	.. cpp:function:: static Upcaster<ClassType> getUpcaster()
	
		**HamishW** Add description. Note that ``ClassType`` is a typename (template parameter).
		
		:returns: **HamishW** Add description. 
		
		
	.. cpp:function:: static Downcaster<ClassType> getDowncaster() 
	
		**HamishW** Add description. Note that ``ClassType`` is a typename (template parameter).
		
		:returns: **HamishW** Add description. 		
		

	.. cpp:function:: static To* convertPointer(From* ptr)
	
		**HamishW** Add description. Note that ``ClassType`` is a typename (template parameter).
		
		:param From* ptr: **HamishW** Add description. 	Note that ``From`` is a typename (template parameter).
		:returns: **HamishW** Add description. Note that ``To`` is a typename (template parameter).

		

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
	
		**HamishW** Don't know what do do with this: ::
		
			class_() = delete;


	.. cpp:function:: EMSCRIPTEN_ALWAYS_INLINE explicit class_(const char* name)
	
		Constructor. **HamishW** Add description.
		
		:param const char* name: **HamishW** Add description. 
		:returns: **HamishW** Add description.  


	.. cpp:function:: EMSCRIPTEN_ALWAYS_INLINE const class_& smart_ptr(const char* name) const
	
		**HamishW** Add description. Note that this is a function template taking a typename ``PointerType``.
		
		:param const char* name: **HamishW** Add description. 
		:returns: **HamishW** Add description.  

		
	.. cpp:function:: EMSCRIPTEN_ALWAYS_INLINE const class_& constructor(Policies... policies) const
	
		**HamishW** Add description. Note that this is a function template taking typenames ``... ConstructorArgs`` and ``... Policies``. 
		
		:param Policies... policies: **HamishW** Add description. Note that ``... Policies`` is a template typename for this function. 
		:returns: **HamishW** Add description.  


	.. cpp:function:: EMSCRIPTEN_ALWAYS_INLINE const class_& constructor(ReturnType (*factory)(Args...), Policies...) const
	
		**HamishW** Add description. Note that this is a function template taking typenames ``... Args``, ``ReturnType`` and ``... Policies``. 
		
		:param ReturnType (*factory)(Args...): **HamishW** Add description. Note that ``Args`` and ``ReturnType`` are template typenames for this function. 
		:param Policies... policies: **HamishW** Add description. Note that ``Policies`` is a template typename for this function. 
		:returns: **HamishW** Add description.
		
		
	.. cpp:function:: EMSCRIPTEN_ALWAYS_INLINE const class_& smart_ptr_constructor(const char* smartPtrName, SmartPtr (*factory)(Args...), Policies...) const 
	
		**HamishW** Add description. Note that this is a function template taking typenames ``SmartPtr``, ``... Args``, and ``... Policies``.
		
		:param const char* smartPtrName: **HamishW** Add description. 
		:param SmartPtr (*factory)(Args...): **HamishW** Add description. Note that ``Args`` and ``SmartPtr`` are template typenames for this function. 
		:param Policies... policies: **HamishW** Add description. Note that ``Policies`` is a template typename for this function. 
		:returns: **HamishW** Add description.		
		

	.. cpp:function:: EMSCRIPTEN_ALWAYS_INLINE const class_& allow_subclass(const char* wrapperClassName, const char* pointerName = "<UnknownPointerName>", ::emscripten::constructor<ConstructorArgs...> = ::emscripten::constructor<ConstructorArgs...>() ) const 
	
		**HamishW** Add description. Note that this is a function template taking typenames ``WrapperType``, ``PointerType``, and ``... ConstructorArgs``.
		
		:param const char* wrapperClassName: **HamishW** Add description. 
		:param const char* pointerName: **HamishW** Add description. Note that this has a default value which is dependent on the template typename parameters.
		:returns: **HamishW** Add description.


	.. cpp:function:: EMSCRIPTEN_ALWAYS_INLINE const class_& allow_subclass(const char* wrapperClassName, ::emscripten::constructor<ConstructorArgs...> constructor) ) const 
	
		**HamishW** Add description. Note that this is a function template taking typenames ``WrapperType`` and ``... ConstructorArgs``: ``template<typename WrapperType, typename... ConstructorArgs>``
		
		:param const char* wrapperClassName: **HamishW** Add description. 
		:param ::emscripten::constructor<ConstructorArgs...> constructor): **HamishW** Add description. Note that ``ConstructorArgs`` is a template typename for this function. 
		:returns: **HamishW** Add description.


		
	.. cpp:function:: EMSCRIPTEN_ALWAYS_INLINE const class_& function(const char* methodName, ReturnType (ClassType::*memberFunction)(Args...), Policies...) const 
	
		**HamishW** Add description. Note that this is a function template taking typenames ``ReturnType``,, ``... Args`` and ``... Policies``: ``template<typename ReturnType, typename... Args, typename... Policies>``
		
		:param const char* methodName: **HamishW** Add description. 
		:param ReturnType (ClassType::*memberFunction)(Args...): **HamishW** Add description. Note that ``ReturnType`` is a template typename for this function and ``ClassType`` is a template typename for the class.
		:param typename... Policies: **HamishW** Add description. Note that ``Policies`` is a template typename for this function. 
		:returns: **HamishW** Add description.


	.. cpp:function:: EMSCRIPTEN_ALWAYS_INLINE const class_& function( const char* methodName, ReturnType (*function)(ThisType, Args...), Policies... ) const
	
		**HamishW** Add description. Note that this is a function template taking typename ``FieldType``, **Not sure how to document the section parameter**: ``template<typename FieldType, typename = typename std::enable_if<!std::is_function<FieldType>::value>::type>``
		
		:param const char* methodName: **HamishW** Add description. 
		:param ReturnType (*function)(ThisType, Args...): **HamishW** Add description. Note that ``ReturnType``, ``ThisType`` and ``Args`` are template typenames for this function.
		:param typename... Policies: **HamishW** Add description. Note that ``Policies`` is a template typename for this function. 
		:returns: **HamishW** Add description.

		
	.. cpp:function:: EMSCRIPTEN_ALWAYS_INLINE const class_& property(const char* fieldName, const FieldType ClassType::*field) const
	
		**HamishW** Add description. Note that this is a function template taking typenames ``ReturnType``, ``ThisType``, ``Args`` and ``... Policies``: ``template<typename ReturnType, typename ThisType, typename... Args, typename... Policies>``
		
		:param const char* fieldName: **HamishW** Add description. 
		:param const FieldType ClassType::*field: **HamishW** Add description. Note that ``FieldType`` is a function template typename, and ``ClassType`` is a class template typename.
		:returns: **HamishW** Add description.


	.. cpp:function:: EMSCRIPTEN_ALWAYS_INLINE const class_& property(const char* fieldName, FieldType ClassType::*field) const 
	
		**HamishW** Add description. Note that this is a function template taking typenames ``FieldType`` - **Not sure how to document/describe second typename parameter** : ``template<typename FieldType, typename = typename std::enable_if<!std::is_function<FieldType>::value>::type>``
		
		:param const char* fieldName: **HamishW** Add description. 
		:param FieldType ClassType::*field: **HamishW** Add description. Note that ``FieldType`` is a function template typename, and ``ClassType`` is a class template typename.
		:returns: **HamishW** Add description.

		
	.. cpp:function:: EMSCRIPTEN_ALWAYS_INLINE const class_& property(const char* fieldName, Getter getter) const
	
		**HamishW** Add description. Note that this is a function template taking typename ``Getter``: ``template<typename Getter>``
		
		:param const char* fieldName: **HamishW** Add description. 
		:param Getter getter: **HamishW** Add description. Note that ``Getter`` is a function template typename.
		:returns: **HamishW** Add description.
		
		
	.. cpp:function:: EMSCRIPTEN_ALWAYS_INLINE const class_& property(const char* fieldName, Getter getter, Setter setter) const
	
		**HamishW** Add description. Note that this is a function template taking typenames ``Setter`` and ``Getter``: ``template<typename Getter, typename Setter>``
		
		:param const char* fieldName: **HamishW** Add description. 
		:param Getter getter: **HamishW** Add description. Note that ``Getter`` is a function template typename.
		:param Setter setter: **HamishW** Add description. Note that ``Setter`` is a function template typename.
		:returns: **HamishW** Add description.
		
	.. cpp:function:: EMSCRIPTEN_ALWAYS_INLINE const class_& class_function(const char* methodName, ReturnType (*classMethod)(Args...), Policies...) const 
	
		**HamishW** Add description. Note that this is a function template taking typenames ``Setter`` and ``Getter``: ``template<typename ReturnType, typename... Args, typename... Policies>``
		
		:param const char* methodName: **HamishW** Add description. 
		:param ReturnType (*classMethod)(Args...): **HamishW** Add description. Note that ``ReturnType`` and ``Args`` are function template typenames.
		:param Policies...: **HamishW** Add description. Note that ``Policies`` is a function template typename.
		:returns: **HamishW** Add description.



	
Vectors
=======

.. cpp:function:: class_<std::vector<T>> register_vector(const char* name)

	**HamishW** Add description. 
	
	Note that this is a templated function: ``template<typename T>``

	:param const char* name: **HamishW** Add description. 
	:returns: **HamishW** Add description.





Maps
====

.. cpp:function::  class_<std::map<K, V>> register_map(const char* name)

	**HamishW** Add description. Note that this is a templated function: ``template<typename K, typename V>``

	:param const char* name: **HamishW** Add description. 
	:returns: **HamishW** Add description.



Enums
=====


.. cpp:class:: enum_

	**HamishW** Add description. Note that this is a templated class: ``template<typename EnumType>``

	.. cpp:type:: enum_type
	
		**HamishW** Add description. A typedef of ``EnumType`` (a typename for the class).


	.. cpp:function::  enum_(const char* name)

		Constructor. **HamishW** Add description. 

		:param const char* name: **HamishW** Add description. 
		:returns: **HamishW** Add description.
		
		
	.. cpp:function::  enum_& value(const char* name, EnumType value)

		Constructor. **HamishW** Add description. 

		:param const char* name: **HamishW** Add description. 
		:param EnumType value: **HamishW** Add description. Note that ``EnumType`` is a class template typename.
		:returns: **HamishW** Add description.		



Constants
=========

.. cpp:function:: void constant(const char* name, const ConstantType& v)

	**HamishW** Add description. Note that this is a templated function: ``template<typename ConstantType>``

	:param const char* name: **HamishW** Add description.
	:param const ConstantType& v: **HamishW** Add description. Note that ``ConstantType`` is a template typename for the function.

	
.. cpp:function:: EMSCRIPTEN_BINDINGS(name)

	**HamishW** Add description.  Note that this is actually a define, but I've implemented it as a function, because that is how it behaves, and it allows me to have the name parameter as shown, which isn't possible on Sphinx type declaration.

	:param name: **HamishW** Add description.


	
.. COMMENT (not rendered): Following values are common to many functions, and currently only updated in one place (here).
.. COMMENT (not rendered): These can be properly replaced if required either wholesale or on an individual basis.

.. |policies-argument| replace:: Some boilerplate, this is an example only and can be removed.
