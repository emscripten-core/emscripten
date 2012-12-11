#pragma once

#include <stddef.h>
#include <assert.h>
#include <string>
#include <type_traits>
#include <emscripten/val.h>
#include <emscripten/wire.h>

namespace emscripten {
    namespace internal {
        typedef void (*GenericFunction)();
        typedef long GenericEnumValue;

        // Implemented in JavaScript.  Don't call these directly.
        extern "C" {
            void _embind_fatal_error(
                const char* name,
                const char* payload) __attribute__((noreturn));

            void _embind_register_void(
                TypeID voidType,
                const char* name);

            void _embind_register_bool(
                TypeID boolType,
                const char* name,
                bool trueValue,
                bool falseValue);

            void _embind_register_integer(
                TypeID integerType,
                const char* name);

            void _embind_register_float(
                TypeID floatType,
                const char* name);
            
            void _embind_register_cstring(
                TypeID stringType,
                const char* name);

            void _embind_register_emval(
                TypeID emvalType,
                const char* name);

            void _embind_register_function(
                const char* name,
                TypeID returnType,
                unsigned argCount,
                TypeID argTypes[],
                GenericFunction invoker,
                GenericFunction function);

            void _embind_register_tuple(
                TypeID tupleType,
                const char* name,
                GenericFunction constructor,
                GenericFunction destructor);
            
            void _embind_register_tuple_element(
                TypeID tupleType,
                TypeID elementType,
                GenericFunction getter,
                GenericFunction setter,
                size_t memberPointerSize,
                void* memberPointer);

            void _embind_register_tuple_element_accessor(
                TypeID tupleType,
                TypeID elementType,
                GenericFunction staticGetter,
                size_t getterSize,
                void* getter,
                GenericFunction staticSetter,
                size_t setterSize,
                void* setter);

            void _embind_register_struct(
                TypeID structType,
                const char* name,
                GenericFunction constructor,
                GenericFunction destructor);
            
            void _embind_register_struct_field(
                TypeID structType,
                const char* name,
                TypeID fieldType,
                GenericFunction getter,
                GenericFunction setter,
                size_t memberPointerSize,
                void* memberPointer);

            void _embind_register_class(
                TypeID classType,
                const char* className,
                GenericFunction destructor);

            void _embind_register_class_constructor(
                TypeID classType,
                unsigned argCount,
                TypeID argTypes[],
                GenericFunction constructor);

            void _embind_register_class_method(
                TypeID classType,
                const char* methodName,
                TypeID returnType,
                unsigned argCount,
                TypeID argTypes[],
                GenericFunction invoker,
                size_t memberFunctionSize,
                void* memberFunction);

            void _embind_register_class_field(
                TypeID classType,
                const char* fieldName,
                TypeID fieldType,
                GenericFunction getter,
                GenericFunction setter,
                size_t memberPointerSize,
                void* memberPointer);

            void _embind_register_class_classmethod(
                TypeID classType,
                const char* methodName,
                TypeID returnType,
                unsigned argCount,
                TypeID argTypes[],
                GenericFunction method);

            void _embind_register_enum(
                TypeID enumType,
                const char* name);

            void _embind_register_enum_value(
                TypeID enumType,
                const char* valueName,
                GenericEnumValue value);

            void _embind_register_interface(
                TypeID interfaceType,
                const char* name,
                GenericFunction constructor,
                GenericFunction destructor);
        }

        extern void registerStandardTypes();

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
    namespace internal {
        template<typename ReturnType, typename... Args>
        struct Invoker {
            static typename internal::BindingType<ReturnType>::WireType invoke(
                ReturnType (fn)(Args...),
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
                void (fn)(Args...),
                typename internal::BindingType<Args>::WireType... args
            ) {
                return fn(
                    internal::BindingType<Args>::fromWireType(args)...
                );
            }
        };
    }

