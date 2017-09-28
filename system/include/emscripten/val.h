#pragma once

#if __cplusplus < 201103L
#error Including <emscripten/val.h> requires building with -std=c++11 or newer!
#else

#include <stdint.h> // uintptr_t
#include <emscripten/wire.h>
#include <array>
#include <vector>


namespace emscripten {

    class val;

    namespace internal {

        template<typename WrapperType>
        val wrapped_extend(const std::string&, const val&);

        // Implemented in JavaScript.  Don't call these directly.
        extern "C" {
            void _emval_register_symbol(const char*);

            enum {
                _EMVAL_UNDEFINED = 1,
                _EMVAL_NULL = 2,
                _EMVAL_TRUE = 3,
                _EMVAL_FALSE = 4
            };

            typedef struct _EM_VAL* EM_VAL;
            typedef struct _EM_DESTRUCTORS* EM_DESTRUCTORS;
            typedef struct _EM_METHOD_CALLER* EM_METHOD_CALLER;
            typedef double EM_GENERIC_WIRE_TYPE;
            typedef const void* EM_VAR_ARGS;

            void _emval_incref(EM_VAL value);
            void _emval_decref(EM_VAL value);

            void _emval_run_destructors(EM_DESTRUCTORS handle);

            EM_VAL _emval_new_array();
            EM_VAL _emval_new_object();
            EM_VAL _emval_new_cstring(const char*);

            EM_VAL _emval_take_value(TYPEID type, EM_VAR_ARGS argv);

            EM_VAL _emval_new(
                EM_VAL value,
                unsigned argCount,
                const TYPEID argTypes[],
                EM_VAR_ARGS argv);

            EM_VAL _emval_get_global(const char* name);
            EM_VAL _emval_get_module_property(const char* name);
            EM_VAL _emval_get_property(EM_VAL object, EM_VAL key);
            void _emval_set_property(EM_VAL object, EM_VAL key, EM_VAL value);
            EM_GENERIC_WIRE_TYPE _emval_as(EM_VAL value, TYPEID returnType, EM_DESTRUCTORS* destructors);

            bool _emval_equals(EM_VAL first, EM_VAL second);
            bool _emval_strictly_equals(EM_VAL first, EM_VAL second);

            EM_VAL _emval_call(
                EM_VAL value,
                unsigned argCount,
                const TYPEID argTypes[],
                EM_VAR_ARGS argv);

            // DO NOT call this more than once per signature. It will
            // leak generated function objects!
            EM_METHOD_CALLER _emval_get_method_caller(
                unsigned argCount, // including return value
                const TYPEID argTypes[]);
            EM_GENERIC_WIRE_TYPE _emval_call_method(
                EM_METHOD_CALLER caller,
                EM_VAL handle,
                const char* methodName,
                EM_DESTRUCTORS* destructors,
                EM_VAR_ARGS argv);
            void _emval_call_void_method(
                EM_METHOD_CALLER caller,
                EM_VAL handle,
                const char* methodName,
                EM_VAR_ARGS argv);
            EM_VAL _emval_typeof(EM_VAL value);
        }

        template<const char* address>
        struct symbol_registrar {
            symbol_registrar() {
                internal::_emval_register_symbol(address);
            }
        };

        template<typename ReturnType, typename... Args>
        struct Signature {
            /*
            typedef typename BindingType<ReturnType>::WireType (*MethodCaller)(
                EM_VAL value,
                const char* methodName,
                EM_DESTRUCTORS* destructors,
                typename BindingType<Args>::WireType...);
            */

            static EM_METHOD_CALLER get_method_caller() {
                static EM_METHOD_CALLER mc = init_method_caller();
                return mc;
            }

        private:
            static EM_METHOD_CALLER init_method_caller() {
                WithPolicies<>::ArgTypeList<ReturnType, Args...> args;
                return _emval_get_method_caller(args.getCount(), args.getTypes());
            }
        };

        struct DestructorsRunner {
        public:
            explicit DestructorsRunner(EM_DESTRUCTORS d)
                : destructors(d)
            {}
            ~DestructorsRunner() {
                _emval_run_destructors(destructors);
            }

            DestructorsRunner(const DestructorsRunner&) = delete;
            void operator=(const DestructorsRunner&) = delete;

        private:
            EM_DESTRUCTORS destructors;
        };

        template<typename WireType>
        struct GenericWireTypeConverter {
            static WireType from(double wt) {
                return static_cast<WireType>(wt);
            }
        };

        template<typename Pointee>
        struct GenericWireTypeConverter<Pointee*> {
            static Pointee* from(double wt) {
                return reinterpret_cast<Pointee*>(static_cast<uintptr_t>(wt));
            }
        };

        template<typename T>
        T fromGenericWireType(double g) {
            typedef typename BindingType<T>::WireType WireType;
            WireType wt = GenericWireTypeConverter<WireType>::from(g);
            return BindingType<T>::fromWireType(wt);
        }

