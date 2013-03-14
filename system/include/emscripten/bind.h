#pragma once

#include <stddef.h>
#include <assert.h>
#include <string>
#include <functional>
#include <vector>
#include <map>
#include <type_traits>
#include <emscripten/val.h>
#include <emscripten/wire.h>

namespace emscripten {
    enum class sharing_policy {
        NONE = 0,
        INTRUSIVE = 1,
        BY_EMVAL = 2,
    };

    namespace internal {
        typedef void (*GenericFunction)();
        typedef long GenericEnumValue;

        // Implemented in JavaScript.  Don't call these directly.
        extern "C" {
            void _embind_fatal_error(
                const char* name,
                const char* payload) __attribute__((noreturn));

            void _embind_register_void(
                TYPEID voidType,
                const char* name);

            void _embind_register_bool(
                TYPEID boolType,
                const char* name,
                bool trueValue,
                bool falseValue);

            void _embind_register_integer(
                TYPEID integerType,
                const char* name);

            void _embind_register_float(
                TYPEID floatType,
                const char* name);
            
            void _embind_register_cstring(
                TYPEID stringType,
                const char* name);

            void _embind_register_emval(
                TYPEID emvalType,
                const char* name);

            void _embind_register_function(
                const char* name,
                unsigned argCount,
                TYPEID argTypes[],
                GenericFunction invoker,
                GenericFunction function);

            void _embind_register_tuple(
                TYPEID tupleType,
                const char* name,
                GenericFunction constructor,
                GenericFunction destructor);
            
            void _embind_register_tuple_element(
                TYPEID tupleType,
                TYPEID elementType,
                GenericFunction getter,
                GenericFunction setter,
                size_t memberPointerSize,
                void* memberPointer);

            void _embind_register_tuple_element_accessor(
                TYPEID tupleType,
                TYPEID elementType,
                GenericFunction staticGetter,
                size_t getterSize,
                void* getter,
                GenericFunction staticSetter,
                size_t setterSize,
                void* setter);

            void _embind_register_struct(
                TYPEID structType,
                const char* name,
                GenericFunction constructor,
                GenericFunction destructor);
            
            void _embind_register_struct_field(
                TYPEID structType,
                const char* name,
                TYPEID fieldType,
                GenericFunction getter,
                GenericFunction setter,
                size_t memberPointerSize,
                void* memberPointer);

            void _embind_register_smart_ptr(
                TYPEID pointerType,
                TYPEID pointeeType,
                const char* pointerName,
                sharing_policy sharingPolicy,
                GenericFunction getPointee,
                GenericFunction constructor,
                GenericFunction share,
                GenericFunction destructor);

            void _embind_register_class(
                TYPEID classType,
                TYPEID pointerType,
                TYPEID constPointerType,
                TYPEID baseClassType,
                GenericFunction upcast,
                GenericFunction downcast,
                bool isPolymorphic,
                const char* className,
                GenericFunction destructor);

            void _embind_register_class_constructor(
                TYPEID classType,
                unsigned argCount,
                TYPEID argTypes[],
                GenericFunction invoker,
                GenericFunction constructor);

            void _embind_register_class_function(
                TYPEID classType,
                const char* methodName,
                unsigned argCount,
                TYPEID argTypes[],
                bool isConst,
                GenericFunction invoker,
                size_t memberFunctionSize,
                void* memberFunction);

            void _embind_register_class_property(
                TYPEID classType,
                const char* fieldName,
                TYPEID fieldType,
                GenericFunction getter,
                GenericFunction setter,
                size_t memberPointerSize,
                void* memberPointer);

            void _embind_register_class_class_function(
                TYPEID classType,
                const char* methodName,
                unsigned argCount,
                TYPEID argTypes[],
                GenericFunction invoker,
                GenericFunction method);

            void _embind_register_enum(
                TYPEID enumType,
                const char* name);

            void _embind_register_enum_value(
                TYPEID enumType,
                const char* valueName,
                GenericEnumValue value);

            void _embind_register_interface(
                TYPEID interfaceType,
                const char* name,
                GenericFunction constructor,
                GenericFunction destructor);
        }