    template<typename ReturnType, typename... Args>
    void function(const char* name, ReturnType (fn)(Args...)) {
        internal::registerStandardTypes();

        internal::ArgTypeList<Args...> args;
        internal::_embind_register_function(
            name,
            internal::getTypeID<ReturnType>(),
            args.count,
            args.types,
            reinterpret_cast<internal::GenericFunction>(&internal::Invoker<ReturnType, Args...>::invoke),
            reinterpret_cast<internal::GenericFunction>(fn));
    }

    namespace internal {
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

        template<typename ClassType, typename FieldType>
        struct FieldAccess {
            typedef FieldType ClassType::*MemberPointer;
            typedef internal::BindingType<FieldType> FieldBinding;
            typedef typename FieldBinding::WireType WireType;
            
            static WireType get(
                ClassType& ptr,
                const MemberPointer& field
            ) {
                return FieldBinding::toWireType(ptr.*field);
            }
            
            static void set(
                ClassType& ptr,
                const MemberPointer& field,
                WireType value
            ) {
                ptr.*field = FieldBinding::fromWireType(value);
            }

            template<typename Getter>
            static WireType propertyGet(
                ClassType& ptr,
                const Getter& getter
            ) {
                return FieldBinding::toWireType(getter(ptr));
            }

            template<typename Setter>
            static void propertySet(
                ClassType& ptr,
                const Setter& setter,
                WireType value
            ) {
                setter(ptr, FieldBinding::fromWireType(value));
            }
        };
    }

    template<typename ClassType>
    class value_tuple {
    public:
        value_tuple(const char* name) {
            internal::registerStandardTypes();
            internal::_embind_register_tuple(
                internal::getTypeID<ClassType>(),
                name,
                reinterpret_cast<internal::GenericFunction>(&internal::raw_constructor<ClassType>),
                reinterpret_cast<internal::GenericFunction>(&internal::raw_destructor<ClassType>));
        }

        template<typename ElementType>
        value_tuple& element(ElementType ClassType::*field) {
            internal::_embind_register_tuple_element(
                internal::getTypeID<ClassType>(),
                internal::getTypeID<ElementType>(),
                reinterpret_cast<internal::GenericFunction>(&internal::FieldAccess<ClassType, ElementType>::get),
                reinterpret_cast<internal::GenericFunction>(&internal::FieldAccess<ClassType, ElementType>::set),
                sizeof(field),
                &field);
                                
            return *this;
        }

        template<typename ElementType>
        value_tuple& element(ElementType (*getter)(const ClassType&), void (*setter)(ClassType&, ElementType)) {
            internal::_embind_register_tuple_element_accessor(
                internal::getTypeID<ClassType>(),
                internal::getTypeID<ElementType>(),
                reinterpret_cast<internal::GenericFunction>(&internal::FieldAccess<ClassType, ElementType>::template propertyGet<ElementType(const ClassType&)>),
                sizeof(getter),
                &getter,
                reinterpret_cast<internal::GenericFunction>(&internal::FieldAccess<ClassType, ElementType>::template propertySet<void(ClassType&, ElementType)>),
                sizeof(setter),
                &setter);
            return *this;
        }

        template<typename ElementType>
        value_tuple& element(ElementType (*getter)(const ClassType&), void (*setter)(ClassType&, const ElementType&)) {
            internal::_embind_register_tuple_element_accessor(
                internal::getTypeID<ClassType>(),
                internal::getTypeID<ElementType>(),
                reinterpret_cast<internal::GenericFunction>(&internal::FieldAccess<ClassType, ElementType>::template propertyGet<ElementType(const ClassType&)>),
                sizeof(getter),
                &getter,
                reinterpret_cast<internal::GenericFunction>(&internal::FieldAccess<ClassType, ElementType>::template propertySet<void(ClassType&, ElementType)>),
                sizeof(setter),
                &setter);
            return *this;
        }

