.. _bind-h:

================================
bind.h (under-construction)
================================

The C++ APIs in `bind.h <https://github.com/kripken/emscripten/blob/master/system/include/emscripten/bind.h>`_ define (**HamishW**-Replace with description.)

**HamishW** Typenames etc.

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


.. cpp:function:: typename internal::MemberFunctionType<ClassType, Signature>::type select_overload(Signature (ClassType::*fn))

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


**HamishW** Note, did not include private class val_deleter. I am assuming all private classes are internal.


Classes
======================================


    // abstract classes
    template<typename T>
    class wrapper : public T, public internal::WrapperBase {
    public:
        typedef T class_type;

        template<typename... Args>
        explicit wrapper(val&& wrapped, Args&&... args)
            : T(std::forward<Args>(args)...)
            , wrapped(std::forward<val>(wrapped))
        {}

        ~wrapper() {
            if (notifyJSOnDestruction) {
                call<void>("__destruct");
            }
        }

        template<typename ReturnType, typename... Args>
        ReturnType call(const char* name, Args&&... args) const {
            return wrapped.call<ReturnType>(name, std::forward<Args>(args)...);
        }



#define EMSCRIPTEN_WRAPPER(T)                                           \
    template<typename... Args>                                          \
    T(::emscripten::val&& v, Args&&... args)                            \
        : wrapper(std::forward<::emscripten::val>(v), std::forward<Args>(args)...) \
    {}



    template<typename BaseClass>
    struct base {
        typedef BaseClass class_type;

        template<typename ClassType>
        static void verify() {
            static_assert(!std::is_same<ClassType, BaseClass>::value, "Base must not have same type as class");
            static_assert(std::is_base_of<BaseClass, ClassType>::value, "Derived class must derive from base");
        }

        static internal::TYPEID get() {
            return internal::TypeID<BaseClass>::get();
        }
        
        template<typename ClassType>
        using Upcaster = BaseClass* (*)(ClassType*);

        template<typename ClassType>
        using Downcaster = ClassType* (*)(BaseClass*);
        
        template<typename ClassType>
        static Upcaster<ClassType> getUpcaster() {
            return &convertPointer<ClassType, BaseClass>;
        }
        
        template<typename ClassType>
        static Downcaster<ClassType> getDowncaster() {
            return &convertPointer<BaseClass, ClassType>;
        }

        template<typename From, typename To>
        static To* convertPointer(From* ptr) {
            return static_cast<To*>(ptr);
        }
    };



    struct pure_virtual {
        template<typename InputType, int Index>
        struct Transform {
            typedef InputType type;
        };
    };



    template<typename... ConstructorArgs>
    struct constructor {
    };

    template<typename ClassType, typename BaseSpecifier = internal::NoBaseClass>
    class class_ {
    public:
        typedef ClassType class_type;
        typedef BaseSpecifier base_specifier;

        class_() = delete;

        EMSCRIPTEN_ALWAYS_INLINE explicit class_(const char* name) {
            using namespace internal;

            BaseSpecifier::template verify<ClassType>();

            auto _getActualType = &getActualType<ClassType>;
            auto upcast = BaseSpecifier::template getUpcaster<ClassType>();
            auto downcast = BaseSpecifier::template getDowncaster<ClassType>();
            auto destructor = &raw_destructor<ClassType>;

            _embind_register_class(
                TypeID<ClassType>::get(),
                TypeID<AllowedRawPointer<ClassType>>::get(),
                TypeID<AllowedRawPointer<const ClassType>>::get(),
                BaseSpecifier::get(),
                getSignature(_getActualType),
                reinterpret_cast<GenericFunction>(_getActualType),
                getSignature(upcast),
                reinterpret_cast<GenericFunction>(upcast),
                getSignature(downcast),
                reinterpret_cast<GenericFunction>(downcast),
                name,
                getSignature(destructor),
                reinterpret_cast<GenericFunction>(destructor));
        }

        template<typename PointerType>
        EMSCRIPTEN_ALWAYS_INLINE const class_& smart_ptr(const char* name) const {
            using namespace internal;

            typedef smart_ptr_trait<PointerType> PointerTrait;
            typedef typename PointerTrait::element_type PointeeType;
            
            static_assert(std::is_same<ClassType, typename std::remove_cv<PointeeType>::type>::value, "smart pointer must point to this class");

            auto get = &PointerTrait::get;
            auto construct_null = &PointerTrait::construct_null;
            auto share = &PointerTrait::share;
            auto destructor = &raw_destructor<PointerType>;

            _embind_register_smart_ptr(
                TypeID<PointerType>::get(),
                TypeID<PointeeType>::get(),
                name,
                PointerTrait::get_sharing_policy(),
                getSignature(get),
                reinterpret_cast<GenericFunction>(get),
                getSignature(construct_null),
                reinterpret_cast<GenericFunction>(construct_null),
                getSignature(share),
                reinterpret_cast<GenericFunction>(share),
                getSignature(destructor),
                reinterpret_cast<GenericFunction>(destructor));
            return *this;
        };

        template<typename... ConstructorArgs, typename... Policies>
        EMSCRIPTEN_ALWAYS_INLINE const class_& constructor(Policies... policies) const {
            return constructor(
                &internal::operator_new<ClassType, ConstructorArgs...>,
                policies...);
        }

        template<typename... Args, typename ReturnType, typename... Policies>
        EMSCRIPTEN_ALWAYS_INLINE const class_& constructor(ReturnType (*factory)(Args...), Policies...) const {
            using namespace internal;

            // TODO: allows all raw pointers... policies need a rethink
            typename WithPolicies<allow_raw_pointers, Policies...>::template ArgTypeList<ReturnType, Args...> args;
            auto invoke = &Invoker<ReturnType, Args...>::invoke;
            _embind_register_class_constructor(
                TypeID<ClassType>::get(),
                args.getCount(),
                args.getTypes(),
                getSignature(invoke),
                reinterpret_cast<GenericFunction>(invoke),
                reinterpret_cast<GenericFunction>(factory));
            return *this;
        }

        template<typename SmartPtr, typename... Args, typename... Policies>
        EMSCRIPTEN_ALWAYS_INLINE const class_& smart_ptr_constructor(const char* smartPtrName, SmartPtr (*factory)(Args...), Policies...) const {
            using namespace internal;

            smart_ptr<SmartPtr>(smartPtrName);

            typename WithPolicies<Policies...>::template ArgTypeList<SmartPtr, Args...> args;
            auto invoke = &Invoker<SmartPtr, Args...>::invoke;
            _embind_register_class_constructor(
                TypeID<ClassType>::get(),
                args.getCount(),
                args.getTypes(),
                getSignature(invoke),
                reinterpret_cast<GenericFunction>(invoke),
                reinterpret_cast<GenericFunction>(factory));
            return *this;
        }

        template<typename WrapperType, typename PointerType = WrapperType*, typename... ConstructorArgs>
        EMSCRIPTEN_ALWAYS_INLINE const class_& allow_subclass(
            const char* wrapperClassName,
            const char* pointerName = "<UnknownPointerName>",
            ::emscripten::constructor<ConstructorArgs...> = ::emscripten::constructor<ConstructorArgs...>()
        ) const {
            using namespace internal;

            auto cls = class_<WrapperType, base<ClassType>>(wrapperClassName)
                .function("notifyOnDestruction", select_overload<void(WrapperType&)>([](WrapperType& wrapper) {
                    wrapper.setNotifyJSOnDestruction(true);
                }))
                ;
            SmartPtrIfNeeded<PointerType> _(cls, pointerName);

            return
                class_function(
                    "implement",
                    &wrapped_new<PointerType, WrapperType, val, ConstructorArgs...>,
                    allow_raw_pointer<ret_val>())
                .class_function(
                    "extend",
                    &wrapped_extend<WrapperType>)
                ;
        }

        template<typename WrapperType, typename... ConstructorArgs>
        EMSCRIPTEN_ALWAYS_INLINE const class_& allow_subclass(
            const char* wrapperClassName,
            ::emscripten::constructor<ConstructorArgs...> constructor
        ) const {
            return allow_subclass<WrapperType, WrapperType*>(wrapperClassName, "<UnknownPointerName>", constructor);
        }

        template<typename ReturnType, typename... Args, typename... Policies>
        EMSCRIPTEN_ALWAYS_INLINE const class_& function(const char* methodName, ReturnType (ClassType::*memberFunction)(Args...), Policies...) const {
            using namespace internal;

            auto invoker = &MethodInvoker<decltype(memberFunction), ReturnType, ClassType*, Args...>::invoke;

            typename WithPolicies<Policies...>::template ArgTypeList<ReturnType, AllowedRawPointer<ClassType>, Args...> args;
            _embind_register_class_function(
                TypeID<ClassType>::get(),
                methodName,
                args.getCount(),
                args.getTypes(),
                getSignature(invoker),
                reinterpret_cast<GenericFunction>(invoker),
                getContext(memberFunction),
                isPureVirtual<Policies...>::value);
            return *this;
        }

        template<typename ReturnType, typename... Args, typename... Policies>
        EMSCRIPTEN_ALWAYS_INLINE const class_& function(const char* methodName, ReturnType (ClassType::*memberFunction)(Args...) const, Policies...) const {
            using namespace internal;

            auto invoker = &MethodInvoker<decltype(memberFunction), ReturnType, const ClassType*, Args...>::invoke;

            typename WithPolicies<Policies...>::template ArgTypeList<ReturnType, AllowedRawPointer<const ClassType>, Args...> args;
            _embind_register_class_function(
                TypeID<ClassType>::get(),
                methodName,
                args.getCount(),
                args.getTypes(),
                getSignature(invoker),
                reinterpret_cast<GenericFunction>(invoker),
                getContext(memberFunction),
                isPureVirtual<Policies...>::value);
            return *this;
        }

        template<typename ReturnType, typename ThisType, typename... Args, typename... Policies>
        EMSCRIPTEN_ALWAYS_INLINE const class_& function(const char* methodName, ReturnType (*function)(ThisType, Args...), Policies...) const {
            using namespace internal;

            typename WithPolicies<Policies...>::template ArgTypeList<ReturnType, ThisType, Args...> args;
            auto invoke = &FunctionInvoker<decltype(function), ReturnType, ThisType, Args...>::invoke;
            _embind_register_class_function(
                TypeID<ClassType>::get(),
                methodName,
                args.getCount(),
                args.getTypes(),
                getSignature(invoke),
                reinterpret_cast<GenericFunction>(invoke),
                getContext(function),
                false);
            return *this;
        }

        template<typename FieldType, typename = typename std::enable_if<!std::is_function<FieldType>::value>::type>
        EMSCRIPTEN_ALWAYS_INLINE const class_& property(const char* fieldName, const FieldType ClassType::*field) const {
            using namespace internal;
            
            auto getter = &MemberAccess<ClassType, FieldType>::template getWire<ClassType>;
            _embind_register_class_property(
                TypeID<ClassType>::get(),
                fieldName,
                TypeID<FieldType>::get(),
                getSignature(getter),
                reinterpret_cast<GenericFunction>(getter),
                getContext(field),
                0,
                0,
                0,
                0);
            return *this;
        }

        template<typename FieldType, typename = typename std::enable_if<!std::is_function<FieldType>::value>::type>
        EMSCRIPTEN_ALWAYS_INLINE const class_& property(const char* fieldName, FieldType ClassType::*field) const {
            using namespace internal;

            auto getter = &MemberAccess<ClassType, FieldType>::template getWire<ClassType>;
            auto setter = &MemberAccess<ClassType, FieldType>::template setWire<ClassType>;
            _embind_register_class_property(
                TypeID<ClassType>::get(),
                fieldName,
                TypeID<FieldType>::get(),
                getSignature(getter),
                reinterpret_cast<GenericFunction>(getter),
                getContext(field),
                TypeID<FieldType>::get(),
                getSignature(setter),
                reinterpret_cast<GenericFunction>(setter),
                getContext(field));
            return *this;
        }

        template<typename Getter>
        EMSCRIPTEN_ALWAYS_INLINE const class_& property(const char* fieldName, Getter getter) const {
            using namespace internal;
            typedef GetterPolicy<Getter> GP;
            auto gter = &GP::template get<ClassType>;
            _embind_register_class_property(
                TypeID<ClassType>::get(),
                fieldName,
                TypeID<typename GP::ReturnType>::get(),
                getSignature(gter),
                reinterpret_cast<GenericFunction>(gter),
                GP::getContext(getter),
                0,
                0,
                0,
                0);
            return *this;
        }

        template<typename Getter, typename Setter>
        EMSCRIPTEN_ALWAYS_INLINE const class_& property(const char* fieldName, Getter getter, Setter setter) const {
            using namespace internal;
            typedef GetterPolicy<Getter> GP;
            typedef SetterPolicy<Setter> SP;

            auto gter = &GP::template get<ClassType>;
            auto ster = &SP::template set<ClassType>;

            _embind_register_class_property(
                TypeID<ClassType>::get(),
                fieldName,
                TypeID<typename GP::ReturnType>::get(),
                getSignature(gter),
                reinterpret_cast<GenericFunction>(gter),
                GP::getContext(getter),
                TypeID<typename SP::ArgumentType>::get(),
                getSignature(ster),
                reinterpret_cast<GenericFunction>(ster),
                SP::getContext(setter));
            return *this;
        }

        template<typename ReturnType, typename... Args, typename... Policies>
        EMSCRIPTEN_ALWAYS_INLINE const class_& class_function(const char* methodName, ReturnType (*classMethod)(Args...), Policies...) const {
            using namespace internal;

            typename WithPolicies<Policies...>::template ArgTypeList<ReturnType, Args...> args;
            auto invoke = &internal::Invoker<ReturnType, Args...>::invoke;
            _embind_register_class_class_function(
                TypeID<ClassType>::get(),
                methodName,
                args.getCount(),
                args.getTypes(),
                getSignature(invoke),
                reinterpret_cast<internal::GenericFunction>(invoke),
                reinterpret_cast<GenericFunction>(classMethod));
            return *this;
        }
    };


	
Vectors
======================================


    template<typename T>
    class_<std::vector<T>> register_vector(const char* name) {
        typedef std::vector<T> VecType;

        void (VecType::*push_back)(const T&) = &VecType::push_back;
        return class_<std::vector<T>>(name)
            .template constructor<>()
            .function("push_back", push_back)
            .function("size", &VecType::size)
            .function("get", &internal::VectorAccess<VecType>::get)
            .function("set", &internal::VectorAccess<VecType>::set)
            ;
    }


Maps
======================================



    template<typename K, typename V>
    class_<std::map<K, V>> register_map(const char* name) {
        typedef std::map<K,V> MapType;

        return class_<MapType>(name)
            .template constructor<>()
            .function("size", &MapType::size)
            .function("get", internal::MapAccess<MapType>::get)
            .function("set", internal::MapAccess<MapType>::set)
            ;
    }



Enums
======================================

    template<typename EnumType>
    class enum_ {
    public:
        typedef EnumType enum_type;

        enum_(const char* name) {
            using namespace internal;
            _embind_register_enum(
                internal::TypeID<EnumType>::get(),
                name,
                sizeof(EnumType),
                std::is_signed<typename std::underlying_type<EnumType>::type>::value);
        }

        enum_& value(const char* name, EnumType value) {
            using namespace internal;
            // TODO: there's still an issue here.
            // if EnumType is an unsigned long, then JS may receive it as a signed long
            static_assert(sizeof(value) <= sizeof(internal::GenericEnumValue), "enum type must fit in a GenericEnumValue");

            _embind_register_enum_value(
                internal::TypeID<EnumType>::get(),
                name,
                static_cast<internal::GenericEnumValue>(value));
            return *this;
        }
    };


Constants
======================================



    template<typename ConstantType>
    void constant(const char* name, const ConstantType& v) {
        using namespace internal;
        typedef BindingType<const ConstantType&> BT;
        _embind_register_constant(
            name,
            TypeID<const ConstantType&>::get(),
            asGenericValue(BT::toWireType(v)));
    }
}

#define EMSCRIPTEN_BINDINGS(name)                                       \
    static struct EmscriptenBindingInitializer_##name {                 \
        EmscriptenBindingInitializer_##name();                          \
    } EmscriptenBindingInitializer_##name##_instance;                   \
    EmscriptenBindingInitializer_##name::EmscriptenBindingInitializer_##name()