        class BindingsDefinition {
        public:
            template<typename Function>
            BindingsDefinition(Function fn) {
                fn();
            }
        };
    }
}

namespace emscripten {
    ////////////////////////////////////////////////////////////////////////////////
    // POLICIES
    ////////////////////////////////////////////////////////////////////////////////

    template<int Index>
    struct arg {
        static constexpr int index = Index + 1;
    };

    struct ret_val {
        static constexpr int index = 0;
    };

    template<typename Slot>
    struct allow_raw_pointer {
        template<typename InputType, int Index>
        struct Transform {
            typedef typename std::conditional<
                Index == Slot::index,
                internal::AllowedRawPointer<typename std::remove_pointer<InputType>::type>,
                InputType
            >::type type;
        };
    };

    // whitelist all raw pointers
    struct allow_raw_pointers {
        template<typename InputType, int Index>
        struct Transform {
            typedef typename std::conditional<
                std::is_pointer<InputType>::value,
                internal::AllowedRawPointer<typename std::remove_pointer<InputType>::type>,
                InputType
            >::type type;
        };
    };

    namespace internal {
        template<typename ReturnType, typename... Args>
        struct Invoker {
            static typename internal::BindingType<ReturnType>::WireType invoke(
                ReturnType (*fn)(Args...),
                typename internal::BindingType<Args>::WireType... args
            ) {
                return internal::BindingType<ReturnType>::toWireType(
                    fn(
                        internal::BindingType<Args>::fromWireType(args)...
                    )
                );
            }
        };

        template<typename... Args>
        struct Invoker<void, Args...> {
            static void invoke(
                void (*fn)(Args...),
                typename internal::BindingType<Args>::WireType... args
            ) {
                return fn(
                    internal::BindingType<Args>::fromWireType(args)...
                );
            }
        };
    }

    ////////////////////////////////////////////////////////////////////////////////
    // FUNCTIONS
    ////////////////////////////////////////////////////////////////////////////////

    extern "C" {
        void* __getDynamicPointerType(void* p);
    }

    template<typename ReturnType, typename... Args, typename... Policies>
    void function(const char* name, ReturnType (*fn)(Args...), Policies...) {
        using namespace internal;
        typename WithPolicies<Policies...>::template ArgTypeList<ReturnType, Args...> args;
        _embind_register_function(
            name,
            args.count,
            args.types,
            reinterpret_cast<GenericFunction>(&Invoker<ReturnType, Args...>::invoke),
            reinterpret_cast<GenericFunction>(fn));
    }

    namespace internal {
        template<typename ClassType, typename... Args>
        ClassType* operator_new(Args... args) {
            return new ClassType(args...);
        }

        template<typename ClassType, typename... Args>
        ClassType* raw_constructor(
            typename internal::BindingType<Args>::WireType... args
        ) {
            return new ClassType(
                internal::BindingType<Args>::fromWireType(args)...
            );
        }

        template<typename ClassType>
        void raw_destructor(ClassType* ptr) {
            delete ptr;
        }

        template<typename ClassType, typename ReturnType, typename... Args>
        struct FunctionInvoker {
            typedef ReturnType (*FunctionPointer)(ClassType& ct, Args...);
            static typename internal::BindingType<ReturnType>::WireType invoke(
                ClassType* ptr,
                FunctionPointer* function,
                typename internal::BindingType<Args>::WireType... args
            ) {
                return internal::BindingType<ReturnType>::toWireType(
                    (*function)(*ptr, internal::BindingType<Args>::fromWireType(args)...)
                );
            }
        };

        template<typename ClassType, typename... Args>
        struct FunctionInvoker<ClassType, void, Args...> {
            typedef void (*FunctionPointer)(ClassType& ct, Args...);
            static void invoke(
                ClassType* ptr,
                FunctionPointer* function,
                typename internal::BindingType<Args>::WireType... args
            ) {
                (*function)(*ptr, internal::BindingType<Args>::fromWireType(args)...);
            }
        };

