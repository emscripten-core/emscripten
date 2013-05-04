#pragma once

// A value moving between JavaScript and C++ has three representations:
// - The original JS value: a String
// - The native on-the-wire value: a stack-allocated char*, say
// - The C++ value: std::string
//
// We'll call the on-the-wire type WireType.

#include <cstdlib>
#include <memory>
#include <string>

namespace emscripten {
    namespace internal {
        typedef void (*GenericFunction)();

        typedef const struct _TYPEID* TYPEID;

        // This implementation is technically not legal, as it's not
        // required that two calls to typeid produce the same exact
        // std::type_info instance.  That said, it's likely to work
        // given Emscripten compiles everything into one binary.
        // Should it not work in the future: replace TypeID with an
        // int, and store all TypeInfo we see in a map, allocating new
        // TypeIDs as we add new items to the map.
        template<typename T>
        struct TypeID {
            static TYPEID get() {
                return reinterpret_cast<TYPEID>(&typeid(T));
            }
        };

        template<typename T>
        struct TypeID<std::unique_ptr<T>> {
            static TYPEID get() {
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
            static TYPEID get() {
                return reinterpret_cast<TYPEID>(&typeid(T*));
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

        // ArgTypes<>

        template<int Index, typename... Args>
        struct ArgTypes;

        template<int Index>
        struct ArgTypes<Index> {
            template<typename... Policies>
            static void fill(TYPEID* argTypes) {
            }
        };

        template<int Index, typename T, typename... Remaining>
        struct ArgTypes<Index, T, Remaining...> {
            template<typename... Policies>
            static void fill(TYPEID* argTypes) {
                typedef typename ExecutePolicies<Policies...>::template With<T, Index>::type TransformT;
                *argTypes = TypeID<TransformT>::get();
                return ArgTypes<Index + 1, Remaining...>::template fill<Policies...>(argTypes + 1);
            }
        };

        // WithPolicies<...>::ArgTypeList<...>
        template<typename... Policies>
        struct WithPolicies {
            template<typename... Args>
            struct ArgTypeList {
                ArgTypeList() {
                    count = sizeof...(Args);
                    ArgTypes<0, Args...>::template fill<Policies...>(types);
                }

                unsigned count;
                TYPEID types[sizeof...(Args)];
            };
        };

        // BindingType<T>

        template<typename T>
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
            static void destroy(WireType) {                         \
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
            static void destroy(WireType) {
            }
        };

        template<>
        struct BindingType<std::string> {
            typedef struct {
                size_t length;
                char data[1]; // trailing data
            }* WireType;
            static WireType toWireType(const std::string& v) {
                WireType wt = (WireType)malloc(sizeof(size_t) + v.length());
                wt->length = v.length();
                memcpy(wt->data, v.data(), v.length());
                return wt;
            }
            static std::string fromWireType(WireType v) {
                return std::string(v->data, v->length);
            }
            static void destroy(WireType v) {
                free(v);
            }
        };

        template<>
        struct BindingType<std::wstring> {
            typedef struct {
                size_t length;
                wchar_t data[1]; // trailing data
            }* WireType;
            static WireType toWireType(const std::wstring& v) {
                WireType wt = (WireType)malloc(sizeof(size_t) + v.length() * sizeof(wchar_t));
                wt->length = v.length();
                wmemcpy(wt->data, v.data(), v.length());
                return wt;
            }
            static std::wstring fromWireType(WireType v) {
                return std::wstring(v->data, v->length);
            }
            static void destroy(WireType v) {
                free(v);
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

            static void destroy(WireType p) {
                delete p;
            }
        };

        // Is this necessary?
        template<typename T>
        struct GenericBindingType<std::unique_ptr<T>> {
            typedef typename BindingType<T>::WireType WireType;

            static WireType toWireType(std::unique_ptr<T> p) {
                return BindingType<T>::toWireType(*p);
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
            static void destroy(WireType) {
            }
        };

        // catch-all generic binding
        template<typename T>
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
        struct WireDeleter {
            typedef typename BindingType<T>::WireType WireType;
            
            WireDeleter(WireType wt)
                : wt(wt)
            {}
            
            ~WireDeleter() {
                BindingType<T>::destroy(wt);
            }
            
            WireType wt;
        };
    }

    struct memory_view {
        enum class Type {
            Int8Array,
            Uint8Array,
            Int16Array,
            Uint16Array,
            Int32Array,
            Uint32Array,
            Float32Array,
            Float64Array,
        };

        memory_view() = delete;
        explicit memory_view(size_t size, const void* data)
            : type(Type::Uint8Array)
            , size(size)
            , data(data)
        {}
        explicit memory_view(Type type, size_t size, const void* data)
            : type(type)
            , size(size)
            , data(data)
        {}

        const Type type;
        const size_t size; // in elements, not bytes
        const void* const data;
    };

    inline memory_view typed_memory_view(size_t size, const int8_t* data) {
        return memory_view(memory_view::Type::Int8Array, size, data);
    }

    inline memory_view typed_memory_view(size_t size, const uint8_t* data) {
        return memory_view(memory_view::Type::Uint8Array, size, data);
    }

    inline memory_view typed_memory_view(size_t size, const int16_t* data) {
        return memory_view(memory_view::Type::Int16Array, size, data);
    }

    inline memory_view typed_memory_view(size_t size, const uint16_t* data) {
        return memory_view(memory_view::Type::Uint16Array, size, data);
    }

    inline memory_view typed_memory_view(size_t size, const int32_t* data) {
        return memory_view(memory_view::Type::Int32Array, size, data);
    }

    inline memory_view typed_memory_view(size_t size, const uint32_t* data) {
        return memory_view(memory_view::Type::Uint32Array, size, data);
    }

    inline memory_view typed_memory_view(size_t size, const float* data) {
        return memory_view(memory_view::Type::Float32Array, size, data);
    }

    inline memory_view typed_memory_view(size_t size, const double* data) {
        return memory_view(memory_view::Type::Float64Array, size, data);
    }

    namespace internal {
        template<>
        struct BindingType<memory_view> {
            // This non-word-sized WireType only works because I
            // happen to know that clang will pass aggregates as
            // pointers to stack elements and we never support
            // converting JavaScript typed arrays back into
            // memory_view.  (That is, fromWireType is not implemented
            // on the C++ side, nor is toWireType implemented in
            // JavaScript.)
            typedef memory_view WireType;
            static WireType toWireType(const memory_view& mv) {
                return mv;
            }
        };
    }
}
