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

        typedef void (*GenericFunction)();

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
                size_t size,
                bool trueValue,
                bool falseValue);

            void _embind_register_integer(
                TYPEID integerType,
                const char* name,
                size_t size,
                long minRange,
                unsigned long maxRange);

            void _embind_register_float(
                TYPEID floatType,
                const char* name,
                size_t size);
            
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
                unsigned typedArrayIndex,
                const char* name);

            void _embind_register_function(
                const char* name,
                unsigned argCount,
                const TYPEID argTypes[],
                const char* signature,
                GenericFunction invoker,
                GenericFunction function);

            void _embind_register_value_array(
                TYPEID tupleType,
                const char* name,
                const char* constructorSignature,
                GenericFunction constructor,
                const char* destructorSignature,
                GenericFunction destructor);
            
            void _embind_register_value_array_element(
                TYPEID tupleType,
                TYPEID getterReturnType,
                const char* getterSignature,
                GenericFunction getter,
                void* getterContext,
                TYPEID setterArgumentType,
                const char* setterSignature,
                GenericFunction setter,
                void* setterContext);

            void _embind_finalize_value_array(TYPEID tupleType);

            void _embind_register_value_object(
                TYPEID structType,
                const char* fieldName,
                const char* constructorSignature,
                GenericFunction constructor,
                const char* destructorSignature,
                GenericFunction destructor);
            
            void _embind_register_value_object_field(
                TYPEID structType,
                const char* fieldName,
                TYPEID getterReturnType,
                const char* getterSignature,
                GenericFunction getter,
                void* getterContext,
                TYPEID setterArgumentType,
                const char* setterSignature,
                GenericFunction setter,
                void* setterContext);

            void _embind_finalize_value_object(TYPEID structType);

            void _embind_register_class(
                TYPEID classType,
                TYPEID pointerType,
                TYPEID constPointerType,
                TYPEID baseClassType,
                const char* getActualTypeSignature,
                GenericFunction getActualType,
                const char* upcastSignature,
                GenericFunction upcast,
                const char* downcastSignature,
                GenericFunction downcast,
                const char* className,
                const char* destructorSignature,
                GenericFunction destructor);

            void _embind_register_class_constructor(
                TYPEID classType,
                unsigned argCount,
                const TYPEID argTypes[],
                const char* invokerSignature,
                GenericFunction invoker,
                GenericFunction constructor);

            void _embind_register_class_function(
                TYPEID classType,
                const char* methodName,
                unsigned argCount,
                const TYPEID argTypes[],
                const char* invokerSignature,
                GenericFunction invoker,
                void* context,
                unsigned isPureVirtual);

            void _embind_register_class_property(
                TYPEID classType,
                const char* fieldName,
                TYPEID getterReturnType,
                const char* getterSignature,
                GenericFunction getter,
                void* getterContext,
                TYPEID setterArgumentType,
                const char* setterSignature,
                GenericFunction setter,
                void* setterContext);

            void _embind_register_class_class_function(
                TYPEID classType,
                const char* methodName,
                unsigned argCount,
                const TYPEID argTypes[],
                const char* invokerSignature,
                GenericFunction invoker,
                GenericFunction method);

            void _embind_register_class_class_property(
                TYPEID classType,
                const char* fieldName,
                TYPEID fieldType,
                const void* fieldContext,
                const char* getterSignature,
                GenericFunction getter,
                const char* setterSignature,
                GenericFunction setter);

            EM_VAL _embind_create_inheriting_constructor(
                const char* constructorName,
                TYPEID wrapperType,
                EM_VAL properties);

            void _embind_register_enum(
                TYPEID enumType,
                const char* name,
                size_t size,
                bool isSigned);

            void _embind_register_smart_ptr(
                TYPEID pointerType,
                TYPEID pointeeType,
                const char* pointerName,
                sharing_policy sharingPolicy,
                const char* getPointeeSignature,
                GenericFunction getPointee,
                const char* constructorSignature,
                GenericFunction constructor,
                const char* shareSignature,
                GenericFunction share,
                const char* destructorSignature,
                GenericFunction destructor);

            void _embind_register_enum_value(
                TYPEID enumType,
                const char* valueName,
                GenericEnumValue value);

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
    Signature* select_overload(Signature* fn) {
        return fn;
    }

    template<typename Signature, typename ClassType>
    auto select_overload(Signature (ClassType::*fn)) -> decltype(fn) {
        return fn;
    }

    template<typename ClassType, typename ReturnType, typename... Args>
    auto select_const(ReturnType (ClassType::*method)(Args...) const) -> decltype(method) {
        return method;
    }

    namespace internal {        
        // this should be in <type_traits>, but alas, it's not
        template<typename T> struct remove_class;
        template<typename C, typename R, typename... A>
        struct remove_class<R(C::*)(A...)> { using type = R(A...); };
        template<typename C, typename R, typename... A>
        struct remove_class<R(C::*)(A...) const> { using type = R(A...); };
        template<typename C, typename R, typename... A>
        struct remove_class<R(C::*)(A...) volatile> { using type = R(A...); };
        template<typename C, typename R, typename... A>
        struct remove_class<R(C::*)(A...) const volatile> { using type = R(A...); };

        template<typename LambdaType>
        using LambdaSignature = typename remove_class<
            decltype(&LambdaType::operator())
        >::type;
    }

    // requires captureless lambda because implicitly coerces to function pointer
    template<typename LambdaType>
    internal::LambdaSignature<LambdaType>* optional_override(const LambdaType& fp) {
        return fp;
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
    // SignatureCode, SignatureString
    ////////////////////////////////////////////////////////////////////////////////

    namespace internal {
        template<typename T>
        struct SignatureCode {};

        template<>
        struct SignatureCode<int> {
            static constexpr char get() {
                return 'i';
            }
        };

        template<>
        struct SignatureCode<void> {
            static constexpr char get() {
                return 'v';
            }
        };

        template<>
        struct SignatureCode<float> {
            static constexpr char get() {
                return 'f';
            }
        };

        template<>
        struct SignatureCode<double> {
            static constexpr char get() {
                return 'd';
            }
        };

        template<typename... Args>
        const char* getGenericSignature() {
            static constexpr char signature[] = { SignatureCode<Args>::get()..., 0 };
            return signature;
        }

        template<typename T> struct SignatureTranslator { using type = int; };
        template<> struct SignatureTranslator<void> { using type = void; };
        template<> struct SignatureTranslator<float> { using type = float; };
        template<> struct SignatureTranslator<double> { using type = double; };

        template<typename... Args>
        EMSCRIPTEN_ALWAYS_INLINE const char* getSpecificSignature() {
            return getGenericSignature<typename SignatureTranslator<Args>::type...>();
        }

        template<typename Return, typename... Args>
        EMSCRIPTEN_ALWAYS_INLINE const char* getSignature(Return (*)(Args...)) {
            return getSpecificSignature<Return, Args...>();
        }
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
        auto invoker = &Invoker<ReturnType, Args...>::invoke;
        _embind_register_function(
            name,
            args.getCount(),
            args.getTypes(),
            getSignature(invoker),
            reinterpret_cast<GenericFunction>(invoker),
            reinterpret_cast<GenericFunction>(fn));
    }

    namespace internal {
        template<typename ClassType, typename... Args>
        ClassType* operator_new(Args&&... args) {
            return new ClassType(std::forward<Args>(args)...);
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

        template<typename FieldType>
        struct GlobalAccess {
            typedef internal::BindingType<FieldType> MemberBinding;
            typedef typename MemberBinding::WireType WireType;

            static WireType get(FieldType* context) {
                return MemberBinding::toWireType(*context);
            }

            static void set(FieldType* context, WireType value) {
                *context = MemberBinding::fromWireType(value);
            }
        };

        // TODO: This could do a reinterpret-cast if sizeof(T) === sizeof(void*)
        template<typename T>
        inline T* getContext(const T& t) {
            // not a leak because this is called once per binding
            return new T(t);
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
    class value_array : public internal::noncopyable {
    public:
        typedef ClassType class_type;

        value_array(const char* name) {
            using namespace internal;

            auto constructor = &raw_constructor<ClassType>;
            auto destructor = &raw_destructor<ClassType>;
            _embind_register_value_array(
                TypeID<ClassType>::get(),
                name,
                getSignature(constructor),
                reinterpret_cast<GenericFunction>(constructor),
                getSignature(destructor),
                reinterpret_cast<GenericFunction>(destructor));
        }

        ~value_array() {
            using namespace internal;
            _embind_finalize_value_array(TypeID<ClassType>::get());
        }

        template<typename InstanceType, typename ElementType>
        value_array& element(ElementType InstanceType::*field) {
            using namespace internal;

            auto getter = &MemberAccess<InstanceType, ElementType>
                ::template getWire<ClassType>;
            auto setter = &MemberAccess<InstanceType, ElementType>
                ::template setWire<ClassType>;

            _embind_register_value_array_element(
                TypeID<ClassType>::get(),
                TypeID<ElementType>::get(),
                getSignature(getter),
                reinterpret_cast<GenericFunction>(getter),
                getContext(field),
                TypeID<ElementType>::get(),
                getSignature(setter),
                reinterpret_cast<GenericFunction>(setter),
                getContext(field));
            return *this;
        }

        template<typename Getter, typename Setter>
        value_array& element(Getter getter, Setter setter) {
            using namespace internal;
            typedef GetterPolicy<Getter> GP;
            typedef SetterPolicy<Setter> SP;

            auto g = &GP::template get<ClassType>;
            auto s = &SP::template set<ClassType>;

            _embind_register_value_array_element(
                TypeID<ClassType>::get(),
                TypeID<typename GP::ReturnType>::get(),
                getSignature(g),
                reinterpret_cast<GenericFunction>(g),
                GP::getContext(getter),
                TypeID<typename SP::ArgumentType>::get(),
                getSignature(s),
                reinterpret_cast<GenericFunction>(s),
                SP::getContext(setter));
            return *this;
        }

        template<int Index>
        value_array& element(index<Index>) {
            using namespace internal;
            ClassType* null = 0;
            typedef typename std::remove_reference<decltype((*null)[Index])>::type ElementType;
            auto getter = &internal::get_by_index<ClassType, ElementType>;
            auto setter = &internal::set_by_index<ClassType, ElementType>;

            _embind_register_value_array_element(
                TypeID<ClassType>::get(),
                TypeID<ElementType>::get(),
                getSignature(getter),
                reinterpret_cast<GenericFunction>(getter),
                reinterpret_cast<void*>(Index),
                TypeID<ElementType>::get(),
                getSignature(setter),
                reinterpret_cast<GenericFunction>(setter),
                reinterpret_cast<void*>(Index));
            return *this;
        }
    };

    ////////////////////////////////////////////////////////////////////////////////
    // VALUE STRUCTS
    ////////////////////////////////////////////////////////////////////////////////

    template<typename ClassType>
    class value_object : public internal::noncopyable {
    public:
        typedef ClassType class_type;

        value_object(const char* name) {
            using namespace internal;

            auto ctor = &raw_constructor<ClassType>;
            auto dtor = &raw_destructor<ClassType>;

            _embind_register_value_object(
                TypeID<ClassType>::get(),
                name,
                getSignature(ctor),
                reinterpret_cast<GenericFunction>(ctor),
                getSignature(dtor),
                reinterpret_cast<GenericFunction>(dtor));
        }

        ~value_object() {
            using namespace internal;
            _embind_finalize_value_object(internal::TypeID<ClassType>::get());
        }

        template<typename InstanceType, typename FieldType>
        value_object& field(const char* fieldName, FieldType InstanceType::*field) {
            using namespace internal;

            auto getter = &MemberAccess<InstanceType, FieldType>
                ::template getWire<ClassType>;
            auto setter = &MemberAccess<InstanceType, FieldType>
                ::template setWire<ClassType>;

            _embind_register_value_object_field(
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
    
        template<typename Getter, typename Setter>
        value_object& field(
            const char* fieldName,
            Getter getter,
            Setter setter
        ) {
            using namespace internal;
            typedef GetterPolicy<Getter> GP;
            typedef SetterPolicy<Setter> SP;

            auto g = &GP::template get<ClassType>;
            auto s = &SP::template set<ClassType>;

            _embind_register_value_object_field(
                TypeID<ClassType>::get(),
                fieldName,
                TypeID<typename GP::ReturnType>::get(),
                getSignature(g),
                reinterpret_cast<GenericFunction>(g),
                GP::getContext(getter),
                TypeID<typename SP::ArgumentType>::get(),
                getSignature(s),
                reinterpret_cast<GenericFunction>(s),
                SP::getContext(setter));
            return *this;
        }

        template<int Index>
        value_object& field(const char* fieldName, index<Index>) {
            using namespace internal;
            ClassType* null = 0;
            typedef typename std::remove_reference<decltype((*null)[Index])>::type ElementType;

            auto getter = &internal::get_by_index<ClassType, ElementType>;
            auto setter = &internal::set_by_index<ClassType, ElementType>;

            _embind_register_value_object_field(
                TypeID<ClassType>::get(),
                fieldName,
                TypeID<ElementType>::get(),
                getSignature(getter),
                reinterpret_cast<GenericFunction>(getter),
                reinterpret_cast<void*>(Index),
                TypeID<ElementType>::get(),
                getSignature(setter),
                reinterpret_cast<GenericFunction>(setter),
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

        static PointerType* construct_null() {
            return new PointerType;
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

        static PointerType* construct_null() {
            return new PointerType;
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

    namespace internal {
        class WrapperBase {
        public:
            void setNotifyJSOnDestruction(bool notify) {
                notifyJSOnDestruction = notify;
            }

        protected:
            bool notifyJSOnDestruction = false;
        };
    }

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

    private:
        val wrapped;
    };

#define EMSCRIPTEN_WRAPPER(T)                                           \
    template<typename... Args>                                          \
    T(::emscripten::val&& v, Args&&... args)                            \
        : wrapper(std::forward<::emscripten::val>(v), std::forward<Args>(args)...) \
    {}

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
            return getLightTypeID(*ptr);
        };
    }

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

    namespace internal {
        template<typename WrapperType>
        val wrapped_extend(const std::string& name, const val& properties) {
            return val::take_ownership(_embind_create_inheriting_constructor(
                name.c_str(),
                TypeID<WrapperType>::get(),
                properties.__get_handle()));
        }
    };

    struct pure_virtual {
        template<typename InputType, int Index>
        struct Transform {
            typedef InputType type;
        };
    };

    namespace internal {
        template<typename... Policies>
        struct isPureVirtual;

        template<typename... Rest>
        struct isPureVirtual<pure_virtual, Rest...> {
            static constexpr bool value = true;
        };

        template<typename T, typename... Rest>
        struct isPureVirtual<T, Rest...> {
            static constexpr bool value = isPureVirtual<Rest...>::value;
        };

        template<>
        struct isPureVirtual<> {
            static constexpr bool value = false;
        };
    }

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

        template<typename WrapperType, typename... ConstructorArgs>
        EMSCRIPTEN_ALWAYS_INLINE const class_& allow_subclass(
            const char* wrapperClassName,
            ::emscripten::constructor<ConstructorArgs...> = ::emscripten::constructor<>()
        ) const {
            using namespace internal;

            auto cls = class_<WrapperType, base<ClassType>>(wrapperClassName)
                .function("notifyOnDestruction", select_overload<void(WrapperType&)>([](WrapperType& wrapper) {
                    wrapper.setNotifyJSOnDestruction(true);
                }))
                ;

            return
                class_function(
                    "implement",
                    &wrapped_new<WrapperType*, WrapperType, val, ConstructorArgs...>,
                    allow_raw_pointer<ret_val>())
                .class_function(
                    "extend",
                    &wrapped_extend<WrapperType>)
                ;
        }

        template<typename WrapperType, typename PointerType, typename... ConstructorArgs>
        EMSCRIPTEN_ALWAYS_INLINE const class_& allow_subclass(
            const char* wrapperClassName,
            const char* pointerName,
            ::emscripten::constructor<ConstructorArgs...> = ::emscripten::constructor<>()
        ) const {
            using namespace internal;

            auto cls = class_<WrapperType, base<ClassType>>(wrapperClassName)
                .function("notifyOnDestruction", select_overload<void(WrapperType&)>([](WrapperType& wrapper) {
                    wrapper.setNotifyJSOnDestruction(true);
                }))
                .template smart_ptr<PointerType>(pointerName)
                ;

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
                reinterpret_cast<GenericFunction>(invoke),
                reinterpret_cast<GenericFunction>(classMethod));
            return *this;
        }

        template<typename FieldType>
        EMSCRIPTEN_ALWAYS_INLINE const class_& class_property(const char* name, const FieldType* field) const {
            using namespace internal;

            auto getter = &GlobalAccess<FieldType>::get;
            _embind_register_class_class_property(
                TypeID<ClassType>::get(),
                name,
                TypeID<FieldType>::get(),
                field,
                getSignature(getter),
                reinterpret_cast<GenericFunction>(getter),
                0,
                0);
            return *this;
        }

        template<typename FieldType>
        EMSCRIPTEN_ALWAYS_INLINE const class_& class_property(const char* name, FieldType* field) const {
            using namespace internal;

            auto getter = &GlobalAccess<FieldType>::get;
            auto setter = &GlobalAccess<FieldType>::set;
            _embind_register_class_class_property(
                TypeID<ClassType>::get(),
                name,
                TypeID<FieldType>::get(),
                field,
                getSignature(getter),
                reinterpret_cast<GenericFunction>(getter),
                getSignature(setter),
                reinterpret_cast<GenericFunction>(setter));
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
        void (VecType::*resize)(const size_t, const T&) = &VecType::resize;
        return class_<std::vector<T>>(name)
            .template constructor<>()
            .function("push_back", push_back)
            .function("resize", resize)
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
            asGenericValue(BT::toWireType(v)));
    }
}

#define EMSCRIPTEN_BINDINGS(name)                                       \
    static struct EmscriptenBindingInitializer_##name {                 \
        EmscriptenBindingInitializer_##name();                          \
    } EmscriptenBindingInitializer_##name##_instance;                   \
    EmscriptenBindingInitializer_##name::EmscriptenBindingInitializer_##name()