        template<typename ClassType, typename ReturnType, typename... Args>
        struct MethodInvoker {
            typedef ReturnType (ClassType::*MemberPointer)(Args...);
            static typename internal::BindingType<ReturnType>::WireType invoke(
                ClassType* ptr,
                const MemberPointer& method,
                typename internal::BindingType<Args>::WireType... args
            ) {
                return internal::BindingType<ReturnType>::toWireType(
                    (ptr->*method)(
                        internal::BindingType<Args>::fromWireType(args)...
                    )
                );
            }
        };

        template<typename ClassType, typename... Args>
        struct MethodInvoker<ClassType, void, Args...> {
            typedef void (ClassType::*MemberPointer)(Args...);
            static void invoke(
                ClassType* ptr,
                const MemberPointer& method,
                typename internal::BindingType<Args>::WireType... args
            ) {
                return (ptr->*method)(
                    internal::BindingType<Args>::fromWireType(args)...
                );
            }
        };

        template<typename ClassType, typename ReturnType, typename... Args>
        struct ConstMethodInvoker {
            typedef ReturnType (ClassType::*MemberPointer)(Args...) const;
            static typename internal::BindingType<ReturnType>::WireType invoke(
                const ClassType* ptr,
                const MemberPointer& method,
                typename internal::BindingType<Args>::WireType... args
            ) {
                return internal::BindingType<ReturnType>::toWireType(
                    (ptr->*method)(
                        internal::BindingType<Args>::fromWireType(args)...
                    )
                );
            }
        };

        template<typename ClassType, typename... Args>
        struct ConstMethodInvoker<ClassType, void, Args...> {
            typedef void (ClassType::*MemberPointer)(Args...) const;
            static void invoke(
                const ClassType* ptr,
                const MemberPointer& method,
                typename internal::BindingType<Args>::WireType... args
            ) {
                return (ptr->*method)(
                    internal::BindingType<Args>::fromWireType(args)...
                );
            }
        };

        template<typename ClassType, typename MemberType>
        struct MemberAccess {
            typedef MemberType ClassType::*MemberPointer;
            typedef internal::BindingType<MemberType> MemberBinding;
            typedef typename MemberBinding::WireType WireType;
            
            static WireType get(
                ClassType& ptr,
                const MemberPointer& field
            ) {
                return MemberBinding::toWireType(ptr.*field);
            }
            
            static void set(
                ClassType& ptr,
                const MemberPointer& field,
                WireType value
            ) {
                ptr.*field = MemberBinding::fromWireType(value);
            }

            template<typename Getter>
            static WireType propertyGet(
                ClassType& ptr,
                const Getter& getter
            ) {
                return MemberBinding::toWireType(getter(ptr));
            }

            template<typename Setter>
            static void propertySet(
                ClassType& ptr,
                const Setter& setter,
                WireType value
            ) {
                setter(ptr, MemberBinding::fromWireType(value));
            }
        };

    }

    ////////////////////////////////////////////////////////////////////////////////
    // VALUE TUPLES
    ////////////////////////////////////////////////////////////////////////////////

    template<typename ClassType>
    class value_tuple {
    public:
        value_tuple(const char* name) {
            internal::_embind_register_tuple(
                internal::TypeID<ClassType>::get(),
                name,
                reinterpret_cast<internal::GenericFunction>(&internal::raw_constructor<ClassType>),
                reinterpret_cast<internal::GenericFunction>(&internal::raw_destructor<ClassType>));
        }

        template<typename ElementType>
        value_tuple& element(ElementType ClassType::*field) {
            internal::_embind_register_tuple_element(
                internal::TypeID<ClassType>::get(),
                internal::TypeID<ElementType>::get(),
                reinterpret_cast<internal::GenericFunction>(&internal::MemberAccess<ClassType, ElementType>::get),
                reinterpret_cast<internal::GenericFunction>(&internal::MemberAccess<ClassType, ElementType>::set),
                sizeof(field),
                &field);
                                
            return *this;
        }

