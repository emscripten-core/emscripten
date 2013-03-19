#include <emscripten/bind.h>
#include <../lib/libcxxabi/src/private_typeinfo.h>
#include <../lib/libcxxabi/include/cxxabi.h>
#include <list>
#include <vector>
#include <typeinfo>
#include <algorithm>
#include <emscripten/emscripten.h>

using namespace emscripten;

namespace emscripten {
    namespace internal {
        extern "C" {
            const char* EMSCRIPTEN_KEEPALIVE __typeName(const std::type_info* ti) {
                size_t nameLen = std::min(strlen(ti->name()), 1024U);
                char* name = (char *)malloc(nameLen+1);
                int stat;

                __cxxabiv1::__cxa_demangle(ti->name(), name, &nameLen, &stat);

                if (stat != 0) {
                    strncpy(name, ti->name(), nameLen);
                    name[nameLen] = '\0';
                }
                return name;
            }

            size_t EMSCRIPTEN_KEEPALIVE __peek32(size_t p) {
                return *reinterpret_cast<size_t*>(p);
            }

        }

        JSInterface* create_js_interface(EM_VAL e) {
            return new JSInterface(e);
        }
    }
}

EMSCRIPTEN_BINDINGS(native_and_builtin_types) {
    using namespace emscripten::internal;

    _embind_register_void(TypeID<void>::get(), "void");
    
    _embind_register_bool(TypeID<bool>::get(), "bool", true, false);

    _embind_register_integer(TypeID<char>::get(), "char");
    _embind_register_integer(TypeID<signed char>::get(), "signed char");
    _embind_register_integer(TypeID<unsigned char>::get(), "unsigned char");
    _embind_register_integer(TypeID<signed short>::get(), "short");
    _embind_register_integer(TypeID<unsigned short>::get(), "unsigned short");
    _embind_register_integer(TypeID<signed int>::get(), "int");
    _embind_register_integer(TypeID<unsigned int>::get(), "unsigned int");
    _embind_register_integer(TypeID<signed long>::get(), "long");
    _embind_register_integer(TypeID<unsigned long>::get(), "unsigned long");
    
    _embind_register_float(TypeID<float>::get(), "float");
    _embind_register_float(TypeID<double>::get(), "double");
    
    _embind_register_cstring(TypeID<std::string>::get(), "std::string");
    _embind_register_emval(TypeID<val>::get(), "emscripten::val");
}
