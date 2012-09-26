#include <emscripten/bind.h>

using namespace emscripten;

namespace emscripten {
    namespace internal {
        void registerStandardTypes() {
            static bool first = true;
            if (first) {
                first = false;

                _embind_register_void(getTypeID<void>(), "void");

                _embind_register_bool(getTypeID<bool>(), "bool", true, false);

                _embind_register_integer(getTypeID<char>(), "char");
                _embind_register_integer(getTypeID<signed char>(), "signed char");
                _embind_register_integer(getTypeID<unsigned char>(), "unsigned char");
                _embind_register_integer(getTypeID<signed short>(), "short");
                _embind_register_integer(getTypeID<unsigned short>(), "unsigned short");
                _embind_register_integer(getTypeID<signed int>(), "int");
                _embind_register_integer(getTypeID<unsigned int>(), "unsigned int");
                _embind_register_integer(getTypeID<signed long>(), "long");
                _embind_register_integer(getTypeID<unsigned long>(), "unsigned long");

                _embind_register_float(getTypeID<float>(), "float");
                _embind_register_float(getTypeID<double>(), "double");

                _embind_register_cstring(getTypeID<std::string>(), "std::string");
                _embind_register_emval(getTypeID<val>(), "emscripten::val");
            }
        }
    }
}
