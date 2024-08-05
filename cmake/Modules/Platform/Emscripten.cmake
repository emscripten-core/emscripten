# This file is a 'toolchain description file' for CMake.
# It teaches CMake about the Emscripten compiler, so that CMake can generate
# makefiles from CMakeLists.txt that invoke emcc.

# To use this toolchain file with CMake, invoke CMake with the following command
# line parameters:
# cmake -DCMAKE_TOOLCHAIN_FILE=<EmscriptenRoot>/cmake/Modules/Platform/Emscripten.cmake
#       -DCMAKE_BUILD_TYPE=<Debug|RelWithDebInfo|Release|MinSizeRel>
#       -G "Unix Makefiles" (Linux and macOS)
#       -G "MinGW Makefiles" (Windows)
#       <path/to/CMakeLists.txt> # Note, pass in here ONLY the path to the file, not the filename 'CMakeLists.txt' itself.

# After that, build the generated Makefile with the command 'make'. On Windows,
# you may download and use 'mingw32-make' instead.

# The following variable describes the target OS we are building to.
set(CMAKE_SYSTEM_NAME Emscripten)
set(CMAKE_SYSTEM_VERSION 1)

set(CMAKE_CROSSCOMPILING TRUE)
set_property(GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS FALSE)

# Advertise Emscripten as a 32-bit platform (as opposed to
# CMAKE_SYSTEM_PROCESSOR=x86_64 for 64-bit platform), since some projects (e.g.
# OpenCV) use this to detect bitness.
# Allow users to ovewrite this on the command line with -DEMSCRIPTEN_SYSTEM_PROCESSOR=arm.
if (NOT DEFINED EMSCRIPTEN_SYSTEM_PROCESSOR)
  set(EMSCRIPTEN_SYSTEM_PROCESSOR x86)
endif()
set(CMAKE_SYSTEM_PROCESSOR ${EMSCRIPTEN_SYSTEM_PROCESSOR})

# Tell CMake how it should instruct the compiler to generate multiple versions
# of an outputted .so library: e.g. "libfoo.so, libfoo.so.1, libfoo.so.1.4" etc.
# This feature is activated if a shared library project has the property
# SOVERSION defined.
set(CMAKE_SHARED_LIBRARY_SONAME_C_FLAG "-Wl,-soname,")

# In CMake, CMAKE_HOST_WIN32 is set when we are cross-compiling from Win32 to
# Emscripten:
# http://www.cmake.org/cmake/help/v2.8.12/cmake.html#variable:CMAKE_HOST_WIN32
# The variable WIN32 is set only when the target arch that will run the code
# will be WIN32, so unset WIN32 when cross-compiling.
set(WIN32)

# The same logic as above applies for APPLE and CMAKE_HOST_APPLE, so unset
# APPLE.
set(APPLE)

# And for UNIX and CMAKE_HOST_UNIX. However, Emscripten is often able to mimic
# being a Linux/Unix system, in which case a lot of existing CMakeLists.txt
# files can be configured for Emscripten while assuming UNIX build, so this is
# left enabled.
set(UNIX 1)

# Do a no-op access on the CMAKE_TOOLCHAIN_FILE variable so that CMake will not
# issue a warning on it being unused.
if (CMAKE_TOOLCHAIN_FILE)
endif()