        template<typename ElementType>
        value_tuple& element(ElementType (*getter)(const ClassType&), void (*setter)(ClassType&, ElementType)) {
            internal::_embind_register_tuple_element_accessor(
                internal::TypeID<ClassType>::get(),
                internal::TypeID<ElementType>::get(),
                reinterpret_cast<internal::GenericFunction>(&internal::MemberAccess<ClassType, ElementType>::template propertyGet<ElementType(const ClassType&)>),
                sizeof(getter),
                &getter,
                reinterpret_cast<internal::GenericFunction>(&internal::MemberAccess<ClassType, ElementType>::template propertySet<void(ClassType&, ElementType)>),
                sizeof(setter),
                &setter);
            return *this;
        }

        template<typename ElementType>
        value_tuple& element(ElementType (*getter)(const ClassType&), void (*setter)(ClassType&, const ElementType&)) {
            internal::_embind_register_tuple_element_accessor(
                internal::TypeID<ClassType>::get(),
                internal::TypeID<ElementType>::get(),
                reinterpret_cast<internal::GenericFunction>(&internal::MemberAccess<ClassType, ElementType>::template propertyGet<ElementType(const ClassType&)>),
                sizeof(getter),
                &getter,
                reinterpret_cast<internal::GenericFunction>(&internal::MemberAccess<ClassType, ElementType>::template propertySet<void(ClassType&, ElementType)>),
                sizeof(setter),
                &setter);
            return *this;
        }

        template<typename ElementType>
        value_tuple& element(ElementType (*getter)(const ClassType&), void (*setter)(ClassType&, const ElementType&&)) {
            internal::_embind_register_tuple_element_accessor(
                internal::TypeID<ClassType>::get(),
                internal::TypeID<ElementType>::get(),
                reinterpret_cast<internal::GenericFunction>(&internal::MemberAccess<ClassType, ElementType>::template propertyGet<ElementType(const ClassType&)>),
                sizeof(getter),
                &getter,
                reinterpret_cast<internal::GenericFunction>(&internal::MemberAccess<ClassType, ElementType>::template propertySet<void(ClassType&, ElementType)>),
                sizeof(setter),
                &setter);
            return *this;
        }

        template<typename ElementType>
        value_tuple& element(ElementType (*getter)(const ClassType&), void (*setter)(ClassType&, ElementType&)) {
            internal::_embind_register_tuple_element_accessor(
                internal::TypeID<ClassType>::get(),
                internal::TypeID<ElementType>::get(),
                reinterpret_cast<internal::GenericFunction>(&internal::MemberAccess<ClassType, ElementType>::template propertyGet<ElementType(const ClassType&)>),
                sizeof(getter),
                &getter,
                reinterpret_cast<internal::GenericFunction>(&internal::MemberAccess<ClassType, ElementType>::template propertySet<void(ClassType&, ElementType)>),
                sizeof(setter),
                &setter);
            return *this;
        }
    };

    ////////////////////////////////////////////////////////////////////////////////
    // VALUE STRUCTS
    ////////////////////////////////////////////////////////////////////////////////

    template<typename ClassType>
    class value_struct {
    public:
        value_struct(const char* name) {
            internal::_embind_register_struct(
                internal::TypeID<ClassType>::get(),
                name,
                reinterpret_cast<internal::GenericFunction>(&internal::raw_constructor<ClassType>),
                reinterpret_cast<internal::GenericFunction>(&internal::raw_destructor<ClassType>));
        }

        template<typename FieldType>
        value_struct& field(const char* fieldName, FieldType ClassType::*field) {
            internal::_embind_register_struct_field(
                internal::TypeID<ClassType>::get(),
                fieldName,
                internal::TypeID<FieldType>::get(),
                reinterpret_cast<internal::GenericFunction>(&internal::MemberAccess<ClassType, FieldType>::get),
                reinterpret_cast<internal::GenericFunction>(&internal::MemberAccess<ClassType, FieldType>::set),
                sizeof(field),
                &field);
                                
            return *this;
        }
    };

    ////////////////////////////////////////////////////////////////////////////////
    // SMART POINTERS
    ////////////////////////////////////////////////////////////////////////////////

