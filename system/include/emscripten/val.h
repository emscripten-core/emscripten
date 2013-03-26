#pragma once

#include <stdint.h> // uintptr_t
#include <emscripten/wire.h>
#include <vector>

namespace emscripten {
    namespace internal {
        // Implemented in JavaScript.  Don't call these directly.
        extern "C" {
            typedef struct _EM_VAL* EM_VAL;
        
            void _emval_incref(EM_VAL value);
            void _emval_decref(EM_VAL value);

            EM_VAL _emval_new_array();
            EM_VAL _emval_new_object();
            EM_VAL _emval_undefined();
            EM_VAL _emval_null();
            EM_VAL _emval_new_cstring(const char*);
            void _emval_take_value(TYPEID type/*, ...*/);

            EM_VAL _emval_new(
                EM_VAL value,
                unsigned argCount,
                internal::TYPEID argTypes[]
                /*, ... */);

            EM_VAL _emval_get_global(const char* name);
            EM_VAL _emval_get_module_property(const char* name);
            EM_VAL _emval_get_property(EM_VAL object, EM_VAL key);
            void _emval_set_property(EM_VAL object, EM_VAL key, EM_VAL value);
            void _emval_as(EM_VAL value, TYPEID returnType);
            EM_VAL _emval_call(
                EM_VAL value,
                unsigned argCount,
                internal::TYPEID argTypes[]
                /*, ... */);
            EM_VAL _emval_call_method(
                EM_VAL value,
                const char* methodName,
                unsigned argCount,
                internal::TYPEID argTypes[]
                /*, ... */);
            void _emval_call_void_method(
                EM_VAL value,
                const char* methodName,
                unsigned argCount,
                internal::TYPEID argTypes[]
                /*, ...*/);
        }
    }

    class val {
    public:
        // missing operators:
        // * delete
        // * in
        // * instanceof
        // * typeof
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
            return val(internal::_emval_undefined());
        }

        static val null() {
            return val(internal::_emval_null());
        }

        static val take_ownership(internal::EM_VAL e) {
            return val(e);
        }

        static val global(const char* name) {
            return val(internal::_emval_get_global(name));
        }

        static val module_property(const char* name) {
            return val(internal::_emval_get_module_property(name));
        }

        template<typename T>
        explicit val(const T& value) {
            typedef internal::BindingType<T> BT;
            auto taker = reinterpret_cast<internal::EM_VAL (*)(internal::TYPEID, typename BT::WireType)>(&internal::_emval_take_value);
            handle = taker(internal::TypeID<T>::get(), BT::toWireType(value));
        }

        val() = delete;

        val(const char* v)
            : handle(internal::_emval_new_cstring(v)) 
        {}

        val(const val& v)
            : handle(v.handle)
        {
            internal::_emval_incref(handle);
        }

        ~val() {
            internal::_emval_decref(handle);
        }

        val& operator=(const val& v) {
            internal::_emval_incref(v.handle);
            internal::_emval_decref(handle);
            handle = v.handle;
            return *this;
        }

        bool hasOwnProperty(const char* key) const {
            return val::global("Object")["prototype"]["hasOwnProperty"].call("call", *this, val(key)).as<bool>();
        }

        template<typename... Args>
        val new_(Args... args) const {
            using namespace internal;

            WithPolicies<>::ArgTypeList<Args...> argList;
            // todo: this is awfully similar to operator(), can we
            // merge them somehow?
            typedef EM_VAL (*TypedNew)(
                EM_VAL,
                unsigned,
                TYPEID argTypes[],
                typename BindingType<Args>::WireType...);
            TypedNew typedNew = reinterpret_cast<TypedNew>(&_emval_new);
            return val(
                typedNew(
                    handle,
                    argList.count,
                    argList.types,
                    toWireType(args)...));
        }
        
        template<typename T>
        val operator[](const T& key) const {
            return val(internal::_emval_get_property(handle, val(key).handle));
        }

        template<typename T>
        void set(const T& key, val v) {
            internal::_emval_set_property(handle, val(key).handle, v.handle);
        }

        template<typename... Args>
        val operator()(Args... args) {
            using namespace internal;

            WithPolicies<>::ArgTypeList<Args...> argList;
            typedef EM_VAL (*TypedCall)(
                EM_VAL,
                unsigned,
                TYPEID argTypes[],
                typename BindingType<Args>::WireType...);
            TypedCall typedCall = reinterpret_cast<TypedCall>(&_emval_call);
            return val(
                typedCall(
                    handle,
                    argList.count,
                    argList.types,
                    toWireType(args)...));
        }

        template<typename ...Args>
        val call(const char* name, Args... args) const {
            using namespace internal;

            WithPolicies<>::ArgTypeList<Args...> argList;
            typedef EM_VAL (*TypedCall)(
                EM_VAL,
                const char* name,
                unsigned,
                TYPEID argTypes[],
                typename BindingType<Args>::WireType...);
            TypedCall typedCall = reinterpret_cast<TypedCall>(&_emval_call_method);
            return val(
                typedCall(
                    handle,
                    name,
                    argList.count,
                    argList.types,
                    toWireType(args)...));
        }

        template<typename ...Args>
        void call_void(const char* name, Args... args) const {
            using namespace internal;

            WithPolicies<>::ArgTypeList<Args...> argList;
            typedef void (*TypedCall)(
                EM_VAL,
                const char* name,
                unsigned,
                TYPEID argTypes[],
                typename BindingType<Args>::WireType...);
            TypedCall typedCall = reinterpret_cast<TypedCall>(&_emval_call_void_method);
            return typedCall(
                handle,
                name,
                argList.count,
                argList.types,
                toWireType(args)...);
        }

        template<typename T>
        T as() const {
            using namespace internal;

            typedef BindingType<T> BT;

            typedef typename BT::WireType (*TypedAs)(
                EM_VAL value,
                TYPEID returnType);
            TypedAs typedAs = reinterpret_cast<TypedAs>(&_emval_as);

            typename BT::WireType wt = typedAs(handle, TypeID<T>::get());
            WireDeleter<T> deleter(wt);
            return BT::fromWireType(wt);
        }

    private:
        // takes ownership, assumes handle already incref'd
        explicit val(internal::EM_VAL handle)
            : handle(handle)
        {}

        internal::EM_VAL handle;

        friend struct internal::BindingType<val>;
    };

    namespace internal {
        template<>
        struct BindingType<val> {
            typedef internal::EM_VAL WireType;
            static WireType toWireType(val v) {
                _emval_incref(v.handle);
                return v.handle;
            }
            static val fromWireType(WireType v) {
                return val::take_ownership(v);
            }
            static void destroy(WireType v) {
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