# Locate where the Emscripten compiler resides in relative to this toolchain file.
if (NOT DEFINED EMSCRIPTEN_ROOT_PATH)
  get_filename_component(GUESS_EMSCRIPTEN_ROOT_PATH "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)
  if (EXISTS "${GUESS_EMSCRIPTEN_ROOT_PATH}/emranlib")
    set(EMSCRIPTEN_ROOT_PATH "${GUESS_EMSCRIPTEN_ROOT_PATH}")
  else()
    # If not found by above search, locate using the EMSCRIPTEN environment variable.
    set(EMSCRIPTEN_ROOT_PATH "$ENV{EMSCRIPTEN}")
    # Abort if not found.
    if ("${EMSCRIPTEN_ROOT_PATH}" STREQUAL "")
      message(FATAL_ERROR "Could not locate the Emscripten compiler toolchain directory! Either set the EMSCRIPTEN environment variable, or pass -DEMSCRIPTEN_ROOT_PATH=xxx to CMake to explicitly specify the location of the compiler!")
    endif()
  endif()
endif()

# Normalize, convert Windows backslashes to forward slashes or CMake will crash.
get_filename_component(EMSCRIPTEN_ROOT_PATH "${EMSCRIPTEN_ROOT_PATH}" ABSOLUTE)

list(APPEND CMAKE_MODULE_PATH "${EMSCRIPTEN_ROOT_PATH}/cmake/Modules")

if (CMAKE_HOST_WIN32)
  set(EMCC_SUFFIX ".bat")
else()
  set(EMCC_SUFFIX "")
endif()

# Specify the compilers to use for C and C++
set(CMAKE_C_COMPILER "${EMSCRIPTEN_ROOT_PATH}/emcc${EMCC_SUFFIX}")
set(CMAKE_CXX_COMPILER "${EMSCRIPTEN_ROOT_PATH}/em++${EMCC_SUFFIX}")
set(CMAKE_NM "${EMSCRIPTEN_ROOT_PATH}/emnm${EMCC_SUFFIX}")
set(CMAKE_AR "${EMSCRIPTEN_ROOT_PATH}/emar${EMCC_SUFFIX}")
set(CMAKE_RANLIB "${EMSCRIPTEN_ROOT_PATH}/emranlib${EMCC_SUFFIX}")
set(CMAKE_C_COMPILER_AR "${CMAKE_AR}")
set(CMAKE_CXX_COMPILER_AR "${CMAKE_AR}")
set(CMAKE_C_COMPILER_RANLIB "${CMAKE_RANLIB}")
set(CMAKE_CXX_COMPILER_RANLIB "${CMAKE_RANLIB}")

# Capture the Emscripten version to EMSCRIPTEN_VERSION variable.
if (NOT EMSCRIPTEN_VERSION)
  execute_process(COMMAND "${CMAKE_C_COMPILER}" "-v" RESULT_VARIABLE _cmake_compiler_result ERROR_VARIABLE _cmake_compiler_output OUTPUT_QUIET)
  if (NOT _cmake_compiler_result EQUAL 0)
    message(FATAL_ERROR "Failed to fetch Emscripten version information with command \"'${CMAKE_C_COMPILER}' -v\"!\n"
                        "Process returned with error code ${_cmake_compiler_result}.\n"
                        "Output:\n${_cmake_compiler_output}")
  endif()
  string(REGEX MATCH "emcc \\(.*\\) ([0-9\\.]+)" _dummy_unused "${_cmake_compiler_output}")
  if (NOT CMAKE_MATCH_1)
    message(FATAL_ERROR "Failed to regex parse Emscripten compiler version from version string: ${_cmake_compiler_output}")
  endif()

  set(EMSCRIPTEN_VERSION "${CMAKE_MATCH_1}")
endif()

# Don't allow CMake to autodetect the compiler, since this is quite slow with
# Emscripten.
# Pass -DEMSCRIPTEN_FORCE_COMPILERS=OFF to disable (sensible mostly only for
# testing/debugging purposes).
option(EMSCRIPTEN_FORCE_COMPILERS "Force C/C++ compiler" ON)
if (EMSCRIPTEN_FORCE_COMPILERS)

  # Detect version of the 'emcc' executable. Note that for CMake, we tell it the
  # version of the Clang compiler and not the version of Emscripten, because
  # CMake understands Clang better.
  # Toolchain script is interpreted multiple times, so don't rerun the check if
  # already done before.
  if (NOT CMAKE_C_COMPILER_VERSION)
    execute_process(COMMAND "${CMAKE_C_COMPILER}" "-v" RESULT_VARIABLE _cmake_compiler_result ERROR_VARIABLE _cmake_compiler_output OUTPUT_QUIET)
    if (NOT _cmake_compiler_result EQUAL 0)
      message(FATAL_ERROR "Failed to fetch compiler version information with command \"'${CMAKE_C_COMPILER}' -v\"! Process returned with error code ${_cmake_compiler_result}.")
    endif()
    if (NOT "${_cmake_compiler_output}" MATCHES "[Ee]mscripten")
      message(FATAL_ERROR "System LLVM compiler cannot be used to build with Emscripten! Check Emscripten's LLVM toolchain location in .emscripten configuration file, and make sure to point CMAKE_C_COMPILER to where emcc is located. (was pointing to \"${CMAKE_C_COMPILER}\")")
    endif()
    string(REGEX MATCH "clang version ([0-9\\.]+)" _dummy_unused "${_cmake_compiler_output}")
    if (NOT CMAKE_MATCH_1)
      message(FATAL_ERROR "Failed to regex parse Clang compiler version from version string: ${_cmake_compiler_output}")
    endif()

    set(CMAKE_C_COMPILER_VERSION "${CMAKE_MATCH_1}")
    set(CMAKE_CXX_COMPILER_VERSION "${CMAKE_MATCH_1}")
    if (${CMAKE_C_COMPILER_VERSION} VERSION_LESS 3.9.0)
      message(WARNING "CMAKE_C_COMPILER version looks too old. Was ${CMAKE_C_COMPILER_VERSION}, should be at least 3.9.0.")
    endif()
  endif()

  set(CMAKE_C_COMPILER_ID_RUN TRUE)
  set(CMAKE_C_COMPILER_FORCED TRUE)
  set(CMAKE_C_COMPILER_WORKS TRUE)
  set(CMAKE_C_COMPILER_ID Clang)
  set(CMAKE_C_COMPILER_FRONTEND_VARIANT GNU)
  set(CMAKE_C_STANDARD_COMPUTED_DEFAULT 11)

  set(CMAKE_CXX_COMPILER_ID_RUN TRUE)
  set(CMAKE_CXX_COMPILER_FORCED TRUE)
  set(CMAKE_CXX_COMPILER_WORKS TRUE)
  set(CMAKE_CXX_COMPILER_ID Clang)
  set(CMAKE_CXX_COMPILER_FRONTEND_VARIANT GNU)
  set(CMAKE_CXX_STANDARD_COMPUTED_DEFAULT 98)

  set(CMAKE_C_PLATFORM_ID "emscripten")
  set(CMAKE_CXX_PLATFORM_ID "emscripten")

  if ("${CMAKE_VERSION}" VERSION_LESS "3.8")
    set(CMAKE_C_COMPILE_FEATURES "c_function_prototypes;c_restrict;c_variadic_macros;c_static_assert")
    set(CMAKE_C90_COMPILE_FEATURES "c_function_prototypes")
    set(CMAKE_C99_COMPILE_FEATURES "c_restrict;c_variadic_macros")
    set(CMAKE_C11_COMPILE_FEATURES "c_static_assert")

    set(CMAKE_CXX_COMPILE_FEATURES "cxx_template_template_parameters;cxx_alias_templates;cxx_alignas;cxx_alignof;cxx_attributes;cxx_auto_type;cxx_constexpr;cxx_decltype;cxx_decltype_incomplete_return_types;cxx_default_function_template_args;cxx_defaulted_functions;cxx_defaulted_move_initializers;cxx_delegating_constructors;cxx_deleted_functions;cxx_enum_forward_declarations;cxx_explicit_conversions;cxx_extended_friend_declarations;cxx_extern_templates;cxx_final;cxx_func_identifier;cxx_generalized_initializers;cxx_inheriting_constructors;cxx_inline_namespaces;cxx_lambdas;cxx_local_type_template_args;cxx_long_long_type;cxx_noexcept;cxx_nonstatic_member_init;cxx_nullptr;cxx_override;cxx_range_for;cxx_raw_string_literals;cxx_reference_qualified_functions;cxx_right_angle_brackets;cxx_rvalue_references;cxx_sizeof_member;cxx_static_assert;cxx_strong_enums;cxx_thread_local;cxx_trailing_return_types;cxx_unicode_literals;cxx_uniform_initialization;cxx_unrestricted_unions;cxx_user_literals;cxx_variadic_macros;cxx_variadic_templates;cxx_aggregate_default_initializers;cxx_attribute_deprecated;cxx_binary_literals;cxx_contextual_conversions;cxx_decltype_auto;cxx_digit_separators;cxx_generic_lambdas;cxx_lambda_init_captures;cxx_relaxed_constexpr;cxx_return_type_deduction;cxx_variable_templates")
    set(CMAKE_CXX98_COMPILE_FEATURES "cxx_template_template_parameters")
    set(CMAKE_CXX11_COMPILE_FEATURES "cxx_alias_templates;cxx_alignas;cxx_alignof;cxx_attributes;cxx_auto_type;cxx_constexpr;cxx_decltype;cxx_decltype_incomplete_return_types;cxx_default_function_template_args;cxx_defaulted_functions;cxx_defaulted_move_initializers;cxx_delegating_constructors;cxx_deleted_functions;cxx_enum_forward_declarations;cxx_explicit_conversions;cxx_extended_friend_declarations;cxx_extern_templates;cxx_final;cxx_func_identifier;cxx_generalized_initializers;cxx_inheriting_constructors;cxx_inline_namespaces;cxx_lambdas;cxx_local_type_template_args;cxx_long_long_type;cxx_noexcept;cxx_nonstatic_member_init;cxx_nullptr;cxx_override;cxx_range_for;cxx_raw_string_literals;cxx_reference_qualified_functions;cxx_right_angle_brackets;cxx_rvalue_references;cxx_sizeof_member;cxx_static_assert;cxx_strong_enums;cxx_thread_local;cxx_trailing_return_types;cxx_unicode_literals;cxx_uniform_initialization;cxx_unrestricted_unions;cxx_user_literals;cxx_variadic_macros;cxx_variadic_templates")
    set(CMAKE_CXX14_COMPILE_FEATURES "cxx_aggregate_default_initializers;cxx_attribute_deprecated;cxx_binary_literals;cxx_contextual_conversions;cxx_decltype_auto;cxx_digit_separators;cxx_generic_lambdas;cxx_lambda_init_captures;cxx_relaxed_constexpr;cxx_return_type_deduction;cxx_variable_templates")
  else() # 3.8+
    set(CMAKE_C90_COMPILE_FEATURES "c_std_90;c_function_prototypes")
    set(CMAKE_C99_COMPILE_FEATURES "c_std_99;c_restrict;c_variadic_macros")
    set(CMAKE_C11_COMPILE_FEATURES "c_std_11;c_static_assert")

    set(CMAKE_CXX98_COMPILE_FEATURES "cxx_std_98;cxx_template_template_parameters")
    set(CMAKE_CXX11_COMPILE_FEATURES "cxx_std_11;cxx_alias_templates;cxx_alignas;cxx_alignof;cxx_attributes;cxx_auto_type;cxx_constexpr;cxx_decltype;cxx_decltype_incomplete_return_types;cxx_default_function_template_args;cxx_defaulted_functions;cxx_defaulted_move_initializers;cxx_delegating_constructors;cxx_deleted_functions;cxx_enum_forward_declarations;cxx_explicit_conversions;cxx_extended_friend_declarations;cxx_extern_templates;cxx_final;cxx_func_identifier;cxx_generalized_initializers;cxx_inheriting_constructors;cxx_inline_namespaces;cxx_lambdas;cxx_local_type_template_args;cxx_long_long_type;cxx_noexcept;cxx_nonstatic_member_init;cxx_nullptr;cxx_override;cxx_range_for;cxx_raw_string_literals;cxx_reference_qualified_functions;cxx_right_angle_brackets;cxx_rvalue_references;cxx_sizeof_member;cxx_static_assert;cxx_strong_enums;cxx_thread_local;cxx_trailing_return_types;cxx_unicode_literals;cxx_uniform_initialization;cxx_unrestricted_unions;cxx_user_literals;cxx_variadic_macros;cxx_variadic_templates")
    set(CMAKE_CXX14_COMPILE_FEATURES "cxx_std_14;cxx_aggregate_default_initializers;cxx_attribute_deprecated;cxx_binary_literals;cxx_contextual_conversions;cxx_decltype_auto;cxx_digit_separators;cxx_generic_lambdas;cxx_lambda_init_captures;cxx_relaxed_constexpr;cxx_return_type_deduction;cxx_variable_templates")
    set(CMAKE_CXX17_COMPILE_FEATURES "cxx_std_17")
    if ("${CMAKE_VERSION}" VERSION_LESS "3.12") # [3.8, 3.12)
      set(CMAKE_C_COMPILE_FEATURES "c_std_90;c_function_prototypes;c_std_99;c_restrict;c_variadic_macros;c_std_11;c_static_assert")
      set(CMAKE_CXX_COMPILE_FEATURES "cxx_std_98;cxx_template_template_parameters;cxx_std_11;cxx_alias_templates;cxx_alignas;cxx_alignof;cxx_attributes;cxx_auto_type;cxx_constexpr;cxx_decltype;cxx_decltype_incomplete_return_types;cxx_default_function_template_args;cxx_defaulted_functions;cxx_defaulted_move_initializers;cxx_delegating_constructors;cxx_deleted_functions;cxx_enum_forward_declarations;cxx_explicit_conversions;cxx_extended_friend_declarations;cxx_extern_templates;cxx_final;cxx_func_identifier;cxx_generalized_initializers;cxx_inheriting_constructors;cxx_inline_namespaces;cxx_lambdas;cxx_local_type_template_args;cxx_long_long_type;cxx_noexcept;cxx_nonstatic_member_init;cxx_nullptr;cxx_override;cxx_range_for;cxx_raw_string_literals;cxx_reference_qualified_functions;cxx_right_angle_brackets;cxx_rvalue_references;cxx_sizeof_member;cxx_static_assert;cxx_strong_enums;cxx_thread_local;cxx_trailing_return_types;cxx_unicode_literals;cxx_uniform_initialization;cxx_unrestricted_unions;cxx_user_literals;cxx_variadic_macros;cxx_variadic_templates;cxx_std_14;cxx_aggregate_default_initializers;cxx_attribute_deprecated;cxx_binary_literals;cxx_contextual_conversions;cxx_decltype_auto;cxx_digit_separators;cxx_generic_lambdas;cxx_lambda_init_captures;cxx_relaxed_constexpr;cxx_return_type_deduction;cxx_variable_templates;cxx_std_17")
    else() # 3.12+
      set(CMAKE_CXX20_COMPILE_FEATURES "cxx_std_20")
      if ("${CMAKE_VERSION}" VERSION_LESS "3.20") # [3.12, 3.20)
        set(CMAKE_C_COMPILE_FEATURES "c_std_90;c_function_prototypes;c_std_99;c_restrict;c_variadic_macros;c_std_11;c_static_assert")
        set(CMAKE_CXX_COMPILE_FEATURES "cxx_std_98;cxx_template_template_parameters;cxx_std_11;cxx_alias_templates;cxx_alignas;cxx_alignof;cxx_attributes;cxx_auto_type;cxx_constexpr;cxx_decltype;cxx_decltype_incomplete_return_types;cxx_default_function_template_args;cxx_defaulted_functions;cxx_defaulted_move_initializers;cxx_delegating_constructors;cxx_deleted_functions;cxx_enum_forward_declarations;cxx_explicit_conversions;cxx_extended_friend_declarations;cxx_extern_templates;cxx_final;cxx_func_identifier;cxx_generalized_initializers;cxx_inheriting_constructors;cxx_inline_namespaces;cxx_lambdas;cxx_local_type_template_args;cxx_long_long_type;cxx_noexcept;cxx_nonstatic_member_init;cxx_nullptr;cxx_override;cxx_range_for;cxx_raw_string_literals;cxx_reference_qualified_functions;cxx_right_angle_brackets;cxx_rvalue_references;cxx_sizeof_member;cxx_static_assert;cxx_strong_enums;cxx_thread_local;cxx_trailing_return_types;cxx_unicode_literals;cxx_uniform_initialization;cxx_unrestricted_unions;cxx_user_literals;cxx_variadic_macros;cxx_variadic_templates;cxx_std_14;cxx_aggregate_default_initializers;cxx_attribute_deprecated;cxx_binary_literals;cxx_contextual_conversions;cxx_decltype_auto;cxx_digit_separators;cxx_generic_lambdas;cxx_lambda_init_captures;cxx_relaxed_constexpr;cxx_return_type_deduction;cxx_variable_templates;cxx_std_17;cxx_std_20")
      else() # 3.20+
        set(CMAKE_CXX23_COMPILE_FEATURES "cxx_std_23")
        set(CMAKE_CXX_COMPILE_FEATURES "cxx_std_98;cxx_template_template_parameters;cxx_std_11;cxx_alias_templates;cxx_alignas;cxx_alignof;cxx_attributes;cxx_auto_type;cxx_constexpr;cxx_decltype;cxx_decltype_incomplete_return_types;cxx_default_function_template_args;cxx_defaulted_functions;cxx_defaulted_move_initializers;cxx_delegating_constructors;cxx_deleted_functions;cxx_enum_forward_declarations;cxx_explicit_conversions;cxx_extended_friend_declarations;cxx_extern_templates;cxx_final;cxx_func_identifier;cxx_generalized_initializers;cxx_inheriting_constructors;cxx_inline_namespaces;cxx_lambdas;cxx_local_type_template_args;cxx_long_long_type;cxx_noexcept;cxx_nonstatic_member_init;cxx_nullptr;cxx_override;cxx_range_for;cxx_raw_string_literals;cxx_reference_qualified_functions;cxx_right_angle_brackets;cxx_rvalue_references;cxx_sizeof_member;cxx_static_assert;cxx_strong_enums;cxx_thread_local;cxx_trailing_return_types;cxx_unicode_literals;cxx_uniform_initialization;cxx_unrestricted_unions;cxx_user_literals;cxx_variadic_macros;cxx_variadic_templates;cxx_std_14;cxx_aggregate_default_initializers;cxx_attribute_deprecated;cxx_binary_literals;cxx_contextual_conversions;cxx_decltype_auto;cxx_digit_separators;cxx_generic_lambdas;cxx_lambda_init_captures;cxx_relaxed_constexpr;cxx_return_type_deduction;cxx_variable_templates;cxx_std_17;cxx_std_20;cxx_std_23")
        if ("${CMAKE_VERSION}" VERSION_LESS "3.21") # 3.20
          set(CMAKE_C_COMPILE_FEATURES "c_std_90;c_function_prototypes;c_std_99;c_restrict;c_variadic_macros;c_std_11;c_static_assert")
        else() # 3.21+
          set(CMAKE_C17_COMPILE_FEATURES "c_std_17")
          set(CMAKE_C23_COMPILE_FEATURES "c_std_23")
          set(CMAKE_C_COMPILE_FEATURES "c_std_90;c_function_prototypes;c_std_99;c_restrict;c_variadic_macros;c_std_11;c_static_assert;c_std_17;c_std_23")
        endif()
      endif()
    endif()
  endif()
endif()

execute_process(COMMAND "${EMSCRIPTEN_ROOT_PATH}/em-config${EMCC_SUFFIX}" "CACHE"
  RESULT_VARIABLE _emcache_result
  OUTPUT_VARIABLE _emcache_output
  OUTPUT_STRIP_TRAILING_WHITESPACE)
if (NOT _emcache_result EQUAL 0)
  message(FATAL_ERROR "Failed to find emscripten cache directory with command \"'${EMSCRIPTEN_ROOT_PATH}/em-config${EMCC_SUFFIX}' CACHE\"! Process returned with error code ${_emcache_result}.")
endif()
file(TO_CMAKE_PATH "${_emcache_output}" _emcache_output)
set(EMSCRIPTEN_SYSROOT "${_emcache_output}/sysroot")

list(APPEND CMAKE_FIND_ROOT_PATH "${EMSCRIPTEN_SYSROOT}")
list(APPEND CMAKE_SYSTEM_PREFIX_PATH /)

if (${CMAKE_C_FLAGS} MATCHES "MEMORY64")
  set(CMAKE_LIBRARY_ARCHITECTURE "wasm64-emscripten")
  set(CMAKE_SIZEOF_VOID_P 8)
  set(CMAKE_C_SIZEOF_DATA_PTR 8)
  set(CMAKE_CXX_SIZEOF_DATA_PTR 8)
else()
  set(CMAKE_LIBRARY_ARCHITECTURE "wasm32-emscripten")
  set(CMAKE_SIZEOF_VOID_P 4)
  set(CMAKE_C_SIZEOF_DATA_PTR 4)
  set(CMAKE_CXX_SIZEOF_DATA_PTR 4)
endif()

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "${EMSCRIPTEN_SYSROOT}" CACHE PATH
    "Install path prefix, prepended onto install directories." FORCE)
