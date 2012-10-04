#pragma once

// A value moving between JavaScript and C++ has three representations:
// - The original JS value: a String
// - The native on-the-wire value: a stack-allocated char*, say
// - The C++ value: std::string
//
// We'll call the on-the-wire type WireType.

namespace emscripten {
    namespace internal {
        typedef const struct _TYPEID* TYPEID;

        // This implementation is technically not legal, as it's not
        // required that two calls to typeid produce the same exact
        // std::type_info instance.  That said, it's likely to work.
        // Should it not work in the future: replace TypeID with
        // an int, and store all TypeInfo we see in a map, allocating
        // new TypeIDs as we add new items to the map.
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

        // count<>

        template<typename... Args>
        struct count;

        template<>
        struct count<> {
            enum { value = 0 };
        };

        template<typename T, typename... Args>
        struct count<T, Args...> {
            enum { value = 1 + count<Args...>::value };
        };

        // ArgTypeList<>

        template<typename... Args>
        struct ArgTypes;

        template<>
        struct ArgTypes<> {
            static void fill(TYPEID* argTypes) {
            }
        };

        template<typename T, typename... Args>
        struct ArgTypes<T, Args...> {
            static void fill(TYPEID* argTypes) {
                *argTypes = TypeID<T>::get();
                return ArgTypes<Args...>::fill(argTypes + 1);
            }
        };

        template<typename... Args>
        struct ArgTypeList {
            enum { args_count = count<Args...>::value };

            ArgTypeList() {
                count = args_count;
                ArgTypes<Args...>::fill(types);
            }

            unsigned count;
            TYPEID types[args_count];
        };

        // BindingType<T>

        template<typename T>
        struct BindingType;

#define EMSCRIPTEN_DEFINE_NATIVE_BINDING_TYPE(type)             \
        template<>                                              \
        struct BindingType<type> {                              \
            typedef type WireType;                              \
                                                                \
            constexpr static WireType toWireType(type v) {      \
                return v;                                       \
            }                                                   \
            constexpr static type fromWireType(WireType v) {    \
                return v;                                       \
            }                                                   \
            static void destroy(WireType) {                     \
            }                                                   \
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
            static WireType toWireType(std::string v) {
                return strdup(v.c_str());
            }
            static std::string fromWireType(char* v) {
                return std::string(v);
            }
        };

        template<>
        struct BindingType<const std::string&> {
            typedef char* WireType;
            static WireType toWireType(std::string v) {
                return strdup(v.c_str());
            }
            static std::string fromWireType(char* v) {
                return std::string(v);
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