        template<typename... Args>
        struct PackSize;

        template<>
        struct PackSize<> {
            static constexpr size_t value = 0;
        };

        template<typename Arg, typename... Args>
        struct PackSize<Arg, Args...> {
            static constexpr size_t value = (sizeof(typename BindingType<Arg>::WireType) + 7) / 8 + PackSize<Args...>::value;
        };

        union GenericWireType {
            union {
                unsigned u;
                float f;
                const void* p;
            } w[2];
            double d;
        };
        static_assert(sizeof(GenericWireType) == 8, "GenericWireType must be 8 bytes");
        static_assert(alignof(GenericWireType) == 8, "GenericWireType must be 8-byte-aligned");

        inline void writeGenericWireType(GenericWireType*& cursor, float wt) {
            cursor->w[0].f = wt;
            ++cursor;
        }

        inline void writeGenericWireType(GenericWireType*& cursor, double wt) {
            cursor->d = wt;
            ++cursor;
        }

        template<typename T>
        void writeGenericWireType(GenericWireType*& cursor, T* wt) {
            cursor->w[0].p = wt;
            ++cursor;
        }

        template<typename ElementType>
        inline void writeGenericWireType(GenericWireType*& cursor, const memory_view<ElementType>& wt) {
            cursor->w[0].u = wt.size;
            cursor->w[1].p = wt.data;
            ++cursor;
        }

        template<typename T>
        void writeGenericWireType(GenericWireType*& cursor, T wt) {
            cursor->w[0].u = static_cast<unsigned>(wt);
            ++cursor;
        }

        inline void writeGenericWireTypes(GenericWireType*&) {
        }

        template<typename First, typename... Rest>
        EMSCRIPTEN_ALWAYS_INLINE void writeGenericWireTypes(GenericWireType*& cursor, First&& first, Rest&&... rest) {
            writeGenericWireType(cursor, BindingType<First>::toWireType(std::forward<First>(first)));
            writeGenericWireTypes(cursor, std::forward<Rest>(rest)...);
        }

        template<typename... Args>
        struct WireTypePack {
            WireTypePack(Args&&... args) {
                GenericWireType* cursor = elements.data();
                writeGenericWireTypes(cursor, std::forward<Args>(args)...);
            }

            operator EM_VAR_ARGS() const {
                return elements.data();
            }

        private:
            std::array<GenericWireType, PackSize<Args...>::value> elements;
        };

        template<typename ReturnType, typename... Args>
        struct MethodCaller {
            static ReturnType call(EM_VAL handle, const char* methodName, Args&&... args) {
                auto caller = Signature<ReturnType, Args...>::get_method_caller();

                WireTypePack<Args...> argv(std::forward<Args>(args)...);
                EM_DESTRUCTORS destructors;
                EM_GENERIC_WIRE_TYPE result = _emval_call_method(
                    caller,
                    handle,
                    methodName,
                    &destructors,
                    argv);
                DestructorsRunner rd(destructors);
                return fromGenericWireType<ReturnType>(result);
            }
        };

        template<typename... Args>
        struct MethodCaller<void, Args...> {
            static void call(EM_VAL handle, const char* methodName, Args&&... args) {
                auto caller = Signature<void, Args...>::get_method_caller();

                WireTypePack<Args...> argv(std::forward<Args>(args)...);
                _emval_call_void_method(
                    caller,
                    handle,
                    methodName,
                    argv);
            }
        };
    }

#define EMSCRIPTEN_SYMBOL(name)                                         \
    static const char name##_symbol[] = #name;                          \
    static const ::emscripten::internal::symbol_registrar<name##_symbol> name##_registrar

    class val {
    public:
        // missing operators:
        // * delete
        // * in
        // * instanceof
        // * ! ~ - + ++ --
        // * * / %
        // * + -
        // * << >> >>>
        // * < <= > >=
        // * == != === !==
        // * & ^ | && || ?:
        //
        // exposing void, comma, and conditional is unnecessary
        // same with: = += -= *= /= %= <<= >>= >>>= &= ^= |=

        static val array() {
            return val(internal::_emval_new_array());
        }

        static val object() {
            return val(internal::_emval_new_object());
        }

        static val undefined() {
            return val(internal::EM_VAL(internal::_EMVAL_UNDEFINED));
        }

        static val null() {
            return val(internal::EM_VAL(internal::_EMVAL_NULL));
        }

        static val take_ownership(internal::EM_VAL e) {
            return val(e);
        }

        static val global(const char* name = 0) {
            return val(internal::_emval_get_global(name));
        }

        static val module_property(const char* name) {
            return val(internal::_emval_get_module_property(name));
        }

        template<typename T>
        explicit val(T&& value) {
            using namespace internal;

            typedef internal::BindingType<T> BT;
            WireTypePack<T> argv(std::forward<T>(value));
            handle = _emval_take_value(
                internal::TypeID<T>::get(),
                argv);
        }