endif()

# To find programs to execute during CMake run time with find_program(), e.g.
# 'git' or so, we allow looking into system paths.
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# Since Emscripten is a cross-compiler, we should never look at the
# system-provided directories like /usr/include and so on. Therefore only
# CMAKE_FIND_ROOT_PATH should be used as a find directory. See
# http://www.cmake.org/cmake/help/v3.0/variable/CMAKE_FIND_ROOT_PATH_MODE_INCLUDE.html
if (NOT CMAKE_FIND_ROOT_PATH_MODE_LIBRARY)
  set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
endif()
if (NOT CMAKE_FIND_ROOT_PATH_MODE_INCLUDE)
  set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
endif()
if (NOT CMAKE_FIND_ROOT_PATH_MODE_PACKAGE)
  set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
endif()

set(_em_pkgconfig_libdir "${EMSCRIPTEN_SYSROOT}/local/lib/pkgconfig" "${EMSCRIPTEN_SYSROOT}/lib/pkgconfig")
if("${CMAKE_VERSION}" VERSION_LESS "3.20")
  file(TO_NATIVE_PATH "${_em_pkgconfig_libdir}" _em_pkgconfig_libdir)
  if(CMAKE_HOST_UNIX)
    string(REPLACE ";" ":" _em_pkgconfig_libdir "${_em_pkgconfig_libdir}")
    string(REPLACE "\\ " " " _em_pkgconfig_libdir "${_em_pkgconfig_libdir}")
  endif()