    template<typename PointerType>
    struct default_smart_ptr_trait {
        static sharing_policy get_sharing_policy() {
            return sharing_policy::NONE;
        }

        static void* share(void* v) {
            return 0; // no sharing
        }
    };

    // specialize if you have a different pointer type
    template<typename PointerType>
    struct smart_ptr_trait : public default_smart_ptr_trait<PointerType> {
        typedef typename PointerType::element_type element_type;

        static element_type* get(const PointerType& ptr) {
            return ptr.get();
        }
    };

    template<typename PointeeType>
    struct smart_ptr_trait<std::shared_ptr<PointeeType>> {
        typedef std::shared_ptr<PointeeType> PointerType;
        typedef typename PointerType::element_type element_type;

        static element_type* get(const PointerType& ptr) {
            return ptr.get();
        }

        static sharing_policy get_sharing_policy() {
            return sharing_policy::BY_EMVAL;
        }

        static std::shared_ptr<PointeeType>* share(PointeeType* p, internal::EM_VAL v) {
            return new std::shared_ptr<PointeeType>(
                p,
                val_deleter(val::take_ownership(v)));
        }

    private:
        class val_deleter {
        public:
            val_deleter() = delete;
            explicit val_deleter(val v)
                : v(v)
            {}
            void operator()(void const*) {
                v();
                // eventually we'll need to support emptied out val
                v = val::undefined();
            }
        private:
            val v;
        };
    };

    ////////////////////////////////////////////////////////////////////////////////
    // CLASSES
    ////////////////////////////////////////////////////////////////////////////////

    // abstract classes
    template<typename T>
    class wrapper : public T {
    public:
        wrapper(const val& wrapped)
            : wrapped(wrapped)
        {}

        template<typename ReturnType, typename... Args>
        ReturnType call(const char* name, Args... args) const {
            return Caller<ReturnType, Args...>::call(wrapped, name, args...);
        }

    private:
        // this class only exists because you can't partially specialize function templates
        template<typename ReturnType, typename... Args>
        struct Caller {
            static ReturnType call(const val& v, const char* name, Args... args) {
                return v.call(name, args...).template as<ReturnType>();
            }
        };

        template<typename... Args>
        struct Caller<void, Args...> {
            static void call(const val& v, const char* name, Args... args) {
                v.call_void(name, args...);
            }
        };

        val wrapped;
    };

#define EMSCRIPTEN_WRAPPER(T) \
    T(const ::emscripten::val& v): wrapper(v) {}

    namespace internal {
        struct NoBaseClass {
            template<typename ClassType>
            static void verify() {
            }

            static TYPEID get() {
                return nullptr;
            }

            template<typename ClassType>
            static GenericFunction getUpcaster() {
                return nullptr;
            }

            template<typename ClassType>
            static GenericFunction getDowncaster() {
                return nullptr;
            }
        };
    }

    template<typename BaseClass>
    struct base {
        template<typename ClassType>
        static void verify() {
            static_assert(!std::is_same<ClassType, BaseClass>::value, "Base must not have same type as class");
            static_assert(std::is_base_of<BaseClass, ClassType>::value, "Derived class must derive from base");
        }

        static internal::TYPEID get() {
            return internal::TypeID<BaseClass>::get();
        }
        
        template<typename ClassType>
        static internal::GenericFunction getUpcaster() {
            return reinterpret_cast<internal::GenericFunction>(&convertPointer<ClassType, BaseClass>);
        }
        
        template<typename ClassType>
        static internal::GenericFunction getDowncaster() {
            return reinterpret_cast<internal::GenericFunction>(&convertPointer<BaseClass, ClassType>);
        }

        template<typename From, typename To>
        static To* convertPointer(From* ptr) {
            return static_cast<To*>(ptr);
        }
    };

    template<typename PointerType>
    struct ptr {
        typedef PointerType pointer_type;
    };

    namespace internal {
        template<typename T>
        struct is_ptr {
            enum { value = false };
        };

        template<typename T>
        struct is_ptr<ptr<T>> {
            enum { value = true };
        };
    };

