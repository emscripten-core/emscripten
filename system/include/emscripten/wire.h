/*
 * Copyright 2012 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#pragma once

#if __cplusplus < 201103L
#error Including <emscripten/wire.h> requires building with -std=c++11 or newer!
#endif

// A value moving between JavaScript and C++ has three representations:
// - The original JS value: a String
// - The native on-the-wire value: a stack-allocated char*, say
// - The C++ value: std::string
//
// We'll call the on-the-wire type WireType.

#include <stdio.h>
#include <cstdlib>
#include <memory>
#include <string>

#define EMSCRIPTEN_ALWAYS_INLINE __attribute__((always_inline))

namespace emscripten {
    #ifndef EMSCRIPTEN_HAS_UNBOUND_TYPE_NAMES
    #define EMSCRIPTEN_HAS_UNBOUND_TYPE_NAMES 1
    #endif


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
        struct CanonicalizedID {
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
                typedef typename Canonicalized<T>::type C;
                if(has_unbound_type_names) {
#if __has_feature(cxx_rtti)
                    return &typeid(T);
#else
                    static_assert(!has_unbound_type_names,
                        "Unbound type names are illegal with RTTI disabled. "
                        "Either add -DEMSCRIPTEN_HAS_UNBOUND_TYPE_NAMES=0 to or remove -fno-rtti "
                        "from the compiler arguments");
#endif
                }

                return CanonicalizedID<C>::get();
            }
        };

        template<typename T>
        constexpr TYPEID getLightTypeID(const T& value) {
            typedef typename Canonicalized<T>::type C;
            if(has_unbound_type_names) {
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
            static constexpr TYPEID get() {
                return TypeID<T>::get();
            }
        };

        template<typename T>
        struct TypeID<T*> {
            static_assert(!std::is_pointer<T*>::value, "Implicitly binding raw pointers is illegal.  Specify allow_raw_pointer<arg<?>>");
        };

        template<typename T>
        struct AllowedRawPointer {
        };

        template<typename T>
        struct TypeID<AllowedRawPointer<T>> {
            static constexpr TYPEID get() {
                return LightTypeID<T*>::get();
            }
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

        // BindingType<T>

        // The second typename is an unused stub so it's possible to
        // specialize groups of classes via SFINAE.
        template<typename T, typename = void>
        struct BindingType;

#define EMSCRIPTEN_DEFINE_NATIVE_BINDING_TYPE(type)                 \
        template<>                                                  \
        struct BindingType<type> {                                  \
            typedef type WireType;                                  \
            constexpr static WireType toWireType(const type& v) {   \
                return v;                                           \
            }                                                       \
            constexpr static type fromWireType(WireType v) {        \
                return v;                                           \
            }                                                       \
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

        template<>
        struct BindingType<void> {
            typedef void WireType;
        };

        template<>
        struct BindingType<bool> {
            typedef bool WireType;
            static WireType toWireType(bool b) {
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
            static WireType toWireType(const String& v) {
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
        struct BindingType<const T&> : public BindingType<T> {
        };

        template<typename T>
        struct BindingType<T&&> {
            typedef typename BindingType<T>::WireType WireType;
            static WireType toWireType(const T& v) {
                return BindingType<T>::toWireType(v);
            }
            static T fromWireType(WireType wt) {
                return BindingType<T>::fromWireType(wt);
            }
        };

        template<typename T>
        struct BindingType<T*> {
            typedef T* WireType;
            static WireType toWireType(T* p) {
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

            static WireType toWireType(const T& v) {
                return new T(v);
            }

            static WireType toWireType(T&& v) {
                return new T(std::forward<T>(v));
            }

            static ActualT& fromWireType(WireType p) {
                return *p;
            }
        };

        template<typename T>
        struct GenericBindingType<std::unique_ptr<T>> {
            typedef typename BindingType<T*>::WireType WireType;

            static WireType toWireType(std::unique_ptr<T> p) {
                return BindingType<T*>::toWireType(p.release());
            }

            static std::unique_ptr<T> fromWireType(WireType wt) {
                return std::unique_ptr<T>(BindingType<T*>::fromWireType(wt));
            }
        };

        template<typename Enum>
        struct EnumBindingType {
            typedef Enum WireType;

            static WireType toWireType(Enum v) {
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
                        (sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4));
        }
    }

    template<typename ElementType>
    struct memory_view {
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
            static WireType toWireType(const memory_view<ElementType>& mv) {
                return mv;
            }
        };
    }
}