else()
  cmake_path(CONVERT "${_em_pkgconfig_libdir}" TO_NATIVE_PATH_LIST _em_pkgconfig_libdir)
endif()
set(ENV{PKG_CONFIG_LIBDIR} "${_em_pkgconfig_libdir}")
unset(_em_pkgconfig_libdir)

option(EMSCRIPTEN_GENERATE_BITCODE_STATIC_LIBRARIES "If set, static library targets generate LLVM bitcode files (.bc). If disabled (default), UNIX ar archives (.a) are generated." OFF)
if (EMSCRIPTEN_GENERATE_BITCODE_STATIC_LIBRARIES)
  message(FATAL_ERROR "EMSCRIPTEN_GENERATE_BITCODE_STATIC_LIBRARIES is not compatible with the llvm backend")
endif()

set(CMAKE_EXECUTABLE_SUFFIX ".js")

set(CMAKE_C_USE_RESPONSE_FILE_FOR_LIBRARIES 1)
set(CMAKE_CXX_USE_RESPONSE_FILE_FOR_LIBRARIES 1)
set(CMAKE_C_USE_RESPONSE_FILE_FOR_OBJECTS 1)
set(CMAKE_CXX_USE_RESPONSE_FILE_FOR_OBJECTS 1)
set(CMAKE_C_USE_RESPONSE_FILE_FOR_INCLUDES 1)
set(CMAKE_CXX_USE_RESPONSE_FILE_FOR_INCLUDES 1)

