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
                const char* name,
                long minRange,
                unsigned long maxRange);

            void _embind_register_float(
                TYPEID floatType,
                const char* name);
            
            void _embind_register_std_string(
                TYPEID stringType,
                const char* name);

            void _embind_register_std_wstring(
                TYPEID stringType,
                size_t charSize,
                const char* name);

            void _embind_register_emval(
                TYPEID emvalType,
                const char* name);

            void _embind_register_memory_view(
                TYPEID memoryViewType,
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
                TYPEID getterReturnType,
                GenericFunction getter,
                void* getterContext,
                TYPEID setterArgumentType,
                GenericFunction setter,
                void* setterContext);

            void _embind_finalize_tuple(TYPEID tupleType);

            void _embind_register_struct(
                TYPEID structType,
                const char* fieldName,
                GenericFunction constructor,
                GenericFunction destructor);
            
            void _embind_register_struct_field(
                TYPEID structType,
                const char* fieldName,
                TYPEID getterReturnType,
                GenericFunction getter,
                void* getterContext,
                TYPEID setterArgumentType,
                GenericFunction setter,
                void* setterContext);

            void _embind_finalize_struct(TYPEID structType);

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
                GenericFunction getActualType,
                GenericFunction upcast,
                GenericFunction downcast,
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
                GenericFunction invoker,
                void* context);

            void _embind_register_class_property(
                TYPEID classType,
                const char* fieldName,
                TYPEID getterReturnType,
                GenericFunction getter,
                void* getterContext,
                TYPEID setterArgumentType,
                GenericFunction setter,
                void* setterContext);

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

            void _embind_register_constant(
                const char* name,
                TYPEID constantType,
                uintptr_t value);
        }
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

    /*
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
    */

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

    // this is temporary until arg policies are reworked
    template<typename Slot>
    struct allow_raw_pointer : public allow_raw_pointers {
    };

    ////////////////////////////////////////////////////////////////////////////////
    // select_overload and select_const
    ////////////////////////////////////////////////////////////////////////////////

    template<typename Signature>
    typename std::add_pointer<Signature>::type select_overload(typename std::add_pointer<Signature>::type fn) {
        return fn;
    }

    namespace internal {
        template<typename ClassType, typename Signature>
        struct MemberFunctionType {
            typedef Signature (ClassType::*type);
        };
    }

    template<typename Signature, typename ClassType>
    typename internal::MemberFunctionType<ClassType, Signature>::type select_overload(Signature (ClassType::*fn)) {
        return fn;
    }

    template<typename ClassType, typename ReturnType, typename... Args>
    auto select_const(ReturnType (ClassType::*method)(Args...) const) -> decltype(method) {
        return method;
    }

    ////////////////////////////////////////////////////////////////////////////////
    // Invoker
    ////////////////////////////////////////////////////////////////////////////////

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

        template<typename WrapperType, typename ClassType, typename... Args>
        WrapperType wrapped_new(Args&&... args) {
            return WrapperType(new ClassType(std::forward<Args>(args)...));
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

        template<typename FunctionPointerType, typename ReturnType, typename ThisType, typename... Args>
        struct FunctionInvoker {
            static typename internal::BindingType<ReturnType>::WireType invoke(
                FunctionPointerType* function,
                typename internal::BindingType<ThisType>::WireType wireThis,
                typename internal::BindingType<Args>::WireType... args
            ) {
                return internal::BindingType<ReturnType>::toWireType(
                    (*function)(
                        internal::BindingType<ThisType>::fromWireType(wireThis),
                        internal::BindingType<Args>::fromWireType(args)...)
                );
            }
        };

        template<typename FunctionPointerType, typename ThisType, typename... Args>
        struct FunctionInvoker<FunctionPointerType, void, ThisType, Args...> {
            static void invoke(
                FunctionPointerType* function,
                typename internal::BindingType<ThisType>::WireType wireThis,
                typename internal::BindingType<Args>::WireType... args
            ) {
                (*function)(
                    internal::BindingType<ThisType>::fromWireType(wireThis),
                    internal::BindingType<Args>::fromWireType(args)...);
            }
        };

        template<typename MemberPointer,
                 typename ReturnType,
                 typename ThisType,
                 typename... Args>
        struct MethodInvoker {
            static typename internal::BindingType<ReturnType>::WireType invoke(
                const MemberPointer& method,
                typename internal::BindingType<ThisType>::WireType wireThis,
                typename internal::BindingType<Args>::WireType... args
            ) {
                return internal::BindingType<ReturnType>::toWireType(
                    (internal::BindingType<ThisType>::fromWireType(wireThis)->*method)(
                        internal::BindingType<Args>::fromWireType(args)...
                    )
                );
            }
        };

        template<typename MemberPointer,
                 typename ThisType,
                 typename... Args>
        struct MethodInvoker<MemberPointer, void, ThisType, Args...> {
            static void invoke(
                const MemberPointer& method,
                typename internal::BindingType<ThisType>::WireType wireThis,
                typename internal::BindingType<Args>::WireType... args
            ) {
                return (internal::BindingType<ThisType>::fromWireType(wireThis)->*method)(
                    internal::BindingType<Args>::fromWireType(args)...
                );
            }
        };

        template<typename InstanceType, typename MemberType>
        struct MemberAccess {
            typedef MemberType InstanceType::*MemberPointer;
            typedef internal::BindingType<MemberType> MemberBinding;
            typedef typename MemberBinding::WireType WireType;
            
            template<typename ClassType>
            static WireType getWire(
                const MemberPointer& field,
                const ClassType& ptr
            ) {
                return MemberBinding::toWireType(ptr.*field);
            }
            
            template<typename ClassType>
            static void setWire(
                const MemberPointer& field,
                ClassType& ptr,
                WireType value
            ) {
                ptr.*field = MemberBinding::fromWireType(value);
            }
        };

        // TODO: This could do a reinterpret-cast if sizeof(T) === sizeof(void*)
        template<typename T>
        inline T* getContext(const T& t) {
            // not a leak because this is called once per binding
            T* p = reinterpret_cast<T*>(malloc(sizeof(T)));
            new(p) T(t);
            return p;
        }

        template<typename T>
        struct GetterPolicy;

        template<typename GetterReturnType, typename GetterThisType>
        struct GetterPolicy<GetterReturnType (GetterThisType::*)() const> {
            typedef GetterReturnType ReturnType;
            typedef GetterReturnType (GetterThisType::*Context)() const;

            typedef internal::BindingType<ReturnType> Binding;
            typedef typename Binding::WireType WireType;

            template<typename ClassType>
            static WireType get(const Context& context, const ClassType& ptr) {
                return Binding::toWireType((ptr.*context)());
            }

            static void* getContext(Context context) {
                return internal::getContext(context);
            }
        };

        template<typename GetterReturnType, typename GetterThisType>
        struct GetterPolicy<GetterReturnType (*)(const GetterThisType&)> {
            typedef GetterReturnType ReturnType;
            typedef GetterReturnType (*Context)(const GetterThisType&);

            typedef internal::BindingType<ReturnType> Binding;
            typedef typename Binding::WireType WireType;

            template<typename ClassType>
            static WireType get(const Context& context, const ClassType& ptr) {
                return Binding::toWireType(context(ptr));
            }

            static void* getContext(Context context) {
                return internal::getContext(context);
            }
        };

        template<typename T>
        struct SetterPolicy;

        template<typename SetterThisType, typename SetterArgumentType>
        struct SetterPolicy<void (SetterThisType::*)(SetterArgumentType)> {
            typedef SetterArgumentType ArgumentType;
            typedef void (SetterThisType::*Context)(SetterArgumentType);

            typedef internal::BindingType<SetterArgumentType> Binding;
            typedef typename Binding::WireType WireType;

            template<typename ClassType>
            static void set(const Context& context, ClassType& ptr, WireType wt) {
                (ptr.*context)(Binding::fromWireType(wt));
            }

            static void* getContext(Context context) {
                return internal::getContext(context);
            }
        };

        template<typename SetterThisType, typename SetterArgumentType>
        struct SetterPolicy<void (*)(SetterThisType&, SetterArgumentType)> {
            typedef SetterArgumentType ArgumentType;
            typedef void (*Context)(SetterThisType&, SetterArgumentType);

            typedef internal::BindingType<SetterArgumentType> Binding;
            typedef typename Binding::WireType WireType;

            template<typename ClassType>
            static void set(const Context& context, ClassType& ptr, WireType wt) {
                context(ptr, Binding::fromWireType(wt));
            }

            static void* getContext(Context context) {
                return internal::getContext(context);
            }
        };

        class noncopyable {
        protected:
            noncopyable() {}
            ~noncopyable() {}
        private:
            noncopyable(const noncopyable&) = delete;
            const noncopyable& operator=(const noncopyable&) = delete;
        };

        template<typename ClassType, typename ElementType>
        typename BindingType<ElementType>::WireType get_by_index(int index, ClassType& ptr) {
            return BindingType<ElementType>::toWireType(ptr[index]);
        }

        template<typename ClassType, typename ElementType>
        void set_by_index(int index, ClassType& ptr, typename BindingType<ElementType>::WireType wt) {
            ptr[index] = BindingType<ElementType>::fromWireType(wt);
        }
    }

    template<int Index>
    struct index {
    };

    ////////////////////////////////////////////////////////////////////////////////
    // VALUE TUPLES
    ////////////////////////////////////////////////////////////////////////////////

    template<typename ClassType>
    class value_tuple : public internal::noncopyable {
    public:
        value_tuple(const char* name) {
            using namespace internal;
            _embind_register_tuple(
                TypeID<ClassType>::get(),
                name,
                reinterpret_cast<GenericFunction>(&raw_constructor<ClassType>),
                reinterpret_cast<GenericFunction>(&raw_destructor<ClassType>));
        }

        ~value_tuple() {
            using namespace internal;
            _embind_finalize_tuple(TypeID<ClassType>::get());
        }

        template<typename InstanceType, typename ElementType>
        value_tuple& element(ElementType InstanceType::*field) {
            using namespace internal;
            _embind_register_tuple_element(
                TypeID<ClassType>::get(),
                TypeID<ElementType>::get(),
                reinterpret_cast<GenericFunction>(
                    &MemberAccess<InstanceType, ElementType>
                    ::template getWire<ClassType>),
                getContext(field),
                TypeID<ElementType>::get(),
                reinterpret_cast<GenericFunction>(
                    &MemberAccess<InstanceType, ElementType>
                    ::template setWire<ClassType>),
                getContext(field));
            return *this;
        }

        template<typename Getter, typename Setter>
        value_tuple& element(Getter getter, Setter setter) {
            using namespace internal;
            typedef GetterPolicy<Getter> GP;
            typedef SetterPolicy<Setter> SP;
            _embind_register_tuple_element(
                TypeID<ClassType>::get(),
                TypeID<typename GP::ReturnType>::get(),
                reinterpret_cast<GenericFunction>(&GP::template get<ClassType>),
                GP::getContext(getter),
                TypeID<typename SP::ArgumentType>::get(),
                reinterpret_cast<GenericFunction>(&SP::template set<ClassType>),
                SP::getContext(setter));
            return *this;
        }

        template<int Index>
        value_tuple& element(index<Index>) {
            using namespace internal;
            ClassType* null = 0;
            typedef typename std::remove_reference<decltype((*null)[Index])>::type ElementType;
            _embind_register_tuple_element(
                TypeID<ClassType>::get(),
                TypeID<ElementType>::get(),
                reinterpret_cast<GenericFunction>(&internal::get_by_index<ClassType, ElementType>),
                reinterpret_cast<void*>(Index),
                TypeID<ElementType>::get(),
                reinterpret_cast<GenericFunction>(&internal::set_by_index<ClassType, ElementType>),
                reinterpret_cast<void*>(Index));
            return *this;
        }
    };

    ////////////////////////////////////////////////////////////////////////////////
    // VALUE STRUCTS
    ////////////////////////////////////////////////////////////////////////////////

    template<typename ClassType>
    class value_struct : public internal::noncopyable {
    public:
        value_struct(const char* name) {
            using namespace internal;
            _embind_register_struct(
                TypeID<ClassType>::get(),
                name,
                reinterpret_cast<GenericFunction>(&raw_constructor<ClassType>),
                reinterpret_cast<GenericFunction>(&raw_destructor<ClassType>));
        }

        ~value_struct() {
            _embind_finalize_struct(internal::TypeID<ClassType>::get());
        }

        template<typename InstanceType, typename FieldType>
        value_struct& field(const char* fieldName, FieldType InstanceType::*field) {
            using namespace internal;
            _embind_register_struct_field(
                TypeID<ClassType>::get(),
                fieldName,
                TypeID<FieldType>::get(),
                reinterpret_cast<GenericFunction>(
                    &MemberAccess<InstanceType, FieldType>
                    ::template getWire<ClassType>),
                getContext(field),
                TypeID<FieldType>::get(),
                reinterpret_cast<GenericFunction>(
                    &MemberAccess<InstanceType, FieldType>
                    ::template setWire<ClassType>),
                getContext(field));
            return *this;
        }
    
        template<typename Getter, typename Setter>
        value_struct& field(
            const char* fieldName,
            Getter getter,
            Setter setter
        ) {
            using namespace internal;
            typedef GetterPolicy<Getter> GP;
            typedef SetterPolicy<Setter> SP;
            _embind_register_struct_field(
                TypeID<ClassType>::get(),
                fieldName,
                TypeID<typename GP::ReturnType>::get(),
                reinterpret_cast<GenericFunction>(&GP::template get<ClassType>),
                GP::getContext(getter),
                TypeID<typename SP::ArgumentType>::get(),
                reinterpret_cast<GenericFunction>(&SP::template set<ClassType>),
                SP::getContext(setter));
            return *this;
        }

        template<int Index>
        value_struct& field(const char* fieldName, index<Index>) {
            using namespace internal;
            ClassType* null = 0;
            typedef typename std::remove_reference<decltype((*null)[Index])>::type ElementType;
            _embind_register_struct_field(
                TypeID<ClassType>::get(),
                fieldName,
                TypeID<ElementType>::get(),
                reinterpret_cast<GenericFunction>(&internal::get_by_index<ClassType, ElementType>),
                reinterpret_cast<void*>(Index),
                TypeID<ElementType>::get(),
                reinterpret_cast<GenericFunction>(&internal::set_by_index<ClassType, ElementType>),
                reinterpret_cast<void*>(Index));
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
        explicit wrapper(val&& wrapped)
            : wrapped(std::forward<val>(wrapped))
        {}

        template<typename ReturnType, typename... Args>
        ReturnType call(const char* name, Args&&... args) const {
            return wrapped.call<ReturnType>(name, std::forward<Args>(args)...);
        }

        template<typename ReturnType, typename... Args, typename Default>
        ReturnType optional_call(const char* name, Default def, Args&&... args) const {
            if (wrapped.has_function(name)) {
                return call<ReturnType>(name, std::forward<Args>(args)...);
            } else {
                return def();
            }
        }

    private:
        val wrapped;
    };

#define EMSCRIPTEN_WRAPPER(T) \
    T(::emscripten::val&& v): wrapper(std::forward<::emscripten::val>(v)) {}

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

        // NOTE: this returns the class type, not the pointer type
        template<typename T>
        inline TYPEID getActualType(T* ptr) {
            assert(ptr);
            return reinterpret_cast<TYPEID>(&typeid(*ptr));
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

    namespace internal {
        template<typename T>
        struct SmartPtrIfNeeded {
            template<typename U>
            SmartPtrIfNeeded(U& cls) {
                cls.template smart_ptr<T>();
            }
        };

        template<typename T>
        struct SmartPtrIfNeeded<T*> {
            template<typename U>
            SmartPtrIfNeeded(U&) {
            }
        };
    };

    template<typename ClassType, typename BaseSpecifier = internal::NoBaseClass>
    class class_ {
    public:
        class_() = delete;

        explicit class_(const char* name) {
            using namespace internal;

            BaseSpecifier::template verify<ClassType>();

            _embind_register_class(
                TypeID<ClassType>::get(),
                TypeID<AllowedRawPointer<ClassType>>::get(),
                TypeID<AllowedRawPointer<const ClassType>>::get(),
                BaseSpecifier::get(),
                reinterpret_cast<GenericFunction>(&getActualType<ClassType>),
                BaseSpecifier::template getUpcaster<ClassType>(),
                BaseSpecifier::template getDowncaster<ClassType>(),
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

        template<typename... Args, typename ReturnType, typename... Policies>
        class_& constructor(ReturnType (*factory)(Args...), Policies...) {
            using namespace internal;

            // TODO: allows all raw pointers... policies need a rethink
            typename WithPolicies<allow_raw_pointers, Policies...>::template ArgTypeList<ReturnType, Args...> args;
            _embind_register_class_constructor(
                TypeID<ClassType>::get(),
                args.count,
                args.types,
                reinterpret_cast<GenericFunction>(&Invoker<ReturnType, Args...>::invoke),
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

        template<typename WrapperType, typename PointerType = WrapperType*>
        class_& allow_subclass() {
            using namespace internal;

            auto cls = class_<WrapperType, base<ClassType>>(typeid(WrapperType).name())
                ;
            SmartPtrIfNeeded<PointerType> _(cls);

            return class_function(
                "implement",
                &wrapped_new<PointerType, WrapperType, val>,
                allow_raw_pointer<ret_val>());
        }

        template<typename ReturnType, typename... Args, typename... Policies>
        class_& function(const char* methodName, ReturnType (ClassType::*memberFunction)(Args...), Policies...) {
            using namespace internal;

            typename WithPolicies<Policies...>::template ArgTypeList<ReturnType, AllowedRawPointer<ClassType>, Args...> args;
            _embind_register_class_function(
                TypeID<ClassType>::get(),
                methodName,
                args.count,
                args.types,
                reinterpret_cast<GenericFunction>(&MethodInvoker<decltype(memberFunction), ReturnType, ClassType*, Args...>::invoke),
                getContext(memberFunction));
            return *this;
        }

        template<typename ReturnType, typename... Args, typename... Policies>
        class_& function(const char* methodName, ReturnType (ClassType::*memberFunction)(Args...) const, Policies...) {
            using namespace internal;

            typename WithPolicies<Policies...>::template ArgTypeList<ReturnType, AllowedRawPointer<const ClassType>, Args...> args;
            _embind_register_class_function(
                TypeID<ClassType>::get(),
                methodName,
                args.count,
                args.types,
                reinterpret_cast<GenericFunction>(&MethodInvoker<decltype(memberFunction), ReturnType, const ClassType*, Args...>::invoke),
                getContext(memberFunction));
            return *this;
        }

        template<typename ReturnType, typename ThisType, typename... Args, typename... Policies>
        class_& function(const char* methodName, ReturnType (*function)(ThisType, Args...), Policies...) {
            using namespace internal;

            typename WithPolicies<Policies...>::template ArgTypeList<ReturnType, ThisType, Args...> args;
            _embind_register_class_function(
                TypeID<ClassType>::get(),
                methodName,
                args.count,
                args.types,
                reinterpret_cast<GenericFunction>(&FunctionInvoker<decltype(function), ReturnType, ThisType, Args...>::invoke),
                getContext(function));
            return *this;
        }

        template<typename FieldType, typename = typename std::enable_if<!std::is_function<FieldType>::value>::type>
        class_& property(const char* fieldName, const FieldType ClassType::*field) {
            using namespace internal;

            _embind_register_class_property(
                TypeID<ClassType>::get(),
                fieldName,
                TypeID<FieldType>::get(),
                reinterpret_cast<GenericFunction>(&MemberAccess<ClassType, FieldType>::template getWire<ClassType>),
                getContext(field),
                0,
                0,
                0);
            return *this;
        }

        template<typename FieldType, typename = typename std::enable_if<!std::is_function<FieldType>::value>::type>
        class_& property(const char* fieldName, FieldType ClassType::*field) {
            using namespace internal;

            _embind_register_class_property(
                TypeID<ClassType>::get(),
                fieldName,
                TypeID<FieldType>::get(),
                reinterpret_cast<GenericFunction>(&MemberAccess<ClassType, FieldType>::template getWire<ClassType>),
                getContext(field),
                TypeID<FieldType>::get(),
                reinterpret_cast<GenericFunction>(&MemberAccess<ClassType, FieldType>::template setWire<ClassType>),
                getContext(field));
            return *this;
        }

        template<typename Getter>
        class_& property(const char* fieldName, Getter getter) {
            using namespace internal;
            typedef GetterPolicy<Getter> GP;
            _embind_register_class_property(
                TypeID<ClassType>::get(),
                fieldName,
                TypeID<typename GP::ReturnType>::get(),
                reinterpret_cast<GenericFunction>(&GP::template get<ClassType>),
                GP::getContext(getter),
                0,
                0,
                0);
            return *this;
        }

        template<typename Getter, typename Setter>
        class_& property(const char* fieldName, Getter getter, Setter setter) {
            using namespace internal;
            typedef GetterPolicy<Getter> GP;
            typedef SetterPolicy<Setter> SP;
            _embind_register_class_property(
                TypeID<ClassType>::get(),
                fieldName,
                TypeID<typename GP::ReturnType>::get(),
                reinterpret_cast<GenericFunction>(&GP::template get<ClassType>),
                GP::getContext(getter),
                TypeID<typename SP::ArgumentType>::get(),
                reinterpret_cast<GenericFunction>(&SP::template set<ClassType>),
                SP::getContext(setter));
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

    ////////////////////////////////////////////////////////////////////////////////
    // CONSTANTS
    ////////////////////////////////////////////////////////////////////////////////

    namespace internal {
        template<typename T>
        uintptr_t asGenericValue(T t) {
            return static_cast<uintptr_t>(t);
        }

        template<typename T>
        uintptr_t asGenericValue(T* p) {
            return reinterpret_cast<uintptr_t>(p);
        }
    }

    template<typename ConstantType>
    void constant(const char* name, const ConstantType& v) {
        using namespace internal;
        typedef BindingType<const ConstantType&> BT;
        _embind_register_constant(
            name,
            TypeID<const ConstantType&>::get(),
            asGenericValue(BindingType<const ConstantType&>::toWireType(v)));
    }
}

#define EMSCRIPTEN_BINDINGS(name)                                       \
    static struct EmscriptenBindingInitializer_##name {                 \
        EmscriptenBindingInitializer_##name();                          \
    } EmscriptenBindingInitializer_##name##_instance;                   \
    EmscriptenBindingInitializer_##name::EmscriptenBindingInitializer_##name()