    template<typename ClassType, typename BaseSpecifier = internal::NoBaseClass>
    class class_ {
    public:
        class_() = delete;

        template<typename = typename std::enable_if<!internal::is_ptr<ClassType>::value>::type>
        explicit class_(const char* name) {
            using namespace internal;

            BaseSpecifier::template verify<ClassType>();

            _embind_register_class(
                TypeID<ClassType>::get(),
                TypeID<AllowedRawPointer<ClassType>>::get(),
                TypeID<AllowedRawPointer<const ClassType>>::get(),
                BaseSpecifier::get(),
                BaseSpecifier::template getUpcaster<ClassType>(),
                BaseSpecifier::template getDowncaster<ClassType>(),
                std::is_polymorphic<ClassType>::value,
                name,
                reinterpret_cast<GenericFunction>(&raw_destructor<ClassType>));
        }

        template<typename PointerType>
        class_& smart_ptr() {
            using namespace internal;

            typedef smart_ptr_trait<PointerType> PointerTrait;
            typedef typename PointerTrait::element_type PointeeType;
            
            static_assert(std::is_same<ClassType, typename std::remove_cv<PointeeType>::type>::value, "smart pointer must point to this class");

            _embind_register_smart_ptr(
                TypeID<PointerType>::get(),
                TypeID<PointeeType>::get(),
                typeid(PointerType).name(),
                PointerTrait::get_sharing_policy(),
                reinterpret_cast<GenericFunction>(&PointerTrait::get),
                reinterpret_cast<GenericFunction>(&operator_new<PointerType>),
                reinterpret_cast<GenericFunction>(&PointerTrait::share),
                reinterpret_cast<GenericFunction>(&raw_destructor<PointerType>));
            return *this;
        };

        template<typename... ConstructorArgs, typename... Policies>
        class_& constructor(Policies... policies) {
            return constructor(
                &internal::operator_new<ClassType, ConstructorArgs...>,
                policies...);
        }

        template<typename... Args, typename... Policies>
        class_& constructor(ClassType* (*factory)(Args...), Policies...) {
            using namespace internal;

            typename WithPolicies<Policies...>::template ArgTypeList<AllowedRawPointer<ClassType>, Args...> args;
            _embind_register_class_constructor(
                TypeID<ClassType>::get(),
                args.count,
                args.types,
                reinterpret_cast<GenericFunction>(&Invoker<ClassType*, Args...>::invoke),
                reinterpret_cast<GenericFunction>(factory));
            return *this;
        }

        template<typename SmartPtr, typename... Args, typename... Policies>
        class_& smart_ptr_constructor(SmartPtr (*factory)(Args...), Policies...) {
            using namespace internal;

            smart_ptr<SmartPtr>();

            typename WithPolicies<Policies...>::template ArgTypeList<SmartPtr, Args...> args;
            _embind_register_class_constructor(
                TypeID<ClassType>::get(),
                args.count,
                args.types,
                reinterpret_cast<GenericFunction>(&Invoker<SmartPtr, Args...>::invoke),
                reinterpret_cast<GenericFunction>(factory));
            return *this;
        }

        template<typename WrapperType>
        class_& allow_subclass() {
            using namespace internal;

            // TODO: unique or anonymous name
            class_<WrapperType, base<ClassType>>("WrapperType")
                .template constructor<val>()
                ;

            return class_function(
                "implement",
                &operator_new<WrapperType, val>,
                allow_raw_pointer<ret_val>());
        }

        template<typename ReturnType, typename... Args, typename... Policies>
        class_& function(const char* methodName, ReturnType (ClassType::*memberFunction)(Args...), Policies...) {
            using namespace internal;

            typename WithPolicies<Policies...>::template ArgTypeList<ReturnType, Args...> args;
            _embind_register_class_function(
                TypeID<ClassType>::get(),
                methodName,
                args.count,
                args.types,
                false,
                reinterpret_cast<GenericFunction>(&MethodInvoker<ClassType, ReturnType, Args...>::invoke),
                sizeof(memberFunction),
                &memberFunction);
            return *this;
        }

