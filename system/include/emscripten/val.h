#pragma once

#include <stdint.h> // uintptr_t
#include <emscripten/wire.h>

namespace emscripten {
    namespace internal {
        // Implemented in JavaScript.  Don't call these directly.
        extern "C" {
            typedef struct _EM_VAL* EM_VAL;
        
            void _emval_incref(EM_VAL value);
            void _emval_decref(EM_VAL value);
            EM_VAL _emval_new_object();
            EM_VAL _emval_new_long(long value);
            EM_VAL _emval_new_cstring(const char* str);
            EM_VAL _emval_get_property(EM_VAL object, const char* key);
            EM_VAL _emval_get_property_by_long(EM_VAL object, long key);
            EM_VAL _emval_get_property_by_unsigned_long(EM_VAL object, unsigned long key);
            void _emval_set_property(EM_VAL object, const char* key, EM_VAL value);
            void _emval_set_property_by_int(EM_VAL object, long key, EM_VAL value);
            void _emval_as(EM_VAL value, emscripten::internal::TYPEID returnType);
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
        static val object() {
            return val(internal::_emval_new_object());
        };

        static val take_ownership(internal::EM_VAL e) {
            return val(e);
        }

        explicit val(long l)
            : handle(internal::_emval_new_long(l))
        {}

        explicit val(const char* str)
            : handle(internal::_emval_new_cstring(str))
        {}

        val() = delete;

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

        val get(const char* key) const {
            return val(internal::_emval_get_property(handle, key));
        }

        val get(int key) const {
            return get(long(key));
        }

        val get(unsigned int key) const {
            typedef unsigned long T;
            return get(T(key));
        }

        val get(long key) const {
            return val(internal::_emval_get_property_by_long(handle, key));
        }

        val get(unsigned long key) const {
            return val(internal::_emval_get_property_by_unsigned_long(handle, key));
        }

        void set(const char* key, val v) {
            internal::_emval_set_property(handle, key, v.handle);
        }

        void set(long key, val v) {
            internal::_emval_set_property_by_int(handle, key, v.handle);
        }

        template<typename ...Args>
        val operator()(Args... args) {
            internal::ArgTypeList<Args...> argList;
            typedef internal::EM_VAL (*TypedCall)(
                internal::EM_VAL,
                unsigned,
                internal::TYPEID argTypes[],
                typename internal::BindingType<Args>::WireType...);
            TypedCall typedCall = reinterpret_cast<TypedCall>(&internal::_emval_call);
            return val(
                typedCall(
                    handle,
                    argList.count,
                    argList.types,
                    internal::toWireType(args)...));
        }

        template<typename ...Args>
        val call(const char* name, Args... args) {
            internal::ArgTypeList<Args...> argList;
            typedef internal::EM_VAL (*TypedCall)(
                internal::EM_VAL,
                const char* name,
                unsigned,
                internal::TYPEID argTypes[],
                typename internal::BindingType<Args>::WireType...);
            TypedCall typedCall = reinterpret_cast<TypedCall>(&internal::_emval_call_method);
            return val(
                typedCall(
                    handle,
                    name,
                    argList.count,
                    argList.types,
                    internal::toWireType(args)...));
        }

        template<typename ...Args>
        void call_void(const char* name, Args... args) {
            internal::ArgTypeList<Args...> argList;
            typedef void (*TypedCall)(
                internal::EM_VAL,
                const char* name,
                unsigned,
                internal::TYPEID argTypes[],
                typename internal::BindingType<Args>::WireType...);
            TypedCall typedCall = reinterpret_cast<TypedCall>(&internal::_emval_call_void_method);
            return typedCall(
                handle,
                name,
                argList.count,
                argList.types,
                internal::toWireType(args)...);
        }

        template<typename T>
        T as() const {
            typedef internal::BindingType<T> BT;

            typedef typename BT::WireType (*TypedAs)(
                internal::EM_VAL value,
                emscripten::internal::TYPEID returnType);
            TypedAs typedAs = reinterpret_cast<TypedAs>(&internal::_emval_as);

            typename BT::WireType wt = typedAs(handle, internal::TypeID<T>::get());
            internal::WireDeleter<T> deleter(wt);
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
                return val(v);
            }
        };
    }
}
