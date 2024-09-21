/*
 * Copyright 2012 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#pragma once

#if __cplusplus < 201103L
#error Including <emscripten/bind.h> requires building with -std=c++11 or newer!
#endif

#include <cassert>
#include <cstddef>
#include <functional>
#include <map>
#include <string>
#include <type_traits>
#include <vector>
#if __cplusplus >= 201703L
#include <optional>
#endif

#include <emscripten/em_macros.h>
#include <emscripten/val.h>
#include <emscripten/wire.h>

#if __has_feature(leak_sanitizer) || __has_feature(address_sanitizer)
#include <sanitizer/lsan_interface.h>
#endif

namespace emscripten {

enum class sharing_policy {
    NONE = 0,
    INTRUSIVE = 1,
    BY_EMVAL = 2,
};

namespace internal {

typedef int GenericEnumValue;

typedef void* GenericFunction;
typedef void (*VoidFunctionPtr)(void);

// Implemented in JavaScript.  Don't call these directly.
extern "C" {

void _embind_fatal_error(
    const char* name,
    const char* payload) __attribute__((__noreturn__));

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
    size_t size,
    int32_t minRange,
    uint32_t maxRange);

void _embind_register_bigint(
    TYPEID integerType,
    const char* name,
    size_t size,
    int64_t minRange,
    uint64_t maxRange);

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
    TYPEID emvalType);

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
    GenericFunction function,
    bool isAsync,
    bool isNonnullReturn);

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
    unsigned isPureVirtual,
    bool isAsync,
    bool isNonnullReturn);

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
    GenericFunction method,
    bool isAsync,
    bool isNonnullReturn);

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
    double value);

void _embind_register_optional(
    TYPEID optionalType,
    TYPEID type);

void _embind_register_user_type(
    TYPEID type,
    const char* typeName);

// Register an InitFunc in the global linked list of init functions.
void _embind_register_bindings(struct InitFunc* f);

// Binding initialization functions registered by EMSCRIPTEN_BINDINGS macro
// below.  Stored as linked list of static data object avoiding std containers
// to avoid static constructor ordering issues.
struct InitFunc {
  InitFunc(void (*init_func)()) : init_func(init_func) {
    // This the function immediately upon constructions, and also register
    // it so that it can be called again on each worker that starts.
    init_func();
    _embind_register_bindings(this);
  }
  void (*init_func)();
  InitFunc* next = nullptr;
};

} // end extern "C"

} // end namespace internal

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

// allow all raw pointers
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

struct async {
    template<typename InputType, int Index>
    struct Transform {
        typedef InputType type;
    };
};

struct pure_virtual {
    template<typename InputType, int Index>
    struct Transform {
        typedef InputType type;
    };
};

template<typename Slot>
struct nonnull {
    static_assert(std::is_same<Slot, ret_val>::value, "Only nonnull return values are currently supported.");
    template<typename InputType, int Index>
    struct Transform {
        typedef InputType type;
    };
};

namespace return_value_policy {

struct take_ownership : public allow_raw_pointers {};
struct reference : public allow_raw_pointers {};

} // end namespace return_value_policy

namespace internal {

#if __cplusplus >= 201703L
template <typename... Args> using conjunction = std::conjunction<Args...>;
template <typename... Args> using disjunction = std::disjunction<Args...>;
#else
// Helper available in C++14.
template <bool _Test, class _T1, class _T2>
using conditional_t = typename std::conditional<_Test, _T1, _T2>::type;

template<class...> struct conjunction : std::true_type {};
template<class B1> struct conjunction<B1> : B1 {};
template<class B1, class... Bn>
struct conjunction<B1, Bn...>
    : conditional_t<bool(B1::value), conjunction<Bn...>, B1> {};

template<class...> struct disjunction : std::false_type {};
template<class B1> struct disjunction<B1> : B1 {};
template<class B1, class... Bn>
struct disjunction<B1, Bn...>
    : conditional_t<bool(B1::value), disjunction<Bn...>, B1> {};
#endif

template<typename... Policies>
struct isPolicy;

template<typename... Rest>
struct isPolicy<return_value_policy::take_ownership, Rest...> {
    static constexpr bool value = true;
};

template<typename... Rest>
struct isPolicy<return_value_policy::reference, Rest...> {
    static constexpr bool value = true;
};

template<typename... Rest>
struct isPolicy<emscripten::async, Rest...> {
    static constexpr bool value = true;
};

template <typename T, typename... Rest>
struct isPolicy<emscripten::allow_raw_pointer<T>, Rest...> {
    static constexpr bool value = true;
};

template<typename... Rest>
struct isPolicy<allow_raw_pointers, Rest...> {
    static constexpr bool value = true;
};

template<typename... Rest>
struct isPolicy<emscripten::pure_virtual, Rest...> {
    static constexpr bool value = true;
};

template<typename T, typename... Rest>
struct isPolicy<emscripten::nonnull<T>, Rest...> {
    static constexpr bool value = true;
};

template<typename T, typename... Rest>
struct isPolicy<T, Rest...> {
    static constexpr bool value = isPolicy<Rest...>::value;
};

template<>
struct isPolicy<> {
    static constexpr bool value = false;
};

template<typename ReturnType, typename... Rest>
struct GetReturnValuePolicy {
    using tag = rvp::default_tag;
};

template<typename ReturnType, typename... Rest>
struct GetReturnValuePolicy<ReturnType, return_value_policy::take_ownership, Rest...> {
    using tag = rvp::take_ownership;
};

template<typename ReturnType, typename... Rest>
struct GetReturnValuePolicy<ReturnType, return_value_policy::reference, Rest...> {
    using tag = rvp::reference;
};

template<typename ReturnType, typename T, typename... Rest>
struct GetReturnValuePolicy<ReturnType, T, Rest...> {
    using tag = GetReturnValuePolicy<ReturnType, Rest...>::tag;
};

template<typename... Policies>
using isAsync = disjunction<std::is_same<async, Policies>...>;

template<typename... Policies>
using isNonnullReturn = disjunction<std::is_same<nonnull<ret_val>, Policies>...>;

}

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
} // end namespace internal

// requires captureless lambda because implicitly coerces to function pointer
template<typename LambdaType>
internal::LambdaSignature<LambdaType>* optional_override(const LambdaType& fp) {
    return fp;
}

////////////////////////////////////////////////////////////////////////////////
// Invoker
////////////////////////////////////////////////////////////////////////////////

namespace internal {

template<typename ReturnPolicy, typename ReturnType, typename... Args>
struct Invoker {
    static typename internal::BindingType<ReturnType>::WireType invoke(
        ReturnType (*fn)(Args...),
        typename internal::BindingType<Args>::WireType... args
    ) {
        return internal::BindingType<ReturnType>::toWireType(
            fn(internal::BindingType<Args>::fromWireType(args)...),
            ReturnPolicy{}
        );
    }
};

template<typename ReturnPolicy, typename... Args>
struct Invoker<ReturnPolicy, void, Args...> {
    static void invoke(
        void (*fn)(Args...),
        typename internal::BindingType<Args>::WireType... args
    ) {
        return fn(
            internal::BindingType<Args>::fromWireType(args)...
        );
    }
};

template<typename ReturnPolicy, typename FunctorType, typename ReturnType, typename... Args>
struct FunctorInvoker {
    static typename internal::BindingType<ReturnType>::WireType invoke(
        FunctorType& function,
        typename internal::BindingType<Args>::WireType... args
    ) {
        return internal::BindingType<ReturnType>::toWireType(
            function(
                internal::BindingType<Args>::fromWireType(args)...)
            , ReturnPolicy{}
        );
    }
};

template<typename ReturnPolicy, typename FunctorType, typename... Args>
struct FunctorInvoker<ReturnPolicy, FunctorType, void, Args...> {
    static void invoke(
        FunctorType& function,
        typename internal::BindingType<Args>::WireType... args
    ) {
        function(
            internal::BindingType<Args>::fromWireType(args)...);
    }
};

} // end namespace internal

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

template<>
struct SignatureCode<void*> {
    static constexpr char get() {
        return 'p';
    }
};
template<>
struct SignatureCode<size_t> {
    static constexpr char get() {
        return 'p';
    }
};

#ifdef __wasm64__
template<>
struct SignatureCode<long> {
    static constexpr char get() {
        return 'j';
    }
};
#endif

template<typename... Args>
const char* getGenericSignature() {
    static constexpr char signature[] = { SignatureCode<Args>::get()..., 0 };
    return signature;
}

template<typename T> struct SignatureTranslator { using type = int; };
template<> struct SignatureTranslator<void> { using type = void; };
template<> struct SignatureTranslator<float> { using type = float; };
template<> struct SignatureTranslator<double> { using type = double; };
#ifdef __wasm64__
template<> struct SignatureTranslator<long> { using type = long; };
#endif
template<> struct SignatureTranslator<size_t> { using type = size_t; };
template<typename PtrType>
struct SignatureTranslator<PtrType*> { using type = void*; };
template<typename PtrType>
struct SignatureTranslator<PtrType&> { using type = void*; };
template<typename ReturnType, typename... Args>
struct SignatureTranslator<ReturnType (*)(Args...)> { using type = void*; };

template<typename... Args>
EMSCRIPTEN_ALWAYS_INLINE const char* getSpecificSignature() {
    return getGenericSignature<typename SignatureTranslator<Args>::type...>();
}

template<typename Return, typename... Args>
EMSCRIPTEN_ALWAYS_INLINE const char* getSignature(Return (*)(Args...)) {
    return getSpecificSignature<Return, Args...>();
}

} // end namespace internal

////////////////////////////////////////////////////////////////////////////////
// FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

template<typename ReturnType, typename... Args, typename... Policies>
void function(const char* name, ReturnType (*fn)(Args...), Policies...) {
    using namespace internal;
    typename WithPolicies<Policies...>::template ArgTypeList<ReturnType, Args...> args;
    using ReturnPolicy = GetReturnValuePolicy<ReturnType, Policies...>::tag;
    auto invoke = Invoker<ReturnPolicy, ReturnType, Args...>::invoke;
    _embind_register_function(
        name,
        args.getCount(),
        args.getTypes(),
        getSignature(invoke),
        reinterpret_cast<GenericFunction>(invoke),
        reinterpret_cast<GenericFunction>(fn),
        isAsync<Policies...>::value,
        isNonnullReturn<Policies...>::value);
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

template<typename ReturnPolicy, typename FunctionPointerType, typename ReturnType, typename ThisType, typename... Args>
struct FunctionInvoker {
    static typename internal::BindingType<ReturnType>::WireType invoke(
        FunctionPointerType* function,
        typename internal::BindingType<ThisType>::WireType wireThis,
        typename internal::BindingType<Args>::WireType... args
    ) {
        return internal::BindingType<ReturnType>::toWireType(
            (*function)(
                internal::BindingType<ThisType>::fromWireType(wireThis),
                internal::BindingType<Args>::fromWireType(args)...),
            ReturnPolicy{}
        );
    }
};

template<typename ReturnPolicy, typename FunctionPointerType, typename ThisType, typename... Args>
struct FunctionInvoker<ReturnPolicy, FunctionPointerType, void, ThisType, Args...> {
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

template<typename ReturnPolicy,
         typename MemberPointer,
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
            ,
            ReturnPolicy{}
        );
    }
};

template<typename ReturnPolicy,
         typename MemberPointer,
         typename ThisType,
         typename... Args>
struct MethodInvoker<ReturnPolicy, MemberPointer, void, ThisType, Args...> {
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

    template<typename ClassType, typename ReturnPolicy = rvp::default_tag>
    static WireType getWire(
        const MemberPointer& field,
        ClassType& ptr
    ) {
        return MemberBinding::toWireType(ptr.*field, ReturnPolicy{});
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
        return MemberBinding::toWireType(*context, rvp::default_tag{});
    }

    static void set(FieldType* context, WireType value) {
        *context = MemberBinding::fromWireType(value);
    }
};

// TODO: This could do a reinterpret-cast if sizeof(T) === sizeof(void*)
template<typename T>
inline T* getContext(const T& t) {
    // not a leak because this is called once per binding
    auto* ret = new T(t);
#if __has_feature(leak_sanitizer) || __has_feature(address_sanitizer)
    __lsan_ignore_object(ret);
#endif
    return ret;
}

template<typename Accessor, typename ValueType>
struct PropertyTag {};

template<typename T>
struct GetterPolicy;

template<typename GetterReturnType, typename GetterThisType>
struct GetterPolicy<GetterReturnType (GetterThisType::*)() const> {
    typedef GetterReturnType ReturnType;
    typedef GetterReturnType (GetterThisType::*Context)() const;

    typedef internal::BindingType<ReturnType> Binding;
    typedef typename Binding::WireType WireType;

    template<typename ClassType, typename ReturnPolicy>
    static WireType get(const Context& context, const ClassType& ptr) {
        return Binding::toWireType((ptr.*context)(), ReturnPolicy{});
    }

    static void* getContext(Context context) {
        return internal::getContext(context);
    }
};

#ifdef __cpp_noexcept_function_type
template<typename GetterReturnType, typename GetterThisType>
struct GetterPolicy<GetterReturnType (GetterThisType::*)() const noexcept>
     : GetterPolicy<GetterReturnType (GetterThisType::*)() const> {};
#endif

template<typename GetterReturnType, typename GetterThisType>
struct GetterPolicy<GetterReturnType (*)(const GetterThisType&)> {
    typedef GetterReturnType ReturnType;
    typedef GetterReturnType (*Context)(const GetterThisType&);

    typedef internal::BindingType<ReturnType> Binding;
    typedef typename Binding::WireType WireType;

    template<typename ClassType, typename ReturnPolicy>
    static WireType get(const Context& context, const ClassType& ptr) {
        return Binding::toWireType(context(ptr), ReturnPolicy{});
    }

    static void* getContext(Context context) {
        return internal::getContext(context);
    }
};

template<typename GetterReturnType, typename GetterThisType>
struct GetterPolicy<std::function<GetterReturnType(const GetterThisType&)>> {
    typedef GetterReturnType ReturnType;
    typedef std::function<GetterReturnType(const GetterThisType&)> Context;

    typedef internal::BindingType<ReturnType> Binding;
    typedef typename Binding::WireType WireType;

    template<typename ClassType, typename ReturnPolicy>
    static WireType get(const Context& context, const ClassType& ptr) {
        return Binding::toWireType(context(ptr), ReturnPolicy{});
    }

    static void* getContext(const Context& context) {
        return internal::getContext(context);
    }
};

template<typename Getter, typename GetterReturnType>
struct GetterPolicy<PropertyTag<Getter, GetterReturnType>> {
    typedef GetterReturnType ReturnType;
    typedef Getter Context;

    typedef internal::BindingType<ReturnType> Binding;
    typedef typename Binding::WireType WireType;

    template<typename ClassType, typename ReturnPolicy>
    static WireType get(const Context& context, const ClassType& ptr) {
        return Binding::toWireType(context(ptr), ReturnPolicy{});
    }

    static void* getContext(const Context& context) {
        return internal::getContext(context);
    }
};

template<typename T>
struct SetterPolicy;

template<typename SetterReturnType, typename SetterThisType, typename SetterArgumentType>
struct SetterPolicy<SetterReturnType (SetterThisType::*)(SetterArgumentType)> {
    typedef SetterArgumentType ArgumentType;
    typedef SetterReturnType (SetterThisType::*Context)(SetterArgumentType);

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

#ifdef __cpp_noexcept_function_type
template<typename SetterReturnType, typename SetterThisType, typename SetterArgumentType>
struct SetterPolicy<SetterReturnType (SetterThisType::*)(SetterArgumentType) noexcept>
     : SetterPolicy<SetterReturnType (SetterThisType::*)(SetterArgumentType)> {};
#endif

template<typename SetterReturnType, typename SetterThisType, typename SetterArgumentType>
struct SetterPolicy<SetterReturnType (*)(SetterThisType&, SetterArgumentType)> {
    typedef SetterArgumentType ArgumentType;
    typedef SetterReturnType (*Context)(SetterThisType&, SetterArgumentType);

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

template<typename SetterReturnType, typename SetterThisType, typename SetterArgumentType>
struct SetterPolicy<std::function<SetterReturnType(SetterThisType&, SetterArgumentType)>> {
    typedef SetterArgumentType ArgumentType;
    typedef std::function<SetterReturnType(SetterThisType&, SetterArgumentType)> Context;

    typedef internal::BindingType<SetterArgumentType> Binding;
    typedef typename Binding::WireType WireType;

    template<typename ClassType>
    static void set(const Context& context, ClassType& ptr, WireType wt) {
        context(ptr, Binding::fromWireType(wt));
    }

    static void* getContext(const Context& context) {
        return internal::getContext(context);
    }
};

template<typename Setter, typename SetterArgumentType>
struct SetterPolicy<PropertyTag<Setter, SetterArgumentType>> {
    typedef SetterArgumentType ArgumentType;
    typedef Setter Context;

    typedef internal::BindingType<SetterArgumentType> Binding;
    typedef typename Binding::WireType WireType;

    template<typename ClassType>
    static void set(const Context& context, ClassType& ptr, WireType wt) {
        context(ptr, Binding::fromWireType(wt));
    }

    static void* getContext(const Context& context) {
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
    return BindingType<ElementType>::toWireType(ptr[index], rvp::default_tag{});
}

template<typename ClassType, typename ElementType>
void set_by_index(int index, ClassType& ptr, typename BindingType<ElementType>::WireType wt) {
    ptr[index] = BindingType<ElementType>::fromWireType(wt);
}

} // end namespace internal

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

        auto g = &GP::template get<ClassType, rvp::default_tag>;
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

    template<typename InstanceType, typename ElementType, int N>
    value_object& field(const char* fieldName, ElementType (InstanceType::*field)[N]) {
        using namespace internal;

        typedef std::array<ElementType, N> FieldType;
        static_assert(sizeof(FieldType) == sizeof(ElementType[N]));

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

        auto g = &GP::template get<ClassType, rvp::default_tag>;
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

    static std::shared_ptr<PointeeType>* share(PointeeType* p, EM_VAL v) {
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

} // end namespace internal

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
    static VoidFunctionPtr getUpcaster() {
        return nullptr;
    }

    template<typename ClassType>
    static VoidFunctionPtr getDowncaster() {
        return nullptr;
    }
};

// NOTE: this returns the class type, not the pointer type
template<typename T>
inline TYPEID getActualType(T* ptr) {
    return getLightTypeID(*ptr);
};

} // end namespace internal

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
        properties.as_handle()));
}

} // end namespace internal

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

struct DeduceArgumentsTag {};

////////////////////////////////////////////////////////////////////////////
// RegisterClassConstructor
////////////////////////////////////////////////////////////////////////////

template <typename T>
struct RegisterClassConstructor;

template<typename ReturnType, typename... Args>
struct RegisterClassConstructor<ReturnType (*)(Args...)> {

    template <typename ClassType, typename... Policies>
    static void invoke(ReturnType (*factory)(Args...)) {
        typename WithPolicies<allow_raw_pointers, Policies...>::template ArgTypeList<ReturnType, Args...> args;
        using ReturnPolicy = rvp::take_ownership;
        auto invoke = &Invoker<ReturnPolicy, ReturnType, Args...>::invoke;
        _embind_register_class_constructor(
            TypeID<ClassType>::get(),
            args.getCount(),
            args.getTypes(),
            getSignature(invoke),
            reinterpret_cast<GenericFunction>(invoke),
            reinterpret_cast<GenericFunction>(factory));
    }
};

template<typename ReturnType, typename... Args>
struct RegisterClassConstructor<std::function<ReturnType (Args...)>> {

    template <typename ClassType, typename... Policies>
    static void invoke(std::function<ReturnType (Args...)> factory) {
        typename WithPolicies<Policies...>::template ArgTypeList<ReturnType, Args...> args;
        using ReturnPolicy = rvp::take_ownership;
        auto invoke = &FunctorInvoker<ReturnPolicy, decltype(factory), ReturnType, Args...>::invoke;
        _embind_register_class_constructor(
            TypeID<ClassType>::get(),
            args.getCount(),
            args.getTypes(),
            getSignature(invoke),
            reinterpret_cast<GenericFunction>(invoke),
            reinterpret_cast<GenericFunction>(getContext(factory)));
    }
};

template<typename ReturnType, typename... Args>
struct RegisterClassConstructor<ReturnType (Args...)> {
    template <typename ClassType, typename Callable, typename... Policies>
    static void invoke(Callable& factory) {
        typename WithPolicies<Policies...>::template ArgTypeList<ReturnType, Args...> args;
        using ReturnPolicy = rvp::take_ownership;
        auto invoke = &FunctorInvoker<ReturnPolicy, decltype(factory), ReturnType, Args...>::invoke;
        _embind_register_class_constructor(
            TypeID<ClassType>::get(),
            args.getCount(),
            args.getTypes(),
            getSignature(invoke),
            reinterpret_cast<GenericFunction>(invoke),
            reinterpret_cast<GenericFunction>(getContext(factory)));
    }
};

////////////////////////////////////////////////////////////////////////////
// RegisterClassMethod
////////////////////////////////////////////////////////////////////////////

template <typename T>
struct RegisterClassMethod;

template<typename ClassType, typename ReturnType, typename... Args>
struct RegisterClassMethod<ReturnType (ClassType::*)(Args...)> {

    template <typename CT, typename... Policies>
    static void invoke(const char* methodName,
                       ReturnType (ClassType::*memberFunction)(Args...)) {
        using ReturnPolicy = GetReturnValuePolicy<ReturnType, Policies...>::tag;
        auto invoke = MethodInvoker<ReturnPolicy, decltype(memberFunction), ReturnType, ClassType*, Args...>::invoke;

        typename WithPolicies<Policies...>::template ArgTypeList<ReturnType, AllowedRawPointer<ClassType>, Args...> args;
        _embind_register_class_function(
            TypeID<ClassType>::get(),
            methodName,
            args.getCount(),
            args.getTypes(),
            getSignature(invoke),
            reinterpret_cast<GenericFunction>(invoke),
            getContext(memberFunction),
            isPureVirtual<Policies...>::value,
            isAsync<Policies...>::value,
            isNonnullReturn<Policies...>::value);
    }
};

#ifdef __cpp_noexcept_function_type
template<typename ClassType, typename ReturnType, typename... Args>
struct RegisterClassMethod<ReturnType (ClassType::*)(Args...) noexcept>
     : RegisterClassMethod<ReturnType (ClassType::*)(Args...)> {};
#endif

template<typename ClassType, typename ReturnType, typename... Args>
struct RegisterClassMethod<ReturnType (ClassType::*)(Args...) const> {

    template <typename CT, typename... Policies>
    static void invoke(const char* methodName,
                       ReturnType (ClassType::*memberFunction)(Args...) const)  {
        using ReturnPolicy = GetReturnValuePolicy<ReturnType, Policies...>::tag;
        auto invoke = MethodInvoker<ReturnPolicy, decltype(memberFunction), ReturnType, const ClassType*, Args...>::invoke;

        typename WithPolicies<Policies...>::template ArgTypeList<ReturnType, AllowedRawPointer<const ClassType>, Args...> args;
        _embind_register_class_function(
            TypeID<ClassType>::get(),
            methodName,
            args.getCount(),
            args.getTypes(),
            getSignature(invoke),
            reinterpret_cast<GenericFunction>(invoke),
            getContext(memberFunction),
            isPureVirtual<Policies...>::value,
            isAsync<Policies...>::value,
            isNonnullReturn<Policies...>::value);
    }
};

#ifdef __cpp_noexcept_function_type
template<typename ClassType, typename ReturnType, typename... Args>
struct RegisterClassMethod<ReturnType (ClassType::*)(Args...) const noexcept>
     : RegisterClassMethod<ReturnType (ClassType::*)(Args...) const> {};
#endif

template<typename ReturnType, typename ThisType, typename... Args>
struct RegisterClassMethod<ReturnType (*)(ThisType, Args...)> {

    template <typename ClassType, typename... Policies>
    static void invoke(const char* methodName,
                       ReturnType (*function)(ThisType, Args...)) {
        typename WithPolicies<Policies...>::template ArgTypeList<ReturnType, ThisType, Args...> args;
        using ReturnPolicy = GetReturnValuePolicy<ReturnType, Policies...>::tag;
        auto invoke = FunctionInvoker<ReturnPolicy, decltype(function), ReturnType, ThisType, Args...>::invoke;
        _embind_register_class_function(
            TypeID<ClassType>::get(),
            methodName,
            args.getCount(),
            args.getTypes(),
            getSignature(invoke),
            reinterpret_cast<GenericFunction>(invoke),
            getContext(function),
            false,
            isAsync<Policies...>::value,
            isNonnullReturn<Policies...>::value);
    }
};

#ifdef __cpp_noexcept_function_type
template<typename ReturnType, typename ThisType, typename... Args>
struct RegisterClassMethod<ReturnType (*)(ThisType, Args...) noexcept>
     : RegisterClassMethod<ReturnType (*)(ThisType, Args...)> {};
#endif

template<typename ReturnType, typename ThisType, typename... Args>
struct RegisterClassMethod<std::function<ReturnType (ThisType, Args...)>> {

    template <typename ClassType, typename... Policies>
    static void invoke(const char* methodName,
                       std::function<ReturnType (ThisType, Args...)> function) {
        typename WithPolicies<Policies...>::template ArgTypeList<ReturnType, ThisType, Args...> args;
        using ReturnPolicy = GetReturnValuePolicy<ReturnType, Policies...>::tag;
        auto invoke = FunctorInvoker<ReturnPolicy, decltype(function), ReturnType, ThisType, Args...>::invoke;
        _embind_register_class_function(
            TypeID<ClassType>::get(),
            methodName,
            args.getCount(),
            args.getTypes(),
            getSignature(invoke),
            reinterpret_cast<GenericFunction>(invoke),
            getContext(function),
            false,
            isAsync<Policies...>::value,
            isNonnullReturn<Policies...>::value);
    }
};

template<typename ReturnType, typename ThisType, typename... Args>
struct RegisterClassMethod<ReturnType (ThisType, Args...)> {

    template <typename ClassType, typename Callable, typename... Policies>
    static void invoke(const char* methodName,
                       Callable& callable) {
        typename WithPolicies<Policies...>::template ArgTypeList<ReturnType, ThisType, Args...> args;
        using ReturnPolicy = GetReturnValuePolicy<ReturnType, Policies...>::tag;
        auto invoke = FunctorInvoker<ReturnPolicy, decltype(callable), ReturnType, ThisType, Args...>::invoke;
        _embind_register_class_function(
            TypeID<ClassType>::get(),
            methodName,
            args.getCount(),
            args.getTypes(),
            getSignature(invoke),
            reinterpret_cast<GenericFunction>(invoke),
            getContext(callable),
            false,
            isAsync<Policies...>::value,
            isNonnullReturn<Policies...>::value);
    }
};

} // end namespace internal

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
        auto upcast   = BaseSpecifier::template getUpcaster<ClassType>();
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

    template<typename Signature = internal::DeduceArgumentsTag, typename Callable, typename... Policies>
    EMSCRIPTEN_ALWAYS_INLINE const class_& constructor(Callable callable, Policies...) const {

        using invoker = internal::RegisterClassConstructor<
            typename std::conditional<std::is_same<Signature, internal::DeduceArgumentsTag>::value,
                                      Callable,
                                      Signature>::type>;

        invoker::template invoke<ClassType, Policies...>(callable);
        return *this;
    }

    template<typename SmartPtr, typename... Args, typename... Policies>
    EMSCRIPTEN_ALWAYS_INLINE const class_& smart_ptr_constructor(const char* smartPtrName, SmartPtr (*factory)(Args...), Policies...) const {
        using namespace internal;

        smart_ptr<SmartPtr>(smartPtrName);

        typename WithPolicies<Policies...>::template ArgTypeList<SmartPtr, Args...> args;
        using ReturnPolicy = GetReturnValuePolicy<SmartPtr, return_value_policy::take_ownership>::tag;
        auto invoke = &Invoker<ReturnPolicy, SmartPtr, Args...>::invoke;
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
                allow_raw_pointer<ret_val>(), nonnull<ret_val>())
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

    template<typename Signature = internal::DeduceArgumentsTag, typename Callable, typename... Policies>
    EMSCRIPTEN_ALWAYS_INLINE const class_& function(const char* methodName, Callable callable, Policies...) const {
        using invoker = internal::RegisterClassMethod<
            typename std::conditional<std::is_same<Signature, internal::DeduceArgumentsTag>::value,
                                      Callable,
                                      Signature>::type>;

        invoker::template invoke<ClassType, Policies...>(methodName, callable);
        return *this;
    }

    template<
        typename FieldType,
        typename... Policies,
        // Prevent the template from wrongly matching the getter function
        // overload.
        typename = typename std::enable_if<
            !std::is_function<FieldType>::value &&
            internal::conjunction<internal::isPolicy<Policies>...>::value>::type>
    EMSCRIPTEN_ALWAYS_INLINE const class_& property(const char* fieldName, const FieldType ClassType::*field, Policies...) const {
        using namespace internal;
        using ReturnPolicy = GetReturnValuePolicy<FieldType, Policies...>::tag;
        typename WithPolicies<Policies...>::template ArgTypeList<FieldType> returnType;

        auto getter = &MemberAccess<ClassType, FieldType>::template getWire<ClassType, ReturnPolicy>;
        _embind_register_class_property(
            TypeID<ClassType>::get(),
            fieldName,
            returnType.getTypes()[0],
            getSignature(getter),
            reinterpret_cast<GenericFunction>(getter),
            getContext(field),
            0,
            0,
            0,
            0);
        return *this;
    }

    template<
        typename FieldType,
        typename... Policies,
        // Prevent the template from wrongly matching the getter function
        // overload.
        typename = typename std::enable_if<
            !std::is_function<FieldType>::value &&
            internal::conjunction<internal::isPolicy<Policies>...>::value>::type>
    EMSCRIPTEN_ALWAYS_INLINE const class_& property(const char* fieldName, FieldType ClassType::*field, Policies...) const {
        using namespace internal;
        using ReturnPolicy = GetReturnValuePolicy<FieldType, Policies...>::tag;
        typename WithPolicies<Policies...>::template ArgTypeList<FieldType> returnType;

        auto getter = &MemberAccess<ClassType, FieldType>::template getWire<ClassType, ReturnPolicy>;
        auto setter = &MemberAccess<ClassType, FieldType>::template setWire<ClassType>;
        _embind_register_class_property(
            TypeID<ClassType>::get(),
            fieldName,
            returnType.getTypes()[0],
            getSignature(getter),
            reinterpret_cast<GenericFunction>(getter),
            getContext(field),
            returnType.getTypes()[0],
            getSignature(setter),
            reinterpret_cast<GenericFunction>(setter),
            getContext(field));
        return *this;
    }

    template<
        typename PropertyType = internal::DeduceArgumentsTag,
        typename Getter,
        typename... Policies,
        // Prevent the template from wrongly matching the getter/setter overload
        // of this function.
        typename = typename std::enable_if<
            internal::conjunction<internal::isPolicy<Policies>...>::value>::type>
    EMSCRIPTEN_ALWAYS_INLINE const class_& property(const char* fieldName, Getter getter, Policies...) const {
        using namespace internal;

        typedef GetterPolicy<
            typename std::conditional<std::is_same<PropertyType, internal::DeduceArgumentsTag>::value,
                                                   Getter,
                                                   PropertyTag<Getter, PropertyType>>::type> GP;
        using ReturnPolicy = GetReturnValuePolicy<typename GP::ReturnType, Policies...>::tag;
        auto gter = &GP::template get<ClassType, ReturnPolicy>;
        typename WithPolicies<Policies...>::template ArgTypeList<typename GP::ReturnType> returnType;
        _embind_register_class_property(
            TypeID<ClassType>::get(),
            fieldName,
            returnType.getTypes()[0],
            getSignature(gter),
            reinterpret_cast<GenericFunction>(gter),
            GP::getContext(getter),
            0,
            0,
            0,
            0);
        return *this;
    }

    template<
        typename PropertyType = internal::DeduceArgumentsTag,
        typename Getter,
        typename Setter,
        typename... Policies,
        // Similar to the other variadic property overloads this can greedily
        // match the wrong overload so we need to ensure the setter is not a
        // policy argument.
        typename = typename std::enable_if<!internal::isPolicy<Setter>::value>::type>
    EMSCRIPTEN_ALWAYS_INLINE const class_& property(const char* fieldName, Getter getter, Setter setter, Policies...) const {
        using namespace internal;
        using ReturnPolicy = GetReturnValuePolicy<PropertyType, Policies...>::tag;

        typedef GetterPolicy<
            typename std::conditional<std::is_same<PropertyType, internal::DeduceArgumentsTag>::value,
                                                   Getter,
                                                   PropertyTag<Getter, PropertyType>>::type> GP;
        typedef SetterPolicy<
            typename std::conditional<std::is_same<PropertyType, internal::DeduceArgumentsTag>::value,
                                                   Setter,
                                                   PropertyTag<Setter, PropertyType>>::type> SP;


        auto gter = &GP::template get<ClassType, ReturnPolicy>;
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
        using ReturnPolicy = GetReturnValuePolicy<ReturnType, Policies...>::tag;
        auto invoke = internal::Invoker<ReturnPolicy, ReturnType, Args...>::invoke;
        _embind_register_class_class_function(
            TypeID<ClassType>::get(),
            methodName,
            args.getCount(),
            args.getTypes(),
            getSignature(invoke),
            reinterpret_cast<GenericFunction>(invoke),
            reinterpret_cast<GenericFunction>(classMethod),
            isAsync<Policies...>::value,
            isNonnullReturn<Policies...>::value);
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

#if __cplusplus >= 201703L
template<typename T>
void register_optional() {
    // Optional types are automatically registered for some internal types so
    // only run the register method once so we don't conflict with a user's
    // bindings if they also register the optional type.
    thread_local bool hasRun;
    if (hasRun) {
        return;
    }
    hasRun = true;
    internal::_embind_register_optional(
        internal::TypeID<std::optional<T>>::get(),
        internal::TypeID<typename std::remove_pointer<T>::type>::get());
}
#endif

////////////////////////////////////////////////////////////////////////////////
// VECTORS
////////////////////////////////////////////////////////////////////////////////

namespace internal {

template<typename VectorType>
struct VectorAccess {
// This nearly duplicated code is used for generating more specific TypeScript
// types when using more modern C++ versions.
#if __cplusplus >= 201703L
    static std::optional<typename VectorType::value_type> get(
        const VectorType& v,
        typename VectorType::size_type index
    ) {
        if (index < v.size()) {
            return v[index];
        } else {
            return {};
        }
    }
#else
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
#endif

    static bool set(
        VectorType& v,
        typename VectorType::size_type index,
        const typename VectorType::value_type& value
    ) {
        v[index] = value;
        return true;
    }
};

} // end namespace internal

template<typename T>
class_<std::vector<T>> register_vector(const char* name) {
    typedef std::vector<T> VecType;
#if __cplusplus >= 201703L
    register_optional<T>();
#endif

    void (VecType::*push_back)(const T&) = &VecType::push_back;
    void (VecType::*resize)(const size_t, const T&) = &VecType::resize;
    size_t (VecType::*size)() const = &VecType::size;
    return class_<std::vector<T>>(name)
        .template constructor<>()
        .function("push_back", push_back)
        .function("resize", resize)
        .function("size", size)
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
// This nearly duplicated code is used for generating more specific TypeScript
// types when using more modern C++ versions.
#if __cplusplus >= 201703L
    static std::optional<typename MapType::mapped_type> get(
        const MapType& m,
        const typename MapType::key_type& k
    ) {
        auto i = m.find(k);
        if (i == m.end()) {
            return {};
        } else {
            return i->second;
        }
    }
#else
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
#endif

    static void set(
        MapType& m,
        const typename MapType::key_type& k,
        const typename MapType::mapped_type& v
    ) {
        m[k] = v;
    }

    static std::vector<typename MapType::key_type> keys(
        const MapType& m
    ) {
      std::vector<typename MapType::key_type> keys;
      keys.reserve(m.size());
      for (const auto& pair : m) {
        keys.push_back(pair.first);
      }
      return keys;
    }
};

} // end namespace internal

template<typename K, typename V>
class_<std::map<K, V>> register_map(const char* name) {
    typedef std::map<K,V> MapType;
#if __cplusplus >= 201703L
    register_optional<V>();
#endif

    size_t (MapType::*size)() const = &MapType::size;
    return class_<MapType>(name)
        .template constructor<>()
        .function("size", size)
        .function("get", internal::MapAccess<MapType>::get)
        .function("set", internal::MapAccess<MapType>::set)
        .function("keys", internal::MapAccess<MapType>::keys)
        ;
}

////////////////////////////////////////////////////////////////////////////////
// std::optional
////////////////////////////////////////////////////////////////////////////////

#if __cplusplus >= 201703L
namespace internal {
template <typename T>
struct BindingType<std::optional<T>> {
    using ValBinding = BindingType<val>;
    using WireType = ValBinding::WireType;

    template<typename ReturnPolicy = void>
    static WireType toWireType(std::optional<T> value, rvp::default_tag) {
        if (value) {
            return ValBinding::toWireType(val(*value), rvp::default_tag{});
        }
        return ValBinding::toWireType(val::undefined(), rvp::default_tag{});
    }


    static std::optional<T> fromWireType(WireType value) {
        val optional = val::take_ownership(value);
        if (optional.isUndefined()) {
            return {};
        }
        return optional.as<T>();
    }
};
} // end namespace internal
#endif


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

template<typename T> double asGenericValue(T t) {
    return static_cast<double>(t);
}

template<typename T> uintptr_t asGenericValue(T* p) {
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
        static_cast<double>(asGenericValue(BT::toWireType(v, rvp::default_tag{}))));
}

template <typename T>
inline void register_type(const char* name) {
  using namespace internal;
  _embind_register_user_type(TypeID<T>::get(), name);
}

// EMSCRIPTEN_BINDINGS creates a static struct to initialize the binding which
// will get included in the program if the translation unit in which it is
// defined gets linked into the program. Using a C++ constructor here ensures it
// occurs after any other C++ constructors in this file, which is not true for
// __attribute__((constructor)) (they run before C++ constructors in the same
// file).
#define EMSCRIPTEN_BINDINGS(name)                                              \
  static void embind_init_##name();                                            \
  static struct EmBindInit_##name : emscripten::internal::InitFunc {           \
    EmBindInit_##name() : InitFunc(embind_init_##name) {}                      \
  } EmBindInit_##name##_instance;                                              \
  static void embind_init_##name()

} // end namespace emscripten
