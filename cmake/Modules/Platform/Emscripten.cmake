# This file is a 'toolchain description file' for CMake.
# It teaches CMake about the Emscripten compiler, so that CMake can generate makefiles
# from CMakeLists.txt that invoke emcc.

# At the moment this required minimum version is not exact (i.e. we do not know of a feature that needs CMake 3.0.0 specifically)
# It is possible that CMake 3.0.0 is too old and will not actually work. If you do find such a case, please report it at Emscripten
# bug tracker to revise the minimum requirement. See also https://github.com/juj/emsdk/issues/108
cmake_minimum_required(VERSION 3.0.0)

# To use this toolchain file with CMake, invoke CMake with the following command line parameters
# cmake -DCMAKE_TOOLCHAIN_FILE=<EmscriptenRoot>/cmake/Modules/Platform/Emscripten.cmake
#       -DCMAKE_BUILD_TYPE=<Debug|RelWithDebInfo|Release|MinSizeRel>
#       -G "Unix Makefiles" (Linux and OSX)
#       -G "MinGW Makefiles" (Windows)
#       <path/to/CMakeLists.txt> # Note, pass in here ONLY the path to the file, not the filename 'CMakeLists.txt' itself.

# After that, build the generated Makefile with the command 'make'. On Windows, you may download and use 'mingw32-make' instead.

# The following variable describes the target OS we are building to.
set(CMAKE_SYSTEM_NAME Emscripten)
set(CMAKE_SYSTEM_VERSION 1)

set(CMAKE_CROSSCOMPILING TRUE)

# Advertise Emscripten as a 32-bit platform (as opposed to CMAKE_SYSTEM_PROCESSOR=x86_64 for 64-bit platform),
# since some projects (e.g. OpenCV) use this to detect bitness.
set(CMAKE_SYSTEM_PROCESSOR x86)

# Tell CMake how it should instruct the compiler to generate multiple versions of an outputted .so library: e.g. "libfoo.so, libfoo.so.1, libfoo.so.1.4" etc.
# This feature is activated if a shared library project has the property SOVERSION defined.
set(CMAKE_SHARED_LIBRARY_SONAME_C_FLAG "-Wl,-soname,")

# In CMake, CMAKE_HOST_WIN32 is set when we are cross-compiling from Win32 to Emscripten: http://www.cmake.org/cmake/help/v2.8.12/cmake.html#variable:CMAKE_HOST_WIN32
# The variable WIN32 is set only when the target arch that will run the code will be WIN32, so unset WIN32 when cross-compiling.
set(WIN32)

# The same logic as above applies for APPLE and CMAKE_HOST_APPLE, so unset APPLE.
set(APPLE)

# And for UNIX and CMAKE_HOST_UNIX. However, Emscripten is often able to mimic being a Linux/Unix system, in which case a lot of existing CMakeLists.txt files can be configured for Emscripten while assuming UNIX build, so this is left enabled.
set(UNIX 1)

# Do a no-op access on the CMAKE_TOOLCHAIN_FILE variable so that CMake will not issue a warning on it being unused.
if (CMAKE_TOOLCHAIN_FILE)
endif()

# In order for check_function_exists() detection to work, we must signal it to pass an additional flag, which causes the compilation
# to abort if linking results in any undefined symbols. The CMake detection mechanism depends on the undefined symbol error to be raised.
# Disable wasm in cmake checks so that (1) we do not depend on wasm support just for configuration (perhaps the user does not intend
# to build to wasm; using asm.js only depends on js which we need anyhow), and (2) we don't have issues with a separate .wasm file
# on the side, async startup, etc..
set(CMAKE_REQUIRED_FLAGS "-s WASM=0")

