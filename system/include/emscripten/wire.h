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
        
        // count<>

        template<typename... Args>
        struct count;

        template<>
        struct count<> {
            constexpr static unsigned value = 0;
        };

        template<typename T, typename... Args>
        struct count<T, Args...> {
            constexpr static unsigned value = 1 + count<Args...>::value;
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
                enum { args_count = count<Args...>::value };

                ArgTypeList() {
                    count = args_count;
                    ArgTypes<0, Args...>::template fill<Policies...>(types);
                }

                unsigned count;
                TYPEID types[args_count];
            };
        };

        // BindingType<T>

        template<typename T>
        struct BindingType;

#define EMSCRIPTEN_DEFINE_NATIVE_BINDING_TYPE(type)                 \
        template<>                                                  \
        struct BindingType<type> {                                  \
            typedef type WireType;                                  \
            constexpr static WireType toWireType(const type&  v) {  \
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
            typedef char* WireType;
            static WireType toWireType(const std::string& v) {
                return strdup(v.c_str());
            }
            static std::string fromWireType(char* v) {
                return std::string(v);
            }
            static void destroy(WireType v) {
                free(v);
            }
        };

        template<typename T>
        struct BindingType<const T&> {
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
        struct BindingType<std::shared_ptr<T>> {
            typedef std::shared_ptr<T> shared_ptr;
            typedef std::shared_ptr<T>* WireType;

            static WireType toWireType(shared_ptr p) {
                return new shared_ptr(p);
            }

            static shared_ptr fromWireType(WireType wt) {
                if (wt) {
                    return shared_ptr(*wt);
                } else {
                    return shared_ptr();
                }
            }

            static void destroy(WireType p) {
                delete p;
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

        template<typename T>
        struct GenericBindingType {
            typedef typename std::remove_reference<T>::type ActualT;
            typedef ActualT* WireType;

            struct Marshaller {
                explicit Marshaller(WireType wt)
                : wireType(wt)
                {}

                Marshaller(Marshaller&& wt)
                : wireType(wt.wireType)
                {
                    wt.wireType = 0;
                }

                operator ActualT&() const {
                    return *wireType;
                }

            private:
                Marshaller() = delete;
                Marshaller(const Marshaller&) = delete;
                ActualT* wireType;
            };

            static WireType toWireType(T v) {
                return new T(v);
            }

            static Marshaller fromWireType(WireType p) {
                return Marshaller(p);
            }

            static void destroy(WireType p) {
                delete p;
            }
        };

        template<typename T>
        struct GenericBindingType<std::unique_ptr<T>> {
            typedef typename BindingType<T>::WireType WireType;

            static WireType toWireType(std::unique_ptr<T> p) {
                return BindingType<T>::toWireType(*p);
            }
        };

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

        // catch-all generic binding
        template<typename T>
        struct BindingType : std::conditional<
            std::is_enum<T>::value,
            EnumBindingType<T>,
            GenericBindingType<T> >::type
        {};

        template<typename T>
        auto toWireType(const T& v) -> typename BindingType<T>::WireType {
            return BindingType<T>::toWireType(v);
        }
    }
}