set(CMAKE_C_RESPONSE_FILE_LINK_FLAG "@")
set(CMAKE_CXX_RESPONSE_FILE_LINK_FLAG "@")

# Set a global EMSCRIPTEN variable that can be used in client CMakeLists.txt to
# detect when building using Emscripten.
set(EMSCRIPTEN 1 CACHE INTERNAL "If true, we are targeting Emscripten output.")

# Hardwire support for cmake-2.8/Modules/CMakeBackwardsCompatibilityC.cmake
# without having CMake to try complex things to autodetect these:
set(CMAKE_SKIP_COMPATIBILITY_TESTS 1)
set(CMAKE_SIZEOF_CHAR 1)
set(CMAKE_SIZEOF_UNSIGNED_SHORT 2)
set(CMAKE_SIZEOF_SHORT 2)
set(CMAKE_SIZEOF_INT 4)
set(CMAKE_SIZEOF_UNSIGNED_LONG 4)
set(CMAKE_SIZEOF_UNSIGNED_INT 4)
set(CMAKE_SIZEOF_LONG 4)
set(CMAKE_SIZEOF_FLOAT 4)
set(CMAKE_SIZEOF_DOUBLE 8)
set(CMAKE_HAVE_LIMITS_H 1)
set(CMAKE_HAVE_UNISTD_H 1)
set(CMAKE_HAVE_PTHREAD_H 1)
set(CMAKE_HAVE_SYS_PRCTL_H 1)
set(CMAKE_WORDS_BIGENDIAN 0)
set(CMAKE_C_BYTE_ORDER "LITTLE_ENDIAN")
set(CMAKE_CXX_BYTE_ORDER "LITTLE_ENDIAN")
set(CMAKE_DL_LIBS)