        template<typename ReturnType, typename... Args, typename... Policies>
        class_& function(const char* methodName, ReturnType (ClassType::*memberFunction)(Args...) const, Policies...) {
            using namespace internal;

            typename WithPolicies<Policies...>::template ArgTypeList<ReturnType, Args...> args;
            _embind_register_class_function(
                TypeID<ClassType>::get(),
                methodName,
                args.count,
                args.types,
                true,
                reinterpret_cast<GenericFunction>(&ConstMethodInvoker<ClassType, ReturnType, Args...>::invoke),
                sizeof(memberFunction),
                &memberFunction);
            return *this;
        }

        template<typename ReturnType, typename... Args, typename... Policies>
        class_& function(const char* methodName, ReturnType (*function)(ClassType& ptr, Args...), Policies...) {
            using namespace internal;

            typename WithPolicies<Policies...>::template ArgTypeList<ReturnType, Args...> args;
            _embind_register_class_function(
                TypeID<ClassType>::get(),
                methodName,
                args.count,
                args.types,
                false,
                reinterpret_cast<GenericFunction>(&FunctionInvoker<ClassType, ReturnType, Args...>::invoke),
                sizeof(function),
                &function);
            return *this;
        }

        template<typename ReturnType, typename... Args, typename... Policies>
        class_& function(const char* methodName, ReturnType (*function)(const ClassType& ptr, Args...), Policies...) {
            using namespace internal;

            typename WithPolicies<Policies...>::template ArgTypeList<ReturnType, Args...> args;
            _embind_register_class_function(
                TypeID<ClassType>::get(),
                methodName,
                args.count,
                args.types,
                true,
                reinterpret_cast<GenericFunction>(&FunctionInvoker<ClassType, ReturnType, Args...>::invoke),
                sizeof(function),
                &function);
            return *this;
        }

        template<typename FieldType>
        class_& property(const char* fieldName, FieldType ClassType::*field) {
            using namespace internal;

            _embind_register_class_property(
                TypeID<ClassType>::get(),
                fieldName,
                TypeID<FieldType>::get(),
                reinterpret_cast<GenericFunction>(&MemberAccess<ClassType, FieldType>::get),
                reinterpret_cast<GenericFunction>(&MemberAccess<ClassType, FieldType>::set),
                sizeof(field),
                &field);
            return *this;
        }

        template<typename ReturnType, typename... Args, typename... Policies>
        class_& class_function(const char* methodName, ReturnType (*classMethod)(Args...), Policies...) {
            using namespace internal;

            typename WithPolicies<Policies...>::template ArgTypeList<ReturnType, Args...> args;
            _embind_register_class_class_function(
                TypeID<ClassType>::get(),
                methodName,
                args.count,
                args.types,
                reinterpret_cast<internal::GenericFunction>(&internal::Invoker<ReturnType, Args...>::invoke),
                reinterpret_cast<GenericFunction>(classMethod));
            return *this;
        }

        template<typename ReturnType, typename... Args, typename... Policies>
        class_& calloperator(const char* methodName, Policies... policies) {
            return function(methodName, &ClassType::operator(), policies...);
        }
    };

    ////////////////////////////////////////////////////////////////////////////////
    // VECTORS
    ////////////////////////////////////////////////////////////////////////////////

    namespace internal {
        template<typename VectorType>
        struct VectorAccess {
            static val get(
                const VectorType& v,
                typename VectorType::size_type index
            ) {
                if (index < v.size()) {
                    return val(v[index]);
                } else {
                    return val::undefined();
                }
            }

            static bool set(
                VectorType& v,
                typename VectorType::size_type index,
                const typename VectorType::value_type& value
            ) {
                v[index] = value;
                return true;
            }
        };
    }

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

    ////////////////////////////////////////////////////////////////////////////////
    // MAPS
    ////////////////////////////////////////////////////////////////////////////////