        template<typename ElementType>
        value_tuple& element(ElementType (*getter)(const ClassType&), void (*setter)(ClassType&, const ElementType&&)) {
            internal::_embind_register_tuple_element_accessor(
                internal::getTypeID<ClassType>(),
                internal::getTypeID<ElementType>(),
                reinterpret_cast<internal::GenericFunction>(&internal::FieldAccess<ClassType, ElementType>::template propertyGet<ElementType(const ClassType&)>),
                sizeof(getter),
                &getter,
                reinterpret_cast<internal::GenericFunction>(&internal::FieldAccess<ClassType, ElementType>::template propertySet<void(ClassType&, ElementType)>),
                sizeof(setter),
                &setter);
            return *this;
        }

        template<typename ElementType>
        value_tuple& element(ElementType (*getter)(const ClassType&), void (*setter)(ClassType&, ElementType&)) {
            internal::_embind_register_tuple_element_accessor(
                internal::getTypeID<ClassType>(),
                internal::getTypeID<ElementType>(),
                reinterpret_cast<internal::GenericFunction>(&internal::FieldAccess<ClassType, ElementType>::template propertyGet<ElementType(const ClassType&)>),
                sizeof(getter),
                &getter,
                reinterpret_cast<internal::GenericFunction>(&internal::FieldAccess<ClassType, ElementType>::template propertySet<void(ClassType&, ElementType)>),
                sizeof(setter),
                &setter);
            return *this;
        }
    };

    template<typename ClassType>
    class value_struct {
    public:
        value_struct(const char* name) {
            internal::registerStandardTypes();
            internal::_embind_register_struct(
                internal::getTypeID<ClassType>(),
                name,
                reinterpret_cast<internal::GenericFunction>(&internal::raw_constructor<ClassType>),
                reinterpret_cast<internal::GenericFunction>(&internal::raw_destructor<ClassType>));
        }

        template<typename FieldType>
        value_struct& field(const char* fieldName, FieldType ClassType::*field) {
            internal::_embind_register_struct_field(
                internal::getTypeID<ClassType>(),
                fieldName,
                internal::getTypeID<FieldType>(),
                reinterpret_cast<internal::GenericFunction>(&internal::FieldAccess<ClassType, FieldType>::get),
                reinterpret_cast<internal::GenericFunction>(&internal::FieldAccess<ClassType, FieldType>::set),
                sizeof(field),
                &field);
                                
            return *this;
        }
    };

    // TODO: support class definitions without constructors.
    // TODO: support external class constructors
    template<typename ClassType>
    class class_ {
    public:
        class_(const char* name) {
            internal::registerStandardTypes();
            internal::_embind_register_class(
                internal::getTypeID<ClassType>(),
                name,
                reinterpret_cast<internal::GenericFunction>(&internal::raw_destructor<ClassType>));
        }

        template<typename... ConstructorArgs>
        class_& constructor() {
            internal::ArgTypeList<ConstructorArgs...> args;
            internal::_embind_register_class_constructor(
                internal::getTypeID<ClassType>(),
                args.count,
                args.types,
                reinterpret_cast<internal::GenericFunction>(&internal::raw_constructor<ClassType, ConstructorArgs...>));
            return *this;
        }

        template<typename ReturnType, typename... Args>
        class_& method(const char* methodName, ReturnType (ClassType::*memberFunction)(Args...)) {
            internal::ArgTypeList<Args...> args;
            internal::_embind_register_class_method(
                internal::getTypeID<ClassType>(),
                methodName,
                internal::getTypeID<ReturnType>(),
                args.count,
                args.types,
                reinterpret_cast<internal::GenericFunction>(&internal::MethodInvoker<ClassType, ReturnType, Args...>::invoke),
                sizeof(memberFunction),
                &memberFunction);
            return *this;
        }