# Locate where the Emscripten compiler resides in relative to this toolchain file.
if ("${EMSCRIPTEN_ROOT_PATH}" STREQUAL "")
	get_filename_component(GUESS_EMSCRIPTEN_ROOT_PATH "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)
	if (EXISTS "${GUESS_EMSCRIPTEN_ROOT_PATH}/emranlib")
		set(EMSCRIPTEN_ROOT_PATH "${GUESS_EMSCRIPTEN_ROOT_PATH}")
	endif()
endif()

# If not found by above search, locate using the EMSCRIPTEN environment variable.
if ("${EMSCRIPTEN_ROOT_PATH}" STREQUAL "")
	set(EMSCRIPTEN_ROOT_PATH "$ENV{EMSCRIPTEN}")
endif()

# Abort if not found. 
if ("${EMSCRIPTEN_ROOT_PATH}" STREQUAL "")
	message(FATAL_ERROR "Could not locate the Emscripten compiler toolchain directory! Either set the EMSCRIPTEN environment variable, or pass -DEMSCRIPTEN_ROOT_PATH=xxx to CMake to explicitly specify the location of the compiler!")
endif()

# Normalize, convert Windows backslashes to forward slashes or CMake will crash.
get_filename_component(EMSCRIPTEN_ROOT_PATH "${EMSCRIPTEN_ROOT_PATH}" ABSOLUTE)

list(APPEND CMAKE_MODULE_PATH "${EMSCRIPTEN_ROOT_PATH}/cmake/Modules")

list(APPEND CMAKE_FIND_ROOT_PATH "${EMSCRIPTEN_ROOT_PATH}/system")

if (CMAKE_HOST_WIN32)
	set(EMCC_SUFFIX ".bat")
else()
	set(EMCC_SUFFIX "")
endif()

# Specify the compilers to use for C and C++
if ("${CMAKE_C_COMPILER}" STREQUAL "")
	set(CMAKE_C_COMPILER "${EMSCRIPTEN_ROOT_PATH}/emcc${EMCC_SUFFIX}")
endif()
if ("${CMAKE_CXX_COMPILER}" STREQUAL "")
	set(CMAKE_CXX_COMPILER "${EMSCRIPTEN_ROOT_PATH}/em++${EMCC_SUFFIX}")
endif()

if ("${CMAKE_AR}" STREQUAL "")
	set(CMAKE_AR "${EMSCRIPTEN_ROOT_PATH}/emar${EMCC_SUFFIX}" CACHE FILEPATH "Emscripten ar")
endif()

if ("${CMAKE_RANLIB}" STREQUAL "")
	set(CMAKE_RANLIB "${EMSCRIPTEN_ROOT_PATH}/emranlib${EMCC_SUFFIX}" CACHE FILEPATH "Emscripten ranlib")
endif()

# Don't allow CMake to autodetect the compiler, since it does not understand Emscripten.
# Pass -DEMSCRIPTEN_FORCE_COMPILERS=OFF to disable (sensible mostly only for testing/debugging purposes).
option(EMSCRIPTEN_FORCE_COMPILERS "Force C/C++ compiler" ON)
if (EMSCRIPTEN_FORCE_COMPILERS)

	# Detect version of the 'emcc' executable. Note that for CMake, we tell it the version of the Clang compiler and not the version of Emscripten,
	# because CMake understands Clang better.
	if (NOT CMAKE_C_COMPILER_VERSION) # Toolchain script is interpreted multiple times, so don't rerun the check if already done before.
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

	# Capture the Emscripten version to EMSCRIPTEN_VERSION variable.
	if (NOT EMSCRIPTEN_VERSION)
		execute_process(COMMAND "${CMAKE_C_COMPILER}" "-v" RESULT_VARIABLE _cmake_compiler_result ERROR_VARIABLE _cmake_compiler_output OUTPUT_QUIET)
		if (NOT _cmake_compiler_result EQUAL 0)
			message(FATAL_ERROR "Failed to fetch Emscripten version information with command \"'${CMAKE_C_COMPILER}' -v\"! Process returned with error code ${_cmake_compiler_result}.")
		endif()
		string(REGEX MATCH "emcc \\(.*\\) ([0-9\\.]+)" _dummy_unused "${_cmake_compiler_output}")
		if (NOT CMAKE_MATCH_1)
			message(FATAL_ERROR "Failed to regex parse Emscripten compiler version from version string: ${_cmake_compiler_output}")
		endif()

		set(EMSCRIPTEN_VERSION "${CMAKE_MATCH_1}")
	endif()

	set(CMAKE_C_COMPILER_ID_RUN TRUE)
	set(CMAKE_C_COMPILER_FORCED TRUE)
	set(CMAKE_C_COMPILER_WORKS TRUE)
	set(CMAKE_C_COMPILER_ID Clang)
	set(CMAKE_C_STANDARD_COMPUTED_DEFAULT 11)

	set(CMAKE_CXX_COMPILER_ID_RUN TRUE)
	set(CMAKE_CXX_COMPILER_FORCED TRUE)
	set(CMAKE_CXX_COMPILER_WORKS TRUE)
	set(CMAKE_CXX_COMPILER_ID Clang)
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
	else()
		set(CMAKE_C_COMPILE_FEATURES "c_std_90;c_function_prototypes;c_std_99;c_restrict;c_variadic_macros;c_std_11;c_static_assert")
		set(CMAKE_C90_COMPILE_FEATURES "c_std_90;c_function_prototypes")
		set(CMAKE_C99_COMPILE_FEATURES "c_std_99;c_restrict;c_variadic_macros")
		set(CMAKE_C11_COMPILE_FEATURES "c_std_11;c_static_assert")

		set(CMAKE_CXX_COMPILE_FEATURES "cxx_std_98;cxx_template_template_parameters;cxx_std_11;cxx_alias_templates;cxx_alignas;cxx_alignof;cxx_attributes;cxx_auto_type;cxx_constexpr;cxx_decltype;cxx_decltype_incomplete_return_types;cxx_default_function_template_args;cxx_defaulted_functions;cxx_defaulted_move_initializers;cxx_delegating_constructors;cxx_deleted_functions;cxx_enum_forward_declarations;cxx_explicit_conversions;cxx_extended_friend_declarations;cxx_extern_templates;cxx_final;cxx_func_identifier;cxx_generalized_initializers;cxx_inheriting_constructors;cxx_inline_namespaces;cxx_lambdas;cxx_local_type_template_args;cxx_long_long_type;cxx_noexcept;cxx_nonstatic_member_init;cxx_nullptr;cxx_override;cxx_range_for;cxx_raw_string_literals;cxx_reference_qualified_functions;cxx_right_angle_brackets;cxx_rvalue_references;cxx_sizeof_member;cxx_static_assert;cxx_strong_enums;cxx_thread_local;cxx_trailing_return_types;cxx_unicode_literals;cxx_uniform_initialization;cxx_unrestricted_unions;cxx_user_literals;cxx_variadic_macros;cxx_variadic_templates;cxx_std_14;cxx_aggregate_default_initializers;cxx_attribute_deprecated;cxx_binary_literals;cxx_contextual_conversions;cxx_decltype_auto;cxx_digit_separators;cxx_generic_lambdas;cxx_lambda_init_captures;cxx_relaxed_constexpr;cxx_return_type_deduction;cxx_variable_templates;cxx_std_17")
		set(CMAKE_CXX98_COMPILE_FEATURES "cxx_std_98;cxx_template_template_parameters")
		set(CMAKE_CXX11_COMPILE_FEATURES "cxx_std_11;cxx_alias_templates;cxx_alignas;cxx_alignof;cxx_attributes;cxx_auto_type;cxx_constexpr;cxx_decltype;cxx_decltype_incomplete_return_types;cxx_default_function_template_args;cxx_defaulted_functions;cxx_defaulted_move_initializers;cxx_delegating_constructors;cxx_deleted_functions;cxx_enum_forward_declarations;cxx_explicit_conversions;cxx_extended_friend_declarations;cxx_extern_templates;cxx_final;cxx_func_identifier;cxx_generalized_initializers;cxx_inheriting_constructors;cxx_inline_namespaces;cxx_lambdas;cxx_local_type_template_args;cxx_long_long_type;cxx_noexcept;cxx_nonstatic_member_init;cxx_nullptr;cxx_override;cxx_range_for;cxx_raw_string_literals;cxx_reference_qualified_functions;cxx_right_angle_brackets;cxx_rvalue_references;cxx_sizeof_member;cxx_static_assert;cxx_strong_enums;cxx_thread_local;cxx_trailing_return_types;cxx_unicode_literals;cxx_uniform_initialization;cxx_unrestricted_unions;cxx_user_literals;cxx_variadic_macros;cxx_variadic_templates")
		set(CMAKE_CXX14_COMPILE_FEATURES "cxx_std_14;cxx_aggregate_default_initializers;cxx_attribute_deprecated;cxx_binary_literals;cxx_contextual_conversions;cxx_decltype_auto;cxx_digit_separators;cxx_generic_lambdas;cxx_lambda_init_captures;cxx_relaxed_constexpr;cxx_return_type_deduction;cxx_variable_templates")
	endif()
endif()

# To find programs to execute during CMake run time with find_program(), e.g. 'git' or so, we allow looking
# into system paths.
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# Since Emscripten is a cross-compiler, we should never look at the system-provided directories like /usr/include and so on.
# Therefore only CMAKE_FIND_ROOT_PATH should be used as a find directory. See http://www.cmake.org/cmake/help/v3.0/variable/CMAKE_FIND_ROOT_PATH_MODE_INCLUDE.html
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

set(CMAKE_SYSTEM_INCLUDE_PATH "${EMSCRIPTEN_ROOT_PATH}/system/include")

# We would prefer to specify a standard set of Clang+Emscripten-friendly common convention for suffix files, especially for CMake executable files,
# but if these are adjusted, ${CMAKE_ROOT}/Modules/CheckIncludeFile.cmake will fail, since it depends on being able to compile output files with predefined names.
#SET(CMAKE_LINK_LIBRARY_SUFFIX "")
#SET(CMAKE_STATIC_LIBRARY_PREFIX "")
#SET(CMAKE_SHARED_LIBRARY_PREFIX "")
#SET(CMAKE_FIND_LIBRARY_PREFIXES "")
#SET(CMAKE_FIND_LIBRARY_SUFFIXES ".bc")
#SET(CMAKE_SHARED_LIBRARY_SUFFIX ".bc")

option(EMSCRIPTEN_GENERATE_BITCODE_STATIC_LIBRARIES "If set, static library targets generate LLVM bitcode files (.bc). If disabled (default), UNIX ar archives (.a) are generated." OFF)
if (EMSCRIPTEN_GENERATE_BITCODE_STATIC_LIBRARIES)
	SET(CMAKE_STATIC_LIBRARY_SUFFIX ".bc")

	SET(CMAKE_C_CREATE_STATIC_LIBRARY "<CMAKE_C_COMPILER> -o <TARGET> <LINK_FLAGS> <OBJECTS>")
	SET(CMAKE_CXX_CREATE_STATIC_LIBRARY "<CMAKE_CXX_COMPILER> -o <TARGET> <LINK_FLAGS> <OBJECTS>")
else()
	# Specify the program to use when building static libraries. Force Emscripten-related command line options to clang.
	SET(CMAKE_C_CREATE_STATIC_LIBRARY "<CMAKE_AR> rc <TARGET> <LINK_FLAGS> <OBJECTS>")
	SET(CMAKE_CXX_CREATE_STATIC_LIBRARY "<CMAKE_AR> rc <TARGET> <LINK_FLAGS> <OBJECTS>")
endif()

SET(CMAKE_EXECUTABLE_SUFFIX ".js")

SET(CMAKE_C_USE_RESPONSE_FILE_FOR_LIBRARIES 1)
SET(CMAKE_CXX_USE_RESPONSE_FILE_FOR_LIBRARIES 1)
SET(CMAKE_C_USE_RESPONSE_FILE_FOR_OBJECTS 1)
SET(CMAKE_CXX_USE_RESPONSE_FILE_FOR_OBJECTS 1)
SET(CMAKE_C_USE_RESPONSE_FILE_FOR_INCLUDES 1)
SET(CMAKE_CXX_USE_RESPONSE_FILE_FOR_INCLUDES 1)

set(CMAKE_C_RESPONSE_FILE_LINK_FLAG "@")
set(CMAKE_CXX_RESPONSE_FILE_LINK_FLAG "@")

# Set a global EMSCRIPTEN variable that can be used in client CMakeLists.txt to detect when building using Emscripten.
set(EMSCRIPTEN 1 CACHE BOOL "If true, we are targeting Emscripten output.")

# Hardwire support for cmake-2.8/Modules/CMakeBackwardsCompatibilityC.cmake without having CMake to try complex things
# to autodetect these:
set(CMAKE_SKIP_COMPATIBILITY_TESTS 1)
set(CMAKE_SIZEOF_CHAR 1)
set(CMAKE_SIZEOF_UNSIGNED_SHORT 2)
set(CMAKE_SIZEOF_SHORT 2)
set(CMAKE_SIZEOF_INT 4)
set(CMAKE_SIZEOF_UNSIGNED_LONG 4)
set(CMAKE_SIZEOF_UNSIGNED_INT 4)
set(CMAKE_SIZEOF_LONG 4)
set(CMAKE_SIZEOF_VOID_P 4)
set(CMAKE_SIZEOF_FLOAT 4)
set(CMAKE_SIZEOF_DOUBLE 8)
set(CMAKE_C_SIZEOF_DATA_PTR 4)
set(CMAKE_CXX_SIZEOF_DATA_PTR 4)
set(CMAKE_HAVE_LIMITS_H 1)
set(CMAKE_HAVE_UNISTD_H 1)
set(CMAKE_HAVE_PTHREAD_H 1)
set(CMAKE_HAVE_SYS_PRCTL_H 1)
set(CMAKE_WORDS_BIGENDIAN 0)
set(CMAKE_DL_LIBS)

set(CMAKE_C_FLAGS_RELEASE "-DNDEBUG -O2" CACHE STRING "Emscripten-overridden CMAKE_C_FLAGS_RELEASE")
set(CMAKE_C_FLAGS_MINSIZEREL "-DNDEBUG -Os" CACHE STRING "Emscripten-overridden CMAKE_C_FLAGS_MINSIZEREL")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O2" CACHE STRING "Emscripten-overridden CMAKE_C_FLAGS_RELWITHDEBINFO")
set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -O2" CACHE STRING "Emscripten-overridden CMAKE_CXX_FLAGS_RELEASE")
set(CMAKE_CXX_FLAGS_MINSIZEREL "-DNDEBUG -Os" CACHE STRING "Emscripten-overridden CMAKE_CXX_FLAGS_MINSIZEREL")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2" CACHE STRING "Emscripten-overridden CMAKE_CXX_FLAGS_RELWITHDEBINFO")

set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-O2" CACHE STRING "Emscripten-overridden CMAKE_EXE_LINKER_FLAGS_RELEASE")
set(CMAKE_EXE_LINKER_FLAGS_MINSIZEREL "-Os" CACHE STRING "Emscripten-overridden CMAKE_EXE_LINKER_FLAGS_MINSIZEREL")
set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "-O2 -g" CACHE STRING "Emscripten-overridden CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO")
set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "-O2" CACHE STRING "Emscripten-overridden CMAKE_SHARED_LINKER_FLAGS_RELEASE")
set(CMAKE_SHARED_LINKER_FLAGS_MINSIZEREL "-Os" CACHE STRING "Emscripten-overridden CMAKE_SHARED_LINKER_FLAGS_MINSIZEREL")
set(CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO "-O2 -g" CACHE STRING "Emscripten-overridden CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO")
set(CMAKE_MODULE_LINKER_FLAGS_RELEASE "-O2" CACHE STRING "Emscripten-overridden CMAKE_MODULE_LINKER_FLAGS_RELEASE")
set(CMAKE_MODULE_LINKER_FLAGS_MINSIZEREL "-Os" CACHE STRING "Emscripten-overridden CMAKE_MODULE_LINKER_FLAGS_MINSIZEREL")
set(CMAKE_MODULE_LINKER_FLAGS_RELWITHDEBINFO "-O2 -g" CACHE STRING "Emscripten-overridden CMAKE_MODULE_LINKER_FLAGS_RELWITHDEBINFO")