        val() = delete;

        explicit val(const char* v)
            : handle(internal::_emval_new_cstring(v))
        {}

        val(val&& v)
            : handle(v.handle)
        {
            v.handle = 0;
        }

        val(const val& v)
            : handle(v.handle)
        {
            internal::_emval_incref(handle);
        }

        ~val() {
            internal::_emval_decref(handle);
        }

        val& operator=(val&& v) {
            internal::_emval_decref(handle);
            handle = v.handle;
            v.handle = 0;
            return *this;
        }

        val& operator=(const val& v) {
            internal::_emval_incref(v.handle);
            internal::_emval_decref(handle);
            handle = v.handle;
            return *this;
        }

        bool hasOwnProperty(const char* key) const {
            return val::global("Object")["prototype"]["hasOwnProperty"].call<bool>("call", *this, val(key));
        }

        bool isNull() const {
            return handle == internal::EM_VAL(internal::_EMVAL_NULL);
        }

        bool isUndefined() const {
            return handle == internal::EM_VAL(internal::_EMVAL_UNDEFINED);
        }

        bool isTrue() const {
            return handle == internal::EM_VAL(internal::_EMVAL_TRUE);
        }

        bool isFalse() const {
            return handle == internal::EM_VAL(internal::_EMVAL_FALSE);
        }

        bool equals(const val& v) const {
            return internal::_emval_equals(handle, v.handle);
        }

        bool strictlyEquals(const val& v) const {
            return internal::_emval_strictly_equals(handle, v.handle);
        }

        template<typename... Args>
        val new_(Args&&... args) const {
            return internalCall(internal::_emval_new,std::forward<Args>(args)...);
        }

        template<typename T>
        val operator[](const T& key) const {
            return val(internal::_emval_get_property(handle, val(key).handle));
        }

        template<typename K>
        void set(const K& key, const val& v) {
            internal::_emval_set_property(handle, val(key).handle, v.handle);
        }

        template<typename K, typename V>
        void set(const K& key, const V& value) {
            internal::_emval_set_property(handle, val(key).handle, val(value).handle);
        }

        template<typename... Args>
        val operator()(Args&&... args) {
            return internalCall(internal::_emval_call, std::forward<Args>(args)...);
        }

        template<typename ReturnValue, typename... Args>
        ReturnValue call(const char* name, Args&&... args) const {
            using namespace internal;

            return MethodCaller<ReturnValue, Args...>::call(handle, name, std::forward<Args>(args)...);
        }

        template<typename T, typename ...Policies>
        T as(Policies...) const {
            using namespace internal;

            typedef BindingType<T> BT;
            typename WithPolicies<Policies...>::template ArgTypeList<T> targetType;

            EM_DESTRUCTORS destructors;
            EM_GENERIC_WIRE_TYPE result = _emval_as(
                handle,
                targetType.getTypes()[0],
                &destructors);
            DestructorsRunner dr(destructors);
            return fromGenericWireType<T>(result);
        }

// If code is not being compiled with GNU extensions enabled, typeof() is not a reserved keyword, so support that as a member function.
#if __STRICT_ANSI__
        val typeof() const {
            return val(_emval_typeof(handle));
        }
#endif

// Prefer calling val::typeOf() over val::typeof(), since this form works in both C++11 and GNU++11 build modes. "typeof" is a reserved word in GNU++11 extensions.
        val typeOf() const {
            return val(_emval_typeof(handle));
        }

    private:
        // takes ownership, assumes handle already incref'd
        explicit val(internal::EM_VAL handle)
            : handle(handle)
        {}

        template<typename WrapperType>
        friend val internal::wrapped_extend(const std::string& , const val& );

        internal::EM_VAL __get_handle() const {
            return handle;
        }

        template<typename Implementation, typename... Args>
        val internalCall(Implementation impl, Args&&... args)const {
            using namespace internal;

            WithPolicies<>::ArgTypeList<Args...> argList;
            WireTypePack<Args...> argv(std::forward<Args>(args)...);
            return val(
                impl(
                    handle,
                    argList.getCount(),
                    argList.getTypes(),
                    argv));
        }

        internal::EM_VAL handle;

        friend struct internal::BindingType<val>;
    };

    namespace internal {
        template<>
        struct BindingType<val> {
            typedef internal::EM_VAL WireType;
            static WireType toWireType(const val& v) {
                _emval_incref(v.handle);
                return v.handle;
            }
            static val fromWireType(WireType v) {
                return val::take_ownership(v);
            }
        };
    }

    template<typename T>
    std::vector<T> vecFromJSArray(val v) {
        auto l = v["length"].as<unsigned>();

        std::vector<T> rv;
        for(unsigned i = 0; i < l; ++i) {
            rv.push_back(v[i].as<T>());
        }

        return rv;
    };
}

#endif // ~C++11 version check