function(em_validate_asmjs_after_build target)
  message(WARNING "em_validate_asmjs_after_build no longer exists")
endfunction()

# Internal function: Do not call from user CMakeLists.txt files. Use one of
# em_link_js_library()/em_link_pre_js()/em_link_post_js() instead.
function(em_add_link_deps target flagname)
  # User can input list of JS files either as a single list, or as variable
  # arguments to this function, so iterate over varargs, and treat each item in
  # varargs as a list itself, to support both syntax forms.
  foreach(jsFileList ${ARGN})
    foreach(jsfile ${jsFileList})
      get_target_property(linkdeps ${target} LINK_DEPENDS)
      if(linkdeps STREQUAL "linkdeps-NOTFOUND")
        set(linkdeps "")
      endif()
      get_filename_component(jsfile_abs "${jsfile}" ABSOLUTE )
      set_target_properties(${target} PROPERTIES LINK_DEPENDS "${linkdeps};${jsfile_abs}")
      target_link_libraries(${target} "${flagname} \"${jsfile_abs}\"")
    endforeach()
  endforeach()
endfunction()

# This function links a (list of ) .js library file(s) to the given CMake project.
# Example: em_link_js_library(my_executable "lib1.js" "lib2.js")
#    will result in emcc passing --js-library lib1.js --js-library lib2.js to
#    the emscripten linker, as well as tracking the modification timestamp
#    between the linked .js files and the main project, so that editing the .js
#    file will cause the target project to be relinked.
function(em_link_js_library target)
  em_add_link_deps(${target} "--js-library" ${ARGN})