function(em_validate_asmjs_after_build target)
	add_custom_command(TARGET ${target} POST_BUILD COMMAND ${CMAKE_COMMAND} -E echo Validating build output for asm.js... COMMAND "python" ARGS "${EMSCRIPTEN_ROOT_PATH}/tools/validate_asmjs.py" "$<TARGET_FILE:${target}>")
endfunction()

# A global counter to guarantee unique names for js library files.
set(link_js_counter 1)

# Internal function: Do not call from user CMakeLists.txt files. Use one of em_link_js_library()/em_link_pre_js()/em_link_post_js() instead.
function(em_add_tracked_link_flag target flagname)

	# User can input list of JS files either as a single list, or as variable arguments to this function, so iterate over varargs, and treat each
	# item in varargs as a list itself, to support both syntax forms.
	foreach(jsFileList ${ARGN})
		foreach(jsfile ${jsFileList})
			# If the user edits the JS file, we want to relink the emscripten application, but unfortunately it is not possible to make a link step
			# depend directly on a source file. Instead, we must make a dummy no-op build target on that source file, and make the project depend on
			# that target.
			
			# Sanitate the source .js filename to a good symbol name to use as a dummy filename.
			get_filename_component(jsname "${jsfile}" NAME)
			string(REGEX REPLACE "[/:\\\\.\ ]" "_" dummy_js_target ${jsname})
			set(dummy_lib_name ${target}_${link_js_counter}_${dummy_js_target})
			set(dummy_c_name "${CMAKE_BINARY_DIR}/${dummy_js_target}_tracker.c")

			# Create a new static library target that with a single dummy .c file.
			add_library(${dummy_lib_name} STATIC ${dummy_c_name})
			# Make the dummy .c file depend on the .js file we are linking, so that if the .js file is edited, the dummy .c file, and hence the static library will be rebuild (no-op). This causes the main application to be relinked, which is what we want.
			# This approach was recommended by http://www.cmake.org/pipermail/cmake/2010-May/037206.html
			add_custom_command(OUTPUT ${dummy_c_name} COMMAND ${CMAKE_COMMAND} -E touch ${dummy_c_name} DEPENDS ${jsfile})
			target_link_libraries(${target} ${dummy_lib_name})

			# Link the js-library to the target
			# When a linked library starts with a "-" cmake will just add it to the linker command line as it is.
			# The advantage of doing it this way is that the js-library will also be automatically linked to targets
			# that depend on this target.
			get_filename_component(js_file_absolute_path "${jsfile}" ABSOLUTE )
			target_link_libraries(${target} "${flagname} \"${js_file_absolute_path}\"")

			math(EXPR link_js_counter "${link_js_counter} + 1")
		endforeach()
	endforeach()