    namespace internal {
        template<typename MapType>
        struct MapAccess {
            static val get(
                const MapType& m,
                const typename MapType::key_type& k
            ) {
                auto i = m.find(k);
                if (i == m.end()) {
                    return val::undefined();
                } else {
                    return val(i->second);
                }
            }

            static void set(
                MapType& m,
                const typename MapType::key_type& k,
                const typename MapType::mapped_type& v
            ) {
                m[k] = v;
            }
        };
    }

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


    ////////////////////////////////////////////////////////////////////////////////
    // ENUMS
    ////////////////////////////////////////////////////////////////////////////////

    template<typename EnumType>
    class enum_ {
    public:
        enum_(const char* name) {
            _embind_register_enum(
                internal::TypeID<EnumType>::get(),
                name);
        }

        enum_& value(const char* name, EnumType value) {
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

    namespace internal {
        template<typename T>
        class optional {
        public:            
            optional()
                : initialized(false)
            {}

            ~optional() {
                if (initialized) {
                    get()->~T();
                }
            }

            optional(const optional& rhs)
                : initialized(false)
            {
                *this = rhs;
            }

            T& operator*() {
                assert(initialized);
                return *get();
            }

            const T& operator*() const {
                assert(initialized);
                return *get();
            }

            explicit operator bool() const {
                return initialized;
            }

            optional& operator=(const T& v) {
                if (initialized) {
                    get()->~T();
                }
                new(get()) T(v);
                initialized = true;
                return *this;
            }

            optional& operator=(const optional& o) {
                if (initialized) {
                    get()->~T();
                }
                if (o.initialized) {
                    new(get()) T(*o);
                }
                initialized = o.initialized;
                return *this;
            }

        private:
            T* get() {
                return reinterpret_cast<T*>(&data);
            }

            T const* get() const {
                return reinterpret_cast<T const*>(&data);
            }

            bool initialized;
            typename std::aligned_storage<sizeof(T)>::type data;
        };
    }

    ////////////////////////////////////////////////////////////////////////////////
    // NEW INTERFACE
    ////////////////////////////////////////////////////////////////////////////////

    class JSInterface {
    public:
        JSInterface(internal::EM_VAL handle) {
            initialize(handle);
        }

        JSInterface(const JSInterface& obj)
            : jsobj(obj.jsobj)
        {}

        template<typename ReturnType, typename... Args>
        ReturnType call(const char* name, Args... args) {
            assertInitialized();
            return Caller<ReturnType, Args...>::call(*jsobj, name, args...);
        }

        static std::shared_ptr<JSInterface> cloneToSharedPtr(const JSInterface& i) {
            return std::make_shared<JSInterface>(i);
        }

    private:
        void initialize(internal::EM_VAL handle) {
            if (jsobj) {
                internal::_embind_fatal_error(
                    "Cannot initialize interface wrapper twice",
                    "JSInterface");
            }
            jsobj = val::take_ownership(handle);
        }

        // this class only exists because you can't partially specialize function templates
        template<typename ReturnType, typename... Args>
        struct Caller {
            static ReturnType call(val& v, const char* name, Args... args) {
                return v.call(name, args...).template as<ReturnType>();
            }
        };

        template<typename... Args>
        struct Caller<void, Args...> {
            static void call(val& v, const char* name, Args... args) {
                v.call_void(name, args...);
            }
        };

        void assertInitialized() {
            if (!jsobj) {
                internal::_embind_fatal_error(
                    "Cannot invoke call on uninitialized Javascript interface wrapper.", "JSInterface");
            }
        }

        internal::optional<val> jsobj;
    };

    namespace internal {
        extern JSInterface* create_js_interface(EM_VAL e);
    }

    class register_js_interface {
    public:
        register_js_interface() {
            _embind_register_interface(
                internal::TypeID<JSInterface>::get(),
                "JSInterface",
                reinterpret_cast<internal::GenericFunction>(&internal::create_js_interface),
                reinterpret_cast<internal::GenericFunction>(&internal::raw_destructor<JSInterface>));
        }
    };
}

#define EMSCRIPTEN_BINDINGS(fn) static emscripten::internal::BindingsDefinition anon_symbol(fn);