endfunction()

# This function is identical to em_link_js_library(), except the .js files will
# be added with '--pre-js file.js' command line flag, which is generally used to
# add some preamble .js code to a generated output file.
function(em_link_pre_js target)
  em_add_link_deps(${target} "--pre-js" ${ARGN})
endfunction()

# This function is identical to em_link_js_library(), except the .js files will
# be added with '--post-js file.js' command line flag, which is generally used
# to add some postamble .js code to a generated output file.
function(em_link_post_js target)
  em_add_link_deps(${target} "--post-js" ${ARGN})
endfunction()

if (NOT DEFINED CMAKE_CROSSCOMPILING_EMULATOR)
  find_program(NODE_JS_EXECUTABLE NAMES nodejs node)
  if(NODE_JS_EXECUTABLE)
    set(CMAKE_CROSSCOMPILING_EMULATOR "${NODE_JS_EXECUTABLE}" CACHE FILEPATH "Path to the emulator for the target system.")
  endif()
endif()

# No-op on CMAKE_CROSSCOMPILING_EMULATOR so older versions of cmake do not
# complain about unused CMake variable.
if (CMAKE_CROSSCOMPILING_EMULATOR)
endif()

# TODO: CMake appends <sysroot>/usr/include to implicit includes; switching to use usr/include will make this redundant.
if (NOT DEFINED CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES)
  set(CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES "${EMSCRIPTEN_SYSROOT}/include")
endif()
if (NOT DEFINED CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES)
  set(CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES "${EMSCRIPTEN_SYSROOT}/include")
endif()