        template<typename ReturnType, typename... Args>
        class_& method(const char* methodName, ReturnType (ClassType::*memberFunction)(Args...) const) {
            internal::ArgTypeList<Args...> args;
            internal::_embind_register_class_method(
                internal::getTypeID<ClassType>(),
                methodName,
                internal::getTypeID<ReturnType>(),
                args.count,
                args.types,
                reinterpret_cast<internal::GenericFunction>(&internal::ConstMethodInvoker<ClassType, ReturnType, Args...>::invoke),
                sizeof(memberFunction),
                &memberFunction);
            return *this;
        }

        template<typename FieldType>
        class_& field(const char* fieldName, FieldType ClassType::*field) {
            internal::_embind_register_class_field(
                internal::getTypeID<ClassType>(),
                fieldName,
                internal::getTypeID<FieldType>(),
                reinterpret_cast<internal::GenericFunction>(&internal::FieldAccess<ClassType, FieldType>::get),
                reinterpret_cast<internal::GenericFunction>(&internal::FieldAccess<ClassType, FieldType>::set),
                sizeof(field),
                &field);
            return *this;
        }

        template<typename ReturnType, typename... Args>
        class_& classmethod(const char* methodName, ReturnType (*classMethod)(Args...)) {
            internal::ArgTypeList<Args...> args;
            internal::_embind_register_class_classmethod(
                internal::getTypeID<ClassType>(),
                methodName,
                internal::getTypeID<ReturnType>(),
                args.count,
                args.types,
                reinterpret_cast<internal::GenericFunction>(classMethod));
            return *this;
        }
    };

    template<typename EnumType>
    class enum_ {
    public:
        enum_(const char* name) {
            _embind_register_enum(
                internal::getTypeID<EnumType>(),
                name);
        }

        enum_& value(const char* name, EnumType value) {
            // TODO: there's still an issue here.
            // if EnumType is an unsigned long, then JS may receive it as a signed long
            static_assert(sizeof(value) <= sizeof(internal::GenericEnumValue), "enum type must fit in a GenericEnumValue");

            _embind_register_enum_value(
                internal::getTypeID<EnumType>(),
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

            optional(const optional&) = delete;

            T& operator*() {
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
            
        private:
            T* get() {
                return reinterpret_cast<T*>(&data);
            }

            bool initialized;
            typename std::aligned_storage<sizeof(T)>::type data;
        };
    }

    template<typename InterfaceType>
    class wrapper : public InterfaceType {
    public:
        // Not necessary in any example so far, but appeases a compiler warning.
        virtual ~wrapper() {}

        typedef InterfaceType interface;

        void initialize(internal::EM_VAL handle) {
            if (jsobj) {
                internal::_embind_fatal_error(
                    "Cannot initialize interface wrapper twice",
                    typeid(InterfaceType).name());
            }
            jsobj = val::take_ownership(handle);
        }

        template<typename ReturnType, typename... Args>
        ReturnType call(const char* name, Args... args) {
            assertInitialized();
            return Caller<ReturnType, Args...>::call(*jsobj, name, args...);
        }

    private:
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
                v.call(name, args...);
            }
        };

        void assertInitialized() {
            if (!jsobj) {
                internal::_embind_fatal_error(
                    "Cannot invoke call on uninitialized interface wrapper.",
                    typeid(InterfaceType).name());
            }
        }

        internal::optional<val> jsobj;
    };

    namespace internal {
        template<typename WrapperType>
        WrapperType* create_interface_wrapper(EM_VAL e) {
            WrapperType* p = new WrapperType;
            p->initialize(e);
            return p;
        }
    }

    template<typename WrapperType>
    class interface {
    public:
        typedef typename WrapperType::interface InterfaceType;

        interface(const char* name) {
            _embind_register_interface(
                internal::getTypeID<InterfaceType>(),
                name,
                reinterpret_cast<internal::GenericFunction>(&internal::create_interface_wrapper<WrapperType>),
                reinterpret_cast<internal::GenericFunction>(&internal::raw_destructor<WrapperType>));
        }
    };
}

#define EMSCRIPTEN_BINDINGS(fn) static emscripten::internal::BindingsDefinition anon_symbol(fn);
