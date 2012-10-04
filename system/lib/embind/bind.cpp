#include <emscripten/bind.h>

using namespace emscripten;

namespace emscripten {
    namespace internal {
        void registerStandardTypes() {
            static bool first = true;
            if (first) {
                first = false;

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
        }
    }
}
