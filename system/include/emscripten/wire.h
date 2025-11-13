/*
 * Copyright 2012 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#pragma once

#if __cplusplus < 201703L
#error "embind requires -std=c++17 or newer"
#endif

// A value moving between JavaScript and C++ has three representations:
// - The original JS value: a String
// - The native on-the-wire value: a stack-allocated char*, say
// - The C++ value: std::string
//
// We'll call the on-the-wire type WireType.

#include <cstdio>
#include <cstdlib>
#include <memory>
#include <string>

#define EMSCRIPTEN_ALWAYS_INLINE __attribute__((always_inline))
#define EMBIND_VISIBILITY_DEFAULT __attribute__((visibility("default")))

#ifndef EMSCRIPTEN_HAS_UNBOUND_TYPE_NAMES
#define EMSCRIPTEN_HAS_UNBOUND_TYPE_NAMES 1
#endif

namespace emscripten {

#if EMSCRIPTEN_HAS_UNBOUND_TYPE_NAMES
constexpr bool has_unbound_type_names = true;
#else
constexpr bool has_unbound_type_names = false;
#endif

namespace internal {

typedef const void* TYPEID;

// We don't need the full std::type_info implementation.  We
// just need a unique identifier per type and polymorphic type
// identification.

template<typename T>
static inline constexpr bool IsCanonicalized = std::is_same<T, typename std::decay<T>::type>::value;

template<typename T>
struct CanonicalizedID {
    static_assert(IsCanonicalized<T>, "T should not be a reference or cv-qualified");
    static char c;
    static constexpr TYPEID get() {
        return &c;
    }
};

template<typename T>
char CanonicalizedID<T>::c;

template<typename T>
struct Canonicalized {
    typedef typename std::remove_cv<typename std::remove_reference<T>::type>::type type;
};

template<typename T>
struct LightTypeID {
    static constexpr TYPEID get() {
        static_assert(IsCanonicalized<T>, "T should not be a reference or cv-qualified");
        if (has_unbound_type_names) {
#if __has_feature(cxx_rtti)
            return &typeid(T);
#else
            static_assert(!has_unbound_type_names,
                "Unbound type names are illegal with RTTI disabled. "
                "Either add -DEMSCRIPTEN_HAS_UNBOUND_TYPE_NAMES=0 to or remove -fno-rtti "
                "from the compiler arguments");
#endif
        }

        typedef typename Canonicalized<T>::type C;
        return CanonicalizedID<C>::get();
    }
};

template<typename T>
constexpr TYPEID getLightTypeID(const T& value) {
    static_assert(IsCanonicalized<T>, "T should not be a reference or cv-qualified");
    if (has_unbound_type_names) {
#if __has_feature(cxx_rtti)
        return &typeid(value);
#else
        static_assert(!has_unbound_type_names,
            "Unbound type names are illegal with RTTI disabled. "
            "Either add -DEMSCRIPTEN_HAS_UNBOUND_TYPE_NAMES=0 to or remove -fno-rtti "
            "from the compiler arguments");
#endif
    }
    return LightTypeID<T>::get();
}

// The second typename is an unused stub so it's possible to
// specialize groups of classes via SFINAE.
template<typename T, typename = void>
struct TypeID {
    static constexpr TYPEID get() {
        return LightTypeID<T>::get();
    }
};

template<typename T>
struct TypeID<std::unique_ptr<T>> {
    static_assert(std::is_class<T>::value, "The type for a std::unique_ptr binding must be a class.");
    static constexpr TYPEID get() {
        return TypeID<T>::get();
    }
};

template<typename T>
struct TypeID<T*> {
    static_assert(!std::is_pointer<T*>::value, "Implicitly binding raw pointers is illegal.  Specify allow_raw_pointer<arg<?>>");
};

namespace rvp {

struct default_tag {};
struct take_ownership : public default_tag {};
struct reference : public default_tag {};

} // end namespace rvp

template<typename T>
struct AllowedRawPointer {
};

template<typename T>
struct TypeID<AllowedRawPointer<T>> {
    static constexpr TYPEID get() {
        return LightTypeID<T*>::get();
    }
};

template<typename T>
struct TypeID<const T> : TypeID<T> {
};

template<typename T>
struct TypeID<T&> : TypeID<T> {
};

template<typename T>
struct TypeID<T&&> : TypeID<T> {
};

// ExecutePolicies<>

template<typename... Policies>
struct ExecutePolicies;

template<>
struct ExecutePolicies<> {
    template<typename T, int Index>
    struct With {
        typedef T type;
    };
};

template<typename Policy, typename... Remaining>
struct ExecutePolicies<Policy, Remaining...> {
    template<typename T, int Index>
    struct With {
        typedef typename Policy::template Transform<
            typename ExecutePolicies<Remaining...>::template With<T, Index>::type,
            Index
        >::type type;
    };
};

// TypeList<>

template<typename...>
struct TypeList {};

// Cons :: T, TypeList<types...> -> Cons<T, types...>

template<typename First, typename TypeList>
struct Cons;

template<typename First, typename... Rest>
struct Cons<First, TypeList<Rest...>> {
    typedef TypeList<First, Rest...> type;
};

// Apply :: T, TypeList<types...> -> T<types...>

template<template<typename...> class Output, typename TypeList>
struct Apply;

template<template<typename...> class Output, typename... Types>
struct Apply<Output, TypeList<Types...>> {
    typedef Output<Types...> type;
};

// MapWithIndex_

template<template<size_t, typename> class Mapper, size_t CurrentIndex, typename... Args>
struct MapWithIndex_;

template<template<size_t, typename> class Mapper, size_t CurrentIndex, typename First, typename... Rest>
struct MapWithIndex_<Mapper, CurrentIndex, First, Rest...> {
    typedef typename Cons<
        typename Mapper<CurrentIndex, First>::type,
        typename MapWithIndex_<Mapper, CurrentIndex + 1, Rest...>::type
        >::type type;
};

template<template<size_t, typename> class Mapper, size_t CurrentIndex>
struct MapWithIndex_<Mapper, CurrentIndex> {
    typedef TypeList<> type;
};

template<template<typename...> class Output, template<size_t, typename> class Mapper, typename... Args>
struct MapWithIndex {
    typedef typename internal::Apply<
        Output,
        typename MapWithIndex_<Mapper, 0, Args...>::type
    >::type type;
};


template<typename ArgList>
struct ArgArrayGetter;

template<typename... Args>
struct ArgArrayGetter<TypeList<Args...>> {
    static const TYPEID* get() {
        static constexpr TYPEID types[] = { TypeID<Args>::get()... };
        return types;
    }
};

// WithPolicies<...>::ArgTypeList<...>

template<typename... Policies>
struct WithPolicies {
    template<size_t Index, typename T>
    struct MapWithPolicies {
        typedef typename ExecutePolicies<Policies...>::template With<T, Index>::type type;
    };

    template<typename... Args>
    struct ArgTypeList {
        unsigned getCount() const {
            return sizeof...(Args);
        }

        const TYPEID* getTypes() const {
            return ArgArrayGetter<
                typename MapWithIndex<TypeList, MapWithPolicies, Args...>::type
            >::get();
        }
    };
};

template<typename... Policies>
struct WithPolicies<std::tuple<Policies...>> : WithPolicies<Policies...> {};

// BindingType<T>

// The second typename is an unused stub so it's possible to
// specialize groups of classes via SFINAE.
template<typename T, typename = void>
struct BindingType;

#define EMSCRIPTEN_DEFINE_NATIVE_BINDING_TYPE(type)                            \
template<>                                                                     \
struct BindingType<type> {                                                     \
    typedef type WireType;                                                     \
    constexpr static WireType toWireType(const type& v, rvp::default_tag) {    \
        return v;                                                              \
    }                                                                          \
    constexpr static type fromWireType(WireType v) {                           \
        return v;                                                              \
    }                                                                          \
}

EMSCRIPTEN_DEFINE_NATIVE_BINDING_TYPE(char);
EMSCRIPTEN_DEFINE_NATIVE_BINDING_TYPE(signed char);
EMSCRIPTEN_DEFINE_NATIVE_BINDING_TYPE(unsigned char);
EMSCRIPTEN_DEFINE_NATIVE_BINDING_TYPE(signed short);
EMSCRIPTEN_DEFINE_NATIVE_BINDING_TYPE(unsigned short);
EMSCRIPTEN_DEFINE_NATIVE_BINDING_TYPE(signed int);
EMSCRIPTEN_DEFINE_NATIVE_BINDING_TYPE(unsigned int);
EMSCRIPTEN_DEFINE_NATIVE_BINDING_TYPE(signed long);
EMSCRIPTEN_DEFINE_NATIVE_BINDING_TYPE(unsigned long);
EMSCRIPTEN_DEFINE_NATIVE_BINDING_TYPE(float);
EMSCRIPTEN_DEFINE_NATIVE_BINDING_TYPE(double);
EMSCRIPTEN_DEFINE_NATIVE_BINDING_TYPE(int64_t);
EMSCRIPTEN_DEFINE_NATIVE_BINDING_TYPE(uint64_t);

template<>
struct BindingType<void> {
    // Using empty struct instead of void is ABI-compatible, but makes it easier
    // to work with wire types in a generic template context, as void can't be
    // stored in local variables or passed around but empty struct can.
    // TODO: switch to std::monostate when we require C++17.
    struct WireType {};

    static void fromWireType(WireType) {
        // No-op, as void has no value.
    }
};

template<>
struct BindingType<bool> {
    typedef bool WireType;
    static WireType toWireType(bool b, rvp::default_tag) {
        return b;
    }
    static bool fromWireType(WireType wt) {
        return wt;
    }
};

template<typename T>
struct BindingType<std::basic_string<T>> {
    using String = std::basic_string<T>;
    static_assert(std::is_trivially_copyable<T>::value, "basic_string elements are memcpy'd");
    typedef struct {
        size_t length;
        T data[1]; // trailing data
    }* WireType;
    static WireType toWireType(const String& v, rvp::default_tag) {
        WireType wt = (WireType)malloc(sizeof(size_t) + v.length() * sizeof(T));
        wt->length = v.length();
        memcpy(wt->data, v.data(), v.length() * sizeof(T));
        return wt;
    }
    static String fromWireType(WireType v) {
        return String(v->data, v->length);
    }
};

template<typename T>
struct BindingType<const T> : public BindingType<T> {
};

template<typename T>
struct BindingType<T&> : public BindingType<T> {
};

template<typename T>
struct BindingType<T&&> {
    typedef typename BindingType<T>::WireType WireType;
    static T fromWireType(WireType wt) {
        return BindingType<T>::fromWireType(wt);
    }
};

template<typename T>
struct BindingType<T*> {
    typedef T* WireType;

    static WireType toWireType(T* p, rvp::default_tag) {
        return p;
    }

    static WireType toWireType(T* p, rvp::take_ownership) {
        return p;
    }

    static WireType toWireType(T* p, rvp::reference) {
        return p;
    }

    static T* fromWireType(WireType wt) {
        return wt;
    }
};

template<typename T>
struct GenericBindingType {
    typedef typename std::remove_reference<T>::type ActualT;
    typedef ActualT* WireType;

    template<typename R>
    static WireType toWireType(R&& v, rvp::default_tag) {
        return new ActualT(v);
    }

    template<typename R>
    static WireType toWireType(R&& v, rvp::take_ownership) {
        return new ActualT(std::move(v));
    }

    template<typename R>
    static WireType toWireType(R&& v, rvp::reference) {
        return &v;
    }

    static ActualT& fromWireType(WireType p) {
        return *p;
    }
};

template<typename T>
struct GenericBindingType<std::unique_ptr<T>> {
    typedef typename BindingType<T*>::WireType WireType;

    static WireType toWireType(std::unique_ptr<T> p, rvp::default_tag) {
        return BindingType<T*>::toWireType(p.release(), rvp::default_tag{});
    }

    static std::unique_ptr<T> fromWireType(WireType wt) {
        return std::unique_ptr<T>(BindingType<T*>::fromWireType(wt));
    }
};

template<typename Enum>
struct EnumBindingType {
    typedef Enum WireType;

    static WireType toWireType(Enum v, rvp::default_tag) {
        return v;
    }
    static Enum fromWireType(WireType v) {
        return v;
    }
};

// catch-all generic binding
template<typename T, typename>
struct BindingType : std::conditional<
    std::is_enum<T>::value,
    EnumBindingType<T>,
    GenericBindingType<T> >::type
{};

template<typename T>
auto toWireType(T&& v) -> typename BindingType<T>::WireType {
    return BindingType<T>::toWireType(std::forward<T>(v));
}

template<typename T>
constexpr bool typeSupportsMemoryView() {
    return (std::is_floating_point<T>::value &&
                (sizeof(T) == 4 || sizeof(T) == 8)) ||
            (std::is_integral<T>::value &&
                (sizeof(T) == 1 || sizeof(T) == 2 ||
                 sizeof(T) == 4 || sizeof(T) == 8));
}

} // namespace internal

template<typename ElementType>
struct EMBIND_VISIBILITY_DEFAULT memory_view {
    memory_view() = delete;
    explicit memory_view(size_t size, const ElementType* data)
        : size(size)
        , data(data)
    {}

    const size_t size; // in elements, not bytes
    const void* const data;
};

// Note that 'data' is marked const just so it can accept both
// const and nonconst pointers.  It is certainly possible for
// JavaScript to modify the C heap through the typed array given,
// as it merely aliases the C heap.
template<typename T>
inline memory_view<T> typed_memory_view(size_t size, const T* data) {
    static_assert(internal::typeSupportsMemoryView<T>(),
        "type of typed_memory_view is invalid");
    return memory_view<T>(size, data);
}

namespace internal {

template<typename ElementType>
struct BindingType<memory_view<ElementType>> {
    // This non-word-sized WireType only works because I
    // happen to know that clang will pass aggregates as
    // pointers to stack elements and we never support
    // converting JavaScript typed arrays back into
    // memory_view.  (That is, fromWireType is not implemented
    // on the C++ side, nor is toWireType implemented in
    // JavaScript.)
    typedef memory_view<ElementType> WireType;
    static WireType toWireType(const memory_view<ElementType>& mv, rvp::default_tag) {
        return mv;
    }
};

}

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
        // Use decay to handle references to pointers e.g.(T*&)->(T*).
        typedef typename std::decay<InputType>::type DecayedType;
        typedef typename std::conditional<
            std::is_pointer<DecayedType>::value,
            internal::AllowedRawPointer<typename std::remove_pointer<DecayedType>::type>,
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

template<typename T>
struct isNotPolicy {
    static constexpr bool value = !isPolicy<T>::value;
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
using isAsync = std::disjunction<std::is_same<async, Policies>...>;

template<typename... Policies>
using isNonnullReturn = std::disjunction<std::is_same<nonnull<ret_val>, Policies>...>;

// Build a tuple type that contains all the types where the predicate is true.
// e.g. FilterTypes<std::is_integral, int, char, float> would return std::tuple<int, char>.
template <template <class> class Predicate, class... T>
using FilterTypes = decltype(std::tuple_cat(
        std::declval<
            typename std::conditional<
                Predicate<T>::value,
                std::tuple<T>,
                std::tuple<>
            >::type
        >()...
    ));

// Build a tuple that contains all the args where the predicate is true.
template<template <class> class Predicate, typename... Args>
auto Filter(Args&&... args) {
    return std::tuple_cat(
        std::get<Predicate<typename std::decay_t<Args>>::value ? 0 : 1>(
            std::make_tuple(
                [](auto&& arg) { return std::forward_as_tuple(std::forward<decltype(arg)>(arg)); },
                [](auto&&) { return std::tuple<>(); }
            )
        )(std::forward<Args>(args))...
    );
}

} // namespace internal

} // namespace emscripten