endfunction()

# This function links a (list of ) .js library file(s) to the given CMake project.
# Example: em_link_js_library(my_executable "lib1.js" "lib2.js")
#    will result in emcc passing --js-library lib1.js --js-library lib2.js to the emscripten linker, as well as
#    tracking the modification timestamp between the linked .js files and the main project, so that editing the .js file
#    will cause the target project to be relinked.
function(em_link_js_library target)
	em_add_tracked_link_flag(${target} "--js-library" ${ARGN})
endfunction()

# This function is identical to em_link_js_library(), except the .js files will be added with '--pre-js file.js' command line flag,
# which is generally used to add some preamble .js code to a generated output file.
function(em_link_pre_js target)
	em_add_tracked_link_flag(${target} "--pre-js" ${ARGN})
endfunction()

# This function is identical to em_link_js_library(), except the .js files will be added with '--post-js file.js' command line flag,
# which is generally used to add some postamble .js code to a generated output file.
function(em_link_post_js target)
	em_add_tracked_link_flag(${target} "--post-js" ${ARGN})
endfunction()

# Experimental support for targeting generation of Visual Studio project files (vs-tool) of Emscripten projects for Windows.
# To use this, pass the combination -G "Visual Studio 10" -DCMAKE_TOOLCHAIN_FILE=Emscripten.cmake
if ("${CMAKE_GENERATOR}" MATCHES "^Visual Studio.*")
	# By default, CMake generates VS project files with a <GenerateManifest>true</GenerateManifest> directive.
	# This causes VS to attempt to invoke rc.exe during the build, which will fail since app manifests are meaningless for Emscripten.
	# To disable this, add the following linker flag. This flag will not go to emcc, since the Visual Studio CMake generator will swallow it.
	set(EMSCRIPTEN_VS_LINKER_FLAGS "/MANIFEST:NO")
	# CMake is hardcoded to write a ClCompile directive <ObjectFileName>$(IntDir)</ObjectFileName> in all VS project files it generates.
	# This makes VS pass emcc a -o param that points to a directory instead of a file, which causes emcc autogenerate the output filename.
	# CMake is hardcoded to assume all object files have the suffix .obj, so adjust the emcc-autogenerated default suffix name to match.
	set(EMSCRIPTEN_VS_LINKER_FLAGS "${EMSCRIPTEN_VS_LINKER_FLAGS} --default-obj-ext .obj")
	# Also hint CMake that it should not hardcode <ObjectFileName> generation. Requires a custom CMake build for this to work (ignored on others)
	# See http://www.cmake.org/Bug/view.php?id=14673 and https://github.com/juj/CMake
	set(CMAKE_VS_NO_DEFAULT_OBJECTFILENAME 1)

	# Apply and cache Emscripten Visual Studio IDE-specific linker flags.
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${EMSCRIPTEN_VS_LINKER_FLAGS}" CACHE STRING "")
	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${EMSCRIPTEN_VS_LINKER_FLAGS}" CACHE STRING "")
	set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} ${EMSCRIPTEN_VS_LINKER_FLAGS}" CACHE STRING "")
endif()

if (NOT DEFINED CMAKE_CROSSCOMPILING_EMULATOR)
  find_program(NODE_JS_EXECUTABLE NAMES nodejs node)
  if(NODE_JS_EXECUTABLE)
    set(CMAKE_CROSSCOMPILING_EMULATOR "${NODE_JS_EXECUTABLE}" CACHE FILEPATH "Path to the emulator for the target system.")
  endif()
endif()
# No-op on CMAKE_CROSSCOMPILING_EMULATOR so older versions of cmake do not
# complain about unused CMake variable.
if(CMAKE_CROSSCOMPILING_EMULATOR)
endif()
